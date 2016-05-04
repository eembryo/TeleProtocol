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
#include <gio/gio.h>

#include <netinet/in.h>	//struct sockaddr_in
#include <string.h>		//memcmp

#include <ref_count.h>
#include <dprint.h>
#include <IpcomMessage.h>
#include <IpcomProtocol.h>
#include <IpcomTransport.h>
#include <IpcomTransportUDPv4.h>
#include <IpcomConnection.h>
#include <IpcomService.h>
#include <SocketUtils.h>
#include <arpa/inet.h>

#include <IpcomNetifcMonitor.h>

#include <netinet/ip.h>
#include <errno.h>

#define UDPV4TRANSPORT_FROM_TRANSPORT(ptr)	container_of(ptr, struct _IpcomTransportUDPv4, _transport)
#define TRANSPORT_FROM_UDPv4TRANSPORT(ptr)	(struct _IpcomTransport *)(&ptr->_transport)
#define MAX_CMSG_SIZE 256

#define IPV4_ANYCAST_ADDRESS gen_anycast_address()
static GInetAddress* gen_anycast_address() {
	static GInetAddress* anycast_addr = NULL;
	if (!anycast_addr) anycast_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	return anycast_addr;
}

struct _IpcomTransportUDPv4 {
	IpcomTransport		_transport;
	GSocketAddress*		boundSockAddr;
	GHashTable*			connHash;
	GMainContext*		mainContext;
	GSource*			socketSource;
	IpcomConnection*	pBroadConn;		//dummy connection, which means broadcasting
	GList*				listBroadAddrs;
};

static gboolean
_UDPv4Receive(IpcomTransport *transport, GSocket *socket)
{
	IpcomTransportUDPv4* udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GSocketAddress*		pRemoteSockAddr = NULL;
	GSocketAddress*		pLocalSockAddr = NULL;
	IpcomMessage*		newMesg = NULL;
	gssize				length;
	IpcomConnectionFlow	flow;
	IpcomConnection*	pConn;

	struct msghdr		msgh = {0};
	struct iovec		iov[1];
	guint8				cmsg_buf[MAX_CMSG_SIZE] = {0};
	struct cmsghdr*		pcmsgh;
	struct sockaddr_in	mRemoteSockAddr;
	struct sockaddr_in	mLocalSockAddr;
	struct in_pktinfo*	ppktinfo;

	DFUNCTION_START;

	newMesg = IpcomMessageNew(IPCOM_MESSAGE_MAX_SIZE);

	iov[0].iov_base = IpcomMessageGetRawData(newMesg);
	iov[0].iov_len = IPCOM_MESSAGE_MAX_SIZE;

	msgh.msg_name = (void *)&mRemoteSockAddr;
	msgh.msg_namelen = sizeof(struct sockaddr_in);
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = cmsg_buf;
	msgh.msg_controllen = sizeof(cmsg_buf);
	msgh.msg_flags = 0;

	/* Receive a message with auxiliary data */
	length = recvmsg (g_socket_get_fd(transport->socket), &msgh, 0);
	if (length == -1) {
		DERROR("%s", strerror(errno));
		IpcomMessageUnref(newMesg);
		return FALSE;
	}

	IpcomMessageSetLength(newMesg, length);

	/* Receive auxiliary data in msgh */
	for (pcmsgh = CMSG_FIRSTHDR(&msgh); pcmsgh != NULL; pcmsgh = CMSG_NXTHDR(&msgh, pcmsgh)) {
		//DPRINT("cmsg level and type = %d, %d\n", pcmsgh->cmsg_level, pcmsgh->cmsg_type);
		if (pcmsgh->cmsg_level == IPPROTO_IP && pcmsgh->cmsg_type == IP_PKTINFO && pcmsgh->cmsg_len) {
			ppktinfo = (struct in_pktinfo *)CMSG_DATA(pcmsgh);
			mLocalSockAddr.sin_family = AF_INET;
			mLocalSockAddr.sin_port = g_htons(g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr)));
			memcpy(&mLocalSockAddr.sin_addr, &ppktinfo->ipi_addr, sizeof(struct in_addr));
			break;
		}
	}

	/* Check the packet is BROADCASTING or UNICASTING */
	pConn = NULL;
	pLocalSockAddr = g_socket_address_new_from_native((gpointer)&mLocalSockAddr, sizeof(struct sockaddr_in));
	pRemoteSockAddr = g_socket_address_new_from_native((gpointer)&mRemoteSockAddr, sizeof(struct sockaddr_in));
