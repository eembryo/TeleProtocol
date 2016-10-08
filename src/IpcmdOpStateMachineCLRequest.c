/*
 * IpcmdOpStateMachineCLRequest.c
 *
 *  Created on: Sep 22, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperation.h"
#include "../include/IpcmdOpStateMachine.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachineCommon.h"
#include "../include/IpcmdCore.h"

static void	_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info);
static void	_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode);
static inline IpcmdMessage*	_GenerateAckMessage (IpcmdOpCtx *ctx);
static inline IpcmdMessage*	_GenerateErrorMessage (IpcmdOpCtx *ctx, guint8 ecode, guint16 einfo);
static inline gboolean _IsDeliverableState (enum _OpContextStates state);

/* *******************************
 * Common Actions
 * *******************************/

/*******************************************
 * WFA timer is expired. Retransmit a message or go to FINALIZE state.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
DECLARE_SM_ENTRY(DoAction_XXX_ANY_WFAEXPIRED)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	//state should be one of OPCONTEXT_STATUS_REQUEST_SENT, OPCONTEXT_STATUS_RESPONSE_SENT or OPCONTEXT_STATUS_NOTIFICATION_SENT
	ctx->numberOfRetries++;
	if (ctx->numberOfRetries > ctx->nWFAMaxRetries) {
		_SetFinalizeState (ctx, OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES);
		return op_state->state_;
	}
	//retransmit message
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	//reset timer
	IpcmdOpCtxSetTimer(ctx, CalculateTimeoutInterval(ctx->nWFABaseTimeout, ctx->nWFAIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFAExpired);

	return op_state->state_;
}

/*******************************************
 * WFR timer is expired. Retransmit a message or go to FINALIZE state.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
DECLARE_SM_ENTRY(DoAction_XXX_ANY_WFREXPIRED)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	ctx->numberOfRetries++;
	if (ctx->numberOfRetries > ctx->nWFRMaxRetries) {
		_SetFinalizeState (ctx, OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES);
		return kOpContextStateFinalize;
	}
	//retransmit message
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	//reset timer
	IpcmdOpCtxSetTimer(ctx, CalculateTimeoutInterval(ctx->nWFRBaseTimeout, ctx->nWFRIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFRExpired);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_XXX_ANY_RecvError)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;
	struct _ErrorPayload *err_payload = (struct _ErrorPayload*)IpcmdMessageGetPayload(message);

	/* Processing - REQPROD 381391, REQPROD 346841
	 * The client receiving an ERROR message with error code processing will then know
	 * that the original message still is being handled and valid.
	 */
	if (err_payload->errorCode == IPCOM_MESSAGE_ECODE_PROCESSING) {
		// we handle this error message as if we got ACK message
		return ctx->mOpState.pSM->actions[ctx->mOpState.state_][kIpcmdTriggerRecvAck] (op_state, kIpcmdTriggerRecvAck, 0);
	}

	/* Handling normal error messages */
	// 1. stop WFX timer if exists
	IpcmdOpCtxCancelTimer (ctx);
	// 2. send ACK
	{
		IpcmdMessage *instant_message = _GenerateAckMessage(ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}
	// 3. deliver to application only if current state is kOpContextStateReqSent or kOpContextStateAckRecv.
	if (_IsDeliverableState(op_state->state_)) {
		IpcmdOperationInfoReceivedMessage info;

		// 3.1 change state to kOpContextStateRespRecv
		op_state->state_ = kOpContextStateRespRecv;
		// 3.2 deliver to application
		IpcmdOperationInfoReceivedMessageInit (&info);
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}
	// 4. finalize operation
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_RECEIVE_ERROR_MESSAGE);
	return op_state->state_;
}
/*******************************************
 * REQUEST Type State Machine
 * @ op_state :
 * @ trigger :
 * @ data : expecting IpcmdOperationInfoInvokeMessage*
 *
 *******************************************/
