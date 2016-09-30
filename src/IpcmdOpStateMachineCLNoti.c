/*
 * IpcmdOpStateMachineCLNoti.c
 *
 *  Created on: Sep 29, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperation.h"
#include "../include/IpcmdOpStateMachine.h"
#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachineCommon.h"
#include "../include/IpcmdCore.h"

static inline void	_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info);
static void	_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode);

DECLARE_SM_ENTRY(DoAction_CLNoti_Idle_SendNoti)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	const IpcmdOperationInfoInvokeMessage *op_info = data;
	IpcmdMessage *message;

	// 1. create Notification message
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
	op_state->state_ = kOpContextStateNotiSent;
	// 4. Send IpcmdMessage
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	// 5. Start WFA Timer
	IpcmdOpCtxStartWFATimer(ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNoti_NotiSent_RecvAck)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	// 1. stop timer
	IpcmdOpCtxCancelTimer (ctx);
	// 2. finalizing
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_NORMAL);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNoti_NotiSent_WFAExpired)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	//1.check that the number of retries exceeds maximum number.
	ctx->numberOfRetries++;
	if (ctx->numberOfRetries > ctx->nWFAMaxRetries) { //If so, deliver IpcmdOperationInfoFail and finalize
		IpcmdOperationInfoFail	info;
		IpcmdOperationInfoFailInit(&info);
		info.reason_ = OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES;
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		_SetFinalizeState (ctx, OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES);
		return op_state->state_;
	}
	//2.retransmit message
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	//3.reset timer
	IpcmdOpCtxSetTimer(ctx, CalculateTimeoutInterval(ctx->nWFABaseTimeout, ctx->nWFAIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFAExpired);

	return op_state->state_;
}

void IpcmdOpStateMachine_CLNoti_Init(IpcmdOpStateMachine *SM)
{
	guint ns,nt;

	for (ns=kOpContextStateStart; ns < kOpContextStateEnd; ns++) {
		for (nt=kIpcmdTriggerStart; nt < kIpcmdTriggerEnd; nt++) {
			SM->actions[ns][nt] = DoAction_NotDetermined;
		}
	}

	SM->actions[kOpContextStateIdle][kIpcmdTriggerSendNoti] = DoAction_CLNoti_Idle_SendNoti;
	SM->actions[kOpContextStateNotiSent][kIpcmdTriggerRecvAck] = DoAction_CLNoti_NotiSent_RecvAck;
	SM->actions[kOpContextStateNotiSent][kIpcmdTriggerWFATimeout] = DoAction_CLNoti_NotiSent_WFAExpired;
}

/* @fn : _SetFinalizeState
 * change operation context state to kOpContextStateFinalize.
 */
static void
_SetFinalizeState (IpcmdOpCtx *ctx, IpcmdOpCtxFinCode fincode)
{
	// call notify_finalizing
	ctx->mOpState.state_ = kOpContextStateFinalize;
	ctx->mOpState.fin_code_ = fincode;

	if (ctx->notify_finalizing_.cb_func) {
		ctx->notify_finalizing_.cb_func (ctx->opctx_id_, ctx->notify_finalizing_.cb_data);
	}
}

static void
_DeliverToApplication (IpcmdOpCtx *ctx, const IpcmdOperationInfo *info)
{
	if (ctx->deliver_to_app_.cb_func) {
		ctx->deliver_to_app_.cb_func ((OpHandle)ctx, info, ctx->deliver_to_app_.cb_data);
	}
}
