#include <glib.h>
#include <IpcomProtocol.h>
#include <IpcomOperationContext.h>
#include <IpcomEnums.h>
#include <IpcomMessage.h>
#include <IpcomConnection.h>
#include <string.h>
#include <dprint.h>
#include <IpcomOpStateMachine.h>

/******************************************
 * static functions
 ******************************************/
static gboolean
DefaultOnWFATimerExpired(gpointer data)
{
	IpcomOpContext *ctx = (IpcomOpContext *)data;

	DFUNCTION_START;

	IpcomOpContextRef(ctx);
	IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_WFA_EXPIRED, GINT_TO_POINTER(0));
	IpcomOpContextUnref(ctx);
	return G_SOURCE_REMOVE;
}

static gboolean
DefaultOnWFRTimerExpired(gpointer data)
{
	IpcomOpContext *ctx = (IpcomOpContext *)data;

	DFUNCTION_START;

	IpcomOpContextRef(ctx);
	IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_WFR_EXPIRED, GINT_TO_POINTER(0));
	IpcomOpContextUnref(ctx);
	return G_SOURCE_REMOVE;
}

static void
_OpContextFree(struct ref *r)
{
	IpcomOpContext *ctx = container_of(r, IpcomOpContext, _ref);

	DPRINT("Destroy operation context (SenderHandleID: 0x%x, OpType: 0x%x)\n", IpcomOpContextGetSenderHandleID(ctx),IpcomOpContextGetType(ctx));

	g_assert(ctx->mOpState.nState == OPCONTEXT_STATUS_NONE || ctx->mOpState.nState == OPCONTEXT_STATUS_FINALIZE);

	if (ctx->timer) IpcomOpContextCancelTimer(ctx);
	IpcomProtocolUnregisterOpContext(IpcomProtocolGetInstance(), IpcomOpContextGetContextId(ctx));
	if (ctx->mOpState.nState == OPCONTEXT_STATUS_FINALIZE && ctx->NotifyDestroyed)
		ctx->NotifyDestroyed(IpcomOpContextGetContextId(ctx), ctx->mOpState.nFinCode, ctx->cb_data);
	IpcomConnectionUnref(ctx->ctxId.connection);
	if (ctx->message) IpcomMessageUnref(ctx->message);

	g_free(ctx);
}

/******************************************
 * extern functions
 ******************************************/
IpcomOpContext *
IpcomOpContextRef(IpcomOpContext *ctx)
{
	g_assert(ctx);
	ref_inc(&ctx->_ref);
	return ctx;
}

void
IpcomOpContextUnref(IpcomOpContext *ctx)
{
	g_assert(ctx);
	ref_dec(&ctx->_ref);
}

gboolean
IpcomOpContextIdEqual(gconstpointer aOpContextId, gconstpointer bOpContextId)
{
	return memcmp(aOpContextId, bOpContextId, sizeof(IpcomOpContextId)) ? FALSE : TRUE;
}

gboolean
IpcomOpContextSetCallbacks(IpcomOpContext *opContext, IpcomReceiveMessageCallback recv_cb, IpcomOpCtxDestroyNotify OnNotify, void *userdata)
{
	opContext->recvCallback = recv_cb;
	opContext->cb_data = userdata;
	opContext->NotifyDestroyed = OnNotify;

	return TRUE;
}
void
IpcomOpContextSetMessage(IpcomOpContext *ctx, IpcomMessage *mesg)
{
	if (ctx->message) {
		IpcomMessageUnref(ctx->message);
		ctx->message = NULL;
	}
	IpcomMessageRef(mesg);
	ctx->message = mesg;
}


