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
#include <glib.h>

struct _IpcmdServer {
	GHashTable	*operation_contexts_;
	GList		*services_;
	IpcmdCore	*core_;
	IpcmdBusEventListener	listener_;
};

#endif /* INCLUDE_IPCMDSERVERIMPL_H_ */
