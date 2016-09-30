/*
 * IpcmdOperationContext.c
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachine.h"
#include "../include/IpcmdConfig.h"
#include "../include/IpcmdChannel.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdCore.h"
#include "IpcmdDeclare.h"
#include <glib.h>
#include <string.h>
#include <math.h>

/*
static const IpcmdOpCtxId VoidOpCtxId = {
		.channel_id_ = 0,
		.sender_handle_id_ = 0,
};
*/

static void _OpCtxFree(struct ref *r);
static gboolean	DefaultOnWFRTimerExpired(gpointer data);
static gboolean	DefaultOnWFATimerExpired(gpointer data);

/* @fn: CalculateTimeoutInterval
 * calculate next timeout value in milliseconds.
 *
 */
gint
CalculateTimeoutInterval(gint base_timeout, gfloat increase_timeout, gint n_of_retry) {
	return (gint)(base_timeout * powf(increase_timeout, n_of_retry));
}

IpcmdOpCtx*
IpcmdOpCtxNew()
{
	struct _IpcmdOperationContext *ctx;

	ctx = g_malloc0(sizeof(struct _IpcmdOperationContext));
	if (!ctx) return NULL;

	/// Increase reference count
	ref_init(&ctx->_ref, _OpCtxFree);

	return IpcmdOpCtxRef(ctx);
}

/* @fn: IpcmdOpCtxInit
 * Initialize IpcmdOpCtx. It is assumed that ctx->opctx_id_ is already assigned.
 */
void
IpcmdOpCtxInit (IpcmdOpCtx *ctx,
		IpcmdCore *core,
		guint16	service_id,
		guint16	operation_id,
		guint16	op_type,
		guint8	flags,
		const IpcmdOpCtxDeliverToAppCallback	*deliver_to_app,
		const IpcmdOpCtxFinalizeCallback		*notify_finalizing)
{
	ctx->core_ = core;
	ctx->serviceId = service_id;
	ctx->operationId = operation_id;
	ctx->protoVersion = IPCMD_PROTOCOL_VERSION;
	ctx->opType = op_type;
	ctx->flags = flags;
	ctx->message = NULL;
	ctx->mOpState.fin_code_ = 0;
	ctx->mOpState.state_ = kOpContextStateIdle;
	ctx->mOpState.pSM = NULL;
	switch (op_type) {
	case IPCMD_OPTYPE_REQUEST:
		ctx->mOpState.pSM = IpcmdChannelIsConnectionOriented (IpcmdBusFindChannelById (IpcmdCoreGetBus(ctx->core_), ctx->opctx_id_.channel_id_)) ? NULL : &SM_CL_Request;
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		ctx->mOpState.pSM = IpcmdChannelIsConnectionOriented (IpcmdBusFindChannelById (IpcmdCoreGetBus(ctx->core_), ctx->opctx_id_.channel_id_)) ? NULL : &SM_CL_Setnor;
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		ctx->mOpState.pSM = IpcmdChannelIsConnectionOriented (IpcmdBusFindChannelById (IpcmdCoreGetBus(ctx->core_), ctx->opctx_id_.channel_id_)) ? NULL : &SM_CL_Setreq;
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		ctx->mOpState.pSM = IpcmdChannelIsConnectionOriented (IpcmdBusFindChannelById (IpcmdCoreGetBus(ctx->core_), ctx->opctx_id_.channel_id_)) ? NULL : &SM_CL_Notreq;
		break;
	case IPCMD_OPTYPE_NOTIFICATION:
		ctx->mOpState.pSM = IpcmdChannelIsConnectionOriented (IpcmdBusFindChannelById (IpcmdCoreGetBus(ctx->core_), ctx->opctx_id_.channel_id_)) ? NULL : &SM_CL_Noti;
		break;
	default:
		g_error("IpcmdOpCtxInit should be called with one of IPCMD_OPTYPE_REQUEST, "
				"IPCMD_OPTYPE_SETREQUEST_NORETURN, IPCMD_OPTYPE_SETREQUEST, IPCMD_OPTYPE_NOTIFICATION_REQUEST "
				"and IPCMD_OPTYPE_NOTIFICATION operation type");
	}
	g_assert (ctx->mOpState.pSM);

	ctx->timer = NULL;
	ctx->nWFABaseTimeout = IpcmdConfigGetInstance()->defaultTimeoutWFA;
	ctx->nWFAIncreaseTimeout = IpcmdConfigGetInstance()->increaseTimerValueWFA;
	ctx->nWFAMaxRetries = IpcmdConfigGetInstance()->numberOfRetriesWFA;
	ctx->nWFRBaseTimeout = IpcmdConfigGetInstance()->defaultTimeoutWFR;
	ctx->nWFRIncreaseTimeout = IpcmdConfigGetInstance()->increaseTimerValueWFR;
	ctx->nWFRMaxRetries = IpcmdConfigGetInstance()->numberOfRetriesWFR;
	ctx->OnWFAExpired = DefaultOnWFATimerExpired;
	ctx->OnWFRExpired = DefaultOnWFRTimerExpired;

	if (deliver_to_app) {
		ctx->deliver_to_app_.cb_func = deliver_to_app->cb_func;
		ctx->deliver_to_app_.cb_destroy = deliver_to_app->cb_destroy;
		ctx->deliver_to_app_.cb_data = deliver_to_app->cb_data;
	}
	else {
		ctx->deliver_to_app_.cb_func = NULL;
		ctx->deliver_to_app_.cb_destroy = NULL;
		ctx->deliver_to_app_.cb_data = NULL;
	}
	if (notify_finalizing) {
		ctx->notify_finalizing_.cb_func = notify_finalizing->cb_func;
		ctx->notify_finalizing_.cb_destroy = notify_finalizing->cb_destroy;
		ctx->notify_finalizing_.cb_data = notify_finalizing->cb_data;
	}
	else {
		ctx->notify_finalizing_.cb_func = NULL;
		ctx->notify_finalizing_.cb_destroy = NULL;
		ctx->notify_finalizing_.cb_data = NULL;
	}
}