IpcomOpContext *
IpcomOpContextNewAndRegister(IpcomConnection *conn, guint32 senderHandleId, guint16 serviceId, guint16 operationId, guint8 protoVersion, guint8 opType)
{
	struct _IpcomOpContext *ctx;

	ctx = g_malloc0(sizeof(IpcomOpContext));
	if (ctx == NULL) {
		DWARN("Cannot allocate memory.\n");
		return NULL;
	}
	ctx->ctxId.connection 		= IpcomConnectionRef(conn); g_assert(ctx->ctxId.connection != NULL);
	ctx->ctxId.senderHandleId 	= senderHandleId;					//sender handle ID of iniate message
	ctx->serviceId				= serviceId;
	ctx->operationId			= operationId;						//operation ID of initiate message
	ctx->protoVersion			= protoVersion;						//protocol version of initiate message
	ctx->opType 				= opType;							//optype of initiate message
	ctx->message 				= NULL;
	ctx->recvCallback 			= NULL;
	ctx->cb_data 				= NULL;
	ctx->NotifyDestroyed		= NULL;

	/// Timer-related variables
	ctx->timer					= NULL;
	ctx->numberOfRetries		= 0;
	ctx->nWFAMaxRetries			= numberOfRetriesWFA;
	ctx->nWFRMaxRetries			= numberOfRetriesWFR;
	ctx->nWFABaseTimeout		= defaultTimeoutWFA;
	ctx->nWFRBaseTimeout		= defaultTimeoutWFR;
	ctx->nWFAIncreaseTimeout	= increaseTimerValueWFA;
	ctx->nWFRIncreaseTimeout	= increaseTimerValueWFR;
	ctx->OnWFAExpired			= DefaultOnWFATimerExpired;
	ctx->OnWFRExpired			= DefaultOnWFRTimerExpired;
	ctx->mOpState.nFinCode		= OPCONTEXT_FINCODE_NORMAL;
	ctx->mOpState.nState		= IPCOM_SERVICE_STATUS_NONE;

	/// operation state
	switch(ctx->opType) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST:
		//if (connection == UDP)
		ctx->mOpState.pSM = &SM_CLRR;
		break;
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		//if (connection == UDP)
		ctx->mOpState.pSM = &SM_CLRO;
		break;
	case IPCOM_OPTYPE_NOTIFICATION:
		//if (connection == UDP)
		ctx->mOpState.pSM = &SM_CLNoti;
		break;
	default:
		g_error("Tried to create Operation Context with wrong OpType.");
	}

	/// Increase reference count
	ref_init(&ctx->_ref, _OpContextFree);
	IpcomOpContextRef(ctx);

	/// Register Operation context to IpcomProtocol
	if (!IpcomProtocolRegisterOpContext(IpcomProtocolGetInstance(), IpcomOpContextGetContextId(ctx), ctx)) {
		DWARN("Failed to register operation context to IpcomProtocol.\n");
		return NULL;
	}
	return ctx;
}

gboolean
IpcomOpContextSetTimer(IpcomOpContext *opContext, gint milliseconds, GSourceFunc func)
{
	GSource *timeoutSource;

	if (milliseconds <= 0) {
		DWARN("Timer should have positive interval value. But milliseconds is %d.\n", milliseconds);
		return FALSE;
	}
	timeoutSource = g_timeout_source_new(milliseconds);			g_assert(timeoutSource);
	g_source_set_callback(timeoutSource, func, opContext, NULL);
	g_source_attach(timeoutSource, IpcomProtocolGetMainContext());
	opContext->timer = timeoutSource;
	g_source_unref(timeoutSource);

	return TRUE;
}

gboolean
IpcomOpContextStartWFATimer(IpcomOpContext *pOpContext)
{
	if (pOpContext->timer) return FALSE;

	pOpContext->numberOfRetries = 0;
	IpcomOpContextSetTimer(pOpContext, CalculateUsedTimeout(pOpContext->nWFABaseTimeout, pOpContext->nWFAIncreaseTimeout, pOpContext->numberOfRetries), pOpContext->OnWFAExpired);

	return TRUE;
}

gboolean
IpcomOpContextStartWFRTimer(IpcomOpContext *pOpContext)
{
	if (pOpContext->timer) return FALSE;

	pOpContext->numberOfRetries = 0;
	IpcomOpContextSetTimer(pOpContext, CalculateUsedTimeout(pOpContext->nWFRBaseTimeout, pOpContext->nWFRIncreaseTimeout, pOpContext->numberOfRetries), pOpContext->OnWFRExpired);
	return TRUE;
}

gboolean
IpcomOpContextCancelTimer(IpcomOpContext *opContext)
{
	if (opContext->timer) g_source_destroy(opContext->timer);
	opContext->timer = NULL;
	opContext->numberOfRetries = 0;
	return TRUE;
}


gint
IpcomOpContextTrigger(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger, gpointer data)
{
	gint oldState = ctx->mOpState.nState;
	gint newState;

	newState = ctx->mOpState.pSM->actions[oldState][trigger](&ctx->mOpState, trigger, data);

	if (newState < 0) return newState;

	if (newState != oldState) {
		/// if this trigger is the first valid trigger, Operation Context will be alive until FINALIZE state.
		if (oldState == OPCONTEXT_STATUS_NONE) IpcomOpContextRef(ctx);
		/// if this is first FINALIZE state, decrease reference count
		if (newState == OPCONTEXT_STATUS_FINALIZE)	IpcomOpContextUnref(ctx);
	}

	return ctx->mOpState.nState;
}
