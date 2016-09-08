/*
 * IpcmdTransportUdpv4.c
 *
 *  Created on: Sep 5, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdTransportUdpv4.h"
#include "../include/IpcmdTransport.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdChannel.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdHost.h"
#include "../include/reference.h"
#include "../include/NetifcMonitor.h"
#include "../include/SocketUtils.h"
#include <glib.h>
#include <string.h>

#include <netinet/ip.h>
#include <errno.h>

#define MAX_CMSG_SIZE 256
#define IPV4_ANYCAST_ADDRESS gen_anycast_address()
static GInetAddress* gen_anycast_address() {
	static GInetAddress* anycast_addr = NULL;
	if (!anycast_addr) anycast_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	return anycast_addr;
}

enum Udpv4Mode {
	kUdpv4ModeNone = 0,
	kUdpv4ModeConnecting,
	kUdpv4ModeListening,
};

struct _IpcmdTransportUdpv4 {
	struct _IpcmdTransport	parent_;
	GSocket				*socket_;
	GInetSocketAddress	*bound_sockaddr_;
	GHashTable			*channels_;			// key: ChannelHashkey, data: IpcmdChannel
	IpcmdChannel		*connect_channel_;	// Only one connect channel exists in a IpcmdTransport
	IpcmdChannel		*broadcast_channel_;
	enum Udpv4Mode		transport_mode_;
};

typedef struct {
	GInetSocketAddress	*local_sockaddr;
	GInetSocketAddress	*remote_sockaddr;
} ChannelHashkey;

static inline struct _IpcmdTransport* IPCMD_TRANSPORT(struct _IpcmdTransportUdpv4 *obj) {
	return &obj->parent_;
}
static inline struct _IpcmdTransportUdpv4* IPCMD_TRANSPORTUDPv4(struct _IpcmdTransport *obj) {
	return obj->type_ == kIpcmdTransportUdpv4 ? container_of(obj, struct _IpcmdTransportUdpv4, parent_) : NULL;
}

static void
_Udpv4ChannelOnDestroyHashkey(gpointer key) {
	ChannelHashkey	*hash_key = (ChannelHashkey*)key;
	if (hash_key->local_sockaddr) g_object_unref (hash_key->local_sockaddr);
	if (hash_key->remote_sockaddr) g_object_unref (hash_key->remote_sockaddr);
	g_free (hash_key);
}

void
_Udpv4ChannelFree(IpcmdChannel *channel) {
	IpcmdHostUnref (channel->local_host_);
	IpcmdHostUnref (channel->remote_host_);
	g_free (channel);
}

void
_Udpv4ChannelOnDestroy(gpointer data) {
	IpcmdChannel				*channel = (IpcmdChannel*)data;
	//struct _IpcmdTransportUdpv4	*udp_transport = IPCMD_TRANSPORTUDPv4(channel->transport_);

	IpcmdBusUnregisterChannel (channel->transport_->bus_, channel);
	_Udpv4ChannelFree (channel);
}
IpcmdChannel *
_Udpv4ChannelNew(GInetSocketAddress *local, GInetSocketAddress *remote, IpcmdTransportUdpv4 *udp_transport) {
	IpcmdChannel *new_channel = g_malloc0(sizeof(IpcmdChannel));

	if (!new_channel) return NULL;
	if (local) new_channel->local_host_ = IpcmdUdpv4HostNew2 (local);
	if (remote) new_channel->remote_host_ = IpcmdUdpv4HostNew2 (remote);
	new_channel->status_ = kChannelClosed;
	new_channel->transport_ = IPCMD_TRANSPORT(udp_transport);

	return new_channel;
}

static guint
_Udpv4ChannelHashFunc(gconstpointer key) {
	const ChannelHashkey	*hash_key = (const ChannelHashkey*)key;
	struct sockaddr_in	native_sockaddr;
	GError				*gerror = NULL;

	g_socket_address_to_native (G_SOCKET_ADDRESS(hash_key->remote_sockaddr), &native_sockaddr, sizeof(struct sockaddr_in), &gerror);
	if (gerror) {
		g_error("%s", gerror->message);
		g_error_free(gerror);
	}

	return (guint)(native_sockaddr.sin_addr.s_addr + native_sockaddr.sin_port);
}

static inline gboolean
_Udpv4ChannelHashEqual(gconstpointer a, gconstpointer b) {
	const ChannelHashkey	*a_key = (const ChannelHashkey*)a;
	const ChannelHashkey	*b_key = (const ChannelHashkey*)b;

	return g_inet_address_equal(g_inet_socket_address_get_address(a_key->local_sockaddr), g_inet_socket_address_get_address(b_key->local_sockaddr)) &&
	g_inet_address_equal(g_inet_socket_address_get_address(a_key->remote_sockaddr), g_inet_socket_address_get_address(b_key->remote_sockaddr)) &&
	g_inet_socket_address_get_port(a_key->local_sockaddr) == g_inet_socket_address_get_port(b_key->local_sockaddr) &&
	g_inet_socket_address_get_port(a_key->remote_sockaddr) == g_inet_socket_address_get_port(b_key->remote_sockaddr);
}

static gboolean
_Udpv4Receive(IpcmdTransportUdpv4 *udp_transport, GSocket *socket)
{
	GInetSocketAddress	*remote_glib_sockaddr = NULL;
	GInetSocketAddress	*local_glib_sockaddr = NULL;
	IpcmdMessage		*new_mesg = NULL;
	gssize				length;
	IpcmdChannel		*channel = NULL;
	ChannelHashkey		key;
	struct sockaddr_in	remote_native_sockaddr;
	struct sockaddr_in	local_native_sockaddr;

	struct msghdr		msgh = {0};
	struct iovec		iov[1];
	guint8				cmsg_buf[MAX_CMSG_SIZE] = {0};
	struct cmsghdr		*pcmsgh;
	struct in_pktinfo	*ppktinfo;

	new_mesg = IpcmdMessageNew(IPCMD_MESSAGE_MAX_SIZE);

	iov[0].iov_base = IpcmdMessageGetRawData(new_mesg);
	iov[0].iov_len = IPCMD_MESSAGE_MAX_SIZE;

	msgh.msg_name = (void *)&remote_native_sockaddr;
	msgh.msg_namelen = sizeof(struct sockaddr_in);
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = cmsg_buf;
	msgh.msg_controllen = sizeof(cmsg_buf);
	msgh.msg_flags = 0;

	/* Receive a message with auxiliary data */
	length = recvmsg (g_socket_get_fd(udp_transport->socket_), &msgh, 0);
	if (length < 0) {
		g_warning("%s", strerror(errno));
		IpcmdMessageUnref(new_mesg);
		return FALSE;
	}

	/* if VCCCPDUHeader->length+8 != length, this packet is damaged! */
	if (IpcmdMessageGetVCCPDULength(new_mesg)+8 != length) {
		g_info("The length field in VCCPDU header is inconsistent with actual received packet length. Silently discard.");
		IpcmdMessageUnref(new_mesg);
		return FALSE;
	}
	IpcmdMessageSetLength(new_mesg, length);

	/* Receive auxiliary data in msgh */
	for (pcmsgh = CMSG_FIRSTHDR(&msgh); pcmsgh != NULL; pcmsgh = CMSG_NXTHDR(&msgh, pcmsgh)) {
		//g_debug("cmsg level and type = %d, %d\n", pcmsgh->cmsg_level, pcmsgh->cmsg_type);
		if (pcmsgh->cmsg_level == IPPROTO_IP && pcmsgh->cmsg_type == IP_PKTINFO && pcmsgh->cmsg_len) {
			ppktinfo = (struct in_pktinfo *)CMSG_DATA(pcmsgh);
			local_native_sockaddr.sin_family = AF_INET;
			local_native_sockaddr.sin_port = g_htons(g_inet_socket_address_get_port(udp_transport->bound_sockaddr_));
			memcpy(&local_native_sockaddr.sin_addr, &ppktinfo->ipi_addr, sizeof(struct in_addr));
			break;
		}
	}

	/* Check the packet is BROADCASTING or UNICASTING */
	channel = NULL;
	local_glib_sockaddr = G_INET_SOCKET_ADDRESS (g_socket_address_new_from_native((gpointer)&local_native_sockaddr, sizeof(struct sockaddr_in)));
	remote_glib_sockaddr = G_INET_SOCKET_ADDRESS (g_socket_address_new_from_native((gpointer)&remote_native_sockaddr, sizeof(struct sockaddr_in)));

