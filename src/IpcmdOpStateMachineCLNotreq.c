/*
 * IpcmdOpStateMachineCLNotreq.c
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
static inline IpcmdMessage*	_GenerateAckMessage (IpcmdOpCtx *ctx);
static inline IpcmdMessage*	_GenerateErrorMessage (IpcmdOpCtx *ctx, guint8 ecode, guint16 einfo);

DECLARE_SM_ENTRY(DoAction_CLNotreq_Idle_SendNotreq)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	const IpcmdOperationInfoInvokeMessage *op_info = data;
	IpcmdMessage *message;

	// 1. create NOTIFICATION_REQUEST message
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
	op_state->state_ = kOpContextStateNotreqSent;
	// 4. Send IpcmdMessage
	IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_ , ctx->message);
	// 5. Start WFA Timer
	IpcmdOpCtxStartWFATimer(ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNotreq_Idle_RecvNotreq)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;

	// 1. change state to kOpContextStateNotreqRecv
	op_state->state_ = kOpContextStateNotreqRecv;

	IpcmdOpCtxSetMessage(ctx, message);

	// 2. send ACK
	{
		IpcmdMessage *instant_message;
		instant_message = _GenerateAckMessage(ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}

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
}

DECLARE_SM_ENTRY(DoAction_CLNotreq_NotreqSent_RecvAck)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	// 1. stop WFA timer
	IpcmdOpCtxCancelTimer (ctx);
	// 2. change state
	op_state->state_ = kOpContextStateAckRecv;
	// 3. start WFR timer
	IpcmdOpCtxStartWFRTimer(ctx);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNotreq_Any_RecvError)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	IpcmdMessage *message = (IpcmdMessage *)data;
	struct _ErrorPayload *err_payload = (struct _ErrorPayload*)IpcmdMessageGetPayload(message);

	/* REQPROD 381652/MAIN;0 : Process-flag, proc
	 * If a message of a type other than Request or SetRequest in combination with
	 * proc-flag set to 0x1 is received the server shall continue processing the message
	 * as if the proc-flag was set 0.
	 */
	if (err_payload->errorCode == IPCOM_MESSAGE_ECODE_PROCESSING) {
		// This should not be happened. we ignore this message.
		return op_state->state_;
	}

	//1.Stop WFX Timer
	IpcmdOpCtxCancelTimer (ctx);
	//2.Deliver ERROR message to application.
	{
		IpcmdOperationInfoReceivedMessage info;
		IpcmdOperationInfoReceivedMessageInit(&info);
		info.raw_message_ = IpcmdMessageRef(message);
		info.sender_ = NULL; //IMPL: find sender
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
		IpcmdMessageUnref(message);
	}
	//3.Finalize
	_SetFinalizeState (ctx, OPCONTEXT_FINCODE_RECEIVE_ERROR_MESSAGE);

	return op_state->state_;
}
DECLARE_SM_ENTRY(DoAction_CLNotreq_NotreqSent_WFAExpired)
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

/* REQPROD 346917/MAIN;2 : OperationType Notification_Reqeust using UDP
 * Hyobeom: In the figure, Client waits for ERROR response after it receives ACK message. It means that we should wait
 * at least a WFR timeout to ensure success of NOTIFICATION_REQUEST operation.
 */
