/*
 * IpcmdServer.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdServer.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdBus.h"
#include <glib.h>

#define IPCMD_SERVER_FROM_LISTENER(l) (container_of(l, struct _IpcmdServer, listener_)

static void _ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);

gint
IpcmdServerHandleMessage(IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg)
{
	return 0;
}

gboolean
IpcmdServerRegisterService(IpcmdServer *self, IpcmdService *service)
{
	return FALSE;
}

void
IpcmdServiceunregisterService(IpcmdServer *self, IpcmdService *service)
{

}

void
IpcmdServerInit(IpcmdServer *self, IpcmdCore *core)
{
	//self->operation_contexts_ = g_hash_table_new (hash_fn, key_equal_fn);
	self->services_ = NULL;
	self->core_ = core;
	self->listener_.OnChannelEvent = _ReceiveChannelEvent;
	IpcmdBusAddEventListener (IpcmdCoreGetBus(self->core_),self->listener_);
}

void
IpcmdServerFinalize(IpcmdServer *self)
{
	IpcmdBusRemoveEventListener (IpcmdCoreGetBus(self->core_), self->listener_);
	// finalize self->services_
}

static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	IpcmdServer *server = IPCMD_SERVER_FROM_LISTNER(self);

	g_debug("Got channel event: id=%d, type=%d", id, type);
}
