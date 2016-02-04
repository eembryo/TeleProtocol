#include <glib.h>
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
	if (ctx->status == OPCONTEXT_STATUS_FINALIZE)
		IpcomOpContextDestroy(ctx);
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

	return ctx;
}

void
IpcomOpContextDestroy(IpcomOpContext *ctx)
{
	if (ctx->message) IpcomMessageUnref(ctx->message);
	IpcomConnectionUnref(ctx->ctxId.connection);
	g_free(ctx);
}