void
IpcmdOpCtxSetMessage(IpcmdOpCtx *self, IpcmdMessage *mesg)
{
	if (self->message) IpcmdMessageUnref(self->message);
	self->message = IpcmdMessageRef(mesg);
}

guint
IpcmdOpCtxIdHashfunc(gconstpointer key) {
	IpcmdOpCtxId *opctx_id = (IpcmdOpCtxId*)key;
	return (guint)(opctx_id->channel_id_ + opctx_id->sender_handle_id_);
}

gboolean
IpcmdOpCtxIdEqual(gconstpointer a, gconstpointer b) {
	return !memcmp(a,b,sizeof(IpcmdOpCtxId)) ? TRUE : FALSE;
}

gint
IpcmdOpCtxTrigger(IpcmdOpCtx *self, gint trigger, gconstpointer data)
{
	gint old_state = self->mOpState.state_;
	gint new_state;

	if (old_state == kOpContextStateFinalize)
		return kOpContextStateFinalize;

	IpcmdOpCtxRef (self);
	// lock mOpState
	// validation check for operation context and message(data)
	new_state = self->mOpState.pSM->actions[old_state][trigger](&self->mOpState, trigger, data);
	// unlock mOpState
	IpcmdOpCtxUnref (self);
	return new_state;
}

gboolean
IpcmdOpCtxStartWFATimer(IpcmdOpCtx *self)
{
	if (self->timer) return FALSE;

	self->numberOfRetries = 0;
	IpcmdOpCtxSetTimer(self, CalculateTimeoutInterval(self->nWFABaseTimeout, self->nWFAIncreaseTimeout, self->numberOfRetries), self->OnWFAExpired);
	return TRUE;
}
gboolean
IpcmdOpCtxStartWFRTimer(IpcmdOpCtx *self)
{
	if (self->timer) return FALSE;

	self->numberOfRetries = 0;
	IpcmdOpCtxSetTimer(self, CalculateTimeoutInterval(self->nWFRBaseTimeout, self->nWFRIncreaseTimeout, self->numberOfRetries), self->OnWFRExpired);
	return TRUE;
}
gboolean
IpcmdOpCtxSetTimer(IpcmdOpCtx *self, gint milliseconds, GSourceFunc func)
{
	GSource *timeoutSource;

	// remove old timer
	if (self->timer) g_source_unref(self->timer);

	timeoutSource = g_timeout_source_new(milliseconds);
	g_assert(timeoutSource);
	g_source_set_callback(timeoutSource, func, self, NULL);
	//g_source_attach(timeoutSource, g_main_context_default());
	g_source_attach(timeoutSource, IpcmdCoreGetGMainContext(self->core_));
	self->timer = timeoutSource;

	return TRUE;
}

gboolean
IpcmdOpCtxCancelTimer(IpcmdOpCtx *self)
{
	if (self->timer) {
		g_source_destroy(self->timer);
		g_source_unref(self->timer);
	}
	self->timer = NULL;
	self->numberOfRetries = 0;
	return TRUE;
}

IpcmdOpCtx*
IpcmdOpCtxRef(IpcmdOpCtx *self)
{
	ref_inc(&self->_ref);
	return self;
}
void
IpcmdOpCtxUnref(IpcmdOpCtx *self)
{
	ref_dec(&self->_ref);
}

static void
_OpCtxFree(struct ref *r)
{
	IpcmdOpCtx *ctx = container_of(r, IpcmdOpCtx, _ref);

	g_debug("Destroy operation context (CONNID=%d, SHID=0x%.04x, OpType=0x%x)", ctx->opctx_id_.channel_id_, ctx->opctx_id_.sender_handle_id_,ctx->opType);

	if (ctx->timer) IpcmdOpCtxCancelTimer(ctx);
	if (ctx->message) IpcmdMessageUnref(ctx->message);
	if (ctx->deliver_to_app_.cb_data && ctx->deliver_to_app_.cb_destroy)
		ctx->deliver_to_app_.cb_destroy (ctx->deliver_to_app_.cb_data);
	if (ctx->notify_finalizing_.cb_data && ctx->notify_finalizing_.cb_destroy)
		ctx->notify_finalizing_.cb_destroy (ctx->notify_finalizing_.cb_data);

	g_free(ctx);
}

static gboolean
DefaultOnWFATimerExpired(gpointer data)
{
	IpcmdOpCtx *op_ctx = (IpcmdOpCtx *)data;

	IpcmdOpCtxRef(op_ctx);
	if (IpcmdOpCtxTrigger(op_ctx, kIpcmdTriggerWFATimeout, GINT_TO_POINTER(0)) < 0) {
		g_error("Failed to trigger WFA_EXPIRED to OperationContext(%p).\n",op_ctx);
	}
	IpcmdOpCtxUnref(op_ctx);
	return G_SOURCE_REMOVE;
}

static gboolean
DefaultOnWFRTimerExpired(gpointer data)
{
	IpcmdOpCtx *op_ctx = (IpcmdOpCtx *)data;

	IpcmdOpCtxRef(op_ctx);
	if (IpcmdOpCtxTrigger(op_ctx, kIpcmdTriggerWFRTimeout, GINT_TO_POINTER(0)) < 0) {
		g_error("Failed to trigger WFR_EXPIRED to OperationContext(%p).\n",op_ctx);
	}
	IpcmdOpCtxUnref(op_ctx);
	return G_SOURCE_REMOVE;
}
