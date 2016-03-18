#include <glib.h>
#include <IpcomProtocol.h>
#include <IpcomOperationContext.h>
#include <IpcomEnums.h>
#include <IpcomMessage.h>
#include <IpcomConnection.h>
#include <string.h>
#include <dprint.h>

gboolean
IpcomOpContextIdEqual(gconstpointer aOpContextId, gconstpointer bOpContextId)
{
	return memcmp(aOpContextId, bOpContextId, sizeof(IpcomOpContextId)) ? FALSE : TRUE;
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

gboolean
IpcomOpContextSetStatus(IpcomOpContext *ctx, gint status)
{
	if (ctx->status == status) return TRUE;

	/// if old status is IPCOM_SERVICE_STATUS_NONE, operation context is not removed until FINALIZE status.
	if (ctx->status == OPCONTEXT_STATUS_NONE) IpcomOpContextRef(ctx);
	/// If new status is OPCONTEXT_STATUS_FINALIZE, operation context will be destroyed in near future.
	if (status == OPCONTEXT_STATUS_FINALIZE) IpcomOpContextUnref(ctx);

	ctx->status = status;

	return TRUE;
}

static void
_OpContextFree(struct ref *r)
{
	IpcomOpContext *ctx = container_of(r, IpcomOpContext, _ref);

	DPRINT("Destroy operation context for %x\n", IpcomOpContextGetSenderHandleID(ctx));

	if (ctx->timer) IpcomOpContextCancelTimer(ctx);
	IpcomProtocolUnregisterOpContext(IpcomProtocolGetInstance(), IpcomOpContextGetContextId(ctx));
	if (ctx->status != OPCONTEXT_STATUS_NONE && ctx->NotifyDestroyed)
		ctx->NotifyDestroyed(IpcomOpContextGetContextId(ctx), ctx->result, ctx->cb_data);
	IpcomConnectionUnref(ctx->ctxId.connection);
	if (ctx->message) IpcomMessageUnref(ctx->message);

	g_free(ctx);
}

IpcomOpContext *
IpcomOpContextNewAndRegister(IpcomConnection *conn, guint32 senderHandleId, guint opType,	IpcomReceiveMessageCallback recv_cb, void *userdata)
{
	struct _IpcomOpContext *ctx;

	ctx = g_malloc0(sizeof(IpcomOpContext));
	if (ctx == NULL) {
		DWARN("Cannot allocate memory.\n");
		return NULL;
	}
	ctx->ctxId.connection 		= IpcomConnectionRef(conn); g_assert(ctx->ctxId.connection != NULL);
	ctx->ctxId.senderHandleId 	= senderHandleId;
	ctx->opType 				= opType;							//optype of request
	ctx->message 				= NULL;
	ctx->recvCallback 			= recv_cb;
	ctx->cb_data 				= userdata;
	ctx->status					= IPCOM_SERVICE_STATUS_NONE;
	ctx->numberOfRetries		= 0;
	ctx->NotifyDestroyed		= NULL;
	ctx->result					= -1;

	ref_init(&ctx->_ref, _OpContextFree);
	IpcomOpContextRef(ctx);

	if (!IpcomProtocolRegisterOpContext(IpcomProtocolGetInstance(), IpcomOpContextGetContextId(ctx), ctx)) {
		DWARN("Failed to register operation context to IpcomProtocol.\n");
		return NULL;
	}
	return ctx;
}

IpcomOpContext *
IpcomOpContextCreate(IpcomConnection *conn, guint32 senderHandleId, guint opType,	IpcomReceiveMessageCallback recv_cb, void *userdata)
{
	struct _IpcomOpContext *ctx;

	ctx = g_malloc0(sizeof(IpcomOpContext));
	ctx->ctxId.connection 		= IpcomConnectionRef(conn); g_assert(ctx->ctxId.connection != NULL);
	ctx->ctxId.senderHandleId 	= senderHandleId;
	ctx->opType 				= opType;							//optype of request
	ctx->message 				= NULL;
	ctx->recvCallback 			= recv_cb;
	ctx->cb_data 				= userdata;
	ctx->status					= IPCOM_SERVICE_STATUS_NONE;
	ctx->numberOfRetries		= 0;
	ctx->NotifyDestroyed		= NULL;
	ctx->result					= -1;

	ref_init(&ctx->_ref, _OpContextFree);

	return IpcomOpContextRef(ctx);
}

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
IpcomOpContextUnsetTimer(IpcomOpContext *opContext)
{
	opContext->timer = NULL;
	opContext->numberOfRetries = 0;
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

static gboolean
_TriggerForREQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else goto _Trigger_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_ACK_RECV);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else goto _Trigger_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_ACK_RECV:
		if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else goto _Trigger_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_RESPONSE:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_SEND_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_RESPONSE_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) break;
		else if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else goto _Trigger_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else goto _Trigger_REQUEST_failed;
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_REQUEST_failed:
	return FALSE;
}

