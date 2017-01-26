/*
 * IpcmdTransportTcpv4.c
 *
 *  Created on: Jan 25, 2017
 *      Author: hyotiger
 */

#include "../include/IpcmdTransportTcpv4.h"
#include "../include/reference.h"

struct _IpcmdTransportTcpv4 {
	struct _IpcmdTransport	parent_;
	GSocket				*socket_;
	GInetSocketAddress	*bound_sockaddr_;
	GHashTable			*channels_;			// key: sockfd, data: IpcmdChannel
};

static inline struct _IpcmdTransport* IPCMD_TRANSPORT(struct _IpcmdTransportTcpv4 *obj)
{return &obj->parent_;}
static inline struct _IpcmdTransportTcpv4* IPCMD_TRANSPORTTCPv4(struct _IpcmdTransport *obj)
{return obj->type_ == kIpcmdTransportTcpv4 ? container_of(obj, struct _IpcmdTransportTcpv4, parent_) : NULL;}

static void
_Tcpv4OnAttachedToBus (IpcmdTransport *transport, IpcmdBus *bus)
{
	struct _IpcmdTransportTcpv4 *tcp_transport = IPCMD_TRANSPORTTCPv4(transport);
	GHashTableIter 	iter;
	gpointer 		key, value;

	transport->bus_ = bus;
	// Register ongoing channels to bus
	g_hash_table_iter_init (&iter, tcp_transport->channels_);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		IpcmdBusRegisterChannel (bus, (IpcmdChannel*)value);
	}
}

static void
_Tcpv4OnDetachedFromBus (IpcmdTransport *transport, IpcmdBus *bus)
{
	struct _IpcmdTransportTcpv4 *tcp_transport = IPCMD_TRANSPORTTCPv4(transport);
	GHashTableIter 	iter;
	gpointer 		key, value;

	transport->bus_ = NULL;

	// Unregister ongoing channels from bus
	g_hash_table_iter_init (&iter, tcp_transport->channels_);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		IpcmdBusUnregisterChannel (bus, (IpcmdChannel*)value);
	}
}

static gint
_Udpv4Transmit(IpcmdTransport *transport, const IpcmdChannel *channel, IpcmdMessage *mesg)
{
	IpcmdTransportUdpv4	*tcp_transport = IPCMD_TRANSPORTTCPv4(transport);
	GError 				*gerror=NULL;
	struct sockaddr_in	remote_native_sockaddr;
	struct sockaddr_in	local_native_sockaddr;
	IpcmdUdpv4Host 		*udp_local_host = IPCMD_UDPv4HOST(channel->local_host_);
	IpcmdUdpv4Host 		*udp_remote_host = IPCMD_UDPv4HOST(channel->remote_host_);

	g_assert(udp_transport->bound_sockaddr_);

#if 0
	if (IpcmdChannelIsBroadcast(channel)) {
		guint16 dport = IpcmdConnectionGetBroadConnectionPort(conn);
		return dport > 0 ?
				_UDPv4Broadcast(transport, dport, mesg) :
				_UDPv4Broadcast(transport, g_inet_socket_address_get_port(udpTransport->boundSockAddr), mesg);
	}
	else {
	}
#endif
	/* convert Local/Remote GSocketAddress to native 'struct sockaddr_in' */
	g_socket_address_to_native (G_SOCKET_ADDRESS(udp_local_host->inet_sockaddr_), &local_native_sockaddr, sizeof(struct sockaddr_in), &gerror);
	if (gerror)	goto _Udpv4Transmit_failed;
	g_socket_address_to_native (G_SOCKET_ADDRESS(udp_remote_host->inet_sockaddr_), &remote_native_sockaddr, sizeof(struct sockaddr_in), &gerror);
	if (gerror)	goto _Udpv4Transmit_failed;

	return _Udpv4NativeSend(g_socket_get_fd (udp_transport->socket_), &local_native_sockaddr, &remote_native_sockaddr, mesg);

	_Udpv4Transmit_failed:
	if (gerror) {
		g_warning("%s", gerror->message);
		g_error_free(gerror);
	}
	return -1;
}


static IpcmdTransport tcpv4 = {
		.type_ 				= kIpcmdTransportUdpv4,
		.OnAttachedToBus 	= _Tcpv4OnAttachedToBus,
		.OnDetachedFromBus 	= _Tcpv4OnDetachedFromBus,
		.transmit 			= _Tcpv4Transmit,
		.bind 				= _Tcpv4Bind,
		.connect 			= _Tcpv4Connect,
		.listen 			= _Tcpv4Listen
};

IpcmdTransport*
IpcmdTransportTcpv4New()
{
	IpcmdTransportTcpv4 *new_transport = NULL;
	GError 				*gerror=NULL;
	GSocket				*tcp_socket = NULL;

	// make TCPv4 socket
	tcp_socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &gerror);
	if (!tcp_socket) {
		g_warning("%s", gerror->message);
		g_error_free(gerror);
		goto _Tcpv4New_failed;
	}
	new_transport = g_malloc0(sizeof(IpcmdTransportUdpv4));
	if (!new_transport) {
		goto _Tcpv4New_failed;
	}

	//initialize IpcmdTransport structure
	new_transport->parent_ = tcpv4;
	new_transport->parent_.source_ = g_socket_create_source (tcp_socket, G_IO_IN, NULL);
	g_source_set_callback(new_transport->parent_.source_, (GSourceFunc)_Tcpv4CheckSocket, new_transport, NULL);

	//initialize IpcmdTransportUDPv4 structure
	new_transport->socket_ = tcp_socket;
	new_transport->bound_sockaddr_= NULL;
	/*
	 * channel hash table
	 * key: sockfd
	 * value: IpcmdChannel*
	 */
	new_transport->channels_ = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, _Tcpv4ChannelOnDestroy);
	//Update Network Information
	//NetifcMonitorUpdate(IpcmdNetifcGetInstance());

	return IPCMD_TRANSPORT(new_transport);

	_Tcpv4New_failed:
	if (new_transport) {
		if (new_transport->channels_) g_hash_table_destroy (new_transport->channels_);
		g_free (new_transport);
	}
	return NULL;
}

void
IpcmdTransportTcpv4Destroy(IpcmdTransport *transport)
{
	IpcmdTransportTcpv4 *tcp_transport = IPCMD_TRANSPORTTCPv4(transport);
	GError 				*gerror = NULL;

	if (tcp_transport->bound_sockaddr_) g_object_unref(tcp_transport->bound_sockaddr_);
	// unregister and remove channels
	if (tcp_transport->channels_) g_hash_table_destroy(tcp_transport->channels_);
	// after unregistering channels, transport should be unregistered.
	if (tcp_transport->parent_.bus_) IpcmdBusDetachTransport (tcp_transport->parent_.bus_, transport);
	if (tcp_transport->socket_) {
		g_socket_close(tcp_transport->socket_, &gerror);
		if (gerror) {
			g_error("%s\n", gerror->message);
			g_error_free(gerror);
		}
	}
	if (tcp_transport->parent_.source_) {
		g_source_unref (tcp_transport->parent_.source_);
	}

	g_free(tcp_transport);
}
