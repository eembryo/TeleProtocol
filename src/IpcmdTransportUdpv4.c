/*
 * IpcmdTransportUdpv4.c
 *
 *  Created on: Sep 5, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdTransportUdpv4.h"
//#include "../include/IpcmdTransport.h"
//#include "../include/IpcmdTransportUdpv4.h"
#include "../include/reference.h"
#include <glib.h>

#include <netinet/ip.h>
#include <errno.h>

#define IPCMD_OBJECT_GET_TRANSPORT(obj) (struct _IpcmdTransport*)(&obj->parent_)
#define IPCMD_OBJECT_GET_TRANSPORT_UDPv4(obj) container_of(obj, struct _IpcmdTransportUdpv4, parent_)
#define MAX_CMSG_SIZE 256
#define IPV4_ANYCAST_ADDRESS gen_anycast_address()
static GInetAddress* gen_anycast_address() {
	static GInetAddress* anycast_addr = NULL;
	if (!anycast_addr) anycast_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	return anycast_addr;
}

enum Udpv4Mode {
	kUdpv4ModeNone = 0,
	kUdpv4ModeClient,
	kUdpv4ModeServer,
};

struct _IpcmdTransportUdpv4 {
	struct _IpcmdTransport	parent_;
	GSocketAddress		*bound_sockaddr_;
	GHashTable			*channels_;
	GMainContext		*main_context_;
	GSource				*socket_source_;
	IpcmdChannel		*connect_channel_;	// Only one connect channel exists in a IpcmdTransport
	IpcmdChannel		*broadcast_channel_;
	enum Udpv4Mode		transport_mode_;
};

typedef struct {
	GInetSocketAddress	*local_sockaddr;
	GInetSocketAddress	*remote_sockaddr;
} ChannelHashkey;

static gboolean
_Udpv4Receive(IpcmdTransport *transport, GSocket *socket)
{
	IpcmdTransportUdpv4* udp_transport = IPCMD_OBJECT_GET_TRANSPORT_UDPv4(transport);
	GSocketAddress*		remote_glib_sockaddr = NULL;
	GSocketAddress*		local_glib_sockaddr = NULL;
	IpcmdMessage*		new_mesg = NULL;
	gssize				length;
	IpcmdChannel		*channel;
	ChannelHashkey		key;
	struct sockaddr_in	remote_native_sockaddr;
	struct sockaddr_in	local_native_sockaddr;
	//IpcomConnection*	pConn;

	struct msghdr		msgh = {0};
	struct iovec		iov[1];
	guint8				cmsg_buf[MAX_CMSG_SIZE] = {0};
	struct cmsghdr*		pcmsgh;
	struct in_pktinfo*	ppktinfo;

	new_mesg = IpcomMessageNew(IPCMD_MESSAGE_MAX_SIZE);

	iov[0].iov_base = IpcomMessageGetRawData(new_mesg);
	iov[0].iov_len = IPCMD_MESSAGE_MAX_SIZE;

	msgh.msg_name = (void *)&remote_native_sockaddr;
	msgh.msg_namelen = sizeof(struct sockaddr_in);
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = cmsg_buf;
	msgh.msg_controllen = sizeof(cmsg_buf);
	msgh.msg_flags = 0;

	/* Receive a message with auxiliary data */
	length = recvmsg (g_socket_get_fd(transport->socket_), &msgh, 0);
	if (length < 0) {
		g_warning("%s", strerror(errno));
		IpcomMessageUnref(new_mesg);
		return FALSE;
	}

	IpcomMessageSetLength(new_mesg, length);

	/* Receive auxiliary data in msgh */
	for (pcmsgh = CMSG_FIRSTHDR(&msgh); pcmsgh != NULL; pcmsgh = CMSG_NXTHDR(&msgh, pcmsgh)) {
		//g_debug("cmsg level and type = %d, %d\n", pcmsgh->cmsg_level, pcmsgh->cmsg_type);
		if (pcmsgh->cmsg_level == IPPROTO_IP && pcmsgh->cmsg_type == IP_PKTINFO && pcmsgh->cmsg_len) {
			ppktinfo = (struct in_pktinfo *)CMSG_DATA(pcmsgh);
			local_native_sockaddr.sin_family = AF_INET;
			local_native_sockaddr.sin_port = g_htons(g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(udp_transport->bound_sockaddr_)));
			memcpy(&local_native_sockaddr.sin_addr, &ppktinfo->ipi_addr, sizeof(struct in_addr));
			break;
		}
	}

	/* Check the packet is BROADCASTING or UNICASTING */
	channel = NULL;
	local_glib_sockaddr = g_socket_address_new_from_native((gpointer)&local_native_sockaddr, sizeof(struct sockaddr_in));
	remote_glib_sockaddr = g_socket_address_new_from_native((gpointer)&remote_native_sockaddr, sizeof(struct sockaddr_in));

