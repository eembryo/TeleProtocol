/*
 * IpcmdService.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdService.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdOperation.h"
#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdOperationContext.h"
#include <glib.h>

gint
IpcmdServiceCompleteOperation(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *reply)
{
	IpcmdServerCompleteOperation (self->server_, &((IpcmdOpCtx *)handle)->opctx_id_, reply);
	return 0;
}

gboolean
IpcmdServiceAddSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdHost *subscriber, gboolean is_static_member)
{
	return TRUE;
}
void
IpcmdServiceInformNotification(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationInfo *operation)
{

}
void
IpcmdServiceRemoveSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdHost *subscriber)
{

}
