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

#ifndef __IpcomProtocolh__
#define __IpcomProtocolh__


#include <glib.h>
#include <IpcomTypes.h>

G_BEGIN_DECLS

IpcomProtocol*		IpcomProtocolInit(GMainContext *gcontext);
IpcomProtocol*		IpcomProtocolGetInstance();
GMainContext *		IpcomProtocolGetMainContext();
gboolean			IpcomProtocolRegisterService(IpcomProtocol *, IpcomService *);
IpcomService*		IpcomProtocolLookupService(IpcomProtocol *,guint serviceId);
//gboolean	IpcomProtocolUnregisterService(IpcomProtocol *, guint serviceId);	//NOT implemented yet
/**
 * IpcomProtocolSendMessageFull
 * @ proto, conn, mesg :
 * @ OnRecvMessage :
 * @ OnOpCtxDestroy : After this function is successfully returned OpContext ID, callback function will be called while destroying the OpContext.
 * @ error : return error message if any.
 */
IpcomOpContextId*	IpcomProtocolSendMessageFull(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg,
												IpcomReceiveMessageCallback OnRecvMessage, IpcomOpCtxDestroyNotify OnOpCtxDestroy,
												gpointer userdata,
												GError **error);
/**
 * IpcomProtocolRespondMessageFull
 * @ proto :
 * @ opContextId :
 * @ mesg :
 * @ onOpCtxDestroy :the function, which will be called before OpContext is destroyed
 *
 * Send respond message to where OpContext indicates. If failed, return negative value.
 */
gint			IpcomProtocolRespondMessageFull(IpcomProtocol *, const IpcomOpContextId *opContextId, IpcomMessage *mesg,
												IpcomOpCtxDestroyNotify OnOpCtxDestroy, gpointer userdata);
gint			IpcomProtocolRespondError(IpcomProtocol *proto, const IpcomOpContextId *opContextId, guint8 ecode, guint16 einfo);

/**
 * IpcomProtocolLookupAndGetOpContext
 *
 * Find and return OpContext in hash table. Before caller finishes, it should decrease reference count using IpcomOpContextUnref().
 */
IpcomOpContext*	IpcomProtocolLookupAndGetOpContext(IpcomProtocol *proto, const IpcomOpContextId *opContextId);

gint			IpcomProtocolHandleMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg);

/**
 * deprecated API
 */
gint			IpcomProtocolSendMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata);
gint			IpcomProtocolRepondMessage(IpcomProtocol *, const IpcomOpContextId *opContextId, IpcomMessage *mesg);

/**
 * IpcomProtocolRegisterOpContext
 * IpcomProtocolUnregisterOpContext
 *
 * Add/Remove OpContext to hash table
 */
gboolean		IpcomProtocolRegisterOpContext(IpcomProtocol *proto, const IpcomOpContextId *pOpContextId, IpcomOpContext *opContext);
gboolean		IpcomProtocolUnregisterOpContext(IpcomProtocol *proto, const IpcomOpContextId *pOpContextId);
void			IpcomProtocolCancelOpContext(IpcomProtocol *proto, const IpcomOpContextId *pOpContextId);

G_END_DECLS
#endif