#if 1
	{
		gchar*	dst = g_inet_address_to_string(g_inet_socket_address_get_address(local_glib_sockaddr));
		guint16	dport = g_inet_socket_address_get_port(local_glib_sockaddr);
		gchar* 	src = g_inet_address_to_string(g_inet_socket_address_get_address(remote_glib_sockaddr));
		guint16	sport = g_inet_socket_address_get_port(remote_glib_sockaddr);

		g_info("Got a packet: <%s:%d> --> <%s:%d>", src,sport,dst,dport);
		g_free(dst);
		g_free(src);

	}
#endif

	switch(NetifcMonitorQueryAddressType(NetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(local_glib_sockaddr)))) {
	case ADDRESSTYPE_IPV4_UNICAST:
		/* Set temporary connection information */
		key.local_sockaddr = G_INET_SOCKET_ADDRESS(local_glib_sockaddr);
		key.remote_sockaddr = G_INET_SOCKET_ADDRESS(remote_glib_sockaddr);

		/* check that the key is already in hash table.*/
		channel = (IpcmdChannel *)g_hash_table_lookup(udp_transport->channels_, &key);
		if (!channel) {	// new channel is detected!
			ChannelHashkey *new_key;

			channel = _Udpv4ChannelNew(local_glib_sockaddr, remote_glib_sockaddr, udp_transport);
			// IMPL: recovery on no memory
			if (!channel) g_error("Not enough memory.");
			//
			channel->status_ = kChannelEstablished;	// we already received a message.

			if (!IpcmdBusRegisterChannel (IPCMD_TRANSPORT(udp_transport)->bus_, channel)) { // bus may reject this channel
				IpcmdHostUnref(channel->local_host_);
				IpcmdHostUnref(channel->remote_host_);
				g_free(channel);
				goto _Udpv4Receive_failed;
			}

			new_key = g_malloc(sizeof(ChannelHashkey));
			new_key->local_sockaddr = g_object_ref (local_glib_sockaddr);
			new_key->remote_sockaddr = g_object_ref (remote_glib_sockaddr);

			if (!g_hash_table_insert (udp_transport->channels_, new_key, channel)) {
				g_error("Failed to add new channel into hash table.");
			}
		}
		break;
	case ADDRESSTYPE_IPV4_BROADCAST:
		/* if broadcast packet is sent by me, ignore it */
		if (NetifcMonitorQueryAddressType( NetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(remote_glib_sockaddr)))
				== ADDRESSTYPE_IPV4_UNICAST)
			break;
		else channel = udp_transport->broadcast_channel_;
		break;
	default:
		g_warning("Receive a packet with wrong destination.\n");
		goto _Udpv4Receive_failed;
	}

	/// set remote address into IpcmdMessage
    IpcmdMessageSetOriginSockAddress(new_mesg, G_SOCKET_ADDRESS(remote_glib_sockaddr));

    if (channel) {
    	IpcmdBusRx(IPCMD_TRANSPORT(udp_transport)->bus_, channel->channel_id_, new_mesg);
    }
    else
    	g_debug("Discarding the packet.");

	IpcmdMessageUnref(new_mesg);
	if (remote_glib_sockaddr) g_object_unref(remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref(local_glib_sockaddr);
	return TRUE;

	_Udpv4Receive_failed:
	IpcmdMessageUnref(new_mesg);
	if (remote_glib_sockaddr) g_object_unref(remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref(local_glib_sockaddr);
	return FALSE;
}

static gboolean
_Udpv4CheckSocket(GSocket *socket, GIOCondition cond, gpointer data)
{
	IpcmdTransportUdpv4 *udp_transport = (IpcmdTransportUdpv4 *)data;

	if (cond & G_IO_IN) {
		_Udpv4Receive (udp_transport, socket);
	}
	else {	//probably G_IO_ERR or G_IO_HUP
		return FALSE;
	}
	return TRUE;	//continue to receive
}

static gboolean
_Udpv4Bind(IpcmdTransport *transport, const gchar *localIp, guint16 localPort)
{
	GError *gerror=NULL;
	GInetSocketAddress *sockaddr;
	IpcmdTransportUdpv4 *udp_transport = IPCMD_TRANSPORTUDPv4 (transport);

	g_assert(udp_transport->socket_);

	if (localIp == NULL || !strcmp(localIp, "0.0.0.0")) {
		sockaddr = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new(IPV4_ANYCAST_ADDRESS, localPort));
	}
	else {
		sockaddr = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new_from_string(localIp, localPort));
	}
	g_assert(sockaddr);

	if (!g_socket_bind(udp_transport->socket_, G_SOCKET_ADDRESS(sockaddr), TRUE, &gerror)) {
		g_critical("%s", gerror->message);
		g_error_free(gerror);
		g_object_unref(sockaddr);
		return FALSE;
	}

	udp_transport->bound_sockaddr_ = sockaddr;
	return TRUE;
}

