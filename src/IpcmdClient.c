/*
 * IpcmdClient.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdClient.h"
#include "../include/IpcmdChannel.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOperation.h"
#include "../include/reference.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdClientImpl.h"

#include <glib.h>

typedef struct _SubscribedNotification {
	guint16		service_id_;
	guint16		operation_id_;
	gboolean	is_cyclic_;
	IpcmdOperationCallback	cb_;
} SubscribedNotification;

#define IPCMD_CLIENT_FROM_LISTENER(l) (container_of(l, struct _IpcmdClient, listener_))

static GList* 	_LookupSubscribedNotification (GList *subscription_list, guint16 service_id, guint16 operation_id, gboolean is_cyclic);
static void		_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
static void		_OnOpCtxFinalized(IpcmdOpCtxId opctx_id, gpointer cb_data);
static void		_RemoveOpCtx (gpointer opctx);
static void		_Init (IpcmdClient *self, IpcmdHost *server_host);
static void		_OnRegisteredToCore (IpcmdClient *self, IpcmdCore *core);
static void 	_OnUnregisteredFromCore (IpcmdClient *self, IpcmdCore *core);

#define REPLY_ERROR(core, channel_id, mesg, ecode, einfo) do {\
		IpcmdMessage *error_message = IpcmdMessageNew(IPCMD_ERROR_MESSAGE_SIZE); \
		IpcmdMessageInitVCCPDUHeader (error_message, IpcmdMessageGetVCCPDUServiceID(mesg), \
				IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUSenderHandleID(mesg), \
				IpcmdMessageGetVCCPDUProtoVersion(mesg), IPCMD_OPTYPE_ERROR, IPCMD_PAYLOAD_NOTENCODED, 0); \
		IpcmdMessageSetErrorPayload (error_message, ecode, einfo); \
		IpcmdCoreTransmit (core, channel_id, error_message); \
		IpcmdMessageUnref(error_message);\
}while(0)

#define REPLY_ACK(core, channel_id, mesg) do {\
		IpcmdMessage *ack_message = IpcmdMessageNew(IPCMD_ACK_MESSAGE_SIZE); \
		IpcmdMessageInitVCCPDUHeader (ack_message, IpcmdMessageGetVCCPDUServiceID(mesg), \
				IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUSenderHandleID(mesg), \
				IpcmdMessageGetVCCPDUProtoVersion(mesg), IPCMD_OPTYPE_ACK, IPCMD_PAYLOAD_NOTENCODED, 0); \
		IpcmdCoreTransmit (core, channel_id, ack_message); \
		IpcmdMessageUnref(ack_message);\
}while(0)

#define IPCMD_CLIENT_ERROR IpcomClientErrorQuark()
static GQuark
IpcomClientErrorQuark(void)
{
	return g_quark_from_static_string ("ipcmd-client-error-quark");
}

/* @fn: IpcmdClientHandleMessage
 * Handle incoming messages such as RESPONSE, NOTIFICATION, NOTIFICATION_CYCLIC, ACK and ERROR.
 *
 * return -1 when client cannot handle the message.
 * return 0 when the message is successfully handled.
 */
