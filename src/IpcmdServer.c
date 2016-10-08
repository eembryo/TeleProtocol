/*
 * IpcmdServer.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdService.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/reference.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdService.h"
#include "../include/IpcmdConfig.h"
#include "../include/IpcmdChannel.h"
#include <glib.h>

#define IPCMD_SERVER_FROM_LISTENER(l) (container_of(l, struct _IpcmdServer, listener_))

static void	_OnOpCtxDeliverToApp(OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data);
static void _ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
static void _RemoveOpCtx (gpointer opctx);
static void	_OnOpCtxFinalized(IpcmdOpCtxId opctx_id, gpointer cb_data);

#define REPLY_ERROR(core, channel_id, mesg, ecode, einfo) do {\
		IpcmdMessage *error_message = IpcmdMessageNew(IPCMD_ERROR_MESSAGE_SIZE); \
		IpcmdMessageInitVCCPDUHeader (error_message, IpcmdMessageGetVCCPDUServiceID(mesg), \
				IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUSenderHandleID(mesg), \
				IpcmdMessageGetVCCPDUProtoVersion(mesg), IPCMD_OPTYPE_ERROR, IPCMD_PAYLOAD_NOTENCODED, 0); \
		IpcmdMessageSetErrorPayload (error_message, ecode, einfo); \
		IpcmdCoreTransmit (core, channel_id, error_message); \
		IpcmdMessageUnref(error_message);\
}while(0)

gint
IpcmdServerCompleteOperation(IpcmdServer *self, const IpcmdOpCtxId *opctx_id, const IpcmdOperationInfo *info)
{
	IpcmdOpCtx *opctx;

	//1. find opctx
	opctx = (IpcmdOpCtx*)g_hash_table_lookup (self->operation_contexts_, opctx_id);
	if (!opctx) {
		g_warning("IpcmdServerCompleteOperation() is called with not existing operation. "
				"(channel: %d, SenderHandleID:%.04x", opctx_id->channel_id_, opctx_id->sender_handle_id_);
		return -1;
	}
	//2. trigger kIpcmdTriggerCompletedAppProcess to opctx with result
	if (IpcmdOpCtxTrigger (opctx, kIpcmdTriggerCompletedAppProcess, info) < 0) {
		g_warning("Wrong trigger(kIpcmdTriggerCompletedAppProcess).");
		return -1;
	}

	return 0;
}

/* IpcmdServerHandleMessage :
 * handle REQUEST, SETREQUEST, SETREQUEST_NORETURN, NOTIFICATION_REQUEST, ACK and ERROR messages.
 * IpcmdServer returns -1 when it cannot handle the received message. For example, IpcmdServer will
 * return -1 when it cannot found an operation context about this message. On the other hand,
 * IpcmdServer will send ERROR message and return 0 when it received malformed message.
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
	IpcmdService	*service;
	IpcmdOpCtxDeliverToAppCallback	to_app_cb = {
			.cb_func = _OnOpCtxDeliverToApp,
			.cb_destroy = NULL,
			.cb_data = NULL
	};
	IpcmdOpCtxFinalizeCallback finalizing_cb = {
			.cb_func = _OnOpCtxFinalized,
			.cb_destroy = NULL,
			.cb_data = self
	};

	IpcmdMessageRef(mesg);

	g_debug ("Server get message: CONNID=%d, SHID=0x%.04x, OP_TYPE=0x%x", opctx_id.channel_id_, opctx_id.sender_handle_id_, IpcmdMessageGetVCCPDUOpType(mesg));
	/* validation check */
	// ProtocolVersion
	if (IpcmdMessageGetVCCPDUProtoVersion(mesg) != IPCMD_PROTOCOL_VERSION) {
		// send IpcmdError with 0x04 (Invalid protocol version) if op_type is not ACK or ERROR
		if (IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ACK && IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ERROR) {
			REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_INVALID_PROTOCOL_VERSION, IPCMD_PROTOCOL_VERSION);
		}
		goto _HandleMessage_done;
	}
	// ServiceID
	service = IpcmdServerLookupService (self, IpcmdMessageGetVCCPDUServiceID(mesg));
	if (!service) {
		// send IpcmdError with 0x01 (ServiceID not available) if op_type is not ACK or ERROR
		if (IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ACK && IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ERROR) {
			REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_SERVICEID_NOT_AVAILABLE, IpcmdMessageGetVCCPDUServiceID(mesg));
		}
		goto _HandleMessage_done;
	}
	to_app_cb.cb_data = service;
	//to_app_cb.cb_data = service;
	// OperationID		-- Application will do
	// OperationType	-- check in below
	// Length			-- Application will do
	// Busy				-- check in below
	// Processing		-- check in Operation Context
	// ApplicationError	-- Application will do
	// Timeout			-- Not yet (IMPL NEEDED)

	// Check opctx_id is already registered in hash table
	opctx = g_hash_table_lookup (self->operation_contexts_, &opctx_id);
	// look up processing list
	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_REQUEST:
		if (!opctx) {	// new operation
			/* Busy - REQPROD 347046
			 * If the server is concurrently handling the maximum number of messages required
			 * in [REQPROD347045], it shall respond with an ERROR message using the ErrorCode busy
			 * (see table in [REQPROD347068]), to any new incoming messages.
			 */
			// If IpcmdServer has maximum number of operation contexts, send BUSY error
			if (g_hash_table_size (self->operation_contexts_) >= IpcmdConfigGetInstance()->maximumConcurrentMessages) {
				REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_BUSY, 0);
				goto _HandleMessage_done;
			}
			opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
			if (!opctx) { // failed to allocate opctx
				g_warning("failed to allocate operation context:");
				goto _HandleMessage_done;
			}
			//1.1. initialize opctx
			IpcmdOpCtxInit (opctx, self->core_,
					IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUOpType(mesg), IpcmdMessageGetVCCPDUFlags(mesg),
					&to_app_cb,
					&finalizing_cb);
			//1.2. Add to hash table
			g_hash_table_insert (self->operation_contexts_, &opctx->opctx_id_, opctx);
		}
		//2. trigger RECV-REQUEST to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvRequest, (gpointer)mesg);
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		if (!opctx) {	// new operation
			// If IpcmdServer has maximum number of operation contexts, send BUSY error
			if (g_hash_table_size (self->operation_contexts_) >= IpcmdConfigGetInstance()->maximumConcurrentMessages) {
				REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_BUSY, 0);
				goto _HandleMessage_done;
			}
			opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
			if (!opctx) { // failed to allocate opctx
				g_warning("failed to allocate operation context:");
				goto _HandleMessage_done;
			}
			//1.1. initialize opctx
			IpcmdOpCtxInit (opctx, self->core_,
					IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUOpType(mesg), IpcmdMessageGetVCCPDUFlags(mesg),
					&to_app_cb,
					&finalizing_cb);
			//1.2. Add to hash table
			g_hash_table_insert (self->operation_contexts_, &opctx->opctx_id_, opctx);
		}
		//3. trigger RECV-SETNOR to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvSetnor, (gpointer)mesg);
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		if (!opctx) {	// new operation
			// If IpcmdServer has maximum number of operation contexts, send BUSY error
			if (g_hash_table_size (self->operation_contexts_) >= IpcmdConfigGetInstance()->maximumConcurrentMessages) {
				REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_BUSY, 0);
				goto _HandleMessage_done;
			}
			opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
			if (!opctx) { // failed to allocate opctx
				g_warning("failed to allocate operation context:");
				goto _HandleMessage_done;
			}
			//1.1. initialize opctx
			IpcmdOpCtxInit (opctx, self->core_,
					IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUOpType(mesg), IpcmdMessageGetVCCPDUFlags(mesg),
					&to_app_cb,
					&finalizing_cb);
			//1.2. Add to hash table
			g_hash_table_insert (self->operation_contexts_, &opctx->opctx_id_, opctx);
		}
		//3. trigger RECV-SETREQ to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvSetreq, (gpointer)mesg);
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		if (!opctx) {	// new operation
			// If IpcmdServer has maximum number of operation contexts, send BUSY error
			if (g_hash_table_size (self->operation_contexts_) >= IpcmdConfigGetInstance()->maximumConcurrentMessages) {
				REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_BUSY, 0);
				goto _HandleMessage_done;
			}
			opctx = IpcmdCoreAllocOpCtx (self->core_, opctx_id);
			if (!opctx) { // failed to allocate opctx
				g_warning("failed to allocate operation context:");
				goto _HandleMessage_done;
			}
			//1.1. initialize opctx
			IpcmdOpCtxInit (opctx, self->core_,
					IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUOpType(mesg), IpcmdMessageGetVCCPDUFlags(mesg),
					&to_app_cb,
					&finalizing_cb);
			//1.2. Add to hash table
			g_hash_table_insert (self->operation_contexts_, &opctx->opctx_id_, opctx);
		}
		//3. trigger RECV-NOTREQ to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvNotreq, (gpointer)mesg);
		break;
	case IPCMD_OPTYPE_ACK:
		if (!opctx) goto _HandleMessage_done;
		//1. trigger RECV-ACK to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvAck, NULL);
		break;
	case IPCMD_OPTYPE_ERROR:
		if (!opctx) goto _HandleMessage_done;
		//1. trigger RECV-ERROR to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvError, (gpointer)mesg);
		break;
	default :
		// send error with 0x03 (OperationType is not available)
		REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_OPERATIONTYPE_NOT_AVAILABLE, IpcmdMessageGetVCCPDUOpType(mesg));
		goto _HandleMessage_done;
	}

	_HandleMessage_done:
	IpcmdMessageUnref(mesg);
	return 0;