static gint
_Udpv4Connect(IpcmdTransport *transport, const gchar *remoteIp, const guint16 remotePort)
{
	IpcmdTransportUdpv4		*udp_transport = IPCMD_TRANSPORTUDPv4 (transport);
	IpcmdChannel			*channel;
	GInetSocketAddress		*remote_glib_sockaddr = NULL;
	GInetSocketAddress		*local_glib_sockaddr = NULL;
	GError					*gerror = NULL;

	if (udp_transport->transport_mode_) {
		g_warning("Transport is already in use.");
		goto _Udpv4Connect_failed;
	}

	/* Make socket address for remote */
	remote_glib_sockaddr = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new_from_string(remoteIp, remotePort));
	g_assert(remote_glib_sockaddr);

	/* If the socket is bound to anycast address, we choose a source address for the 'remoteIp'.
	 * If not, use bound socket address.
	 */
	if (g_inet_address_equal(g_inet_socket_address_get_address(udp_transport->bound_sockaddr_), IPV4_ANYCAST_ADDRESS)) {
		struct sockaddr_in		remote_native_sockaddr;
		struct sockaddr_in		local_native_sockaddr;

		local_native_sockaddr.sin_family = AF_INET;
		local_native_sockaddr.sin_port = g_htons(g_inet_socket_address_get_port (udp_transport->bound_sockaddr_));

		g_socket_address_to_native (G_SOCKET_ADDRESS(remote_glib_sockaddr), &remote_native_sockaddr, sizeof(struct sockaddr_in), &gerror);
		if (gerror) {
			g_critical("%s", gerror->message);
			g_error_free(gerror);
			return FALSE;
		}
		/// Lookup source address to use for remoteIP
		if (QuerySrcIpv4AddrForDst(&remote_native_sockaddr.sin_addr, &local_native_sockaddr.sin_addr) == -1) {
			g_warning("Cannot reach to the destination.\n");
			return FALSE;
		}

		local_glib_sockaddr = G_INET_SOCKET_ADDRESS(g_socket_address_new_from_native((gpointer)&local_native_sockaddr, sizeof(struct sockaddr_in)));
	} else
		local_glib_sockaddr = g_object_ref(udp_transport->bound_sockaddr_);

	channel = g_malloc0(sizeof(IpcmdChannel));
	g_assert(channel);
	channel->local_host_ = IpcmdUdpv4HostNew2 (local_glib_sockaddr);
	channel->remote_host_ = IpcmdUdpv4HostNew2 (remote_glib_sockaddr);
	channel->status_ = kChannelOpening;
	channel->transport_ = transport;

	if (!IpcmdBusRegisterChannel (transport->bus_, channel)) { // bus may reject this channel
		IpcmdHostUnref (channel->local_host_);
		IpcmdHostUnref (channel->remote_host_);
		g_free (channel);
		goto _Udpv4Connect_failed;
	}

	if (remote_glib_sockaddr) g_object_unref (remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref (local_glib_sockaddr);

	udp_transport->transport_mode_ = kUdpv4ModeConnecting;

	return channel->channel_id_;

	_Udpv4Connect_failed:
	if (remote_glib_sockaddr) g_object_unref (remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref (local_glib_sockaddr);
	return -1;
}


static gint
_UDPv4NativeSend(gint sockFd, struct sockaddr_in *local, struct sockaddr_in *remote, IpcmdMessage *mesg)
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
	iov[1].iov_len = IpcmdMessageGetPaylodLength(mesg);

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
		g_warning("%s\n", strerror(errno));
		goto _UDPv4NativeSend_failed;
	}
