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

void
IpcmdCoreInit(IpcmdCore *self, GMainContext *context)
{
	self->main_context = context;
	self->operation_contexts_ = NULL;
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
/*
IpcmdOperationContext *
IpcmdCoreAllocOperationContext(struct _IpcmdCore *self, IpcmdOperationContextId opctx_id)
{
	return NULL;
}

void
IpcmdCoreReleaseOperationContext(struct _IpcmdCore *self, IpcmdOperationContextId opctx_id)
{

}

gboolean
IpcmdCoreAddClient(struct _IpcmdCore *self, IpcmdClient *client)
{
	return FALSE;
}
*/
