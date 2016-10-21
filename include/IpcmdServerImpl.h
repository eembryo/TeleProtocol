/*
 * IpcmdServerImpl.h
 *
 *  Created on: Sep 9, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDSERVERIMPL_H_
#define INCLUDE_IPCMDSERVERIMPL_H_

#include "IpcmdDeclare.h"
#include "IpcmdBus.h"
#include "IpcmdMessageHandler.h"
#include <glib.h>

G_BEGIN_DECLS

struct _IpcmdServer {
	IpcmdMessageHandlerInterface	msg_handler_;
	GHashTable	*operation_contexts_;
	GList		*services_;
	IpcmdCore	*core_;
	IpcmdBusEventListener	listener_;
};

G_END_DECLS

#endif /* INCLUDE_IPCMDSERVERIMPL_H_ */
