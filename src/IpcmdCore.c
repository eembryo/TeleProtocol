/*
 * IpcmdCore.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdCore.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdClient.h"
#include "../include/IpcmdServerImpl.h"
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
IpcmdCoreUnregisterClient(IpcmdCore *self, IpcmdClient *client)
{
	self->clients_ = g_list_remove (self->clients_, client);
}

void
IpcmdCoreDispatch(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	// IMPL: whole function
	g_message ("%s: channel_id = %d", __func__, channel_id);
}

gint
IpcmdCoreTransmit(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	IpcmdBusTx (&self->bus_, channel_id, mesg);
	return 0;
}

IpcmdBus *
IpcmdCoreGetBus(IpcmdCore *self)
{
	return &self->bus_;
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
 * @ ret_opctx: Allocated IpcmdOpCtx is set to *ret_opctx
 *
 * return 0 on success
 * return -1 when memory is not enough
 * return -2 when 'opctx_id' is already in use
 */
gint
IpcmdCoreAllocOpCtx(IpcmdCore *self, IpcmdOpCtxId opctx_id, IpcmdOpCtx **ret_opctx)
{
	IpcmdOpCtx *opctx;

	if (g_hash_table_contains (self->operation_contexts_, (gconstpointer)&opctx_id)) { // opctx_id is already in use
		return -2;
	}

	opctx = IpcmdOpCtxNew();
	if (!opctx) { // not enough memory
		return -1;
	}
	opctx->opctx_id_ = opctx_id;
	g_hash_table_insert (self->operation_contexts_, (gpointer)&opctx->opctx_id_, opctx);

	*ret_opctx = IpcmdOpCtxRef (opctx);
	return 0;
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

	for (l = self->clients_; l != NULL; l = l->next) {
		if ( IpcmdClientGetServiceid((IpcmdClient*)l->data) == IpcmdClientGetServiceid(client) ) {	// each client has different service_id
			return FALSE;
		}
	}

	self->clients_ = g_list_append (self->clients_, client);
	return TRUE;
}

static void
_RemoveOpCtx(gpointer opctx)
{
	IpcmdOpCtxUnref((IpcmdOpCtx*)opctx);
}