/*
	_HandleMessage_failed:
	IpcmdMessageUnref(mesg);
	return -1;
 */
}

/*
IpcmdService *
IpcmdServerNewService (IpcmdServer *self, guint16 service_id, ExecuteOperation exec_func)
{
	IpcmdService *service;

	service = g_malloc (sizeof(struct _IpcmdService));
	service->server_ = self;
	service->service_id_ = service_id;
	service->exec_ = exec_func;

	if (!IpcmdServerRegisterService (self, service)) goto _NewService_failed;

	return service;

	_NewService_failed:
	if (service) g_free (service);
	return NULL;
}
*/
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
IpcmdServerUnregisterService(IpcmdServer *self, IpcmdService *service)
{
	self->services_ = g_list_remove (self->services_, service);
}

IpcmdService*
IpcmdServerLookupService (IpcmdServer *self, guint16 service_id)
{
	GList *l;
	for (l=self->services_; l!=NULL; l=l->next) {
		if (((IpcmdService*)l->data)->service_id_ == service_id) {
			return (IpcmdService *)l->data;
		}
	}
	return NULL;
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


/*
 * return:	0 on success
 * 			-1 on not enough memory
 * 			-2 on unreachable target
 * 			-3 on no available SenderHandleId
 *
 */
gint
IpcmdServerSendNotification(IpcmdServer *self, guint16 service_id, guint16 operation_id, guint8 *seq_num, IpcmdHost *target, const IpcmdOperationInfo *info)
{
	IpcmdOpCtx 		*ctx = NULL;
	IpcmdOpCtxId	opctx_id;
	IpcmdChannelId	channel_id = 0;
	GList			*ret = NULL;
	guint32			num;
	IpcmdOpCtxFinalizeCallback 	finalizing_cb = {
			.cb_func = _OnOpCtxFinalized,
			.cb_destroy = NULL,
			.cb_data = self
	};
	const IpcmdOperationInfoReplyMessage	*reply_info = (const IpcmdOperationInfoReplyMessage*)info;

	// info should be IpcmdOperationInfoReplyMessage
	g_assert(info->type_ == kOperationInfoReplyMessage);

	// 1. Find out that the target is reachable or not.
	ret = IpcmdBusFindChannelIdsByPeerHost(IpcmdCoreGetBus (self->core_), target);
	if (ret) {
		channel_id = (IpcmdChannelId)GPOINTER_TO_INT(ret->data);
		g_list_free(ret);
	}
	else {
		return -2;	// no channel for target
	}

	if (reply_info->op_type_ == IPCMD_OPTYPE_NOTIFICATION) {
		// 2. allocate IpcmdOpCtx from core and setup it
		opctx_id.channel_id_ = channel_id;
		num = *seq_num;
		do {
			opctx_id.sender_handle_id_ = BUILD_SENDERHANDLEID(service_id, operation_id, reply_info->op_type_, num);
			ctx = IpcmdCoreAllocOpCtx(self->core_, opctx_id);
			if (ctx) break;
			num++;
		} while(num!=*seq_num);
		if (!ctx) {
			return -3; //no available SenderHandleId
		}
		*seq_num = num;
		IpcmdOpCtxInit (ctx, self->core_,
				service_id, operation_id, reply_info->op_type_, 0,
				NULL,
				&finalizing_cb);
		// 3. add IpcmdOpCtx to self->contexts_
		g_hash_table_insert (self->operation_contexts_, &ctx->opctx_id_, ctx);
		// 4. trigger to send
		IpcmdOpCtxTrigger (ctx, kIpcmdTriggerSendNoti, info);
	}
	else {	// IPCMD_OPTYPE_NOTIFICATION_CYCLIC
		IpcmdMessage *notif_cyclic_mesg = IpcmdMessageNew (VCCPDUHEADER_SIZE + reply_info->payload_.length_);
		IpcmdMessageInitVCCPDUHeader (notif_cyclic_mesg, service_id, operation_id, BUILD_SENDERHANDLEID(service_id, operation_id, reply_info->op_type_, *seq_num), IPCMD_PROTOCOL_VERSION, reply_info->op_type_, reply_info->payload_.type_,0);
		IpcmdMessageCopyToPayloadBuffer (notif_cyclic_mesg, reply_info->payload_.data_, reply_info->payload_.length_);
		IpcmdCoreTransmit(self->core_, channel_id, notif_cyclic_mesg);
		IpcmdMessageUnref (notif_cyclic_mesg);
	}

	return 0;
}
/*****************************
 * static functions
 *****************************/
static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	IpcmdServer *server = IPCMD_SERVER_FROM_LISTENER(self);

	switch (type) {
	case kBusEventChannelAdd:
		// nothing to do
		break;
	case kBusEventChannelRemove:
		// IMPL: remove operations related to id
		break;
	case kBusEventChannelStatusChange:
		// IMPL: not implemented
		break;
	default:
		break;
	}
}


static void
_OnOpCtxDeliverToApp(OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data)
{
	IpcmdService *service = (IpcmdService*)cb_data;

	service->exec_.cb_func (handle, result, service->exec_.cb_data);
	//service->exec_(service, handle, result);
}


/* @fn: _RemoveOpCtx
 * remove IpcmdOpCtx from hashtable. MUST call IpcmdCoreReleaseOpCtx() after this.
 *
 */
static void
_RemoveOpCtx (gpointer opctx)
{
	IpcmdOpCtxUnref(opctx);
}

static void
_OnOpCtxFinalized(IpcmdOpCtxId opctx_id, gpointer cb_data)
{
	IpcmdServer *server = (IpcmdServer *)cb_data;

	g_hash_table_remove (server->operation_contexts_, &opctx_id);
	IpcmdCoreReleaseOpCtx (server->core_, opctx_id);
}
