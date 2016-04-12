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

#ifndef __IpcomTransportUDPv4_h__
#define __IpcomTransportUDPv4_h__


#include <IpcomTypes.h>
#include <ref_count.h>
#include <glib.h>

G_BEGIN_DECLS

IpcomTransport *IpcomTransportUDPv4New();
void IpcomTransportUDPv4Destroy(IpcomTransport *transport);
gint IpcomTransportGetIpv4Address(IpcomTransport *transport, const char *ifcName, guint *oAddr);
gboolean IpcomTransportUDPv4AttachGlibContext(IpcomTransport *transport, GMainContext *ctx);
gboolean IpcomTransportUDPv4DetachGlibContext(IpcomTransport *transport);

static gboolean _UDPv4Receive(IpcomTransport* transport, GSocket* socket) {
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(
			transport);
	GSocketAddress* sockaddr;
	IpcomConnection* conn;
	GError *gerror = NULL;
	IpcomMessage *newMesg = IpcomMessageNew(IPCOM_MESSAGE_MAX_SIZE);
	gssize length;
	struct msghdr msgh = { 0 };
	struct sockaddr_in remote;
	struct iovec iov;
	struct cmsghdr* cmsg;
	struct in_pktinfo* pktinfo;
	gchar control_buf[100];
	IpcomConnectionFlow flow;
	;
	newMesg = IpcomMessageNew(IPCOM_MESSAGE_MAX_SIZE);
	//pktinfo control message
	iov.iov_base = IpcomMessageGetRawData(newMesg);
	iov.iov_len = IPCOM_MESSAGE_MAX_SIZE;
	msgh.msg_name = &remote;
	msgh.msg_namelen = sizeof(struct sockaddr_in);
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = control_buf;
	msgh.msg_controllen = sizeof(control_buf);
	msgh.msg_flags = 0;
	length = recvmsg(g_socket_get_fd(socket), &msgh, 0);
	if (length == -1) {
	}
	for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
			cmsg = CMSG_NXTHDR(&msgh, cmsg)) {
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
			pktinfo = CMSG_DATA(cmsg);
			flow.nProto = IPCOM_TRANSPORT_UDPV4;
			flow.pLocalAddr = g_inet_address_new_from_bytes(pktinfo->ipi_addr,
					G_SOCKET_FAMILY_IPV4);
			flow.nLocalPort = g_inet_socket_address_get_port(
					udpTransport->boundSockAddr);
			flow.pRemoteAddr = remote.sin_addr;
			flow.nRemotePort = remote.sin_port;
		}
	}
	IpcomMessageSetLength(newMesg, length);
	conn = (IpcomConnection*) g_hash_table_lookup(udpTransport->connHash,
			sockaddr);
	if (!conn && transport->onNewConn) { //try to accept new connection
		conn = IpcomConnectionNew(transport, NULL, sockaddr);
		g_assert(conn);
		if (!transport->onNewConn(conn, transport->onNewConn_data)) {
			IpcomConnectionUnref(conn);
			conn = NULL;
		} else {
			DPRINT("Accept new connection.\n");
			g_hash_table_insert(udpTransport->connHash, sockaddr, conn);
		}
	}
	if (conn) {
		DPRINT("Received data from known connection.\n");
		IpcomConnectionPushIncomingMessage(conn, newMesg);
	} else {
		DPRINT("Got new connection but denied it.\n");
	}
	IpcomMessageUnref(newMesg);
	g_object_unref(sockaddr);
	return TRUE;
	_UDPv4Receive_failed: if (newMesg)
		IpcomMessageUnref(newMesg);
}

G_END_DECLS

#endif