DECLARE_SM_ENTRY(DoAction_CLREQUEST_Idle_SendRequest)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	const IpcmdOperationInfoInvokeMessage *op_info = data;
	IpcmdMessage *message;

	// 1. create IpcmdMessage
	message = IpcmdMessageNew(VCCPDUHEADER_SIZE + op_info->payload_.length_);
	if (!message) { //not enough memory
		g_warning("Not enough memory");
		return -1;
	}
	// 2. fill IpcmdMessage with IpcmdOperationInfo
	IpcmdMessageInitVCCPDUHeader (message,
			ctx->serviceId, ctx->operationId, ctx->opctx_id_.sender_handle_id_, ctx->protoVersion, ctx->opType,
			op_info->payload_.type_, ctx->flags);
	IpcmdMessageCopyToPayloadBuffer (message, op_info->payload_.data_, op_info->payload_.length_);
	IpcmdOpCtxSetMessage (ctx, message);
	IpcmdMessageUnref(message);
	// 3. change state
	op_state->state_ = kOpContextStateReqSent;
	// 4. Send IpcmdMessage
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	// 5. Start Timer
	IpcmdOpCtxStartWFATimer(ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_ReqSent_RecvAck)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	// 1. stop WFA timer
	IpcmdOpCtxCancelTimer(ctx);
	// 2. change state
	op_state->state_ = kOpContextStateAckRecv;
	// 3. start WFR timer
	IpcmdOpCtxStartWFRTimer(ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_ReqSent_RecvResp)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;

	IpcmdOpCtxSetMessage(ctx, message);

	// 1. stop timer
	IpcmdOpCtxCancelTimer(ctx);
	// 2. change state
	op_state->state_ = kOpContextStateRespRecv;
	// 3. send ACK
	{
		IpcmdMessage *instant_message = _GenerateAckMessage(ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}
	// 4. acknowledge to application
	{
		IpcmdOperationInfoReceivedMessage info;

		info.parent_.type_ = kOperationInfoReceivedMessage;
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}
	// 4. finalize
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_NORMAL);
	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_AckRecv_RecvResp)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;

	IpcmdOpCtxSetMessage(ctx, message);

	// 1. stop timer
	IpcmdOpCtxCancelTimer(ctx);
	// 2. change state
	op_state->state_ = kOpContextStateRespRecv;
	// 3. send ACK
	{
		IpcmdMessage *instant_message = _GenerateAckMessage(ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}
	// 4. acknowledge to application
	{
		IpcmdOperationInfoReceivedMessage info;

		info.parent_.type_ = kOperationInfoReceivedMessage;
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}
	// 4. finalize this operation
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_NORMAL);

	return op_state->state_;
}
#define REPLY_ERROR(core, channel_id, ecode, einfo) do {\
		IpcmdMessage *error_message = IpcmdMessageNew(IPCMD_ERROR_MESSAGE_SIZE); \
		IpcmdMessageInitVCCPDUHeader (error_message, IpcmdMessageGetVCCPDUServiceID(mesg), \
				IpcmdMessageGetVCCPDUOperationID(mesg), IpcmdMessageGetVCCPDUSenderHandleID(mesg), \
				IpcmdMessageGetVCCPDUProtoVersion(mesg), IPCMD_OPTYPE_ERROR, IPCMD_PAYLOAD_NOTENCODED, 0); \
		IpcmdMessageSetErrorPayload (error_message, ecode, einfo); \
		IpcmdCoreTransmit (core, channel_id, error_message); \
		IpcmdMessageUnref(error_message);\
}while(0)

