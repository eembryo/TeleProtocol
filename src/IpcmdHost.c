#include "../include/IpcmdHost.h"
#include "../include/reference.h"
#include <glib.h>
#include <gio/gio.h>

/*
 * IpcmdHostRef :
 * Increase reference count for IpcmdHost instance.
 *
 * @host : IpcmdHost object
 * return a pointer about IpcmdHost instancet
 */
gpointer
IpcmdHostRef (IpcmdHost *host)
{
	if(get_ref_count(&host->ref_) < 0)
		g_error ("IpcmdHost has negative reference count(%d).", get_ref_count(&host->ref_));
	ref_inc(&host->ref_);

	return host;
}

/*
 * IpcmdHostUnref :
 * Decrease reference count for IpcmdHost object. If reference count reaches to zero,
 * the instance is destroyed.
 *
 * @host : IpcmdHost object
 */
void
IpcmdHostUnref (IpcmdHost *host)
{
	if(get_ref_count(&host->ref_) < 0)
		g_error ("IpcmdHost has negative reference count(%d).", get_ref_count(&host->ref_));
	ref_dec(&host->ref_);
}

gboolean
IpcmdHostIsConnectionless (IpcmdHost *host)
{
	switch (host->host_type_) {
	case IPCMD_HOSTLINK_UDPv4:
		return TRUE;
	case IPCMD_HOSTLINK_TCPv4:
		return FALSE;
	default:
		g_error("IpcmdHostLink type is strange.");
		return FALSE;
	}
}
static gboolean
_Udpv4HostEqual(const IpcmdHost* a, const IpcmdHost* b)
{
	struct _IpcmdUdpv4Host* udpv4_host_a = (IpcmdUdpv4Host*)a;
	struct _IpcmdUdpv4Host* udpv4_host_b = (IpcmdUdpv4Host*)b;

	if (a->host_type_ != IPCMD_HOSTLINK_UDPv4 || b->host_type_ != IPCMD_HOSTLINK_UDPv4) return FALSE;

	{
		GInetAddress *a_inet_addr = g_inet_socket_address_get_address(udpv4_host_a->inet_sockaddr_);
		GInetAddress *b_inet_addr = g_inet_socket_address_get_address(udpv4_host_b->inet_sockaddr_);
#if 0
		{
			gchar *a_str = g_inet_address_to_string (a_inet_addr);
			gchar *b_str = g_inet_address_to_string (b_inet_addr);;
			g_debug("%s:%d == %s:%d ?", a_str, g_inet_socket_address_get_port (udpv4_host_a->inet_sockaddr_), b_str, g_inet_socket_address_get_port (udpv4_host_b->inet_sockaddr_));
		}
#endif
		return g_inet_address_equal (a_inet_addr, b_inet_addr) &&
				g_inet_socket_address_get_port (udpv4_host_a->inet_sockaddr_) == g_inet_socket_address_get_port (udpv4_host_b->inet_sockaddr_) ? TRUE : FALSE;
	}
	/*
	return	g_inet_address_equal (g_inet_socket_address_get_address (udpv4_host_a->inet_sockaddr_), g_inet_socket_address_get_address (udpv4_host_b->inet_sockaddr_)) &&
			g_inet_socket_address_get_port (udpv4_host_a->inet_sockaddr_) == g_inet_socket_address_get_port (udpv4_host_a->inet_sockaddr_) ? TRUE : FALSE;
			*/
}
gboolean
IpcmdUdpv4HostEqual(const IpcmdHost *a, GInetAddress *addr, guint16 port)
{
	struct _IpcmdUdpv4Host* udpv4_host_a = (IpcmdUdpv4Host*)a;
	GInetAddress *a_inet_addr;

	if (a->host_type_ != IPCMD_HOSTLINK_UDPv4) return FALSE;

	a_inet_addr = g_inet_socket_address_get_address(udpv4_host_a->inet_sockaddr_);
	return g_inet_address_equal (a_inet_addr, addr) && g_inet_socket_address_get_port (udpv4_host_a->inet_sockaddr_) == port;
}

gboolean
IpcmdUdpv4HostEqual2(const IpcmdHost *a, GInetSocketAddress *sock_addr)
{
	return IpcmdUdpv4HostEqual (a, g_inet_socket_address_get_address (sock_addr), g_inet_socket_address_get_port (sock_addr));
}

static IpcmdHost*
_Udpv4HostDuplicate(const IpcmdHost *host)
{
	return IpcmdUdpv4HostNew2( ((struct _IpcmdUdpv4Host*)host)->inet_sockaddr_);
}

static gchar*
_Udpv4HostToString(IpcmdHost *self)
{
	struct _IpcmdUdpv4Host *host = (struct _IpcmdUdpv4Host*)self;
	gchar *string;

	if (host->string_) return host->string_;
	host->string_ = g_malloc(25+1);	//udp_xxx.xxx.xxx.xxx_ppppp
	if (!host->string_) return NULL;	// not enough memory
	string = g_inet_address_to_string (g_inet_socket_address_get_address (host->inet_sockaddr_));
	g_snprintf(host->string_, 25+1, "udp_%s_%d", string, g_inet_socket_address_get_port (host->inet_sockaddr_));
	g_free(string);

	return host->string_;
}
/* _Udpv4HostFree :
 * This is called when reference count for IpcmdHost reaches to zero.
 *
 * @r : reference count object
 */
