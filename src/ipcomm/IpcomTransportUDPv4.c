#include <glib.h>
//#include <glib-object.h>
#include <netinet/in.h>	//struct sockaddr_in
#include <string.h>		//memcmp

#include <IpcomMessage.h>
#include <IpcomProtocol.h>
#include <IpcomTransport.h>
#include <IpcomTransportUDPv4.h>
#include <IpcomConnection.h>
#include <ref_count.h>
#include <dprint.h>

struct _IpcomTransportUDPv4 {
	IpcomTransport _transport;
	IpcomService	*service;
	GSocketAddress	*boundSockAddr;
	GHashTable		*connHash;
	GMainContext	*mainContext;
};

static gboolean
_UDPv4Receive(IpcomTransport *transport, GSocket *socket)
{
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	GSocketAddress	*sockaddr;
	GError			*gerror;
	IpcomMessage	*newMesg = IpcomMessageNew(0);
	gssize			length;

	DFUNCTION_START;

	length = g_socket_receive_from(socket, &sockaddr, newMesg->message, newMesg->actual_size, NULL, &gerror);
	IpcomMessageInit(newMesg, length);
	if (gerror) {
		DERROR("%s", gerror->message);
		IpcomMessagePut(newMesg);
		g_error_free(gerror);
		return FALSE;
	}
	else {
		IpcomConnection *conn = (IpcomConnection *)g_hash_table_lookup(udpTransport->connHash, sockaddr);

		if (!conn) { //New Connection
			//IMPLEMENT:
			DPRINT("Get New connection.\n");
		}
		if (conn) {
			DPRINT("Received data from known connection.\n");
			IpcomConnectionPushIncomingMessage(conn, newMesg);
		}
	}
	g_object_unref(sockaddr);

	return TRUE;
}

static gboolean
_UDPv4CheckSocket(GSocket *socket, GIOCondition cond, gpointer data)
{
	IpcomTransportUDPv4 *udpTransport = (IpcomTransportUDPv4 *)data;
	GError				*gerror;

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
	GError *gerror;
	GSocketAddress *sockaddr;
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);

	DFUNCTION_START;

	g_assert(transport->socket);

	sockaddr = g_inet_socket_address_new_from_string(localIp, localPort);
	g_assert(sockaddr);

	if (!g_socket_bind(transport->socket, sockaddr, TRUE, &gerror)) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		g_assert(FALSE);
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

	pConn = IpcomConnectionNew(NULL, remoteSockAddr);
	g_assert(pConn);
	pConn->transport = transport;
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
	GError *gerror;
	gssize sent_bytes;
	sent_bytes = g_socket_send_to(transport->socket, conn->remoteSockAddr, mesg->message, mesg->length, NULL, &gerror);

	if (sent_bytes != mesg->length) {
		if (gerror) {
			DERROR("%s\n", gerror->message);
			g_error_free(gerror);
			return -1;
		}
		DWARN("Only part of IpcomMessage was sent.(%d/%d)\n", sent_bytes, mesg->length);
		return sent_bytes;
	}

	return sent_bytes;
}

static IpcomTransport udpv4 = {
	.transmit = _UDPv4Transmit,
	.bind = _UDPv4Bind,
	.connect = _UDPv4Connect,
	.onReadyToReceive = _UDPv4Receive,
};

static guint
_UDPv4ConnHashFunc(gconstpointer key)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)key;
	return (guint)(addr->sin_addr.s_addr + addr->sin_port);
}

static gboolean
_UDPv4ConnEqual(gconstpointer a, gconstpointer b)
{
	return !memcmp(a, b, sizeof(struct sockaddr_in));
}

gboolean
IpcomTransportUDPv4AttachGlibContext(IpcomTransportUDPv4 *transport, GMainContext *ctx)
{
	GSource *source;

	if (!transport || !transport->_transport.socket || !ctx) {
		return FALSE;
	}

	transport->mainContext = ctx;
	source = g_socket_create_source (transport->_transport.socket, G_IO_ERR|G_IO_HUP|G_IO_IN, NULL);
	g_source_set_callback(source, (GSourceFunc)_UDPv4CheckSocket, transport, NULL);
	g_source_attach(source, transport->mainContext);
	g_source_unref(source);

	return TRUE;

}
IpcomTransport *
IpcomTransportUDPv4New()
{
	IpcomTransportUDPv4 *new;
	GError *gerror;

	new = g_malloc0(sizeof(IpcomTransportUDPv4));
	g_assert(new);

	new->_transport = udpv4;
	new->_transport.socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_DEFAULT, &gerror);
	new->_transport.protocol = IpcomProtocolGetInstance();
	//key: struct sockaddr_in for remote sockaddr
	//value: IpcomConnection
	new->connHash = g_hash_table_new(_UDPv4ConnHashFunc, _UDPv4ConnEqual);

	if (gerror) g_error_free(gerror);

	return &new->_transport;
}

void
IpcomTransportUDPv4Destroy(IpcomTransport *transport)
{
	IpcomTransportUDPv4 *udpTransport = UDPV4TRANSPORT_FROM_TRANSPORT(transport);
	g_free(udpTransport);
}
