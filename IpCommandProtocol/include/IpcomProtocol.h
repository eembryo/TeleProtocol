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

IpcomProtocol	*IpcomProtocolGetInstance();
gboolean	IpcomProtocolRegisterService(IpcomProtocol *, IpcomService *);
//gboolean	IpcomProtocolUnregisterService(IpcomProtocol *, guint serviceId);	//NOT implemented yet
gint		IpcomProtocolSendMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata);
gint		IpcomProtocolRepondMessage(IpcomProtocol *, const IpcomOpContextId *opContextId, IpcomMessage *mesg);
gint		IpcomProtocolHandleMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg);
gint		IpcomProtocolProcessDone(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomServiceReturn result);

G_END_DECLS
#endif