static void
_Udpv4HostFree(struct ref *r)
{
	struct _IpcmdUdpv4Host *host = (struct _IpcmdUdpv4Host*)container_of(r, struct _IpcmdHost, ref_);

	g_object_unref (host->inet_sockaddr_);
	if (host->string_) g_free(host->string_);
	g_free (host);
}

static struct _IpcmdHost udpv4_ops = {
		.host_type_ = IPCMD_HOSTLINK_UDPv4,
		.equal = _Udpv4HostEqual,
		.duplicate = _Udpv4HostDuplicate,
		.to_string = _Udpv4HostToString,
};

/* IpcmdUdpv4HostNew :
 * Create IpcmdUdpv4Host instance and return IpcmdHost*.
 *
 * @address : IPv4 address
 * @port : UDP port
 * return NULL if no memory is available
 * return a pointer to new allocated IpcmdHost memory
 */
IpcmdHost*
IpcmdUdpv4HostNew (GInetAddress *address, guint16 port)
{
	struct _IpcmdUdpv4Host* host = g_malloc(sizeof(struct _IpcmdUdpv4Host));

	if (!host) goto _IpcmdUdpv4HostNew_failed;

	host->parent_ 			= udpv4_ops;
	host->string_ 			= NULL;
	host->inet_sockaddr_	= G_INET_SOCKET_ADDRESS (g_inet_socket_address_new (address, port));
	if (host->inet_sockaddr_ == NULL) goto _IpcmdUdpv4HostNew_failed;
	ref_init(&host->parent_.ref_, _Udpv4HostFree);

	return IpcmdHostRef(&host->parent_);

	_IpcmdUdpv4HostNew_failed:
	if (host) g_free(host);
	return NULL;
}

/* IpcmdUdpv4HostNew :
 * same as IpcmdUdpv4Host, but the socket address, 'sock_addr' which is passed by argument, is
 * referred.
 *
 * @sock_addr : socket address, which is composed of IPv4 address and port.
 * return NULL if no memory is available
 * return a pointer to new allocated IpcmdHost memory
 */
IpcmdHost*
IpcmdUdpv4HostNew2 (GInetSocketAddress *sock_addr)
{
	struct _IpcmdUdpv4Host* host = g_malloc(sizeof(struct _IpcmdUdpv4Host));

	if (!host) goto _IpcmdUdpv4HostNew2_failed;

	host->parent_ 			= udpv4_ops;
	host->string_ 			= NULL;
	host->inet_sockaddr_	= g_object_ref(sock_addr);

	ref_init(&host->parent_.ref_, _Udpv4HostFree);

	return IpcmdHostRef(&host->parent_);

	_IpcmdUdpv4HostNew2_failed:
	if (host) g_free(host);
	return NULL;
}

/* @fn: IpcmdUdpv4HostNew3
 * wrapper function of IpcmdUdpv4HostNew2
 */
IpcmdHost*
IpcmdUdpv4HostNew3 (const gchar *ip_addr, guint16 port)
{
	GInetSocketAddress	*sock_addr = G_INET_SOCKET_ADDRESS(g_inet_socket_address_new_from_string (ip_addr, port));
	IpcmdHost			*ret;

	ret = IpcmdUdpv4HostNew2 (sock_addr);
	g_object_unref(sock_addr);

	return ret;
}

#if 0
/**
 * IpcmdTcpv4Host: contains IPv4 address and UDP port information.
 */
struct _IpcmdTcpv4Host {
	struct _IpcmdHost parent_;
	GInetSocketAddress*	socket_address_;
};


static gint
_Tcpv4HostCompare(const IpcmdHost* a, const IpcmdHost* b)
{
	struct _IpcmdInetHost* inet_host_a = (IpcmdInetHost*)a;
	struct _IpcmdInetHost* inet_host_b = (IpcmdInetHost*)b;

	if (a->host_type_ != IPCMD_TCPv4_HOST || b->host_type_ != IPCMD_TCPv4_HOST) return -1;

	return	g_inet_address_equal (g_inet_socket_address_get_address ((IpcmdInetHost*)a), g_inet_socket_address_get_address ((IpcmdInetHost*)b)) &&
			g_inet_socket_address_get_port ((IpcmdInetHost*)a) == g_inet_socket_address_get_port ((IpcmdInetHost*)b) ? 0 : -1;
}

static void
_Tcpv4HostFree(struct ref *r)
{
	struct _IpcmdTcpv4Host *host = (struct _IpcmdTcpv4Host*)container_of(r, struct _IpcmdHost, ref_);

	g_object_unref (host->socket_address_);
	g_free (host);
}

IpcmdInetHost*
IpcmdTcpv4HostNew (GInetAddress *address, guint16 port)
{
	struct _IpcmdTcpv4Host* host = g_malloc(sizeof(struct _IpcmdTcpv4Host));

	if (!host) goto _IpcmdTcpv4HostNew_failed;

	host->parent_.host_type_ = IPCMD_TCPv4_HOST;
	host->parent_.compare = _Tcpv4HostCompare;
	host->socket_address_ = g_inet_socket_address_new (address, port);

	if (host->socket_address_ == NULL) goto _IpcmdTcpv4HostNew_failed;

	_IpcmdTcpv4HostNew_failed:
	if (host) g_free(host);
	return NULL;
}
#endif