#if 0
#if DLOG_LEVEL_DEBUG <= DEBUG
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
		DPRINT("Send a packet: <%s:%d> --> <%s:%d>\n", src, sport, dst, dport);
	}
#endif
#endif
	return IpcmdMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE;

	_UDPv4NativeSend_failed:
	return -1;
}

#if 0
static gint
_UDPv4Broadcast(IpcmdTransport *transport, guint16 dport, IpcmdMessage *mesg)
{
	IpcmdTransportUdpv4		*udp_transport = IPCMD_TRANSPORTUDPv4(transport);
	GInetAddress			*bound_glib_inetaddr;
	GInetAddress			*broadcast_glib_inetaddr;
	GList					*broadcast_addr_list;
	GList					*iter;
	struct sockaddr_in		remote_sockaddr = {0};

	if (!udp_transport->bound_sockaddr_) {
		DERROR("UDPv4 transport is not bound.\n");
		return -1;
	}

	remote_sockaddr.sin_family = AF_INET;
	remote_sockaddr.sin_port = g_htons(dport);
	bound_glib_inetaddr = g_inet_socket_address_get_address(udp_transport->bound_sockaddr_);

	/// If socket is bound to anycast address, we broadcast for each broadcast address.
	if (g_inet_address_equal(bound_glib_inetaddr, IPV4_ANYCAST_ADDRESS)) {
		broadcast_addr_list = NetifcMonitorGetAllIpv4BroadAddr(NetifcGetInstance());
		if (!broadcast_addr_list) goto _UDPv4Broadcast_failed;

		for (iter = g_list_first(broadcast_addr_list); iter != NULL; iter = g_list_next(iter)) {
			if (g_inet_address_get_family((GInetAddress*)iter->data) == G_SOCKET_FAMILY_IPV4) {
				memcpy(&remote_sockaddr.sin_addr, g_inet_address_to_bytes(iter->data), sizeof(struct in_addr));
				_UDPv4NativeSend(g_socket_get_fd (transport->socket_), NULL, &remote_sockaddr, mesg);
			}
		}
		g_list_free(broadcast_addr_list);
	}
	else { /// socket is bound to unicast address
		pBroadcastAddress = NetifcMonitorQueryBroadcastAddressWithSrc(NetifcGetInstance(), pBoundInetAddress);
		if (!pBroadcastAddress) goto _UDPv4Broadcast_failed;

		memcpy(&mRemoteSockAddr.sin_addr, g_inet_address_to_bytes(pBroadcastAddress), sizeof(struct in_addr));
		_UDPv4NativeSend(g_socket_get_fd (transport->socket), NULL, &mRemoteSockAddr, mesg);
	}

	return IpcmdMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE;

	_UDPv4Broadcast_failed:
	return -1;
}
#endif
static gint
_Udpv4Transmit(IpcmdTransport *transport, const IpcmdChannel *channel, IpcmdMessage *mesg)
{
	IpcmdTransportUdpv4	*udp_transport = IPCMD_TRANSPORTUDPv4(transport);
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

	return _UDPv4NativeSend(g_socket_get_fd (udp_transport->socket_), &local_native_sockaddr, &remote_native_sockaddr, mesg);

	_Udpv4Transmit_failed:
	if (gerror) {
		g_warning("%s", gerror->message);
		g_error_free(gerror);
	}
	return -1;
}