gint
IpcmdClientHandleMessage(IpcmdClient *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	IpcmdOpCtx 		*opctx = NULL;
	IpcmdOpCtxId	opctx_id = {
			.channel_id_ = channel_id,
			.sender_handle_id_ = IpcmdMessageGetVCCPDUSenderHandleID(mesg)
	};
	IpcmdMessageRef(mesg);
	// IMPL: Check that remote host of channel_id is same as self->server_host_
	// workaround : check channel_id == self->channel_id
	if (self->channel_id_ != channel_id) goto _ClientHandleMessage_failed;

	//g_debug ("Client get message: CONNID=%d, SHID=0x%.04x, OP_TYPE=0x%x", opctx_id.channel_id_, opctx_id.sender_handle_id_, IpcmdMessageGetVCCPDUOpType(mesg));

	/* validation check */
	// ProtocolVersion
	if (IpcmdMessageGetVCCPDUProtoVersion(mesg) != IPCMD_PROTOCOL_VERSION) {
		//send IpcmdError with 0x04 (Invalid protocol version) if op_type is not ACK or ERROR
		if (IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ACK && IpcmdMessageGetVCCPDUOpType(mesg) != IPCMD_OPTYPE_ERROR) {
			REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_INVALID_PROTOCOL_VERSION, IPCMD_PROTOCOL_VERSION);
		}
		goto _ClientHandleMessage_done;
	}
	// Service ID - will be checked at application
	// OperationID - will be checked at application
	// OperationType - will be checked at State Machine
	// Length
	if (IpcmdMessageGetVCCPDULength(mesg)+8 != IpcmdMessageGetLength(mesg) ) {
		REPLY_ERROR (self->core_, channel_id, mesg, IPCOM_MESSAGE_ECODE_INVALID_LENGTH, 0);
		goto _ClientHandleMessage_done;
	}

	/* Handle Notification message */
	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_NOTIFICATION:
	{
		GList *sub_list;
		IpcmdOperationInfoReceivedMessage noti_info;
		IpcmdOperationInfoReceivedMessageInit (&noti_info);

		g_assert("HERE");
		// send back ACK message in case that the channel is connection-less
		if (!IpcmdBusIsChannelConnectionOriented (IpcmdCoreGetBus(self->core_), channel_id)) {
			REPLY_ACK(self->core_, channel_id, mesg);
		}

		// lookup subscribers;
		sub_list = _LookupSubscribedNotification (self->subscribed_notifications_, IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), FALSE);
		if (sub_list) {
			GList *l;
			noti_info.raw_message_ = IpcmdMessageRef(mesg);
			//IMPL: noti_info.sender_
			//noti_info.sender_ = IpcmdHostRef(sender);

			for (l = sub_list; l!=NULL; l=l->next) {
				((SubscribedNotification*)l->data)->cb_.cb_func(NULL, (IpcmdOperationInfo*)&noti_info,((SubscribedNotification*)l->data)->cb_.cb_data);
			}
			IpcmdMessageUnref(mesg);
		}
		g_list_free(sub_list);
		goto _ClientHandleMessage_done;
	}
	case IPCMD_OPTYPE_NOTIFICATION_CYCLIC:
	{
		GList *sub_list;
		IpcmdOperationInfoReceivedMessage noti_info;

		IpcmdOperationInfoReceivedMessageInit (&noti_info);
		sub_list = _LookupSubscribedNotification (self->subscribed_notifications_, IpcmdMessageGetVCCPDUServiceID(mesg), IpcmdMessageGetVCCPDUOperationID(mesg), TRUE);
		if (sub_list) {
			GList *l;
			noti_info.raw_message_ = IpcmdMessageRef(mesg);
			noti_info.sender_ = NULL;
			//IMPL: noti_info.sender_ =

			for (l = sub_list; l!=NULL; l=l->next) {
				((SubscribedNotification*)l->data)->cb_.cb_func(NULL, (IpcmdOperationInfo*)&noti_info,((SubscribedNotification*)l->data)->cb_.cb_data);
			}
			IpcmdMessageUnref(mesg);
		}
		g_list_free(sub_list);
		goto _ClientHandleMessage_done;
	}
	default:
		break;
	}

	/* Handle RESPONSE, ACK and ERROR message */
	// Check that opctx_id is already registered in hash table
	opctx = g_hash_table_lookup (self->operation_contexts_, &opctx_id);
	if (!opctx) {	// Not requested operation. Probably, delayed message came. ignore the message
		g_debug ("Got delayed message (CONNID=%d, SHID=0x%.04x, OP_TYPE=0x%x)", opctx_id.channel_id_, opctx_id.sender_handle_id_, IpcmdMessageGetVCCPDUOpType(mesg));
		goto _ClientHandleMessage_done;
	}
	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_RESPONSE:
		// trigger RECV_RESPONSE to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvResp, (gpointer)mesg);
		break;
	case IPCMD_OPTYPE_ACK:
		// trigger kIpcmdTriggerRecvAck to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvAck, 0);
		break;
	case IPCMD_OPTYPE_ERROR:
		// trigger kIpcmdTriggerRecvError to opctx
		IpcmdOpCtxTrigger (opctx, kIpcmdTriggerRecvError, (gpointer)mesg);
		break;
	default:
		goto _ClientHandleMessage_done;
	}

	_ClientHandleMessage_done:
	IpcmdMessageUnref(mesg);
	return 0;

	_ClientHandleMessage_failed:
	IpcmdMessageUnref(mesg);
	return -1;
}

/* IpmdClientInvokeOperation :
 * create IP COMMAND message and send it to bus.
 * @param[out] error: error code is -1 if Server host is unreachable, -2 on wrong op_type, -3 if too many operations exist.
 * @return operation handle or NULL
 * @retval NULL on failed to invoke the operation.
 * @retval a constant pointer on success
 */
