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
	ctx->status = status;
	return TRUE;
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

void
IpcomOpContextDestroy(IpcomOpContext *ctx)
{
	if (ctx->timer) IpcomOpContextCancelTimer(ctx);
	IpcomConnectionUnref(ctx->ctxId.connection);
	if (ctx->message) IpcomMessageUnref(ctx->message);

	g_free(ctx);
}

static gboolean
_TriggerForREQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	gchar *error = NULL;

	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_NONE) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_ACK_RECV);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else if (trigger == OPCONTEXT_TRIGGER_EXPIRE_WFA) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_REQUEST_SENT) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_ACK_RECV:
		if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_ACK_RECV) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_PROCESS_RESPONSE:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_PROCESS_RESPONSE) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_SEND_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_RESPONSE_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST ||
				trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_PROCESS_REQUEST) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else if (trigger == OPCONTEXT_TRIGGER_EXPIRE_WFA) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_RESPONSE_SENT) does not accept this trigger.\n";
			goto _Trigger_REQUEST_failed;
		}
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_REQUEST_failed:
	if (error) DERROR("%s\n", error);
	return FALSE;
}

static gboolean
_TriggerForSETREQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	gchar *error = NULL;

	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_NONE) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_ACK_RECV);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else if (trigger == OPCONTEXT_TRIGGER_EXPIRE_WFA) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_REQUEST_SENT) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_ACK_RECV:
		if (trigger == OPCONTEXT_TRIGGER_RECV_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_ACK_RECV) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_PROCESS_RESPONSE:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_PROCESS_RESPONSE) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_SEND_RESPONSE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_RESPONSE_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST ||
				trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_PROCESS_REQUEST) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else if (trigger == OPCONTEXT_TRIGGER_EXPIRE_WFA) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_RESPONSE_SENT) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_failed;
		}
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_SETREQUEST_failed:
	if (error) DERROR("%s\n", error);
	return FALSE;
}

static gboolean
_TriggerForSETREQUEST_NORETURN(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	gchar *error = NULL;

	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		else if (trigger == OPCONTEXT_TRIGGER_RECV_REQUEST) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_NONE) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_NORETURN_failed;
		}
		break;
	case OPCONTEXT_STATUS_REQUEST_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else if (trigger == OPCONTEXT_TRIGGER_EXPIRE_WFA) break;
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_REQUEST_SENT) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_NORETURN_failed;
		}
		break;
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		if (trigger == OPCONTEXT_TRIGGER_PROCESS_DONE) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_PROCESS_REQUEST) does not accept this trigger.\n";
			goto _Trigger_SETREQUEST_NORETURN_failed;
		}
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_SETREQUEST_NORETURN_failed:
	if (error) DERROR("%s\n", error);
	return FALSE;
}

static gboolean
_TriggerForNOTIFICATION_REQUEST(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	return FALSE;
}

static gboolean
_TriggerForNOTIFICATION(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	gchar *error = NULL;

	switch(ctx->status) {
	case OPCONTEXT_STATUS_NONE:
		if (trigger == OPCONTEXT_TRIGGER_SEND_NOTIFICATION) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_NOTIFICATION_SENT);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_NONE) does not accept this trigger.\n";
			goto _Trigger_NOTIFICATION_failed;
		}
		break;
	case OPCONTEXT_STATUS_NOTIFICATION_SENT:
		if (trigger == OPCONTEXT_TRIGGER_RECV_ACK) IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_FINALIZE);
		else {
			error = "Current OpContext status (OPCONTEXT_STATUS_NOTIFICATION_SENT) does not accept this trigger.\n";
			goto _Trigger_NOTIFICATION_failed;
		}
		break;
	default:
		g_assert(FALSE);
	}
	return TRUE;

	_Trigger_NOTIFICATION_failed:
	if (error) DERROR("%s\n", error);
	return FALSE;
}
gboolean
IpcomOpContextTrigger(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger)
{
	switch(ctx->opType) {
	case IPCOM_OPTYPE_REQUEST:
		return _TriggerForREQUEST(ctx, trigger);
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
		return _TriggerForSETREQUEST_NORETURN(ctx, trigger);
	case IPCOM_OPTYPE_SETREQUEST:
		return _TriggerForSETREQUEST(ctx, trigger);
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		return _TriggerForNOTIFICATION_REQUEST(ctx, trigger);
	case IPCOM_OPTYPE_NOTIFICATION:
		return _TriggerForNOTIFICATION(ctx, trigger);
	default:
		DERROR("This cannot be happen.\n");
		return FALSE;
	}
}