#if 0
#if DLOG_LEVEL_DEBUG <= DEBUG
	{
		gchar*	dst = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pLocalSockAddr)));
		guint16	dport = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pLocalSockAddr));
		gchar* 	src = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(pRemoteSockAddr)));
		guint16	sport = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(pRemoteSockAddr));

		DPRINT("Got a packet: <%s:%d> --> <%s:%d>\n", src,sport,dst,dport);
		g_free(dst);
		g_free(src);

	}
#endif
#endif
	switch(IpcomNetifcMonitorQueryAddressType(IpcomNetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(local_glib_sockaddr)))) {
	case ADDRESSTYPE_IPV4_UNICAST:
		/* Set temporary connection information */
		key.local_sockaddr = local_glib_sockaddr;
		key.remote_sockaddr = remote_glib_sockaddr;

		/* check that the key is already in hash table.*/
		channel = (IpcmdChannel *)g_hash_table_lookup(udp_transport->channels_, &key);
		if (!channel) {	// new channel is detected!
			ChannelHashkey new_key;

			channel = g_malloc0(sizeof(IpcmdChannel));
			// IMPL: recovery on no memory
			g_error("Not enough memory.");
			//
			channel->local_host_ = IpcmdUdpv4HostNew2 (local_glib_sockaddr);
			channel->remote_host_ = IpcmdUdpv4HostNew2 (remote_glib_sockaddr);
			channel->status_ = kChannelEstablished;	// we already received a message.
			channel->transport_ = transport;

			if (!IpcmdBusRegisterChannel (transport->bus_, channel)) { // bus may reject this channel
				IpcmdHostUnref(channel->local_host_);
				IpcmdHostUnref(channel->remote_host_);
				g_free(channel);
				goto _Udpv4Receive_failed;
			}

			new_key= g_malloc(sizeof(ChannelHashkey));
			new_key->local_sockaddr = g_object_ref (local_glib_sockaddr);
			new_key->remote_sockaddr = g_object_ref (remote_glib_sockaddr);

			if (!g_hash_table_insert (udp_transport->channels_, new_key, channel)) {
				g_error("Failed to add new channel into hash table.");
			}
		}
		break;
	case ADDRESSTYPE_IPV4_BROADCAST:
		/* if broadcast packet is sent by me, ignore it */
		if (IpcomNetifcMonitorQueryAddressType(	IpcomNetifcGetInstance(), g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS(remote_glib_sockaddr)))
				== ADDRESSTYPE_IPV4_UNICAST)
			break;
		else channel = udp_transport->broadcast_channel_;
		break;
	default:
		DWARN("Receive a packet with wrong destination.\n");
		goto _Udpv4Receive_failed;
	}

	/// set remote address into IpcomMessage
    IpcomMessageSetOriginSockAddress(new_mesg, remote_glib_sockaddr);

    if (channel) {
    	//IMPL: forward to IpcmdBus
    }
    else
    	g_debug("Discarding the packet.");

	IpcomMessageUnref(new_mesg);
	if (remote_glib_sockaddr) g_object_unref(remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref(local_glib_sockaddr);
	return TRUE;

	_Udpv4Receive_failed:
	IpcomMessageUnref(new_mesg);
	if (remote_glib_sockaddr) g_object_unref(remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref(local_glib_sockaddr);
	return FALSE;
}

static gboolean
_Udpv4CheckSocket(GSocket *socket, GIOCondition cond, gpointer data)
{
	IpcmdTransportUdpv4 *udpTransport = (IpcmdTransportUdpv4 *)data;

	if (cond & G_IO_IN) {
		IPCMD_OBJECT_GET_TRANSPORT (udpTransport)->onReadyToReceive (IPCMD_OBJECT_GET_TRANSPORT (udpTransport), socket);
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
	GSocketAddress *sockaddr;
	IpcmdTransportUdpv4 *udp_transport = IPCMD_OBJECT_GET_TRANSPORT (transport);

	g_assert(transport->socket_);

	if (localIp == NULL || !strcmp(localIp, "0.0.0.0")) {
		sockaddr = g_inet_socket_address_new(IPV4_ANYCAST_ADDRESS, localPort);
	}
	else {
		sockaddr = g_inet_socket_address_new_from_string(localIp, localPort);
	}
	g_assert(sockaddr);

	if (!g_socket_bind(transport->socket_, sockaddr, TRUE, &gerror)) {
		g_critical("%s", gerror->message);
		g_error_free(gerror);
		g_object_unref(sockaddr);
		return FALSE;
	}

	udp_transport->bound_sockaddr_ = sockaddr;
	return TRUE;
}

static gboolean
_UDPv4Connect(IpcmdTransport *transport, const gchar *remoteIp, guint16 remotePort)
{
	IpcmdTransportUdpv4		*udp_transport = IPCMD_OBJECT_GET_TRANSPORT_UDPv4 (transport);
	IpcmdChannel			*channel;
	GSocketAddress			*remote_glib_sockaddr = NULL;
	GSocketAddress			*local_glib_sockaddr = NULL;
	GError					*gerror = NULL;

	if (udp_transport->transport_mode_) {
		g_warning("Transport is already in use.");
		goto _UDPv4Connect_failed;
	}

	/* Make socket address for remote */
	remote_glib_sockaddr = g_inet_socket_address_new_from_string(remoteIp, remotePort);
	g_assert(remote_glib_sockaddr);

	/* If the socket is bound to anycast address, we choose a source address for the 'remoteIp'.
	 * If not, use bound socket address.
	 */
	if (g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(udp_transport->bound_sockaddr_)), IPV4_ANYCAST_ADDRESS)) {
		struct sockaddr_in		remote_native_sockaddr;
		struct sockaddr_in		local_native_sockaddr;

		local_native_sockaddr.sin_family = AF_INET;
		local_native_sockaddr.sin_port = g_htons(g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS(udp_transport->bound_sockaddr_)));

		g_socket_address_to_native (remote_glib_sockaddr, &remote_native_sockaddr, sizeof(struct sockaddr_in), &gerror);
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

		local_glib_sockaddr = g_socket_address_new_from_native((gpointer)&local_native_sockaddr, sizeof(struct sockaddr_in));
	} else
		local_glib_sockaddr = g_object_ref(G_INET_SOCKET_ADDRESS(udp_transport->bound_sockaddr_));

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
		goto _UDPv4Connect_failed;
	}

	if (remote_glib_sockaddr) g_object_unref (remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref (local_glib_sockaddr);

	udp_transport->transport_mode_ = kUdpv4ModeClient;

	return TRUE;

	_UDPv4Connect_failed:
	if (remote_glib_sockaddr) g_object_unref (remote_glib_sockaddr);
	if (local_glib_sockaddr) g_object_unref (local_glib_sockaddr);
	return FALSE;
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
	return IpcomMessageGetPaylodLength(mesg) + VCCPDUHEADER_SIZE;

	_UDPv4NativeSend_failed:
	return -1;
}

static gint
_UDPv4Broadcast(IpcmdTransport *transport, guint16 dport, IpcmdMessage *mesg)
{
	IpcmdTransportUdpv4		*udp_transport = IPCMD_OBJECT_GET_TRANSPORT_UDPv4(transport);
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
	bound_glib_inetaddr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(udp_transport->bound_sockaddr_));

	/// If socket is bound to anycast address, we broadcast for each broadcast address.
	if (g_inet_address_equal(bound_glib_inetaddr, IPV4_ANYCAST_ADDRESS)) {
		broadcast_addr_list = IpcomNetifcMonitorGetAllIpv4BroadAddr(IpcomNetifcGetInstance());
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
