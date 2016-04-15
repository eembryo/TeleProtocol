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
/*
struct _IpcomConnectionFlow {
    GSocketAddress*	pLocalSockAddr;
    GSocketAddress*	pRemoteSockAddr;
	guint			nProto;		// IPCOM_TRANSPORT_UDPV4, IPCOM_TRANSPORT_TCPV4
};
*/
struct _IpcomConnection {
	struct _IpcomConnectionFlow	_flow;
	guint16			proto;			// IPCOM_TRANSPORT_UDPV4, IPCOM_TRANSPORT_TCPV4
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
gint IpcomConnectionTransmitMessage(IpcomConnection *, IpcomMessage *);

static inline gboolean IpcomConnectionMatch(IpcomConnection* pConn, guint proto, GInetAddress *srcAddr, guint16 sport, GInetAddress *dstAddr, guint16 dport)
{
	return pConn->proto == proto &&
			g_inet_address_equal(g_inet_socket_address_get_address(pConn->localSockAddr), srcAddr) && g_inet_socket_address_get_port(pConn->localSockAddr) == sport &&
			g_inet_address_equal(g_inet_socket_address_get_address(pConn->remoteSockAddr), dstAddr) && g_inet_socket_address_get_port(pConn->remoteSockAddr) == dport ? TRUE : FALSE;
}
static inline gboolean IpcomConnectionEqual(IpcomConnection *a, IpcomConnection *b)
{
	return IpcomConnectionMatch(a, b->proto,
			g_inet_socket_address_get_address(b->localSockAddr), g_inet_socket_address_get_port(b->localSockAddr),
			g_inet_socket_address_get_address(b->remoteSockAddr),g_inet_socket_address_get_port(b->remoteSockAddr));
}
/*
static inline gboolean IpcomConnectionFlowEqual(IpcomConnectionFlow* pConnFlow, guint proto, GInetAddress *srcAddr, guint16 sport, GInetAddress *dstAddr, guint16 dport)
{
	if (pConnFlow->nProto != proto) return FALSE;
	else if (!g_inet_address_new_any(pConnFlow->pLocalAddr) && !g_inet_address_equal(pConnFlow->pLocalAddr, srcAddr)) return FALSE;
	else if (pConnFlow->nLocalPort != sport) return FALSE;
	else if (!g_inet_address_new_any(pConnFlow->pRemoteAddr) && !g_inet_address_equal(pConnFlow->pRemoteAddr, dstAddr)) return FALSE;
	else if (pConnFlow->nRemotePort != dport) return FALSE;

	return TRUE;
}
*/
G_END_DECLS

#endif