#ifdef DEBUG
	{
		gchar*	dst = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pLocalSockAddr)));
		guint16	dport = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pLocalSockAddr));
		gchar* 	src = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pRemoteSockAddr)));
		guint16	sport = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pRemoteSockAddr));

		DINFO("Got a packet: <%s:%d> --> <%s:%d>\n", src,sport,dst,dport);
		g_free(dst);
		g_free(src);

	}
#endif //DEBUG
	switch(IpcomNetifcMonitorQueryAddressType(IpcomNetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(pLocalSockAddr)))) {
	case ADDRESSTYPE_IPV4_UNICAST:
		/* Set temporary connection information */
		flow.pLocalSockAddr = pLocalSockAddr;
		flow.pRemoteSockAddr = pRemoteSockAddr;
		flow.nProto = IPCOM_TRANSPORT_UDPV4;

		/* Lookup connection in hash table. key value is IpcomConnectionFlow* */
		pConn = (IpcomConnection *)g_hash_table_lookup(udpTransport->connHash, &flow);

		if (!pConn && transport->onNewConn) {
			pConn = IpcomConnectionNew(transport, IPCOM_TRANSPORT_UDPV4, pLocalSockAddr, pRemoteSockAddr);
			g_assert(pConn);
			if (!transport->onNewConn(pConn, transport->onNewConn_data)) {
				IpcomConnectionUnref(pConn);
				pConn = NULL;
			}
			else {
				DPRINT("Adding accepted connection.\n");
				g_hash_table_insert(udpTransport->connHash, IpcomConnectionGetFlow(pConn), pConn);
			}
		}
		break;
	case ADDRESSTYPE_IPV4_BROADCAST:
		/* if broadcast packet is sent by me, ignore it */
		if (IpcomNetifcMonitorQueryAddressType(	IpcomNetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(pRemoteSockAddr)))
				== ADDRESSTYPE_IPV4_UNICAST)
			break;
		else pConn = udpTransport->pBroadConn;
		break;
	default:
		DWARN("Receive a packet with wrong destination.\n");
		goto _UDPv4Receive_failed;
	}

	if (pConn) IpcomConnectionPushIncomingMessage(pConn, newMesg);
	else DPRINT("Discarding the packet.\n");

	IpcomMessageUnref(newMesg);
	if (pRemoteSockAddr) g_object_unref(pRemoteSockAddr);
	if (pLocalSockAddr) g_object_unref(pLocalSockAddr);
	return TRUE;

	_UDPv4Receive_failed:
	IpcomMessageUnref(newMesg);
	if (pRemoteSockAddr) g_object_unref(pRemoteSockAddr);
	if (pLocalSockAddr) g_object_unref(pLocalSockAddr);
	return FALSE;
}

static gboolean
_UDPv4CheckSocket(GSocket *socket, GIOCondition cond, gpointer data)
{
	IpcomTransportUDPv4 *udpTransport = (IpcomTransportUDPv4 *)data;

	DFUNCTION_START;

	if (cond & G_IO_IN) {
		udpTransport->_transport.onReadyToReceive(&udpTransport->_transport, socket);
	}
	else {	//probably G_IO_ERR or G_IO_HUP
		return FALSE;
	}

	return TRUE;	//continue to receive
}