static gboolean
_UDPv4Listen(IpcmdTransport *transport, gint backlog)
{
	struct _IpcmdTransportUdpv4 *udp_transport = IPCMD_TRANSPORTUDPv4(transport);

	if (udp_transport->transport_mode_) {
		g_warning("Transport is already in use.");
		return FALSE;
	}
	udp_transport->transport_mode_ = kUdpv4ModeListening;
	return TRUE;
}

static IpcmdTransport udpv4 = {
		.type_ = kIpcmdTransportUdpv4,
		.transmit = _Udpv4Transmit,
		.bind = _Udpv4Bind,
		.connect = _Udpv4Connect,
		.listen = _UDPv4Listen,
};

/**
 * _Udpv4EnableBroadcast:
 * enable the UDP transport and create new broadcast channel to send broadcast packets
 */
static gboolean
_Udpv4EnableBroadcast(IpcmdTransportUdpv4 *udp_transport)
{
	IpcmdChannel *broadcast_channel;
	GSocketAddress *anycast_sockaddr;

	if (!udp_transport->broadcast_channel_) { // if broadcast channel does not exist
		anycast_sockaddr = g_inet_socket_address_new (IPV4_ANYCAST_ADDRESS, 0);
		broadcast_channel = _Udpv4ChannelNew (G_INET_SOCKET_ADDRESS(anycast_sockaddr), G_INET_SOCKET_ADDRESS(anycast_sockaddr), udp_transport);
		g_object_unref (anycast_sockaddr);

		udp_transport->broadcast_channel_ = broadcast_channel;
		g_socket_set_broadcast (udp_transport->socket_, TRUE);
	}
	if (IPCMD_TRANSPORT(udp_transport)->bus_ && !udp_transport->broadcast_channel_->channel_id_) { // if broadcast channel is not registered to IpcmdBus
		if (!IpcmdBusRegisterChannel(IPCMD_TRANSPORT(udp_transport)->bus_, udp_transport->broadcast_channel_)) {
			g_warning("Failed to register broadcast channel.");
		}
	}
	return TRUE;
}