DECLARE_SM_ENTRY(DoAction_CLNotreq_AckRecv_WFRExpired)
{
	IpcmdOpCtx *ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	//1.Deliver OK to application
	{
		IpcmdOperationInfoOk info;
		IpcmdOperationInfoOkInit(&info);
		_DeliverToApplication (ctx, (IpcmdOperationInfo*)&info);
	}
	//2.Finalize
	_SetFinalizeState(ctx, OPCONTEXT_FINCODE_NORMAL);

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNotreq_AppProcess_CompleteAppProcess)
{
	IpcmdOpCtx 					*ctx = container_of(op_state, IpcmdOpCtx, mOpState);
	const IpcmdOperationInfo	*op_info = (IpcmdOperationInfo*)data;	//we are expecting IpcmdOperationInfoOk or IpcmdOperationInfoReplyMessage
	IpcmdMessage 				*error_message = NULL;

	//1.if Application return with OK, finalize
	if (op_info->type_ == kOperationInfoOk) {
		_SetFinalizeState(ctx, OPCONTEXT_FINCODE_NORMAL);
		return op_state->state_;
	}
	//2.if Application return with ReplyMessage(ERROR), send the message and change state to 'kOpContextStateErrorSent'
	else if(op_info->type_ == kOperationInfoReplyMessage && ((IpcmdOperationInfoReplyMessage*)op_info)->op_type_ == IPCMD_OPTYPE_ERROR) {
		//2.1 create ERROR message
		struct _ErrorPayload *err_payload = (struct _ErrorPayload*)((IpcmdOperationInfoReplyMessage*)op_info)->payload_.data_;
		error_message = _GenerateErrorMessage(ctx, err_payload->errorCode, g_ntohs(err_payload->errorInformation));
	}
	//3.otherwise, send NOT_OK ERROR and change state to 'kOpContextStateErrorSent'
	else {
		//3.1 create ERROR message
		error_message = _GenerateErrorMessage(ctx, IPCOM_MESSAGE_ECODE_NOT_OK, 0);
	}

	//4. if there is ERROR message to send, send it, change state to kOpContextStateErrorSent and finalize
	if (error_message) {
		IpcmdOpCtxSetMessage(ctx, error_message);
		//4.1 send ERROR message
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, error_message);
		IpcmdMessageUnref(error_message);
		//4.2 change state to kOpContextStateErrorSent
		op_state->state_ = kOpContextStateErrorSent;
		_SetFinalizeState(ctx, OPCONTEXT_FINCODE_NORMAL);
	}

	return op_state->state_;
}

DECLARE_SM_ENTRY(DoAction_CLNotreq_AppProcess_RecvNotreq)
{
	IpcmdOpCtx 					*ctx = container_of(op_state, IpcmdOpCtx, mOpState);

	//Send ACK message
	{
		IpcmdMessage *instant_message;
		instant_message = _GenerateAckMessage(ctx);
		IpcmdCoreTransmit (ctx->core_, ctx->opctx_id_.channel_id_, instant_message);
		IpcmdMessageUnref(instant_message);
	}
	return op_state->state_;
}

void
IpcmdOpStateMachine_CLNotreq_Init(IpcmdOpStateMachine *SM)
{
	guint ns,nt;

	for (ns=kOpContextStateStart; ns < kOpContextStateEnd; ns++) {
		for (nt=kIpcmdTriggerStart; nt < kIpcmdTriggerEnd; nt++) {
			SM->actions[ns][nt] = DoAction_NotDetermined;
		}
	}
	SM->actions[kOpContextStateIdle][kIpcmdTriggerSendNotreq] = DoAction_CLNotreq_Idle_SendNotreq;
	SM->actions[kOpContextStateIdle][kIpcmdTriggerRecvNotreq] = DoAction_CLNotreq_Idle_RecvNotreq;
	SM->actions[kOpContextStateNotreqSent][kIpcmdTriggerRecvAck] = DoAction_CLNotreq_NotreqSent_RecvAck;
	SM->actions[kOpContextStateNotreqSent][kIpcmdTriggerRecvError] = DoAction_CLNotreq_Any_RecvError;
	SM->actions[kOpContextStateNotreqSent][kIpcmdTriggerWFATimeout] = DoAction_CLNotreq_NotreqSent_WFAExpired;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvError] = DoAction_CLNotreq_Any_RecvError;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerWFRTimeout] = DoAction_CLNotreq_AckRecv_WFRExpired;
	SM->actions[kOpContextStateAckRecv][kIpcmdTriggerRecvAck] = DoAction_Ignore;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerCompletedAppProcess] = DoAction_CLNotreq_AppProcess_CompleteAppProcess;
	SM->actions[kOpContextStateAppProcess][kIpcmdTriggerRecvNotreq] = DoAction_CLNotreq_AppProcess_RecvNotreq;
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
