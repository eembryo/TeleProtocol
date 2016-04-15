#include <glib.h>
#include <IpcomOperationContext.h>
#include <IpcomProtocol.h>
#include <IpcomOpStateMachine.h>
#include <IpcomConnection.h>
#include <ref_count.h>
#include <dprint.h>

#define DECLARE_SM_ENTRY(fn_name) \
	static int fn_name(IpcomOpState *pOpState, IpcomProtocolOpContextTriggers trigger, gpointer data)

struct _IpcomOpStateMachine SM_CLRO;	//Request Only / connectionless
struct _IpcomOpStateMachine SM_CLRR;	//Request and Response / connectionless
struct _IpcomOpStateMachine SM_CLNoti;	//Notification / connectionless

static gint DoAction_NotDetermined(IpcomOpState *pOpState, IpcomProtocolOpContextTriggers trigger, gpointer data);
static gint DoAction_Ignore(IpcomOpState *pOpState, IpcomProtocolOpContextTriggers trigger, gpointer data);

/* *******************************
 * Common Actions
 *  *******************************/
DECLARE_SM_ENTRY(DoAction_NotDetermined)
{
	return -1;
}
DECLARE_SM_ENTRY(DoAction_Ignore)
{
	//silently ignore
	return pOpState->nState;
}

DECLARE_SM_ENTRY(DoAction_XXX_ANY_WFAEXPIRED)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	//state should be one of OPCONTEXT_STATUS_REQUEST_SENT, OPCONTEXT_STATUS_RESPONSE_SENT or OPCONTEXT_STATUS_NOTIFICATION_SENT
	switch(pOpState->nState) {
	case OPCONTEXT_STATUS_REQUEST_SENT:
	case OPCONTEXT_STATUS_RESPONSE_SENT:
	case OPCONTEXT_STATUS_NOTIFICATION_SENT:
		ctx->numberOfRetries++;
		if (ctx->numberOfRetries > ctx->nWFAMaxRetries) {
			pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
			pOpState->nFinCode = OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES;
		}
		//retransmit message
		g_assert(ctx->message);
		IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), ctx->message);
		//reset timer
		IpcomOpContextSetTimer(ctx, CalculateUsedTimeout(ctx->nWFABaseTimeout, ctx->nWFAIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFAExpired);
		break;
	default:
		DWARN("Trigger(OPCONTEXT_TRIGGER_WFA_EXPIRED) is not valid in state(%s).", OpContextStatusString[pOpState->nState]);
		return -1;
	}

	return pOpState->nState;
}

DECLARE_SM_ENTRY(DoAction_XXX_ANY_WFREXPIRED)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	//state should be one of OPCONTEXT_STATUS_REQUEST_SENT, OPCONTEXT_STATUS_RESPONSE_SENT or OPCONTEXT_STATUS_NOTIFICATION_SENT
	switch(pOpState->nState) {
	case OPCONTEXT_STATUS_REQUEST_SENT:		//in connection-oriented
	case OPCONTEXT_STATUS_ACK_RECV:			//in connection-less
		ctx->numberOfRetries++;
		if (ctx->numberOfRetries > ctx->nWFRMaxRetries) {
			pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
			pOpState->nFinCode = OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES;
		}
		//retransmit message
		g_assert(ctx->message);
		IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), ctx->message);
		//reset timer
		IpcomOpContextSetTimer(ctx, CalculateUsedTimeout(ctx->nWFRBaseTimeout, ctx->nWFRIncreaseTimeout, ctx->numberOfRetries), ctx->OnWFRExpired);
		break;
	default:
		DWARN("Trigger(OPCONTEXT_TRIGGER_WFR_EXPIRED) is not valid in state(%s).", OpContextStatusString[pOpState->nState]);
		return -1;
	}

	return pOpState->nState;
}

