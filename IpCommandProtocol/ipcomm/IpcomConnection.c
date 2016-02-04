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

#include <glib.h>
#include <IpcomConnection.h>
#include <IpcomProtocol.h>
#include <IpcomTransport.h>
#include <IpcomMessage.h>
#include <ref_count.h>
#include <dprint.h>

gint
IpcomConnectionTrnasmitMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	g_assert(conn->transport);
	conn->transport->transmit(conn->transport, conn, mesg);
	return 0;
}

static void _IpcomConnectionFree(struct ref *r)
{
	IpcomConnection *pConn = container_of(r, IpcomConnection, _ref);

	if (pConn->localSockAddr) g_object_unref(pConn->localSockAddr);
	if (pConn->remoteSockAddr) g_object_unref(pConn->remoteSockAddr);

	r->count = -1;
	g_free(pConn);
}

IpcomConnection *
IpcomConnectionRef(IpcomConnection *conn)
{
	ref_inc(&conn->_ref);
	return conn;
}

void
IpcomConnectionUnref(IpcomConnection *conn)
{
	ref_dec(&conn->_ref);
}

IpcomConnection *
IpcomConnectionNew(IpcomTransport *transport, GSocketAddress *localSockAddr, GSocketAddress *remoteSockAddr)
{
	IpcomConnection *conn;

	conn = g_malloc0(sizeof(IpcomConnection));
	if (localSockAddr)
		conn->localSockAddr = g_object_ref(localSockAddr);

	g_assert(remoteSockAddr);
	conn->remoteSockAddr = g_object_ref(remoteSockAddr);
	conn->protocol = IpcomProtocolGetInstance();
	conn->transport = transport;

	ref_init(&conn->_ref, _IpcomConnectionFree);

	return IpcomConnectionRef(conn);
}

gint
IpcomConnectionPushOutgoingMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomMessageRef(mesg);
	//if outgoing queue exists {
	//	insert message to the queue
	//} else {
	conn->transport->transmit(conn->transport, conn, mesg);
	IpcomMessageUnref(mesg);
	//}
	return 0;
}

gint
IpcomConnectionPushIncomingMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomMessageRef(mesg);
	//if incoming queue exists {
	//	insert message to the queue
	//} else {
	IpcomProtocolHandleMessage(conn->protocol, conn, mesg);
	IpcomMessageUnref(mesg);
	//}
	return 0;
}

#if 0
IpcomMessage *
IpcomConnectionPopOutgoingMessage(IpcomConnection *conn)
{
	//pop message from outgoing queue
	IpcomMessageUnref(mesg);
	return mesg;
}

IpcomMessage *
IpcomConnectionPopIncomingMessage(IpcomConnection *conn)
{
	//pop message from incoming queue
	IpcomMessageUnref(mesg)
	return mesg;
}
#endif
