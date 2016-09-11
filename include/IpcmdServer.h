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
gboolean	IpcmdServerRegisterService(IpcmdServer *self, IpcmdService *service);
void		IpcmdServerUnregisterService(IpcmdServer *self, IpcmdService *service);
gint		IpcmdServerCompleteOperation(IpcmdServer *self, IpcmdOpCtxId opctx_id, const IpcmdOperationResult *result);
gint		IpcmdServerHandleMessage(IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg);

G_END_DECLS

#endif /* INCLUDE_IPCMDSERVER_H_ */
