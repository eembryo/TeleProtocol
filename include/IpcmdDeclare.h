/*
 * IpcmdDeclare.h
 *
 *  Created on: Sep 8, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDDECLARE_H_
#define INCLUDE_IPCMDDECLARE_H_

#include <glib.h>

G_BEGIN_DECLS

// IpcmdCore.h
typedef struct _IpcmdCore 		IpcmdCore;
// IpcmdChannel.h
typedef guint16					IpcmdChannelId;
typedef struct _IpcmdChannel	IpcmdChannel;
// IpcmdBus.h
typedef struct _IpcmdBus 		IpcmdBus;
typedef struct _IpcmdBusEventListener	IpcmdBusEventListener;
// IpcmdMessage.h
typedef struct _IpcmdMessage 	IpcmdMessage;
// IpcmdHost.h
typedef struct _IpcmdHost 		IpcmdHost;
// IpcmdTransport.h
typedef struct _IpcmdTransport 	IpcmdTransport;
// IpcmdOperationContext.h
typedef struct _IpcmdOperationContextId	IpcmdOpCtxId;
typedef struct _IpcmdOperationContext	IpcmdOpCtx;
// IpcmdOpState.h
typedef struct _IpcomOpState 	IpcomOpState;
// IpcmdServer.h
typedef struct _IpcmdServer		IpcmdServer;
// IpcmdClient.h
typedef struct _IpcmdClient IpcmdClient;
// IpcmdOperation.h
typedef gconstpointer					OpHandle;			// OpHandle indicates the memory address of IpcmdOpCtx
typedef struct _IpcmdOperation 			IpcmdOperation;
typedef struct _IpcmdOperationResult	IpcmdOperationResult;
typedef struct _IpcmdPayloadData		IpcmdPayloadData;
typedef struct _IpcmdOperationResultNotification	IpcmdOperationResultNotification;
typedef struct _IpcmdOperationResultCallback 		IpcmdOperationResultCallback;
// IpcmdService.h
typedef struct _IpcmdService IpcmdService;

G_END_DECLS

#endif /* INCLUDE_IPCMDDECLARE_H_ */
