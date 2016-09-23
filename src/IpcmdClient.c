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

#include <glib.h>

struct _IpcmdClient {
	IpcmdChannelId	channel_id_;
	GHashTable		*operation_contexts_;
	GList			*subscribed_notifications_;
	guint16			service_id_;
	guint8			seq_num_;
	IpcmdCore		*core_;
	IpcmdBusEventListener	listener_;
};

typedef struct _SubscribedNotification SubscribedNotification;
struct _SubscribedNotification {
	guint16		service_id_;
	guint16		operation_id_;
	gboolean	is_cyclic_;
	IpcmdOperationCallback	cb_;
};

#define IPCMD_CLIENT_FROM_LISTENER(l) (container_of(l, struct _IpcmdClient, listener_))

static GList* 	_LookupSubscribedNotification (GList *subscription_list, guint16 service_id, guint16 operation_id, gboolean is_cyclic);
static void		_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);

guint16
IpcmdClientGetServiceid(IpcmdClient *self)
{
	return self->service_id_;
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
	/*
	IpcmdOpCtxId	opctx_id = {
			.channel_id_ 		=	channel_id,
			.sender_handle_id_	=	IpcmdMessageGetVCCPDUSenderHandleID(mesg)
	};
	*/

	IpcmdMessageRef(mesg);

	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_RESPONSE:
		// trigger RECV_RESPONSE to opctx
		break;
	case IPCMD_OPTYPE_NOTIFICATION:
	case IPCMD_OPTYPE_NOTIFICATION_CYCLIC:
	{
#if 0
		GList *sub_list;
		IpcmdOperationResultNotification noti_result = {
				.parent_.result_type_ = OPERATION_RESULT_NOTIFICATION,
		};
		IpcmdPayloadData payload;

		// IMPL: if (channel is connection-less) immediately send ACK
		sub_list = _LookupSubscribedNotification (self->subscribed_notifications_, self->service_id_, IpcmdMessageGetVCCPDUOperationID(mesg), FALSE);
		if (sub_list) {
			GList *l;
			noti_result.notification_.service_id_ = IpcmdMessageGetVCCPDUServiceID(mesg);
			noti_result.notification_.operation_id_ = IpcmdMessageGetVCCPDUOperationID(mesg);
			noti_result.notification_.sender_handle_id_ = IpcmdMessageGetVCCPDUSenderHandleID(mesg);
			noti_result.notification_.op_type_ = IpcmdMessageGetVCCPDUOpType(mesg);
			noti_result.notification_.payload_data_ = &payload;
			payload.type_ = IpcmdMessageGetVCCPDUDataType(mesg);
			payload.length_ = IpcmdMessageGetPaylodLength(mesg);
			payload.data_ = IpcmdMessageGetPayload(mesg);

			for (l = sub_list; l!=NULL; l=l->next) {
				((SubscribedNotification*)l->data)->cb_.cb_func(&VoidOpCtxId, &noti_result.parent_,((SubscribedNotification*)l->data)->cb_.cb_data);
			}
		}
		g_list_free(sub_list);
#endif
	}
		break;
	case IPCMD_OPTYPE_ACK:
		// trigger RECV_ACK to opctx
		break;
	case IPCMD_OPTYPE_ERROR:
		// trigger RECV_ERROR to opctx
		break;
	default:
		goto _ClientHandleMessage_failed;
	}

	IpcmdMessageUnref(mesg);
	return 0;

	_ClientHandleMessage_failed:
	IpcmdMessageUnref(mesg);
	return -1;
}

/* IpmdClientInvokeOperation :
 * create IP COMMAND message and send it to bus.
 */
