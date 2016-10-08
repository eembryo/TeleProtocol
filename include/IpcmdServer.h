/*
 * IpcmdServer.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDSERVER_H_
#define INCLUDE_IPCMDSERVER_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

void		IpcmdServerInit(IpcmdServer *self, IpcmdCore *core);
void		IpcmdServerFinalize(IpcmdServer *self);
//IpcmdService*	IpcmdServerNewService (IpcmdServer *self, guint16 service_id, ExecuteOperation exec_func);
gboolean	IpcmdServerRegisterService(IpcmdServer *self, IpcmdService *service);
void		IpcmdServerUnregisterService(IpcmdServer *self, IpcmdService *service);
gint		IpcmdServerCompleteOperation(IpcmdServer *self, const IpcmdOpCtxId *opctx_id, const IpcmdOperationInfo *info);
gint		IpcmdServerHandleMessage(IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg);
IpcmdService*	IpcmdServerLookupService (IpcmdServer *self, guint16 service_id);
/*
 * return : negative value on no memory, no available SHID, unreachable target.
 */
gint		IpcmdServerSendNotification(IpcmdServer *self, guint16 service_id, guint16 operation_id, guint8 *seq_num, IpcmdHost *target, const IpcmdOperationInfo *info);
G_END_DECLS

#endif /* INCLUDE_IPCMDSERVER_H_ */
