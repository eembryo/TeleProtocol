/*
 * IpcmdCore.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdCore.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdBus.h"
#include <glib.h>

struct _IpcmdCore {
	IpcmdServer		server_;
	GList			*clients_;
	GMainContext	*main_context;
	IpcmdBus		bus_;
	GHashTable		*operation_contexts_;
};

static void _DestroyOpCtx(gpointer opctx);

void
IpcmdCoreInit(IpcmdCore *self, GMainContext *context)
{
	self->main_context = context;
	self->operation_contexts_ = g_hash_table_new_full(IpcmdOpCtxIdHashfunc, IpcmdOpCtxIdEqual, NULL, _DestroyOpCtx);
	self->clients_ = NULL;

	/* initialize bus */
	IpcmdBusInit (&self->bus_,self);
	/* initialize server */
	IpcmdServerInit (&self->server_,self);
}

void
IpcmdCoreFinalize(IpcmdCore *self)
{
	// IMPL: free self->operation_contexts_
	// IMPL: free self->clients_
	// g_list_free (self->clients_);
	IpcmdServerFinalize (self->server_);
	IpcmdBusFinalize (self->bus_);
}

gboolean
IpcmdCoreRegisterClient(IpcmdCore *self, IpcmdClient *client)
{
	GList *l = g_list_find (self->clients_, client);

	if (l) return TRUE;

	self->clients_ = g_list_append (self->clients_, client);

	return TRUE;
}

void
IpcmdCoreUnregisterClient(IpcmdCore *self, IpcmdClient *client)
{
	self->clients_ = g_list_remove (self->clients_, client);
}

void
IpcmdCoreDispatch(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
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
		if ( ((IpcmdClient*)l->data)->service_id_ == client->service_id_ ) {	// each client has different service_id
			return FALSE;
		}
	}

	self->clients_ = g_list_append (self->clients_, client);
	return TRUE;
}

void
IpcmdCoreUnregisterClient(IpcmdCore *self, IpcmdClient *client)
{
	g_list_remove (self->clients_, client);
}

static void
_DestroyOpCtx(gpointer opctx)
{
	// reference count of 'opctx' should be 1
	// ensure that (((IpcmdOpCtx*)opctx)->_ref->count == 1) is TRUE
	IpcmdOpCtxUnref((IpcmdOpCtx*)opctx);
}
