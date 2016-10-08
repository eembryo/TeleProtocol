#ifndef INCLUDE_IPCMDHOST_H_
#define INCLUDE_IPCMDHOST_H_

#include "IpcmdDeclare.h"
#include "reference.h"
#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

enum IpcmdHostLinkType {
	/* INET hosts */
	IPCMD_HOSTLINK_UDPv4 = 0,
	IPCMD_HOSTLINK_TCPv4,
	//IPCMD_UDPv6_HOST,
	//IPCMD_TCPV6_HOST,

	/* something different hosts */
	// unix socket host
	// pipe endpoint
};

/* IpcmdHostHostCompare: return TRUE if two instances are same or return FALSE */
typedef gboolean (*IpcmdHostEqual)(const IpcmdHost* a, const IpcmdHost* b);
typedef IpcmdHost* (*IpcmdHostDup)(const IpcmdHost* a);

/*
 * IpcmdHost: abstract class for containing location information of specific device.
 */
struct _IpcmdHost {
	enum IpcmdHostLinkType	host_type_;
	IpcmdHostEqual		equal;
	IpcmdHostDup		duplicate;
	struct ref			ref_;
};

#define IPCMD_HOST(o) (IpcmdHost*)(o)

gpointer	IpcmdHostRef (IpcmdHost *host);
void		IpcmdHostUnref (IpcmdHost *host);
gboolean	IpcmdHostIsConnectionless(IpcmdHost *host);
static inline guint		IpcmdHostType (const IpcmdHost *host) {return host->host_type_;}

/**
 * IpcmdUdpv4Host: contains IPv4 address and UDP port information.
 */
struct _IpcmdUdpv4Host {
	struct _IpcmdHost parent_;
	GInetSocketAddress*	inet_sockaddr_;
};
typedef struct _IpcmdUdpv4Host	IpcmdUdpv4Host;

static inline IpcmdUdpv4Host* IPCMD_UDPv4HOST(IpcmdHost *obj) {
	if (obj->host_type_ != IPCMD_HOSTLINK_UDPv4) return NULL;
	return container_of (obj, struct _IpcmdUdpv4Host, parent_);
}

IpcmdHost*	IpcmdUdpv4HostNew (GInetAddress *address, guint16 port);
IpcmdHost*	IpcmdUdpv4HostNew2 (GInetSocketAddress *sock_addr);
IpcmdHost*	IpcmdUdpv4HostNew3 (const gchar *ip_addr, guint16 port);

G_END_DECLS

#endif
