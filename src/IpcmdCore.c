/*
 * IpcmdCore.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdCore.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdBus.h"
//#include "../include/reference.h"
#include <glib.h>
/*
#define IPCMD_BUS_TO_CORE(b) (container_of (b, IpcmdCore, bus_))
#define IPCMD_CORE_TO_BUS(c) (&c->bus_)
*/

void
IpcmdCoreInit(IpcmdCore *self, GMainContext *context)
{
	self->main_context = context;
	IpcmdBusInit (&self->bus_);
	self->operation_contexts_ = NULL;
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
