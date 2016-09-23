/*
 * IpcmdOpStateMachineCLRequest.c
 *
 *  Created on: Sep 22, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOpStateMachine.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachineCommon.h"

static void	_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info);
static void	_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode);

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
	// IMPL: IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
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
	if (ctx->numberOfRetries > ctx->nWFAMaxRetries) {
		_SetFinalizeState (ctx, OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES);
		return kOpContextStateFinalize;
	}
	//retransmit message
	// IMPL: IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	//reset timer
	IpcmdOpCtxSetTimer(ctx, CalculateTimeoutInterval(ctx->nWFABaseTimeout, ctx->nWFAIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFAExpired);

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
		return -1;
	}
	// 2. fill IpcmdMessage with IpcmdOperationInfo
	IpcmdMessageInitVCCPDUHeader (message,
			ctx->serviceId, ctx->operationId, ctx->opctx_id_.sender_handle_id_, ctx->protoVersion, ctx->opType,
			op_info->payload_.type_, ctx->flags);
	IpcmdMessageCopyToPayloadBuffer (message, op_info->payload_.data_, op_info->payload_.length_);
	ctx->message = message;
	// 3. Send IpcmdMessage
	// IMPL: IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	// 4. Start Timer
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
	IpcmdMessage *message = IpcmdMessageRef((IpcmdMessage *)data);

	if (ctx->message) IpcmdMessageUnref(ctx->message);
	ctx->message = message;

	// 1. stop timer
	IpcmdOpCtxCancelTimer(ctx);
	// 2. change state
	op_state->state_ = kOpContextStateRespRecv;
	// 3. acknowledge to application
	{
		IpcmdOperationInfoReceivedMessage info;

		info.parent_.type_ = kOperationInfoReceivedMessage;
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_AckRecv_RecvResp)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = IpcmdMessageRef((IpcmdMessage *)data);

	if (ctx->message) IpcmdMessageUnref(ctx->message);
	ctx->message = message;

	// 1. stop timer
	IpcmdOpCtxCancelTimer(ctx);
	// 2. change state
	op_state->state_ = kOpContextStateRespRecv;
	// 3. acknowledge to application
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

DECLARE_SM_ENTRY(DoAction_CLREQUEST_Idle_RecvRequest)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = IpcmdMessageRef((IpcmdMessage *)data);

	ctx->message = message;

	op_state->state_ = kOpContextStateAppProcess;

	// 1. send ACK
	// IMPL:
	// 2. deliver to application
	{
		IpcmdOperationInfoReceivedMessage info;

		info.parent_.type_ = kOperationInfoReceivedMessage;
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}

	return op_state->state_;

	_Idle_RecvRequest_fail:

	return -1;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_AppProcess_RecvRequest)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	// 1. send ACK
	// IMPL:
	// 2. ignore, since we already processing it
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
		// send the IpcmdMessage
		// IMPL: IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
		break;
	default:
		g_error("Use kOperationInfoPayload when complete operation.");
		break;
	}

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLREQUEST_RespSent_RecvAck)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_NORMAL);

	return op_state->state_;
}



static void
_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info)
{
	if (ctx->deliver_to_app_.cb_func) {
		ctx->deliver_to_app_.cb_func ((OpHandle)ctx, info, ctx->deliver_to_app_.cb_data);
	}
}

static void
_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode)
{
	ctx->mOpState.state_ = kOpContextStateFinalize;
	ctx->mOpState.fin_code_ = fincode;

	if (ctx->notify_finalizing_.cb_func) {
		ctx->notify_finalizing_.cb_func (ctx->opctx_id_, ctx->notify_finalizing_.cb_data);
	}
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
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvResp] = DoAction_CLREQUEST_AckRecv_RecvResp;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerWFRTimeout] = DoAction_XXX_ANY_WFREXPIRED;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvAck] = DoAction_Ignore;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerCompletedAppProcess] = DoAction_CLREQUEST_AppProcess_CompletedAppProcess;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerRecvRequest] = DoAction_CLREQUEST_AppProcess_RecvRequest;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerRecvAck] = DoAction_CLREQUEST_RespSent_RecvAck;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerRecvRequest] = DoAction_Ignore;
	SM->actions[kOpContextStateRespSent][kIpcmdTriggerWFATimeout] = DoAction_XXX_ANY_WFAEXPIRED;
}
