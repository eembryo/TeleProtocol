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

#define UDPV4TRANSPORT_FROM_TRANSPORT(ptr)	container_of(ptr, struct _IpcomTransportUDPv4, _transport)
#define TRANSPORT_FROM_UDPv4TRANSPORT(ptr)	(struct _IpcomTransport *)(&ptr->_transport)

struct _IpcomTransportUDPv4 {
	IpcomTransport _transport;
//	IpcomService	*service;
	GSocketAddress	*boundSockAddr;
	GHashTable		*connHash;
	GMainContext	*mainContext;
	GSource			*socketSource;
};

static gboolean
_UDPv4Receive(IpcomTransport *transport, GSocket *socket)
{
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GSocketAddress	*sockaddr;
	GError			*gerror=NULL;
	IpcomMessage	*newMesg = IpcomMessageNew(IPCOM_MESSAGE_MAX_SIZE);
	gssize			length;
	IpcomConnection *conn;

	DFUNCTION_START;

	newMesg = IpcomMessageNew(IPCOM_MESSAGE_MAX_SIZE);

	//sockaddr gets source address of the received packet
	length = g_socket_receive_from(socket, &sockaddr, newMesg->message, newMesg->actual_size, NULL, &gerror);
	if (gerror) {
		DERROR("%s", gerror->message);
		IpcomMessageUnref(newMesg);
		g_error_free(gerror);
		return FALSE;
	}

	conn = (IpcomConnection *)g_hash_table_lookup(udpTransport->connHash, sockaddr);
	if (!conn && transport->onNewConn) { //try to accept new connection
		conn = IpcomConnectionNew(transport, NULL, sockaddr);
		g_assert(conn);
		if (!transport->onNewConn(conn, transport->onNewConn_data)) {
			IpcomConnectionUnref(conn);
			conn = NULL;
		}
		else {
			DPRINT("Accept new connection.\n");
			g_hash_table_insert(udpTransport->connHash, sockaddr, conn);
		}
	}
	if (conn) {
		DPRINT("Received data from known connection.\n");
		IpcomConnectionPushIncomingMessage(conn, newMesg);
	}
	else {
		DPRINT("Got new connection but denied it.\n");
	}

	IpcomMessageUnref(newMesg);
	g_object_unref(sockaddr);

	return TRUE;
}

static gboolean
_UDPv4CheckSocket(GSocket *socket, GIOCondition cond, gpointer data)
{
	IpcomTransportUDPv4 *udpTransport = (IpcomTransportUDPv4 *)data;
	GError				*gerror=NULL;

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

	if (localIp == NULL) {
		GInetAddress *anyAddr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
		sockaddr = g_inet_socket_address_new(anyAddr, localPort);
		g_object_unref(anyAddr);
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
	IpcomConnection *pConn;
	GSocketAddress *remoteSockAddr;

	DFUNCTION_START;

	remoteSockAddr = g_inet_socket_address_new_from_string(remoteIp, remotePort);
	g_assert(remoteSockAddr);

	pConn = IpcomConnectionNew(transport, NULL, remoteSockAddr);
	g_assert(pConn);
	{
		//add IpcomConnection to hash table of IpcomTransportUDPv4
		IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
		gpointer test = g_hash_table_lookup(udpTransport->connHash, remoteSockAddr);
		g_assert(!test);	//IMPLEMENT: If test exists, try to remove connection.
		g_hash_table_insert(udpTransport->connHash, remoteSockAddr, pConn);
	}

	return pConn;
}

static gint
_UDPv4Transmit(IpcomTransport *transport, IpcomConnection *conn, IpcomMessage *mesg)
{
	GError *gerror=NULL;
	gssize sent_bytes;
	GOutputVector msg_vector[2];

	msg_vector[0].buffer = mesg->vccpdu_ptr;
	msg_vector[0].size = VCCPDUHEADER_SIZE;
	msg_vector[1].buffer = mesg->payload_ptr;
	msg_vector[1].size = IpcomMessageGetPaylodLength(mesg);

	sent_bytes = g_socket_send_message(transport->socket, conn->remoteSockAddr, msg_vector, 2, NULL, 0, G_SOCKET_MSG_NONE, NULL, &gerror);
	//sent_bytes = g_socket_send_to(transport->socket, conn->remoteSockAddr, IpcomMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE, mesg->length, NULL, &gerror);

	if (sent_bytes != IpcomMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE) {
		if (gerror) {
			DERROR("%s\n", gerror->message);
			g_error_free(gerror);
			return -1;
		}
		DWARN("Only part of IpcomMessage was sent.(%d/%d)\n", (int)sent_bytes, IpcomMessageGetPacketSize(mesg));
	}

	return sent_bytes;
}

static gboolean
_UDPv4Listen(IpcomTransport *transport, gint backlog)
{
	DFUNCTION_START;

	return TRUE;
}

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

static IpcomTransport udpv4 = {
		.type = IPCOM_TRANSPORT_UDPV4,
		.transmit = _UDPv4Transmit,
		.bind = _UDPv4Bind,
		.connect = _UDPv4Connect,
		.onReadyToReceive = _UDPv4Receive,
		.listen = _UDPv4Listen,
		.lookup = _UDPv4LookupConnection,
};

//key is assumed to GSocketAddress
static guint
_UDPv4ConnHashFunc(gconstpointer key)
{
	struct sockaddr_in skaddr;
	GError *gerror = NULL;

	g_socket_address_to_native((GSocketAddress *)key, &skaddr, sizeof(struct sockaddr_in), &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_assert(FALSE);
	}

	return (guint)(skaddr.sin_addr.s_addr + skaddr.sin_port);
}

static gboolean
_UDPv4ConnEqual(gconstpointer a, gconstpointer b)
{
	struct sockaddr_in skaddr_a, skaddr_b;
	GError *gerror = NULL;

	g_socket_address_to_native((GSocketAddress *)a, &skaddr_a, sizeof(struct sockaddr_in), &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_assert(FALSE);
	}

	g_socket_address_to_native((GSocketAddress *)b, &skaddr_b, sizeof(struct sockaddr_in), &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_assert(FALSE);
	}

	return !memcmp(&skaddr_a, &skaddr_b, sizeof(struct sockaddr_in));
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
	//IP_RECVORIGDSTADDR: socket option for get destination address of received UDP packet
	new->_transport.socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_DEFAULT, &gerror);
	new->_transport.protocol = IpcomProtocolGetInstance();

	//initialize IpcomTransportUDPv4 structure
	new->boundSockAddr = NULL;
	///key: struct sockaddr_in for remote sockaddr
	///value: IpcomConnection
	new->connHash = g_hash_table_new(_UDPv4ConnHashFunc, _UDPv4ConnEqual);
	new->mainContext = NULL;
	new->socketSource = NULL;

	if (gerror) g_error_free(gerror);

	return &new->_transport;
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
	if (udpTransport->connHash) g_hash_table_remove_all(udpTransport->connHash);
	if (udpTransport->boundSockAddr) g_object_unref(udpTransport->boundSockAddr);

	g_free(udpTransport);
}
