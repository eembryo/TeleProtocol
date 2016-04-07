// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#ifndef __IPCOMMANDPROTOCOL_INCLUDE_IPCOMOPERATIONCONTEXT_H_
#define __IPCOMMANDPROTOCOL_INCLUDE_IPCOMOPERATIONCONTEXT_H_

#include <glib.h>
#include <IpcomTypes.h>
#include <IpcomOpStateMachine.h>
#include <ref_count.h>
#include <math.h>

struct _IpcomOpContextId
{
	guint32				senderHandleId;
	IpcomConnection		*connection;
} __attribute__ ((packed));

struct _IpcomOpContext
{
	struct _IpcomOpContextId			ctxId;
	/// operation information
	guint16								serviceId;
	guint16								operationId;
	guint8								protoVersion;
	guint8								opType;
	IpcomMessage						*message;

	/// operation state
	IpcomOpState						mOpState;

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
	IpcomOpCtxDestroyNotify				NotifyDestroyed;
	IpcomReceiveMessageCallback			recvCallback;
	void 								*cb_data;

	struct ref							_ref;
};

static inline IpcomConnection*				IpcomOpContextGetConnection(IpcomOpContext *ctx)
{ return ctx->ctxId.connection; }
static inline guint32 						IpcomOpContextGetSenderHandleID(const IpcomOpContext *ctx)
{ return ctx->ctxId.senderHandleId;}
static inline guint16						IpcomOpContextGetServiceID(const IpcomOpContext *ctx)
{ return ctx->serviceId; }
static inline guint16						IpcomOpContextGetOperationID(const IpcomOpContext *ctx)
{ return ctx->operationId; }
static inline guint8						IpcomOpContextGetProtoVersion(const IpcomOpContext *ctx)
{ return ctx->protoVersion; }
static inline guint8 						IpcomOpContextGetType(const IpcomOpContext *ctx)
{ return ctx->opType; }
static inline IpcomReceiveMessageCallback	IpcomOpContextGetRecvCallback(const IpcomOpContext *ctx)
{ return ctx->recvCallback;}
static inline void*							IpcomOpContextGetRecvCallbackData(const IpcomOpContext *ctx)
{ return ctx->cb_data;}
static inline IpcomOpContextId*				IpcomOpContextGetContextId(IpcomOpContext *ctx)
{ return &ctx->ctxId; }
static inline IpcomMessage*					IpcomOpContextGetMessage(IpcomOpContext *ctx)
{ return ctx->message; }
static inline guint							IpcomOpContextIdHashFunc(const gconstpointer key)
{ return ((struct _IpcomOpContextId *)key)->senderHandleId; }
static inline IpcomProtocolOpContextStatus IpcomOpContextGetState(IpcomOpContext *ctx)
{ return ctx->mOpState.nState; }
static inline gint							CalculateUsedTimeout(gint baseTimeoutValue, gint increaseTimeoutValue, gint nOfRetries) {
	gint usedTimeoutValue = baseTimeoutValue * powf(increaseTimeoutValue, nOfRetries);
	return usedTimeoutValue;
}
gboolean 									IpcomOpContextIdEqual(gconstpointer aOpContextId, gconstpointer bOpContextId);
IpcomOpContext *							IpcomOpContextNewAndRegister(IpcomConnection *conn, guint32 senderHandleId, guint16 serviceId, guint16 operationId, guint8 protoVersion, guint8 opType);
gboolean									IpcomOpContextSetCallbacks(IpcomOpContext *opContext, IpcomReceiveMessageCallback recv_cb, IpcomOpCtxDestroyNotify OnNotify, void *userdata);
void										IpcomOpContextSetMessage(IpcomOpContext *ctx, IpcomMessage *mesg);
gint										IpcomOpContextTrigger(IpcomOpContext *ctx, IpcomProtocolOpContextTriggers trigger, gpointer data);
gboolean									IpcomOpContextUnsetTimer(IpcomOpContext *opContext);
static inline void							IpcomOpContextSetOnDestroy(IpcomOpContext *ctx, IpcomOpCtxDestroyNotify OnNotify)
{ ctx->NotifyDestroyed = OnNotify; }
gboolean									IpcomOpContextStartWFATimer();
gboolean									IpcomOpContextStartWFRTimer();
gboolean									IpcomOpContextSetTimer(IpcomOpContext *opContext, gint milliseconds, GSourceFunc func);
gboolean									IpcomOpContextCancelTimer(IpcomOpContext *opContext);

IpcomOpContext*								IpcomOpContextRef(IpcomOpContext *ctx);
void										IpcomOpContextUnref(IpcomOpContext *ctx);

#endif /* IPCOMMANDPROTOCOL_INCLUDE_IPCOMOPERATIONCONTEXT_H_ */
