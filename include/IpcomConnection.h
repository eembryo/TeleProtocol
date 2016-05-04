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

typedef struct _IpcomConnectionFlow IpcomConnectionFlow;

struct _IpcomConnectionFlow {
    GSocketAddress*		pLocalSockAddr;
    GSocketAddress*		pRemoteSockAddr;
    IpcomTransportType	nProto;		// IPCOM_TRANSPORT_UDPV4, IPCOM_TRANSPORT_TCPV4
};

struct _IpcomConnection {
	struct _IpcomConnectionFlow	_flow;
	GSocket			*socket;
	IpcomProtocol	*protocol;
	IpcomTransport	*transport;
	//<-- Implement incomming queue
	//<-- Implement outgoing queue
	struct ref		_ref;
};

IpcomConnection*		IpcomConnectionNew(IpcomTransport *transport, IpcomTransportType proto, GSocketAddress *localSockAddr, GSocketAddress *remoteSockAddr);
IpcomConnection*		IpcomConnectionRef(IpcomConnection *conn);
void 					IpcomConnectionUnref(IpcomConnection *conn);
gint 					IpcomConnectionPushOutgoingMessage(IpcomConnection *, IpcomMessage *);
gint 					IpcomConnectionPushIncomingMessage(IpcomConnection *, IpcomMessage *);
gint 					IpcomConnectionTransmitMessage(IpcomConnection *, IpcomMessage *);

static inline IpcomConnectionFlow*	IpcomConnectionGetFlow(IpcomConnection *conn)
{ return &conn->_flow; }
static inline GSocketAddress*		IpcomConnectionGetLocalSockAddr(IpcomConnection *conn)
{ return conn->_flow.pLocalSockAddr; }
static inline GSocketAddress*		IpcomConnectionGetRemoteSockAddr(IpcomConnection *conn)
{ return conn->_flow.pRemoteSockAddr; }
static inline gboolean				IpcomConnectionFlowEqual(const IpcomConnectionFlow *a, const IpcomConnectionFlow *b) {
	return a->nProto == b->nProto &&
			g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(a->pLocalSockAddr)), g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(b->pLocalSockAddr))) &&
			g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(a->pRemoteSockAddr)), g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(b->pRemoteSockAddr))) &&
			g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(a->pLocalSockAddr)) == g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(b->pLocalSockAddr)) &&
			g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(a->pRemoteSockAddr)) == g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(b->pRemoteSockAddr));
}
static inline gboolean				IpcomConnectionIsBroadcast(const IpcomConnection *conn)
{ return conn->_flow.pLocalSockAddr == NULL ? TRUE : FALSE; }
static inline gboolean				IpcomConnectionSetBroadConnectionPort(IpcomConnection *conn, guint16 dport) {
	if (!IpcomConnectionIsBroadcast(conn)) return FALSE;
	conn->_flow.pRemoteSockAddr = (GSocketAddress *)GINT_TO_POINTER(dport);
	return TRUE;
}
static inline gint					IpcomConnectionGetBroadConnectionPort(IpcomConnection *conn) {
	if (!IpcomConnectionIsBroadcast(conn)) return -1;
	return GPOINTER_TO_INT(conn->_flow.pRemoteSockAddr);
}

//deprecated
#if 0
static inline gboolean IpcomConnectionMatch(IpcomConnection* pConn, guint proto, GInetAddress *srcAddr, guint16 sport, GInetAddress *dstAddr, guint16 dport)
{
	return pConn->proto == proto &&
			g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pConn->localSockAddr)), srcAddr) && g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pConn->localSockAddr)) == sport &&
			g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pConn->remoteSockAddr)), dstAddr) && g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pConn->remoteSockAddr)) == dport
			? TRUE : FALSE;
}
static inline gboolean IpcomConnectionEqual(IpcomConnection *a, IpcomConnection *b)
{
	return IpcomConnectionMatch(a, b->proto,
			g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(b->localSockAddr)), g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(b->localSockAddr)),
			g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(b->remoteSockAddr)), g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(b->remoteSockAddr)));
}
#endif
G_END_DECLS

#endif
