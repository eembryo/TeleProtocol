/*
 * IpcmdClient.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCLIENT_H_
#define INCLUDE_IPCMDCLIENT_H_

#include <glib.h>
#include <IpcmdDeclare.h>

G_BEGIN_DECLS

guint16		IpcmdClientGetServiceid(IpcmdClient *self);
gint		IpcmdClientHandleMessage(IpcmdClient *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
OpHandle	IpcmdClientInvokeOperation(IpcmdClient *self, const IpcmdOperation *operation, const IpcmdOperationResultCallback *cb);
gint		IpcmdClientSubscribeNotification(IpcmdClient *self, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationResultCallback *cb);
void		IpcmdClientUnsubscribeNotification(IpcmdClient *self, guint16 operation_id);
void		IpcmdClientFinalize (IpcmdClient *self);

G_END_DECLS

#endif /* INCLUDE_IPCMDCLIENT_H_ */
