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
#include "IpcmdDeclare.h"
#include <glib.h>
#include <string.h>
#include <math.h>

static const IpcmdOpCtxId VoidOpCtxId = {
		.channel_id_ = 0,
		.sender_handle_id_ = 0,
};

static void _OpCtxFree(struct ref *r);

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

	ctx->nWFABaseTimeout = IpcmdConfigGetInstance()->defaultTimeoutWFA;
	ctx->nWFAIncreaseTimeout = IpcmdConfigGetInstance()->increaseTimerValueWFA;
	ctx->nWFAMaxRetries = IpcmdConfigGetInstance()->numberOfRetriesWFA;
	ctx->nWFRBaseTimeout = IpcmdConfigGetInstance()->defaultTimeoutWFR;
	ctx->nWFRIncreaseTimeout = IpcmdConfigGetInstance()->increaseTimerValueWFR;
	ctx->nWFRMaxRetries = IpcmdConfigGetInstance()->numberOfRetriesWFR;

	/// Increase reference count
	ref_init(&ctx->_ref, _OpCtxFree);
	IpcmdOpCtxRef(ctx);

	return ctx;
}

void
IpcmdOpCtxSetMessage(IpcmdOpCtx *self, IpcmdMessage *mesg)
{
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
IpcomOpCtxTrigger(IpcmdOpCtx *self, gint trigger, gconstpointer data)
{
	gint state = self->mOpState.state_;

	if (state == kOpContextStateFinalize)
		return kOpContextStateFinalize;

	return self->mOpState.pSM->actions[state][trigger](&self->mOpState, trigger, data);
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
	g_source_attach(timeoutSource, g_main_context_default());
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

	g_debug("Destroy operation context (SenderHandleID: 0x%x, OpType: 0x%x)\n", ctx->opctx_id_.sender_handle_id_,ctx->opType);

	if (ctx->timer) IpcmdOpCtxCancelTimer(ctx);
	if (ctx->message) IpcmdMessageUnref(ctx->message);

	g_free(ctx);
}