DECLARE_SM_ENTRY(DoAction_XXX_ANY_FINALIZE)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);
	gint fincode = GPOINTER_TO_INT(data);

	if (pOpState->nState == OPCONTEXT_STATUS_FINALIZE)
		return OPCONTEXT_STATUS_FINALIZE;

	/// stop timer
	IpcomOpContextCancelTimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = fincode;

	return pOpState->nState;
}

/***********************************************************
 * Initialize connectionless Request Only State machine
 ************************************************************/
DECLARE_SM_ENTRY(DoAction_CLRO_NONE_SENDREQUEST)
{
	IpcomOpContext* ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);
	IpcomMessage* pMessage = data;

	if (IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), pMessage) < 0) {
		return -1;
	}
	IpcomOpContextSetMessage(ctx, pMessage);

	//start WFA timer
	IpcomOpContextStartWFATimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_REQUEST_SENT;

	return pOpState->nState;
}

DECLARE_SM_ENTRY(DoAction_CLRO_NONE_RECVREQUEST)
{
	pOpState->nState = OPCONTEXT_STATUS_PROCESS_REQUEST;
	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRO_REQUESTSENT_RECVACK)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	//stop WFA timer
	IpcomOpContextCancelTimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = OPCONTEXT_FINCODE_NORMAL;

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRO_PROCESSREQUEST_PROCESSDONE)
{
	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = OPCONTEXT_FINCODE_NORMAL;

	return pOpState->nState;
}
static void IpcomOpStateMachine_CLRO_Init() {
	SM_CLRO.actions[OPCONTEXT_STATUS_NONE][OPCONTEXT_TRIGGER_SEND_REQUEST] = DoAction_CLRO_NONE_SENDREQUEST;
	SM_CLRO.actions[OPCONTEXT_STATUS_NONE][OPCONTEXT_TRIGGER_RECV_REQUEST] = DoAction_CLRO_NONE_RECVREQUEST;
	SM_CLRO.actions[OPCONTEXT_STATUS_REQUEST_SENT][OPCONTEXT_TRIGGER_RECV_ACK] = DoAction_CLRO_REQUESTSENT_RECVACK;
	SM_CLRO.actions[OPCONTEXT_STATUS_PROCESS_REQUEST][OPCONTEXT_TRIGGER_PROCESS_DONE] = DoAction_CLRO_PROCESSREQUEST_PROCESSDONE;
}

/***********************************************************
 * Initialize connectionless Notification State machine
 ************************************************************/
DECLARE_SM_ENTRY(DoAction_CLRO_NONE_SENDNOTIFICATION)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);
	IpcomMessage* pMessage = data;

	if (IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), pMessage) < 0)
		return -1;
	IpcomOpContextSetMessage(ctx, pMessage);

	//start WFA timer
	IpcomOpContextStartWFATimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_NOTIFICATION_SENT;

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRO_NOTIFICATIONSENT_RECVACK)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	//stop WFA timer
	IpcomOpContextCancelTimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = OPCONTEXT_FINCODE_NORMAL;

	return pOpState->nState;
}

static void IpcomOpStateMachine_CLNoti_Init() {
	SM_CLNoti.actions[OPCONTEXT_STATUS_NONE][OPCONTEXT_TRIGGER_SEND_NOTIFICATION] = DoAction_CLRO_NONE_SENDNOTIFICATION;
	SM_CLNoti.actions[OPCONTEXT_STATUS_NOTIFICATION_SENT][OPCONTEXT_TRIGGER_RECV_ACK] = DoAction_CLRO_NOTIFICATIONSENT_RECVACK;
}
/***********************************************************
 * Initialize connectionless Request Response State machine
 ************************************************************/