static gboolean
_UDPv4Bind(IpcomTransport *transport, const gchar *localIp, guint16 localPort)
{
	GError *gerror=NULL;
	GSocketAddress *sockaddr;
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);

	DFUNCTION_START;

	g_assert(transport->socket);

	if (localIp == NULL || !strcmp(localIp, "0.0.0.0")) {
		sockaddr = g_inet_socket_address_new(IPV4_ANYCAST_ADDRESS, localPort);
	}
	else {
		sockaddr = g_inet_socket_address_new_from_string(localIp, localPort);
	}
	g_assert(sockaddr);

	if (!g_socket_bind(transport->socket, sockaddr, TRUE, &gerror)) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);

		// [TBD] Should be removed after adding the init/deinit functions
		return FALSE;
		//g_assert(FALSE);
	}

	udpTransport->boundSockAddr = sockaddr;
	return TRUE;
}

static IpcomConnection *
_UDPv4Connect(IpcomTransport *transport, const gchar *remoteIp, guint16 remotePort)
{
	IpcomTransportUDPv4*	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	IpcomConnection*		pConn;
	GSocketAddress*			pRemoteSockAddr = NULL;
	GSocketAddress* 		pLocalSockAddr = NULL;
	IpcomConnectionFlow		flow;
	GError*					gerror = NULL;

	DFUNCTION_START;

	/* Make socket address for remote */
	pRemoteSockAddr = g_inet_socket_address_new_from_string(remoteIp, remotePort);
	g_assert(pRemoteSockAddr);

	/* If the socket is bound to anycast address, we choose a source address for the 'remoteIp'.
	 * If not, use bound socket address.
	 */
	if (g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr)), IPV4_ANYCAST_ADDRESS)) {
		struct sockaddr_in		mRemoteSockAddr;
		struct sockaddr_in		mLocalSockAddr;

		mLocalSockAddr.sin_family = AF_INET;
		mLocalSockAddr.sin_port = g_htons(g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr)));

		g_socket_address_to_native (pRemoteSockAddr, &mRemoteSockAddr, sizeof(struct sockaddr_in), &gerror);
		if (gerror) {
			DERROR("%s\n", gerror->message);
			g_error_free(gerror);
			return NULL;
		}
		/// Lookup source address to use for remoteIP
		if (QuerySrcIpv4AddrForDst(&mRemoteSockAddr.sin_addr, &mLocalSockAddr.sin_addr) == -1) {
			DERROR("Cannot reach to destination.\n");
			return NULL;
		}

		pLocalSockAddr = g_socket_address_new_from_native((gpointer)&mLocalSockAddr, sizeof(struct sockaddr_in));
	} else
		pLocalSockAddr = g_object_ref(G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr));

	/* Look up that the connection flow already exists or not. */
	flow.nProto = IPCOM_TRANSPORT_UDPV4;
	flow.pLocalSockAddr = pLocalSockAddr;
	flow.pRemoteSockAddr = pRemoteSockAddr;
	if (!g_hash_table_contains(udpTransport->connHash, &flow)) {
		pConn = IpcomConnectionNew(transport, IPCOM_TRANSPORT_UDPV4, pLocalSockAddr, pRemoteSockAddr);
		g_assert(pConn);
		g_hash_table_insert(udpTransport->connHash, IpcomConnectionGetFlow(pConn), pConn);
	}
	else {
		DWARN("Connection already exists.\n");
	}

	if (pRemoteSockAddr) g_object_unref(pRemoteSockAddr);
	if (pLocalSockAddr) g_object_unref(pLocalSockAddr);

	return pConn;
}