static gboolean
_TriggerForSETREQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else goto _Trigger_SETREQUEST_failed;
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_ACK_RECV);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else goto _Trigger_SETREQUEST_failed;
		break;
	case OPCONTEXT_STATUS_ACK_RECV:
		if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else goto _Trigger_SETREQUEST_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_RESPONSE:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_SETREQUEST_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_SEND_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_RESPONSE_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST || trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else goto _Trigger_SETREQUEST_failed;
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else goto _Trigger_SETREQUEST_failed;
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_SETREQUEST_failed:
	return FALSE;
}

static gboolean
_TriggerForSETREQUEST_NORETURN(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else goto _Trigger_SETREQUEST_NORETURN_failed;
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_SETREQUEST_NORETURN_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_SETREQUEST_NORETURN_failed;
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_SETREQUEST_NORETURN_failed:
	return FALSE;
}

static gboolean
_TriggerForNOTIFICATION_REQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else goto _Trigger_NOTIFICATION_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_NOTIFICATION_REQUEST_failed;
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_NOTIFICATION_REQUEST_failed;
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_NOTIFICATION_REQUEST_failed:
	return FALSE;
}

static gboolean
_TriggerForNOTIFICATION(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_NOTIFICATION) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_NOTIFICATION_SENT);
		else goto _Trigger_NOTIFICATION_failed;
		break;
	case OPCONTEXT_STATUS_NOTIFICATION_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) {IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE); ctx->result = 0;}
		else goto _Trigger_NOTIFICATION_failed;
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_NOTIFICATION_failed:
	return FALSE;
}

gint
IpcomOpContextTrigger(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	gboolean ret;

	if (trigger & OPCONTEXT_TRIGGER_FINALIZE) {
		IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		ctx->result = (trigger & (~OPCONTEXT_TRIGGER_FINALIZE));
		return ctx->status;
	}

	if (ctx->status == OPCONTEXT_STATUS_FINALIZE) {
		DWARN("OPCONTEXT_STATUS_FINALIZE does not accept any trigger.\n");
		return -1;
	}

	switch(ctx->opType) {
	case IPCOM_OPTYPE_REQUEST:
		ret = _TriggerForREQUEST(ctx, trigger);
		break;
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
		ret = _TriggerForSETREQUEST_NORETURN(ctx, trigger);
		break;
	case IPCOM_OPTYPE_SETREQUEST:
		ret = _TriggerForSETREQUEST(ctx, trigger);
		break;
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		ret = _TriggerForNOTIFICATION_REQUEST(ctx, trigger);
		break;
	case IPCOM_OPTYPE_NOTIFICATION:
		ret = _TriggerForNOTIFICATION(ctx, trigger);
		break;
	default:
		DERROR("This cannot be happen.\n");
		return -1;
	}

	if (!ret) {
		DERROR("Current OpContext status(%s) does not accept this trigger(%s)\n", OpContextStatusString[ctx->status], OpContextTriggerString[trigger]);
		return -1;
	}

	return ctx->status;
}


/**
 * deprecated
 */
#if 0
void
IpcomOpContextDestroy(IpcomOpContext *ctx)
{
	DPRINT("Destroy operation context for %x\n", IpcomOpContextGetSenderHandleID(ctx));

	if (ctx->timer) IpcomOpContextCancelTimer(ctx);
	if (ctx->NotifyDestroyed)
		ctx->NotifyDestroyed(IpcomOpContextGetContextId(ctx), ctx->result, ctx->cb_data);
	IpcomConnectionUnref(ctx->ctxId.connection);
	if (ctx->message) IpcomMessageUnref(ctx->message);

	g_free(ctx);
}

#endif