DECLARE_SM_ENTRY(DoAction_CLREQUEST_Idle_RecvRequest)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;
	IpcmdMessage *instant_message;

	// 1. change state to kOpContextStateReqRecv
	op_state->state_ = kOpContextStateReqRecv;
	IpcmdOpCtxSetMessage(ctx, message);

	// 2. send ACK
	instant_message = _GenerateAckMessage(ctx);
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
	IpcmdMessageUnref(instant_message);

	// 3. change state to kOpContextStateAppProcess
	op_state->state_ = kOpContextStateAppProcess;
	// 4. deliver to application
	{
		IpcmdOperationInfoReceivedMessage info;

		IpcmdOperationInfoReceivedMessageInit(&info);
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}

	return op_state->state_;

	/*
	_Idle_RecvRequest_fail:

	return -1;
	*/
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_AppProcess_RecvRequest)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *instant_message;

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
	if (ctx->flags & IPCMD_MESSAGE_FLAGS_PROC) {
		// 1. send ERROR
		instant_message = _GenerateErrorMessage (ctx, IPCOM_MESSAGE_ECODE_PROCESSING, 0);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref (instant_message);
	}
	else {
		// 1. send ACK
		instant_message = _GenerateAckMessage (ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_AppProcess_CompletedAppProcess)
{
	IpcmdOpCtx 					*ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	const IpcmdOperationInfoReplyMessage	*op_info = data;	//we are expecting IpcmdOperationInfoReplyMessage from (gpointer)data
	IpcmdMessage 				*message;

	switch (op_info->parent_.type_) {
	case kOperationInfoReplyMessage:
		message = IpcmdMessageNew(VCCPDUHEADER_SIZE + op_info->payload_.length_);
		g_assert(message);

		// fill IpcmdMessage with IpcmdOperationInfo
		IpcmdMessageInitVCCPDUHeader (message,
				ctx->serviceId, ctx->operationId, ctx->opctx_id_.sender_handle_id_, ctx->protoVersion, op_info->op_type_,
				op_info->payload_.type_, 0x0);
		IpcmdMessageCopyToPayloadBuffer (message, op_info->payload_.data_,op_info->payload_.length_);
		IpcmdOpCtxSetMessage (ctx, message);
		IpcmdMessageUnref (message);
		break;
	default:
		g_warning("Use kOperationInfoPayload when complete operation. sending NOT_OK error message.");
		// send Error message
		message = _GenerateErrorMessage (ctx, IPCOM_MESSAGE_ECODE_NOT_OK, 0);
		IpcmdOpCtxSetMessage (ctx, message);
		IpcmdMessageUnref (message);
		break;
	}

	// change state to kOpContextStateRespSent
	ctx->mOpState.state_ = kOpContextStateRespSent;
	// send the IpcmdMessage
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, ctx->message);
	// start WFA timer
	IpcmdOpCtxStartWFATimer (ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_RespSent_RecvAck)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	// 1. stop timer
	IpcmdOpCtxCancelTimer (ctx);
	// 2. finalizing
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_NORMAL);

	return op_state->state_;
}


void
IpcmdOpStateMachine_CLREQUEST_Init(IpcmdOpStateMachine *SM)
{
	guint ns,nt;

	for (ns=kOpContextStateStart; ns < kOpContextStateEnd; ns++) {
		for (nt=kIpcmdTriggerStart; nt < kIpcmdTriggerEnd; nt++) {
			SM->actions[ns][nt] = DoAction_NotDetermined;
		}
	}

	SM->actions[kOpContextStateIdle][kIpcmdTriggerSendRequest] = DoAction_CLREQUEST_Idle_SendRequest;
	SM->actions[kOpContextStateIdle][kIpcmdTriggerRecvRequest] = DoAction_CLREQUEST_Idle_RecvRequest;
	SM->actions[kOpContextStateReqSent][kIpcmdTriggerRecvAck] = DoAction_CLREQUEST_ReqSent_RecvAck;
	SM->actions[kOpContextStateReqSent][kIpcmdTriggerRecvResp] = DoAction_CLREQUEST_ReqSent_RecvResp;
	SM->actions[kOpContextStateReqSent][kIpcmdTriggerWFATimeout] = DoAction_XXX_ANY_WFAEXPIRED;
	SM->actions[kOpContextStateReqSent][kIpcmdTriggerRecvError] = DoAction_XXX_ANY_RecvError;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvResp] = DoAction_CLREQUEST_AckRecv_RecvResp;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerWFRTimeout] = DoAction_XXX_ANY_WFREXPIRED;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvAck] = DoAction_Ignore;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvError] = DoAction_XXX_ANY_RecvError;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerCompletedAppProcess] = DoAction_CLREQUEST_AppProcess_CompletedAppProcess;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerRecvRequest] = DoAction_CLREQUEST_AppProcess_RecvRequest;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerRecvAck] = DoAction_CLREQUEST_RespSent_RecvAck;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerRecvRequest] = DoAction_Ignore;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerWFATimeout] = DoAction_XXX_ANY_WFAEXPIRED;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerRecvError] = DoAction_XXX_ANY_RecvError;
}