static gint
_UDPv4NativeSend(gint sockFd, struct sockaddr_in* local, struct sockaddr_in* remote, IpcomMessage *mesg)
{
	struct iovec	iov[2];
	struct msghdr	msgh = {0};
	struct cmsghdr*	pcmsgh;
	guint8			cmsg_buf[MAX_CMSG_SIZE] = {0};
	struct in_pktinfo*	ppktinfo;

	g_assert(remote);

	/* set iov for sending packet*/
	iov[0].iov_base = mesg->vccpdu_ptr;
	iov[0].iov_len = VCCPDUHEADER_SIZE;
	iov[1].iov_base = mesg->payload_ptr;
	iov[1].iov_len = IpcomMessageGetPaylodLength(mesg);

	/* fill msghdr for sendmsg() */
	msgh.msg_name = remote;				// destination socket address
	msgh.msg_namelen = sizeof(struct sockaddr_in);
	msgh.msg_iov = iov;								// packet data
	msgh.msg_iovlen = 2;

	if (local) {
		msgh.msg_control = &cmsg_buf;					// set control message buffer
		msgh.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));

		/* fill control message for PKTINFO */
		pcmsgh = CMSG_FIRSTHDR(&msgh);
		pcmsgh->cmsg_level = IPPROTO_IP;
		pcmsgh->cmsg_type = IP_PKTINFO;
		pcmsgh->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

		/* fill PKTINFO structure */
		ppktinfo = (struct in_pktinfo*)CMSG_DATA(pcmsgh);
		memcpy(&ppktinfo->ipi_spec_dst, &local->sin_addr, sizeof(struct in_addr));	// specify source IPv4 address for the packet
	}

	/* Send the packet */
	if ( sendmsg(sockFd, &msgh, 0) < 0 ) {
		DERROR("%s\n", strerror(errno));
		goto _UDPv4NativeSend_failed;
	}
#ifdef DEBUG
	{
		gchar src[30], dst[30];
		guint16 sport, dport;
		if (local) {
			inet_ntop(AF_INET, &local->sin_addr, src, 30);
			sport = g_ntohs(local->sin_port);
		}
		else {
			strcpy(src, "0.0.0.0");
			sport = 0;
		}
		inet_ntop(AF_INET, &remote->sin_addr, dst, 30);
		dport = g_ntohs(remote->sin_port);
		DINFO("Send a packet: <%s:%d> --> <%s:%d>\n", src, sport, dst, dport);
	}
#endif
	return IpcomMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE;

	_UDPv4NativeSend_failed:
	return -1;
}

static gint
_UDPv4Broadcast(IpcomTransport *transport, guint16 dport, IpcomMessage *mesg)
{
	IpcomTransportUDPv4*	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GInetAddress*		pBoundInetAddress;
	GInetAddress*		pBroadcastAddress;
	GList*				listBroadcastAddrs;
	GList* 				iter;
	struct sockaddr_in	mRemoteSockAddr = {0};

	if (!udpTransport->boundSockAddr) {
		DERROR("UDPv4 transport is not bound.\n");
		return -1;
	}

	mRemoteSockAddr.sin_family = AF_INET;
	mRemoteSockAddr.sin_port = g_htons(dport);
	pBoundInetAddress = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr));

	/// If socket is bound to anycast address, we broadcast for each broadcast address.
	if (g_inet_address_equal(pBoundInetAddress, IPV4_ANYCAST_ADDRESS)) {
		listBroadcastAddrs = IpcomNetifMonitorGetAllBroadcastAddress(IpcomNetifcGetInstance());
		if (!listBroadcastAddrs) goto _UDPv4Broadcast_failed;

		for (iter = g_list_first(listBroadcastAddrs); iter != NULL; iter = g_list_next(iter)) {
			if (g_inet_address_get_family((GInetAddress*)iter->data) == G_SOCKET_FAMILY_IPV4) {
				memcpy(&mRemoteSockAddr.sin_addr, g_inet_address_to_bytes(iter->data), sizeof(struct in_addr));
				_UDPv4NativeSend(g_socket_get_fd (transport->socket), NULL, &mRemoteSockAddr, mesg);
			}
		}
		g_list_free(listBroadcastAddrs);
	}
	else { /// socket is bound to unicast address
		pBroadcastAddress = IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcGetInstance(), pBoundInetAddress);
		if (!pBroadcastAddress) goto _UDPv4Broadcast_failed;

		memcpy(&mRemoteSockAddr.sin_addr, g_inet_address_to_bytes(pBroadcastAddress), sizeof(struct in_addr));
		_UDPv4NativeSend(g_socket_get_fd (transport->socket), NULL, &mRemoteSockAddr, mesg);
	}

	return IpcomMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE;

	_UDPv4Broadcast_failed:
	return -1;
}

