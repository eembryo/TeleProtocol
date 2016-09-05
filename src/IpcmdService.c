/*
 * IpcmdService.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdService.h"
#include <glib.h>

gint
IpcmdServiceCompleteOperation(OpHandle handle, const IpcmdOperationResult *result)
{

}

gint
IpcmdServiceAddSubscriber(guint16 operation_id, guint8 notification_type, const IpcmdHost *subscriber, gboolean is_static)
{

}

void
IpcmdServiceInformNotification(guint16 operation_id, guint8 notification_type, const IpcmdOperation *operation)
{

}

void
IpcmdServiceRemoveSubscriber(guint16 operation_id, guint8 notification_type, const IpcmdHost *subscriber)
{

}
