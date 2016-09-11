/*
 * IpcmdOperationContext.h
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPERATIONCONTEXT_H_
#define INCLUDE_IPCMDOPERATIONCONTEXT_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

struct _IpcmdOperationContextId {
	IpcmdChannelId	channel_id_;
	guint32			sender_handle_id_;
} __attribute__ ((packed));

struct _IpcmdOperationContext {
	struct _IpcmdOperationContextId		opctx_id_;

	/// operation information
	guint16								serviceId;
	guint16								operationId;
	guint8								protoVersion;
	guint8								opType;
	//gboolean							procflag;
	IpcmdMessage						*message;

	/// operation state
	IpcmdOpState						mOpState;

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

	///callback functions
	void								NotifyFinalize(IpcmdOpCtx opctx,gpointer cb_data);
	//IpcomReceiveMessageCallback		recvCallback;
	void 								*cb_data;

	struct ref							_ref;
};

IpcmdOpCtx*									IpcmdOpCtxNew();
guint										IpcmdOpCtxIdHashfunc(gconstpointer key);
gboolean									IpcmdOpCtxIdEqual(gconstpointer a, gconstpointer b);
void										IpcmdOpCtxSetMessage(IpcmdOpCtx *ctx, IpcmdMessage *mesg);
gint										IpcomOpCtxTrigger(IpcmdOpCtx *ctx, gint trigger, gpointer data);
//gboolean									IpcomOpContextSetCallbacks(IpcomOpContext *opContext, IpcomReceiveMessageCallback recv_cb, IpcomOpCtxDestroyNotify OnNotify, void *userdata);

gboolean									IpcmdOpCtxStartWFATimer();
gboolean									IpcmdOpCtxStartWFRTimer();
gboolean									IpcmdOpCtxSetTimer(IpcmdOpCtx *opContext, gint milliseconds, GSourceFunc func);
gboolean									IpcmdOpCtxUnsetTimer(IpcmdOpCtx *opContext);
gboolean									IpcmdOpCtxCancelTimer(IpcmdOpCtx *opContext);

IpcmdOpCtx*									IpcmmOpCtxRef(IpcmdOpCtx *ctx);
void										IpcmdOpCtxUnref(IpcmdOpCtx *ctx);

inline gboolean
IpcmdOpCtxIdIsMatch(const IpcmdOpCtxId *a, const IpcmdOpCtxId *b)
{
	return a->channel_id_ == b->channel_id_ && a->sender_handle_id_ == b->sender_handle_id_ ? TRUE : FALSE;
}

G_END_DECLS

#endif /* INCLUDE_IPCMDOPERATIONCONTEXT_H_ */