DECLARE_SM_ENTRY(DoAction_CLRR_NONE_SENDREQUEST)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);
	IpcomMessage* pMessage = data;

	if (IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), pMessage) < 0)
		return -1;
	IpcomOpContextSetMessage(ctx, pMessage);

	pOpState->nState = OPCONTEXT_STATUS_REQUEST_SENT;

	//start WFA timer
	IpcomOpContextStartWFATimer(ctx);

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_NONE_RECVREQUEST)
{
	pOpState->nState = OPCONTEXT_STATUS_PROCESS_REQUEST;

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_REQUESTSENT_RECVACK)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	pOpState->nState = OPCONTEXT_STATUS_ACK_RECV;

	//stop WFA timer
	IpcomOpContextCancelTimer(ctx);

	//start WFR timer
	IpcomOpContextStartWFRTimer(ctx);

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_REQUESTSENT_RECVRESPONSE)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	pOpState->nState = OPCONTEXT_STATUS_PROCESS_RESPONSE;

	//stop WFA timer
	IpcomOpContextCancelTimer(ctx);

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_ACKRECV_RECVRESPONSE)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	pOpState->nState = OPCONTEXT_STATUS_PROCESS_RESPONSE;

	//stop WFR timer
	IpcomOpContextCancelTimer(ctx);

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_PROCESSREQUEST_PROCESSDONE)
{
	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = OPCONTEXT_FINCODE_NORMAL;

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_PROCESSREQUEST_SENDRESPONSE)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);
	IpcomMessage* pMessage = data;

	if (IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), pMessage) < 0) {
		return -1;
	}
	IpcomOpContextSetMessage(ctx, pMessage);

	pOpState->nState = OPCONTEXT_STATUS_RESPONSE_SENT;

	//start WFA timer
	IpcomOpContextStartWFATimer(ctx);

	return pOpState->nState;
}
DECLARE_SM_ENTRY(DoAction_CLRR_RESPONSESENT_RECVACK)
{
	IpcomOpContext *ctx = container_of(pOpState, struct _IpcomOpContext, mOpState);

	//stop WFA timer
	IpcomOpContextCancelTimer(ctx);

	pOpState->nState = OPCONTEXT_STATUS_FINALIZE;
	pOpState->nFinCode = OPCONTEXT_FINCODE_NORMAL;

	return pOpState->nState;
}