/*********************
 * static functions
 *********************/
static void
_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info)
{
	if (ctx->deliver_to_app_.cb_func) {
		ctx->deliver_to_app_.cb_func ((OpHandle)ctx, info, ctx->deliver_to_app_.cb_data);
	}
}

/* @fn : _SetFinalizeState
 * change operation context state to kOpContextStateFinalize. In case that current state is
 * one of kOpContextStateReqSent or kOpContextStateAckRecv, this function will inform to application
 * with finalizing reason.
 */
static void
_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode)
{
	union {
		IpcmdOperationInfoFail	fail_;
		IpcmdOperationInfoOk	ok_;
	} ret_info;

	// if state is one of kOpContextStateReqSent or kOpContextStateAckRecv, deliver to application
	switch (ctx->mOpState.state_) {
	case kOpContextStateReqSent:
	case kOpContextStateAckRecv:
		if (fincode) {
			ret_info.fail_.parent_.type_ = kOperationInfoFail;
			ret_info.fail_.reason_ = fincode;
		}
		else {
			ret_info.ok_.parent_.type_ = kOperationInfoOk;
		}
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&ret_info);
		break;
	default:
		break;
	}

	// call notify_finalizing
	ctx->mOpState.state_ = kOpContextStateFinalize;
	ctx->mOpState.fin_code_ = fincode;

	if (ctx->notify_finalizing_.cb_func) {
		ctx->notify_finalizing_.cb_func (ctx->opctx_id_, ctx->notify_finalizing_.cb_data);
	}
}

static IpcmdMessage*
_GenerateAckMessage (IpcmdOpCtx *ctx)
{
	IpcmdMessage *ack_message = IpcmdMessageNew(IPCMD_ACK_MESSAGE_SIZE);

	IpcmdMessageInitVCCPDUHeader(ack_message, ctx->serviceId, ctx->operationId, ctx->opctx_id_.sender_handle_id_, IPCMD_PROTOCOL_VERSION, IPCMD_OPTYPE_ACK, IPCMD_PAYLOAD_NOTENCODED, 0);
	return ack_message;
}

static IpcmdMessage*
_GenerateErrorMessage (IpcmdOpCtx *ctx, guint8 ecode, guint16 einfo)
{
	IpcmdMessage *error_message = IpcmdMessageNew(IPCMD_ERROR_MESSAGE_SIZE);
	IpcmdMessageInitVCCPDUHeader (error_message, ctx->serviceId, ctx->operationId, ctx->opctx_id_.sender_handle_id_, IPCMD_PROTOCOL_VERSION, IPCMD_OPTYPE_ERROR, IPCMD_PAYLOAD_NOTENCODED, 0);
	IpcmdMessageSetErrorPayload (error_message, ecode, einfo);
	return error_message;
}

static gboolean _IsDeliverableState (enum _OpContextStates state) {
	return state==kOpContextStateReqSent || state==kOpContextStateAckRecv ? TRUE : FALSE;
}
