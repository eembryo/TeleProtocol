/*
 * IpcmdCore.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCORE_H_
#define INCLUDE_IPCMDCORE_H_

#include <glib.h>
//#include "reference.h"
#include "IpcmdDeclare.h"

G_BEGIN_DECLS

IpcmdCore*		IpcmdCoreNew(GMainContext *context);
void			IpcmdCoreInit(IpcmdCore *self, GMainContext *context);
void			IpcmdCoreFree(IpcmdCore *self);
void			IpcmdCoreFinalize(IpcmdCore *self);
IpcmdBus*		IpcmdCoreGetBus(IpcmdCore *self);
GMainContext*	IpcmdCoreGetGMainContext(IpcmdCore *self);
void			IpcmdCoreDispatch(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
gint			IpcmdCoreTransmit(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
gint			IpcmdCoreAllocOpCtx(IpcmdCore *self, IpcmdOpCtxId opctx_id, IpcmdOpCtx **ret_opctx);
void			IpcmdCoreReleaseOpCtx(IpcmdCore *self, IpcmdOpCtxId opctx_id);
gboolean		IpcmdCoreRegisterClient(IpcmdCore *self, IpcmdClient *client);
void			IpcmdCoreUnregisterClient(IpcmdCore *self, IpcmdClient *client);


G_END_DECLS

#endif /* INCLUDE_IPCMDCORE_H_ */