static void
IpcomOpStateMachine_CLRR_Init()
{
	int i;

	for (i=0; i< OPCONTEXT_STATUS_END; i++) {
		SM_CLRR.actions[i][OPCONTEXT_TRIGGER_FINALIZE] = DoAction_XXX_ANY_FINALIZE;
	}

	SM_CLRR.actions[OPCONTEXT_STATUS_NONE][OPCONTEXT_TRIGGER_SEND_REQUEST] = DoAction_CLRR_NONE_SENDREQUEST;
	SM_CLRR.actions[OPCONTEXT_STATUS_NONE][OPCONTEXT_TRIGGER_RECV_REQUEST] = DoAction_CLRR_NONE_RECVREQUEST;
	SM_CLRR.actions[OPCONTEXT_STATUS_REQUEST_SENT][OPCONTEXT_TRIGGER_RECV_ACK] = DoAction_CLRR_REQUESTSENT_RECVACK;
	SM_CLRR.actions[OPCONTEXT_STATUS_REQUEST_SENT][OPCONTEXT_TRIGGER_RECV_RESPONSE] = DoAction_CLRR_REQUESTSENT_RECVRESPONSE;
	SM_CLRR.actions[OPCONTEXT_STATUS_REQUEST_SENT][OPCONTEXT_TRIGGER_WFA_EXPIRED] = DoAction_XXX_ANY_WFAEXPIRED;
	SM_CLRR.actions[OPCONTEXT_STATUS_ACK_RECV][OPCONTEXT_TRIGGER_RECV_RESPONSE] = DoAction_CLRR_ACKRECV_RECVRESPONSE;
	SM_CLRR.actions[OPCONTEXT_STATUS_ACK_RECV][OPCONTEXT_TRIGGER_WFR_EXPIRED] = DoAction_XXX_ANY_WFREXPIRED;
	SM_CLRR.actions[OPCONTEXT_STATUS_ACK_RECV][OPCONTEXT_TRIGGER_RECV_ACK] = DoAction_Ignore;
	SM_CLRR.actions[OPCONTEXT_STATUS_PROCESS_RESPONSE][OPCONTEXT_TRIGGER_PROCESS_DONE] = DoAction_CLRR_PROCESSREQUEST_PROCESSDONE;
	SM_CLRR.actions[OPCONTEXT_STATUS_PROCESS_REQUEST][OPCONTEXT_TRIGGER_SEND_RESPONSE] = DoAction_CLRR_PROCESSREQUEST_SENDRESPONSE;
	SM_CLRR.actions[OPCONTEXT_STATUS_PROCESS_REQUEST][OPCONTEXT_TRIGGER_RECV_REQUEST] = DoAction_Ignore;
	SM_CLRR.actions[OPCONTEXT_STATUS_PROCESS_REQUEST][OPCONTEXT_TRIGGER_PROCESS_DONE] = DoAction_Ignore;
	SM_CLRR.actions[OPCONTEXT_STATUS_RESPONSE_SENT][OPCONTEXT_TRIGGER_RECV_ACK] = DoAction_CLRR_RESPONSESENT_RECVACK;
	SM_CLRR.actions[OPCONTEXT_STATUS_RESPONSE_SENT][OPCONTEXT_TRIGGER_RECV_REQUEST] = DoAction_Ignore;
	SM_CLRR.actions[OPCONTEXT_STATUS_RESPONSE_SENT][OPCONTEXT_TRIGGER_PROCESS_DONE] = DoAction_Ignore;
	SM_CLRR.actions[OPCONTEXT_STATUS_RESPONSE_SENT][OPCONTEXT_TRIGGER_WFA_EXPIRED] = DoAction_XXX_ANY_WFAEXPIRED;
}


void IpcomOpStateMachineInit()
{
	int ns, nt;

	for (ns=OPCONTEXT_STATUS_START; ns < OPCONTEXT_STATUS_END; ns++) {
		for (nt=OPCONTEXT_TRIGGER_START; nt < OPCONTEXT_TRIGGER_END; nt++) {
			SM_CLRO.actions[ns][nt] = DoAction_NotDetermined;
			SM_CLRR.actions[ns][nt] = DoAction_NotDetermined;
			SM_CLNoti.actions[ns][nt] = DoAction_NotDetermined;
		}
	}

	/// In each state, FINALIZE trigger is handled by DoAction_XXX_ANY_FINALIZE
	for (ns=OPCONTEXT_STATUS_START; ns < OPCONTEXT_STATUS_END; ns++) {
		SM_CLRO.actions[ns][OPCONTEXT_TRIGGER_FINALIZE] = DoAction_XXX_ANY_FINALIZE;
		SM_CLRR.actions[ns][OPCONTEXT_TRIGGER_FINALIZE] = DoAction_XXX_ANY_FINALIZE;
		SM_CLNoti.actions[ns][OPCONTEXT_TRIGGER_FINALIZE] = DoAction_XXX_ANY_FINALIZE;
	}

	/// In FINALIZE status, all kind of trigger will be ignored.
	for (nt=OPCONTEXT_TRIGGER_START; nt < OPCONTEXT_TRIGGER_END; nt++) {
		SM_CLRO.actions[OPCONTEXT_STATUS_FINALIZE][nt] = DoAction_Ignore;
		SM_CLRR.actions[OPCONTEXT_STATUS_FINALIZE][nt] = DoAction_Ignore;
		SM_CLNoti.actions[OPCONTEXT_STATUS_FINALIZE][nt] = DoAction_Ignore;
	}

	IpcomOpStateMachine_CLRO_Init();
	IpcomOpStateMachine_CLRR_Init();
	IpcomOpStateMachine_CLNoti_Init();
}
