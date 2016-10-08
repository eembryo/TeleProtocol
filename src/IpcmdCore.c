/*
 * IpcmdCore.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdCore.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdClient.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdClientImpl.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdOperationContext.h"

#include <glib.h>

struct _IpcmdCore {
	IpcmdServer		server_;
	GList			*clients_;
	GMainContext	*main_context;
	IpcmdBus		bus_;
	GHashTable		*operation_contexts_;
};

static void _RemoveOpCtx(gpointer opctx);

IpcmdCore *
IpcmdCoreNew(GMainContext *context)
{
	IpcmdCore *core = g_malloc0(sizeof(struct _IpcmdCore));

	if (!core) return NULL;
	IpcmdCoreInit(core, context);
	return core;
}

void
IpcmdCoreFree(IpcmdCore *self)
{
	IpcmdCoreFinalize(self);
	g_free(self);
}

void
IpcmdCoreInit(IpcmdCore *self, GMainContext *context)
{
	self->main_context = context;
	self->operation_contexts_ = g_hash_table_new_full(IpcmdOpCtxIdHashfunc, IpcmdOpCtxIdEqual, NULL, _RemoveOpCtx);
	self->clients_ = NULL;

	/* initialize bus */
	IpcmdBusInit (&self->bus_,self);
	/* initialize server */
	IpcmdServerInit (&self->server_,self);
}

void
IpcmdCoreFinalize(IpcmdCore *self)
{
	IpcmdServerFinalize (&self->server_);
	// IMPL: free self->clients_
	// IMPL: free self->operation_contexts_

	IpcmdBusFinalize (&self->bus_);
}

void
IpcmdCoreDispatch(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	gint ret = -1;

	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_REQUEST:
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
	case IPCMD_OPTYPE_SETREQUEST:
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		// if op_type is one of REQUEST, SETREQUEST, SETREQUEST_NORETURN and NOTIFICATION_REQUEST, it should be delivered to IpcmdServer
		if (IpcmdServerHandleMessage (&self->server_, channel_id, mesg) < 0)
			g_debug("IpcmdServer cannot handle this message(0x%.04x). Ignore it.", IpcmdMessageGetVCCPDUSenderHandleID(mesg));
		break;
	case IPCMD_OPTYPE_RESPONSE:
	case IPCMD_OPTYPE_NOTIFICATION:
	case IPCMD_OPTYPE_NOTIFICATION_CYCLIC:
	{
		// if op_type is one of RESPONSE, NOTIFICATION and NOTIFICATION_CYCLIC, delivered to IpcmdClient.
		GList *l;
		for (l=self->clients_; l!=NULL; l=l->next) {
			ret = IpcmdClientHandleMessage ((IpcmdClient*)l->data, channel_id, mesg);
			if (!ret) break;	//it is successfully handled.
		}
		if (ret < 0)
			g_debug("IpcmdClients cannot handle this message(0x%.04x). Ignore it.", IpcmdMessageGetVCCPDUSenderHandleID(mesg));
	}
	break;
	case IPCMD_OPTYPE_ACK:
	case IPCMD_OPTYPE_ERROR:
	{
		// if op_type is ACK or ERROR, it may be delivered to IpcmdServer or IpcmdClient
		// Try IpcmdClients first
		GList *l;
		for (l=self->clients_; l!=NULL; l=l->next) {
			ret = IpcmdClientHandleMessage ((IpcmdClient*)l->data, channel_id, mesg);
			if (!ret) break;	//it is successfully handled.
		}
		if (ret) { // if it cannot handled by IpcmdClients, handle it with IpcmdServer
			ret = IpcmdServerHandleMessage (&self->server_, channel_id, mesg);
		}
		if (ret)
			g_debug("IpcmdServer or IpcmdClient cannot handle this message(0x%.04x). Ignore it.", IpcmdMessageGetVCCPDUSenderHandleID(mesg));
	}
	break;
	default:
		g_debug("Got Unknown IpcmdMessage(0x%.04x) type(%d). Ignore it.", IpcmdMessageGetVCCPDUSenderHandleID(mesg), IpcmdMessageGetVCCPDUOpType(mesg));
		break;
	}
}

/* @fn : IpcmdCoreTransmit
 * @return : 	-1 on invalid channel_id
 * 				sent bytes on success
 */
gint
IpcmdCoreTransmit(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	return IpcmdBusTx (&self->bus_, channel_id, mesg);
}

IpcmdBus *
IpcmdCoreGetBus(IpcmdCore *self)
{
	return &self->bus_;
}

IpcmdServer *
IpcmdCoreGetServer(IpcmdCore *self)
{
	return &self->server_;
}

GMainContext*
IpcmdCoreGetGMainContext(IpcmdCore *self)
{
	return self->main_context;
}

/* IpcmdCoreAllocOpCtx :
 * Allocate IpcmdOpCtx for requested 'opctx_id'. If the 'opctx_id' is already used by different IpcmdOpCtx, return error.
 *
 * @ self:
 * @ opctx_id: requested operation context id by IpcmdClient or IpcmdServer
 *
 * return NULL on failed
 * return allocated IpcmdOpCtx *
 */
IpcmdOpCtx *
IpcmdCoreAllocOpCtx(IpcmdCore *self, IpcmdOpCtxId opctx_id)
{
	IpcmdOpCtx *opctx;

	if (g_hash_table_contains (self->operation_contexts_, (gconstpointer)&opctx_id)) { // opctx_id is already in use
		return NULL;
	}

	opctx = IpcmdOpCtxNew();
	if (!opctx) { // not enough memory
		return NULL;
	}
	opctx->opctx_id_ = opctx_id;
	g_hash_table_insert (self->operation_contexts_, (gpointer)&opctx->opctx_id_, opctx);

	return IpcmdOpCtxRef (opctx);
}

void
IpcmdCoreReleaseOpCtx(struct _IpcmdCore *self, IpcmdOpCtxId opctx_id)
{
	g_hash_table_remove (self->operation_contexts_, (gconstpointer)&opctx_id);
}

gboolean
IpcmdCoreRegisterClient(IpcmdCore *self, IpcmdClient *client)
{
	GList *l;
	IpcmdClient *tmp;

	for (l = self->clients_; l != NULL; l = l->next) {
		tmp = (IpcmdClient*)l->data;
		if ( IpcmdClientGetServiceid(tmp) == IpcmdClientGetServiceid(client) && IpcmdHostType(tmp->server_host_) == IpcmdHostType(client->server_host_)) {	// each client has different service_id
			return FALSE;
		}
	}

	client->OnRegisteredToCore (client, self);
	self->clients_ = g_list_append (self->clients_, client);

	return TRUE;
}

void
IpcmdCoreUnregisterClient(IpcmdCore *self, IpcmdClient *client)
{
	client->OnUnregisteredFromCore (client, self);
	self->clients_ = g_list_remove (self->clients_, client);
}

static void
_RemoveOpCtx(gpointer opctx)
{
	IpcmdOpCtxUnref((IpcmdOpCtx*)opctx);
}