OpHandle
IpcmdClientInvokeOperation(IpcmdClient *self, guint16 service_id, guint16 operation_id, guint8 op_type, guint8 flags, const IpcmdOperationPayload *payload, const IpcmdOperationCallback *cb, GError **error)
{
	IpcmdOpCtx *ctx = NULL;
	IpcmdOperationInfoInvokeMessage info;
	gint	ret;
	guint8	num;
	IpcmdOpCtxId	opctx_id;
	IpcmdOpCtxDeliverToAppCallback	to_app_cb = {
			.cb_func = cb->cb_func,
			.cb_destroy = cb->cb_destroy,
			.cb_data = cb->cb_data
	};
	IpcmdOpCtxFinalizeCallback finalizing_cb = {
			.cb_func = _OnOpCtxFinalized,
			.cb_destroy = NULL,
			.cb_data = self
	};

	if (self->channel_id_ == 0) { // if no channel for server_host, return NULL
		g_set_error (error, IPCMD_CLIENT_ERROR, -1, "Host(%s) is unreachable", self->server_host_->to_string (self->server_host_));
		goto IpcmdClientInvokeOperation_failed;
	}

	// 1. allocate IpcmdOpCtx from core and setup it
	opctx_id.channel_id_ = self->channel_id_;
	for (num = self->seq_num_+1; num != self->seq_num_; num++) {
		opctx_id.sender_handle_id_ = BUILD_SENDERHANDLEID(service_id, operation_id, op_type, num);
		ctx = IpcmdCoreAllocOpCtx(self->core_, opctx_id, &self->msg_handler_);
		if (ctx) {
			self->seq_num_ = num;
			break;
		}
	}
	if (!ctx) return NULL; // not enough memory or sequence number is exhausted.
	IpcmdOpCtxInit (ctx, self->core_,
			service_id, operation_id, op_type, flags,
			&to_app_cb,
			&finalizing_cb);

	// 2. add IpcmdOpCtx to self->contexts_
	g_hash_table_insert (self->operation_contexts_, &ctx->opctx_id_, ctx);

	// 3. trigger
	info.parent_.type_ = kOperationInfoInvokeMessage;
	info.header_.op_type_ = op_type;
	info.header_.flags_ = flags;
	info.header_.operation_id_ = operation_id;
	info.header_.service_id_ = service_id;
	if (payload == NULL) {
		info.payload_.data_ = NULL;
		info.payload_.length_ = 0;
		info.payload_.type_ = 0x01;	//no encoding
	}
	else {
		info.payload_ = *payload;
	}

	switch (op_type) {
	case IPCMD_OPTYPE_REQUEST:
		ret = IpcmdOpCtxTrigger (ctx, kIpcmdTriggerSendRequest, (const IpcmdOperationInfo*)&info);
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		ret = IpcmdOpCtxTrigger (ctx, kIpcmdTriggerSendSetnor, (const IpcmdOperationInfo*)&info);
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		ret = IpcmdOpCtxTrigger (ctx, kIpcmdTriggerSendSetreq, (const IpcmdOperationInfo*)&info);
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		ret = IpcmdOpCtxTrigger (ctx, kIpcmdTriggerSendNotreq, (const IpcmdOperationInfo*)&info);
		break;
	default:
		g_set_error (error, IPCMD_CLIENT_ERROR, -2, "A wrong operation type(%d) is requested", op_type);
		goto IpcmdClientInvokeOperation_failed;
		break;
	}
	if (ret < 0) {	//failed to send request
		g_error ("Failed to trigger opeartion context.");
		return NULL;
	}

	return ctx;

	IpcmdClientInvokeOperation_failed:
	if (ctx) _OnOpCtxFinalized ( ctx->opctx_id_, (gpointer)self);
	return NULL;
}


/******************************************************************************************************
 *  @fn: IpcmdClientSubscribeNotification :
 * subscribe to IpcmdBus to receive notification messages, which are for service_id and operation_id.
 *
 * type should be one of NOTIFICATION or NOTIFICATION_CYCLIC
 *
 * @operation_id: 0 means any operations.
 *
 * return -1 on failed
 * return 0 on successfully subscribed
 ******************************************************************************************************/
gint
IpcmdClientSubscribeNotification(IpcmdClient *self, guint16 service_id, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationCallback *cb)
{
	SubscribedNotification *new_sn;
	//GList *l;

	new_sn = g_malloc(sizeof(SubscribedNotification));
	if (!new_sn) {
		g_warning("Not enough memory.");
		return -1;	// not enough memory
	}

	new_sn->service_id_ = service_id;
	new_sn->operation_id_ = operation_id;
	new_sn->is_cyclic_ = is_cyclic;
	new_sn->cb_ = *cb;

	self->subscribed_notifications_ = g_list_append(self->subscribed_notifications_, new_sn);

	return 0;
}

/* IpcmdClientUnsubscribeNotification :
 * stop to receive notification message, which are for service_id and operation_id.
 */
