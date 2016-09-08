/*
 * IpcmdCore.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCORE_H_
#define INCLUDE_IPCMDCORE_H_

#include <glib.h>
#include "reference.h"
#include "IpcmdDeclare.h"
#include "IpcmdBus.h"

G_BEGIN_DECLS

struct _IpcmdCore {
	//IpcmdServer	*server_;
	//GList			*clients_;
	GMainContext	*main_context;
	IpcmdBus		bus_;
	GHashTable		*operation_contexts_;
};

void			IpcmdCoreInit(IpcmdCore *self, GMainContext *context);
IpcmdBus*		IpcmdCoreGetBus(IpcmdCore *self);
GMainContext*	IpcmdCoreGetGMainContext(IpcmdCore *self);
void			IpcmdCoreDispatch(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
gint			IpcmdCoreTransmit(IpcmdCore *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);

G_END_DECLS

#endif /* INCLUDE_IPCMDCORE_H_ */
