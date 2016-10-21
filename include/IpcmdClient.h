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

IpcmdClient*	IpcmdClientNew (IpcmdCore *core, IpcmdHost *server_host);
//guint16		IpcmdClientGetServiceid(IpcmdClient *self);
gint		IpcmdClientHandleMessage(IpcmdClient *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
OpHandle	IpcmdClientInvokeOperation(IpcmdClient *self, guint16 service_id, guint16 operation_id, guint8 op_type, guint8 flags, const IpcmdOperationPayload *payload, const IpcmdOperationCallback *cb);
gint		IpcmdClientSubscribeNotification(IpcmdClient *self, guint16 service_id, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationCallback *cb);
void		IpcmdClientUnsubscribeNotification(IpcmdClient *self, guint16 service_id, guint16 operation_id);
void		IpcmdClientFinalize (IpcmdClient *self);
void		IpcmdClientDestroy (IpcmdClient *self);
G_END_DECLS

#endif /* INCLUDE_IPCMDCLIENT_H_ */
