/*
 * IpcmdOperationContext.h
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPERATIONCONTEXT_H_
#define INCLUDE_IPCMDOPERATIONCONTEXT_H_

#include "IpcmdDeclare.h"
#include "IpcmdOperation.h"
#include "reference.h"
#include "IpcmdOpStateMachine.h"
#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdOpCtxFinalizeCallback {
	gint		(*cb_func)(IpcmdOpCtxId opctx_id, gpointer cb_data);
	/* cb_destroy :
	 * called to release cb_data memory when _IpcmdOpCtxFinalizeCallback is destroyed. If it is NULL, 'cb_data' is not released.
	 */
	gint		(*cb_destroy)(gpointer cb_data);
	gpointer	cb_data;
} IpcmdOpCtxFinalizeCallback;

struct _IpcmdOperationContextId {
	IpcmdChannelId	channel_id_;
	guint32			sender_handle_id_;
} __attribute__ ((packed));

static const IpcmdOpCtxId VoidOpCtxId;

struct _IpcmdOperationContext {
	struct _IpcmdOperationContextId		opctx_id_;	// unique id for each operation context

	/// operation information
	guint16								serviceId;
	guint16								operationId;
	guint8								protoVersion;
	guint8								opType;
	guint8								flags;
	IpcmdMessage						*message;

	/// operation state
	struct _IpcmdOpState				mOpState;

	/// timer-related
	GSource								*timer;
	gint								numberOfRetries;
	gint								nWFAMaxRetries;
	gint								nWFRMaxRetries;
	gint 								nWFABaseTimeout;
	gint 								nWFRBaseTimeout;
	gfloat 								nWFAIncreaseTimeout;
	gfloat 								nWFRIncreaseTimeout;
	gboolean							(*OnWFAExpired)(gpointer data);
	gboolean							(*OnWFRExpired)(gpointer data);

	/* callback functions */
	IpcmdOperationCallback				deliver_to_app_;				// callback function to deliver the operation to the application.
	IpcmdOpCtxFinalizeCallback			notify_finalizing_;

	struct ref							_ref;
};

IpcmdOpCtx*								IpcmdOpCtxNew();
guint									IpcmdOpCtxIdHashfunc(gconstpointer key);
gboolean								IpcmdOpCtxIdEqual(gconstpointer a, gconstpointer b);
void									IpcmdOpCtxSetMessage(IpcmdOpCtx *ctx, IpcmdMessage *mesg);
gint									IpcomOpCtxTrigger(IpcmdOpCtx *ctx, gint trigger, gconstpointer data);
//gboolean								IpcomOpContextSetCallbacks(IpcomOpContext *opContext, IpcomReceiveMessageCallback recv_cb, IpcomOpCtxDestroyNotify OnNotify, void *userdata);

gboolean								IpcmdOpCtxStartWFATimer(IpcmdOpCtx *opctx);
gboolean								IpcmdOpCtxStartWFRTimer(IpcmdOpCtx *opctx);
gboolean								IpcmdOpCtxSetTimer(IpcmdOpCtx *opContext, gint milliseconds, GSourceFunc func);
//gboolean								IpcmdOpCtxUnsetTimer(IpcmdOpCtx *opContext);
gboolean								IpcmdOpCtxCancelTimer(IpcmdOpCtx *opContext);

IpcmdOpCtx*								IpcmdOpCtxRef(IpcmdOpCtx *ctx);
void									IpcmdOpCtxUnref(IpcmdOpCtx *ctx);

inline gboolean			IpcmdOpCtxIdIsMatch(const IpcmdOpCtxId *a, const IpcmdOpCtxId *b)
{
	return a->channel_id_ == b->channel_id_ && a->sender_handle_id_ == b->sender_handle_id_ ? TRUE : FALSE;
}

/* @fn: CalculateTimeoutInterval
 *
 * @return: timeout value (milliseconds).
 * Calculate timeout interval according to retransmission formula.(REQPROD 347050)
 */
gint		CalculateTimeoutInterval(gint base_timeout, gfloat increase_timeout, gint n_of_retry);
G_END_DECLS

#endif /* INCLUDE_IPCMDOPERATIONCONTEXT_H_ */