/**
 * _Udpv4DisableBroadcast:
 * disable broadcasting and remove a broadcast channel.
 */
static void
_Udpv4DisableBroadcast(IpcmdTransportUdpv4 *udp_transport)
{
	if (!udp_transport->broadcast_channel_) return;

	g_socket_set_broadcast (udp_transport->socket_, FALSE);
	if (udp_transport->broadcast_channel_->channel_id_) { // if broadcast channel was registered to IpcmdBus, unregister it
		IpcmdBusUnregisterChannel(IPCMD_TRANSPORT(udp_transport)->bus_, udp_transport->broadcast_channel_);
	}
	_Udpv4ChannelFree (udp_transport->broadcast_channel_);
	udp_transport->broadcast_channel_ = NULL;
}

IpcmdTransport *
IpcmdTransportUdpv4New()
{
	IpcmdTransportUdpv4 *new_transport;
	GError 				*gerror=NULL;
	GSocket				*udp_socket;

	// make UDPv4 socket
	udp_socket = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_DEFAULT, &gerror);
	if (!udp_socket) {
		g_warning("%s", gerror->message);
		g_error_free(gerror);
		goto _Udpv4New_failed;
	}
	if (!g_socket_set_option(udp_socket, IPPROTO_IP, IP_PKTINFO, 1, &gerror)) {
		g_warning("%s", gerror->message);
		g_error_free(gerror);
		goto _Udpv4New_failed;
	}

	new_transport = g_malloc0(sizeof(IpcmdTransportUdpv4));
	if (!new_transport) {
		goto _Udpv4New_failed;
	}

	//initialize IpcmdTransport structure
	new_transport->parent_ = udpv4;
	new_transport->parent_.source_ = g_socket_create_source (udp_socket, G_IO_IN, NULL);
	g_source_set_callback(new_transport->parent_.source_, (GSourceFunc)_Udpv4CheckSocket, new_transport, NULL);

	//initialize IpcmdTransportUDPv4 structure
	new_transport->socket_ = udp_socket;
	new_transport->bound_sockaddr_= NULL;
	new_transport->broadcast_channel_ = NULL;
	/*
	 * channel hash table
	 * key: ChannelHashkey
	 * value: IpcmdChannel*
	 */
	new_transport->channels_ = g_hash_table_new_full(_Udpv4ChannelHashFunc, _Udpv4ChannelHashEqual, _Udpv4ChannelOnDestroyHashkey, _Udpv4ChannelOnDestroy);

	//Update Network Information
	//NetifcMonitorUpdate(IpcmdNetifcGetInstance());

	return IPCMD_TRANSPORT(new_transport);

	_Udpv4New_failed:
	if (new_transport) {
		if (new_transport->broadcast_channel_) {}
		if (new_transport->channels_) {}
		g_free (new_transport);
	}
	return NULL;
}

void
IpcmdTransportUdpv4Destroy(IpcmdTransport *transport)
{
	IpcmdTransportUdpv4 *udp_transport = IPCMD_TRANSPORTUDPv4(transport);
	GError 				*gerror = NULL;

	if (udp_transport->bound_sockaddr_) g_object_unref(udp_transport->bound_sockaddr_);
	// unregister and remove channels
	if (udp_transport->broadcast_channel_) _Udpv4DisableBroadcast(udp_transport);
	if (udp_transport->channels_) g_hash_table_destroy(udp_transport->channels_);
	if (udp_transport->connect_channel_) {
		IpcmdBusUnregisterChannel (transport->bus_, udp_transport->connect_channel_);
		_Udpv4ChannelFree (udp_transport->connect_channel_);
	}
	// after unregistering channels, transport should be unregistered.
	if (udp_transport->parent_.bus_) IpcmdBusDetachTransport (udp_transport->parent_.bus_, transport);
	if (udp_transport->socket_) {
		g_socket_close(udp_transport->socket_, &gerror);
		if (gerror) {
			g_error("%s\n", gerror->message);
			g_error_free(gerror);
		}
	}
	if (udp_transport->parent_.source_) {
		g_source_unref (transport->source_);
	}

	g_free(udp_transport);
}