static gint
_UDPv4Transmit(IpcomTransport *transport, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomTransportUDPv4*	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GError *gerror=NULL;
	struct sockaddr_in	mRemoteSockAddr;
	struct sockaddr_in	mLocalSockAddr;

	g_assert(udpTransport->boundSockAddr);

	if (IpcomConnectionIsBroadcast(conn)) {
		guint16 dport = IpcomConnectionGetBroadConnectionPort(conn);
		return dport > 0 ?
				_UDPv4Broadcast(transport, dport, mesg) :
				_UDPv4Broadcast(transport, g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(udpTransport->boundSockAddr)), mesg);
	}
	else {
		/* convert Local/Remote GSocketAddress to native 'struct sockaddr_in' */
		g_socket_address_to_native (IpcomConnectionGetLocalSockAddr(conn), &mLocalSockAddr, sizeof(struct sockaddr_in), &gerror);
		if (gerror)	goto _UDPv4Transmit_failed;
		g_socket_address_to_native (IpcomConnectionGetRemoteSockAddr(conn), &mRemoteSockAddr, sizeof(struct sockaddr_in), &gerror);
		if (gerror)	goto _UDPv4Transmit_failed;

		return _UDPv4NativeSend(g_socket_get_fd (transport->socket), &mLocalSockAddr, &mRemoteSockAddr, mesg);
	}
	_UDPv4Transmit_failed:
	if (gerror) {
		DWARN("%s", gerror->message);
		g_error_free(gerror);
	}
	return -1;
}

static gboolean
_UDPv4Listen(IpcomTransport *transport, gint backlog)
{
	DFUNCTION_START;

	return TRUE;
}

static IpcomConnection*
_UDPv4GetBroadConnection(IpcomTransport *transport)
{
	IpcomTransportUDPv4*	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	return udpTransport->pBroadConn;
}
#if 0
static IpcomConnection *
_UDPv4LookupConnection(IpcomTransport *transport, const gchar *localIpAddr, guint16 localPort, const gchar *remoteIpAddr, guint16 remotePort)
{
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GSocketAddress *sockaddr = NULL;
	IpcomConnection *conn;

	sockaddr = g_inet_socket_address_new_from_string(remoteIpAddr, remotePort);
	g_assert(sockaddr);

	conn = (IpcomConnection *)g_hash_table_lookup(udpTransport->connHash, sockaddr);
	g_object_unref(sockaddr);

	return conn;
}
#endif

static IpcomTransport udpv4 = {
		.type = IPCOM_TRANSPORT_UDPV4,
		.transmit = _UDPv4Transmit,
		.bind = _UDPv4Bind,
		.connect = _UDPv4Connect,
		.onReadyToReceive = _UDPv4Receive,
		.listen = _UDPv4Listen,
		.getBroadConnection = _UDPv4GetBroadConnection,
//		.lookup = _UDPv4LookupConnection,
};

//key is assumed to be IpcomConnectionFlow
static guint
_UDPv4ConnHashFunc(gconstpointer key)
{
	IpcomConnectionFlow *pFlow = (IpcomConnectionFlow *)key;
	struct sockaddr_in skaddr;
	GError *gerror = NULL;

	g_socket_address_to_native(pFlow->pRemoteSockAddr, &skaddr, sizeof(struct sockaddr_in), &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_assert(FALSE);
	}

	return (guint)(skaddr.sin_addr.s_addr + skaddr.sin_port);
}

static gboolean
_UDPv4ConnEqual(gconstpointer a, gconstpointer b)
{
	return IpcomConnectionFlowEqual((IpcomConnectionFlow*)a, (IpcomConnectionFlow*)b);
}

