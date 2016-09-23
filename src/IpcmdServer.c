/*
 * IpcmdServer.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdServer.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/reference.h"
#include "../include/IpcmdCore.h"
#include <glib.h>

#define IPCMD_SERVER_FROM_LISTENER(l) (container_of(l, struct _IpcmdServer, listener_))

static void _ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
static void _RemoveOpCtx (gpointer opctx);

gint
IpcmdServerCompleteOperation(IpcmdServer *self, IpcmdOpCtxId opctx_id, const IpcmdOperationInfo *info)
{
	//1. find opctx
	//2. trigger kIpcmdTriggerCompletedAppProcess to opctx with result
	return 0;
}

/* IpcmdServerHandleMessage :
 * handle REQUEST, SETREQUEST, SETREQUEST_NORETURN, NOTIFICATION_REQUEST, ACK and ERROR messages.
 *
 * return -1 when server cannot handle it.
 * return 0 on successfully handled.
 */
gint
IpcmdServerHandleMessage(IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg)
{
	IpcmdOpCtx 		*opctx = NULL;
	IpcmdOpCtxId	opctx_id = {
			.channel_id_ = channel_id,
			.sender_handle_id_ = IpcmdMessageGetVCCPDUSenderHandleID(mesg)
	};

	IpcmdMessageRef(mesg);

	/* validation check */
	// ProtocolVersion
	// ServiceID
	// OperationID
	// OperationType
	// Length
	/* Busy - REQPROD 347046
	 * If the server is concurrently handling the maximum number of messages required
	 * in [REQPROD347045], it shall respond with an ERROR message using the ErrorCode busy
	 * (see table in [REQPROD347068]), to any new incomming messages.
	 */
	/* Processing - REQPROD 381391, REQPROD 346841
	 * If the server is busy processing an operationID requested with the proc-flag
	 * (decribed in [REQPROD381652]) set to 0x1 and the same operationID is requested
	 * within a new message, it shall drop the newly received message and respond with
	 * an ERROR message using the ErrorCode processing (see table in [REQPROD 347068])
	 *
	 * The server shall enter the processing state described in [REQPROD 381652] and
	 * if the server receives a retransmission of such message, it shall respond with an
	 * ERROR message sending error code processing. The client receiving an ERROR message
	 * with error code processing will then know that the original message still is being
	 * handled and valid
	 */

	// look up processing list

	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_REQUEST:
		opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
		if (!opctx) { // failed to allocate opctx
			g_warning("failed to allocate operation context:");
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-REQUEST to opctx
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
		if (!opctx) { // failed to allocate opctx
			g_warning("failed to allocate operation context:");
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-SETNOR to opctx
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
		if (!opctx) { // failed to allocate opctx
			g_warning("failed to allocate operation context:");
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-SETREQ to opctx
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
		if (!opctx) { // failed to allocate opctx
			g_warning("failed to allocate operation context:");
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-NOTREQ to opctx
		break;
	case IPCMD_OPTYPE_ACK:
		//1. find opctx from operations_
		//2. trigger RECV-ACK to opctx
		break;
	case IPCMD_OPTYPE_ERROR:
		//1. find opctx from operations_
		//2. trigger RECV-ERROR to opctx
		break;
	default :
		goto _HandleMessage_failed;
	}

	IpcmdMessageUnref(mesg);
	return 0;

	_HandleMessage_failed:
	IpcmdMessageUnref(mesg);
	return -1;
}

gboolean
IpcmdServerRegisterService(IpcmdServer *self, IpcmdService *service)
{
	GList *l;

	for (l = self->services_; l!=NULL; l=l->next) {
		if (l->data == service) // already registered service
			return FALSE;
	}

	self->services_ = g_list_append (self->services_, service);

	return TRUE;
}

void
IpcmdServiceUnregisterService(IpcmdServer *self, IpcmdService *service)
{
	self->services_ = g_list_remove (self->services_, service);
}

void
IpcmdServerInit(IpcmdServer *self, IpcmdCore *core)
{
	self->services_ = NULL;
	self->core_ = core;
	self->listener_.OnChannelEvent = _ReceiveChannelEvent;
	self->operation_contexts_ = g_hash_table_new_full (IpcmdOpCtxIdHashfunc, IpcmdOpCtxIdEqual, NULL, _RemoveOpCtx);
	IpcmdBusAddEventListener (IpcmdCoreGetBus(self->core_),&self->listener_);
}

void
IpcmdServerFinalize(IpcmdServer *self)
{
	IpcmdBusRemoveEventListener (IpcmdCoreGetBus(self->core_), &self->listener_);
	// IMPL: finalize self->services_
	// IMPL: free self->operation_contexts_;
}

static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	//IpcmdServer *server = IPCMD_SERVER_FROM_LISTENER(self);

	// IMPL: whole function
	g_debug("Got channel event: id=%d, type=%d", id, type);
}


static void
_RemoveOpCtx (gpointer opctx)
{
	IpcmdOpCtxUnref(opctx);
}
