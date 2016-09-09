/*
 * IpcmdDeclare.h
 *
 *  Created on: Sep 8, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDDECLARE_H_
#define INCLUDE_IPCMDDECLARE_H_

#include <glib.h>

// IpcmdCore.h
typedef struct _IpcmdCore 		IpcmdCore;
// IpcmdChannel.h
typedef guint16					IpcmdChannelId;
typedef struct _IpcmdChannel	IpcmdChannel;
// IpcmdBus.h
typedef struct _IpcmdBus 		IpcmdBus;
// IpcmdMessage.h
typedef struct _IpcmdMessage 	IpcmdMessage;
// IpcmdHost.h
typedef struct _IpcmdHost 		IpcmdHost;
// IpcmdTransport.h
typedef struct _IpcmdTransport 	IpcmdTransport;
// IpcmdOperationContext.h
typedef struct _IpcmdOperationContextId	IpcmdOpCtxId;
typedef struct _IpcmdOperationContext	IpcmdOpCtx;

#endif /* INCLUDE_IPCMDDECLARE_H_ */
