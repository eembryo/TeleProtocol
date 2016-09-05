/*
 * IpcmdCore.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdCore.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdMessage.h"
#include <glib.h>

struct _IpcmdCore {
	IpcmdServer	server_;
	GList		*clients_;
	IpcmdBus	bus_;
	GHashTable	operation_contexts_;
};

void
IpcmdCoreDispatch(struct _IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{

}

gint
IpcmdCoreTransmit(struct _IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	return 0;
}

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
