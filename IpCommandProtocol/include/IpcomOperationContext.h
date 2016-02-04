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

struct _IpcomOpContextId
{
	guint32				senderHandleId;
	IpcomConnection		*connection;
} __attribute__ ((packed));

struct _IpcomOpContext
{
	struct _IpcomOpContextId			ctxId;
	IpcomReceiveMessageCallback			recvCallback;
	void 								*cb_data;
	gint								opType;
	gint								status;
	gint								serviceId;
	IpcomMessage						*message;
	//timer
};

static inline IpcomConnection *				IpcomOpContextGetConnection(IpcomOpContext *ctx)
{ return ctx->ctxId.connection; }
static inline guint32 						IpcomOpContextGetSenderHandleID(const IpcomOpContext *ctx)
{ return ctx->ctxId.senderHandleId;}
static inline gint 							IpcomOpContextGetType(const IpcomOpContext *ctx)
{ return ctx->opType; }
static inline gint							IpcomOpContextGetStatus(const IpcomOpContext *ctx)
{ return ctx->status; }
static inline IpcomReceiveMessageCallback	IpcomOpContextGetRecvCallback(const IpcomOpContext *ctx)
{ return ctx->recvCallback;}
static inline void *						IpcomOpContextGetRecvCallbackData(const IpcomOpContext *ctx)
{ return ctx->cb_data;}
static inline const IpcomOpContextId *		IpcomOpContextGetContextId(IpcomOpContext *ctx)
{ return &ctx->ctxId; }
static inline guint							IpcomOpContextIdHashFunc(const gconstpointer key)
{ return ((struct _IpcomOpContextId *)key)->senderHandleId; }

gboolean 									IpcomOpContextIdEqual(gconstpointer aOpContextId, gconstpointer bOpContextId);
IpcomOpContext *							IpcomOpContextCreate(IpcomConnection *conn, guint32 senderHandleId, guint opType,	IpcomReceiveMessageCallback recv_cb, void *userdata);
void										IpcomOpContextDestroy(IpcomOpContext *ctx);
void										IpcomOpContextSetMessage(IpcomOpContext *ctx, IpcomMessage *mesg);
gboolean									IpcomOpContextSetStatus(IpcomOpContext *ctx, gint status);

#endif /* IPCOMMANDPROTOCOL_INCLUDE_IPCOMOPERATIONCONTEXT_H_ */