void
IpcmdClientUnsubscribeNotification(IpcmdClient *self, guint16 service_id, guint16 operation_id)
{
	GList 	*l;
	SubscribedNotification *sn;

	for (l=self->subscribed_notifications_;l!=NULL;l=l->next) {
		if ( ((SubscribedNotification*)l->data)->service_id_ == service_id &&
				((SubscribedNotification*)l->data)->operation_id_ == operation_id) {
			self->subscribed_notifications_ = g_list_remove_link (self->subscribed_notifications_,l);
			sn = l->data;
			IpcmdOperationCallbackClear(&sn->cb_);
			g_free(sn);
			g_list_free(l);
		}
	}
}

IpcmdClient*
IpcmdClientNew (IpcmdCore *core, IpcmdHost *server_host)
{
	IpcmdClient *client = g_malloc(sizeof(struct _IpcmdClient));

	_Init(client, server_host);
	return client;
}

void
IpcmdClientDestroy (IpcmdClient *self)
{
	// IMPL: free self->subscribed_notifications_;
	IpcmdHostUnref (self->server_host_);
	g_free(self);
}

static GList*
_LookupSubscribedNotification (GList *subscription_list, guint16 service_id, guint16 operation_id, gboolean is_cyclic)
{
	GList *l;
	GList *ret_list = NULL;

	for (l=subscription_list; l!=NULL;l=l->next) {
		if ( ((SubscribedNotification*)l->data)->service_id_ == service_id &&
				((SubscribedNotification*)l->data)->operation_id_ == operation_id &&
				((SubscribedNotification*)l->data)->is_cyclic_ == is_cyclic ) {
			ret_list = g_list_append (ret_list, l->data);
		}
		else if ( ((SubscribedNotification*)l->data)->service_id_ == 0) { // receive all notification messages
			ret_list = g_list_append(ret_list,l->data);
		}
		else if ( ((SubscribedNotification*)l->data)->service_id_ == service_id && // receive all notification messages for specific service
				((SubscribedNotification*)l->data)->operation_id_ == 0) {
			ret_list = g_list_append(ret_list,l->data);
		}
	}
	return ret_list;
}

static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	IpcmdClient *client = IPCMD_CLIENT_FROM_LISTENER(self);

	switch (type) {
	case kBusEventChannelAdd:
		if (client->channel_id_) break;
		if (IpcmdBusIsChannelIdForPeerHost (IpcmdCoreGetBus (client->core_), id, client->server_host_)) {
			client->channel_id_ = id;
			g_debug("[IpcmdClient] Server is reachable.");
		}
		break;
	case kBusEventChannelRemove:
		if (client->channel_id_ == id) {
			// IMPL: cancel operations
			g_debug("[IpcmdClient] Server is unreachable.");
			client->channel_id_ = 0;
		}
		break;
	case kBusEventChannelStatusChange:
		// IMPL: not implemented
		break;
	default:
		break;
	}
}

static inline gint
_MessageHandler(IpcmdMessageHandlerInterface *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	IpcmdClient *client = container_of(self, struct _IpcmdClient, msg_handler_);
	return IpcmdClientHandleMessage (client, channel_id, mesg);
}

static void
_Init (IpcmdClient *self, IpcmdHost *server_host)
{
	g_assert(server_host != NULL);

	self->msg_handler_.handle = _MessageHandler;
	self->server_host_ = IpcmdHostRef(server_host);
	self->subscribed_notifications_ = NULL;
	self->operation_contexts_ = g_hash_table_new_full (IpcmdOpCtxIdHashfunc, IpcmdOpCtxIdEqual, NULL, _RemoveOpCtx);
	self->seq_num_ = 0;
	self->listener_.OnChannelEvent = _ReceiveChannelEvent;
	self->OnRegisteredToCore = _OnRegisteredToCore;
	self->OnUnregisteredFromCore = _OnUnregisteredFromCore;
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
	IpcmdClient *client = (IpcmdClient *)cb_data;

	g_hash_table_remove (client->operation_contexts_, &opctx_id);
	IpcmdCoreReleaseOpCtx (client->core_, opctx_id);
}

static void
_OnRegisteredToCore (IpcmdClient *self, IpcmdCore *core)
{
	GList *l;

	self->core_ = core;

	IpcmdBusAddEventListener (IpcmdCoreGetBus(self->core_),&self->listener_);

	// Lookup channel_id for server_host
	l = IpcmdBusFindChannelIdsByPeerHost (IpcmdCoreGetBus(self->core_), self->server_host_);
	self->channel_id_ = l ? GPOINTER_TO_INT (l->data) : 0;	// set the first channel in the list if it exists
	g_list_free (l);

}

static void
_OnUnregisteredFromCore (IpcmdClient *self, IpcmdCore *core)
{
	IpcmdBusRemoveEventListener (IpcmdCoreGetBus(self->core_), &self->listener_);
	// IMPL: free self->operation_contexts_;
	self->core_ = NULL;
	self->channel_id_ = 0;
}