gboolean
IpcomTransportUDPv4AttachGlibContext(IpcomTransport *transport, GMainContext *ctx)
{
	IpcomTransportUDPv4 *udpTransport;
	GSource *source;

	if (!transport  || !ctx) {
		return FALSE;
	}
	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	if (!udpTransport->_transport.socket)
		return FALSE;

	udpTransport->mainContext = ctx;
	source = g_socket_create_source (udpTransport->_transport.socket, G_IO_ERR|G_IO_HUP|G_IO_IN, NULL);
	g_source_set_callback(source, (GSourceFunc)_UDPv4CheckSocket, udpTransport, NULL);
	g_source_attach(source, udpTransport->mainContext);
	udpTransport->socketSource = source;

	return TRUE;
}

gboolean
IpcomTransportUDPv4DetachGlibContext(IpcomTransport *transport)
{
	IpcomTransportUDPv4 *udpTransport;

	udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);

	if (!udpTransport->socketSource) return TRUE;

	g_source_destroy(udpTransport->socketSource);
	g_source_unref(udpTransport->socketSource);
	udpTransport->socketSource = NULL;

	return TRUE;
}

IpcomTransport *
IpcomTransportUDPv4New()
{
	IpcomTransportUDPv4 *new;
	GError *gerror=NULL;

	new = g_malloc0(sizeof(IpcomTransportUDPv4));
	g_assert(new);

	//initialize IpcomTransport structure
	new->_transport = udpv4;
	new->_transport.socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_DEFAULT, &gerror);
	if (!new->_transport.socket) goto _UDPv4New_failed;
	if (!g_socket_set_option(new->_transport.socket, IPPROTO_IP, IP_PKTINFO, 1, &gerror)) {
		DWARN("%s\n", gerror->message);
		g_error_free(gerror);
		goto _UDPv4New_failed;
	}
	new->_transport.protocol = IpcomProtocolGetInstance();

	//initialize IpcomTransportUDPv4 structure
	new->boundSockAddr = NULL;
	/*
	 * connection hash
	 * key: IpcomConnection*
	 * value: IpcomConnection*
	 */
	new->connHash = g_hash_table_new(_UDPv4ConnHashFunc, _UDPv4ConnEqual);
	new->mainContext = NULL;
	new->socketSource = NULL;
	///enable broadcasting
	new->pBroadConn = IpcomConnectionNew(TRANSPORT_FROM_UDPv4TRANSPORT(new), 0, NULL, NULL);
	g_socket_set_broadcast(new->_transport.socket, TRUE);

	if (gerror) g_error_free(gerror);

	//Update Network Information
	IpcomNetifcMonitorUpdate(IpcomNetifcGetInstance());

	return &new->_transport;

	_UDPv4New_failed:
	return NULL;
}

void
IpcomTransportUDPv4Destroy(IpcomTransport *transport)
{
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GError *gerror = NULL;

	if (udpTransport->mainContext) {
		IpcomTransportUDPv4DetachGlibContext(transport);
	}

	if (transport->socket) {
		g_socket_close(transport->socket, &gerror);
		if (gerror) {
			DERROR("%s\n", gerror->message);
			g_assert(FALSE);
			g_error_free(gerror);
			gerror = NULL;
		}
	}
	if (udpTransport->socketSource) g_source_unref(udpTransport->socketSource);
	if (udpTransport->connHash) g_hash_table_destroy(udpTransport->connHash);
	if (udpTransport->boundSockAddr) g_object_unref(udpTransport->boundSockAddr);
	if (udpTransport->pBroadConn) IpcomConnectionUnref(udpTransport->pBroadConn);

	g_free(udpTransport);
}


IpcomConnection *
IpcomTransportUDPv4GetBroadConnection(IpcomTransportUDPv4 *udpTransport)
{
	return udpTransport->pBroadConn;
}
