#include "../include/IpcmdHost.h"
#include "../include/reference.h"
#include <glib.h>
#include <gio/gio.h>

enum IpcmdHostType {
	// inet hosts
	IPCMD_UDPv4_HOST = 0,
	IPCMD_TCPv4_HOST,
	// something different hosts
};
/*
 * IpcmdHostHostCompare: return 0 if two instances are same or -1
 */
typedef gboolean (*IpcmdHostEqual)(const IpcmdHost* a, const IpcmdHost* b);

/*
 * IpcmdHost: abstract class for containing location information of specific device.
 *
 */
struct _IpcmdHost {
	enum IpcmdHostType	host_type_;
	IpcmdHostEqual		equal;
	struct ref			ref_;
};

gpointer
IpcmdHostRef (IpcmdHost *host)
{
	if(get_ref_count(&host->ref_) < 0)
		g_error ("IpcmdHost has negative reference count(%d).", get_ref_count(&host->ref_));
	ref_inc(&host->ref_);

	return host;
}

void
IpcmdHostUnref (IpcmdHost *host)
{
	if(get_ref_count(&host->ref_) < 0)
		g_error ("IpcmdHost has negative reference count(%d).", get_ref_count(&host->ref_));
	ref_dec(&host->ref_);
}

/**
 * IpcmdUdpv4Host: contains IPv4 address and UDP port information.
 */
struct _IpcmdUdpv4Host {
	struct _IpcmdHost parent_;
	GInetSocketAddress*	socket_address_;
};


static gboolean
_Udpv4HostEqual(const IpcmdHost* a, const IpcmdHost* b)
{
	struct _IpcmdUdpv4Host* udpv4_host_a = (IpcmdUdpv4Host*)a;
	struct _IpcmdUdpv4Host* udpv4_host_b = (IpcmdUdpv4Host*)b;

	if (a->host_type_ != IPCMD_UDPv4_HOST || b->host_type_ != IPCMD_UDPv4_HOST) return -1;

	return	g_inet_address_equal (g_inet_socket_address_get_address (udpv4_host_a->socket_address_), g_inet_socket_address_get_address (udpv4_host_b->socket_address_)) &&
			g_inet_socket_address_get_port (udpv4_host_a->socket_address_) == g_inet_socket_address_get_port (udpv4_host_a->socket_address_) ? TRUE : FALSE;
}

static void
_Udpv4HostFree(struct ref *r)
{
	struct _IpcmdUdpv4Host *host = (struct _IpcmdUdpv4Host*)container_of(r, struct _IpcmdHost, ref_);

	g_object_unref (host->socket_address_);
	g_free (host);
}

IpcmdUdpv4Host*
IpcmdUdpv4HostNew (GInetAddress *address, guint16 port)
{
	struct _IpcmdUdpv4Host* host = g_malloc(sizeof(struct _IpcmdUdpv4Host));

	if (!host) goto _IpcmdUdpv4HostNew_failed;

	host->parent_.host_type_ = IPCMD_UDPv4_HOST;
	host->parent_.equal = _Udpv4HostEqual;
	host->socket_address_ = g_inet_socket_address_new (address, port);
	if (host->socket_address_ == NULL) goto _IpcmdUdpv4HostNew_failed;

	ref_init(&host->parent_.ref_, _Udpv4HostFree);

	_IpcmdUdpv4HostNew_failed:
	if (host) g_free(host);
	return NULL;
}

IpcmdUdpv4Host*
IpcmdUdpv4HostNew2 (GInetSocketAddress *sock_addr)
{
	struct _IpcmdUdpv4Host* host = g_malloc(sizeof(struct _IpcmdUdpv4Host));

	if (!host) goto _IpcmdUdpv4HostNew2_failed;

	host->parent_.host_type_ = IPCMD_UDPv4_HOST;
	host->parent_.equal = _Udpv4HostEqual;
	host->socket_address_ = g_object_ref(sock_addr);

	ref_init(&host->parent_.ref_, _Udpv4HostFree);

	_IpcmdUdpv4HostNew2_failed:
	if (host) g_free(host);
	return NULL;
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