OpHandle
IpcmdClientInvokeOperation(IpcmdClient *self, guint16 operation_id, guint8 op_type, guint8 flags, const IpcmdOperationPayload *payload, const IpcmdOperationCallback *cb)
{
	IpcmdOpCtx *ctx;
	IpcmdOperationInfoInvokeMessage info;
	gint	ret;
	guint8	num;
	IpcmdOpCtxId	opctx_id;

	if (self->channel_id_ == 0) { // there is no active channel
		return NULL;
	}

	// 1. allocate IpcmdOpCtx from core and setup it
	opctx_id.channel_id_ = self->channel_id_;
	for (num = self->seq_num_+1; num != self->seq_num_; num++) {
		opctx_id.sender_handle_id_ = BUILD_SENDERHANDLEID(self->service_id_, operation_id, op_type, num);
		ctx = IpcmdCoreAllocOpCtx(self->core_, opctx_id);
		if (ctx) {
			self->seq_num_ = num;
			break;
		}
	}
	if (!ctx) return NULL; // not enough memory or sequence number is exhausted.

	ctx->serviceId = self->service_id_;
	ctx->operationId = operation_id;
	ctx->protoVersion = IPCMD_PROTOCOL_VERSION;
	ctx->opType = op_type;
	ctx->flags = flags;
	ctx->deliver_to_app_ = *cb;
	// IMPL: ctx->notify_finalizing_ =
	//ctx->nWFABaseTimeout = ;
	//ctx->nWFAIncreaseTimeout = ;
	//ctx->nWFAMaxRetries = ;
	//ctx->nWFRBaseTimeout = ;
	//ctx->nWFRIncreaseTimeout = ;
	//ctx->nWFRMaxRetries = ;

	// 2. add IpcmdOpCtx to self->contexts_
	g_hash_table_insert (self->operation_contexts_, &ctx->opctx_id_,ctx);

	// 3. trigger
	info.parent_.type_ = kOperationInfoInvokeMessage;
	info.header_.op_type_ = op_type;
	info.header_.flags_ = flags;
	info.header_.operation_id_ = operation_id;
	info.header_.service_id_ = self->service_id_;
	info.payload_ = *payload;

	ret = IpcomOpCtxTrigger (ctx, kIpcmdTriggerSendRequest, (const IpcmdOperationInfo*)&info);
	if (ret < 0) {	//failed to send request
		//IMPL: need to report error message
		return NULL;
	}

	return ctx;
}

/******************************************************************************************************
 *  @fn: IpcmdClientSubscribeNotification :
 * subscribe to IpcmdBus to receive notification messages, which are for service_id and operation_id.
 *
 * type should be one of NOTIFICATION or NOTIFICATION_CYCLIC
 *
 * return -1 on failed
 * return 0 on successfully subscribed
 ******************************************************************************************************/
gint
IpcmdClientSubscribeNotification(IpcmdClient *self, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationCallback *cb)
{
	SubscribedNotification *new_sn;
	//GList *l;

	new_sn = g_malloc(sizeof(SubscribedNotification));
	if (!new_sn) {
		g_warning("Not enough memory.");
		return -1;	// not enough memory
	}

	new_sn->service_id_ = self->service_id_;
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
IpcmdClientUnsubscribeNotification(IpcmdClient *self, guint16 operation_id)
{
	GList 	*l;
	SubscribedNotification *sn;

	for (l=self->subscribed_notifications_;l!=NULL;l=l->next) {
		if (((SubscribedNotification*)l->data)->operation_id_ == operation_id) {
			self->subscribed_notifications_ = g_list_remove_link (self->subscribed_notifications_,l);
			sn = l->data;
			IpcmdOperationCallbackClear(&sn->cb_);
			g_free(sn);
			g_list_free(l);
		}
	}
}

void
IpcmdClientInit (IpcmdClient *self, IpcmdCore *core)
{
	self->subscribed_notifications_ = NULL;
	self->operation_contexts_ = NULL;
	self->service_id_ = 0;
	self->channel_id_ = 0;
	self->seq_num_ = 0;
	self->core_ = core;
	self->listener_.OnChannelEvent = _ReceiveChannelEvent;
	IpcmdBusAddEventListener (IpcmdCoreGetBus(self->core_),&self->listener_);
}
void
IpcmdClientFinalize (IpcmdClient *self)
{
	IpcmdBusRemoveEventListener (IpcmdCoreGetBus(self->core_), &self->listener_);
	// IMPL: free self->subscribed_notifications_;
	// IMPL: free self->operation_contexts_;
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
		else if ( ((SubscribedNotification*)l->data)->service_id_ == 0 &&
				((SubscribedNotification*)l->data)->operation_id_ == 0) {	// receive all notification messages
			ret_list = g_list_append(ret_list,l->data);
		}
	}
	return ret_list;
}

static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	//IpcmdClient *client = IPCMD_CLIENT_FROM_LISTENER(self);

	// IMPL: whole function
	g_debug("Got channel event: id=%d, type=%d", id, type);
}
