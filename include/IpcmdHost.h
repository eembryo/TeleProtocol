#ifndef INCLUDE_IPCMDHOST_H_
#define INCLUDE_IPCMDHOST_H_

#include "IpcmdDeclare.h"
#include "reference.h"
#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

enum IpcmdHostType {
	// inet hosts
	IPCMD_UDPv4_HOST = 0,
	IPCMD_TCPv4_HOST,
	// something different hosts
};

/* IpcmdHostHostCompare: return TRUE if two instances are same or return FALSE */
typedef gboolean (*IpcmdHostEqual)(const IpcmdHost* a, const IpcmdHost* b);

/*
 * IpcmdHost: abstract class for containing location information of specific device.
 */
struct _IpcmdHost {
	enum IpcmdHostType	host_type_;
	IpcmdHostEqual		equal;
	struct ref			ref_;
};

gpointer	IpcmdHostRef (IpcmdHost *host);
void		IpcmdHostUnref (IpcmdHost *host);

/**
 * IpcmdUdpv4Host: contains IPv4 address and UDP port information.
 */
struct _IpcmdUdpv4Host {
	struct _IpcmdHost parent_;
	GInetSocketAddress*	inet_sockaddr_;
};
typedef struct _IpcmdUdpv4Host	IpcmdUdpv4Host;

static inline IpcmdHost* IPCMD_HOST(gpointer obj) {
	return (IpcmdHost*)obj;
}
static inline IpcmdUdpv4Host* IPCMD_UDPv4HOST(IpcmdHost *obj) {
	if (obj->host_type_ != IPCMD_UDPv4_HOST) return NULL;
	return container_of (obj, struct _IpcmdUdpv4Host, parent_);
}

IpcmdHost*	IpcmdUdpv4HostNew (GInetAddress *address, guint16 port);
IpcmdHost*	IpcmdUdpv4HostNew2 (GInetSocketAddress *sock_addr);
G_END_DECLS

#endif
