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

#ifndef __IpcomConnection__h__
#define __IpcomConnection__h__

#include <glib.h>
#include <gio/gio.h>
#include <IpcomTypes.h>
#include <ref_count.h>

G_BEGIN_DECLS

struct _IpcomConnection {
	GSocketAddress	*localSockAddr;
	GSocketAddress	*remoteSockAddr;
	IpcomProtocol	*protocol;
	IpcomTransport	*transport;
	//<-- Implement incomming queue
	//<-- Implement outgoing queue
	struct ref		_ref;
};

IpcomConnection *IpcomConnectionNew(IpcomTransport *transport, GSocketAddress *localSockAddr, GSocketAddress *remoteSockAddr);
IpcomConnection *IpcomConnectionRef(IpcomConnection *conn);
void IpcomConnectionUnref(IpcomConnection *conn);
gint IpcomConnectionPushOutgoingMessage(IpcomConnection *, IpcomMessage *);
gint IpcomConnectionPushIncomingMessage(IpcomConnection *, IpcomMessage *);

G_END_DECLS

#endif
