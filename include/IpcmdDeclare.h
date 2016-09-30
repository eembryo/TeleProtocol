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

#define IPCMD_PROTOCOL_VERSION 0x03

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
typedef gconstpointer						OpHandle;			// OpHandle indicates the memory address of IpcmdOpCtx
typedef struct _IpcmdOperationHeader		IpcmdOperationHeader;
typedef struct _IpcmdOperationPayload		IpcmdOperationPayload;

typedef struct _IpcmdOperationInfo			IpcmdOperationInfo;
typedef struct _IpcmdOperationInfoOk		IpcmdOperationInfoOk;
typedef struct _IpcmdOperationInfoFail		IpcmdOperationInfoFail;
typedef struct _IpcmdOperationInfoReceivedMessage	IpcmdOperationInfoReceivedMessage;
typedef struct _IpcmdOperationInfoReplyMessage		IpcmdOperationInfoReplyMessage;
typedef struct _IpcmdOperationInfoInvokeMessage		IpcmdOperationInfoInvokeMessage;
typedef struct _IpcmdOperationCallback		IpcmdOperationCallback;
// IpcmdService.h
typedef struct _IpcmdService IpcmdService;
typedef void (*ExecuteOperation)(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *operation);

// common
typedef struct {
	IpcmdHost		*host;
	IpcmdChannelId	id_;
} IpcmdChannelId2HostPair ;

G_END_DECLS

#endif /* INCLUDE_IPCMDDECLARE_H_ */
