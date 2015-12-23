#ifndef __IpcomConnection__h__
#define __IpcomConnection__h__

#include <glib.h>
#include <gio/gio.h>
#include <IpcomTypes.h>
#include <ref_count.h>

struct _IpcomConnection {
	GSocketAddress	*localSockAddr;
	GSocketAddress	*remoteSockAddr;
	IpcomProtocol	*protocol;
	IpcomTransport	*transport;
	//<-- Implement incomming queue
	//<-- Implement outgoing queue
	struct ref		_ref;
};

IpcomConnection *IpcomConnectionNew(GSocketAddress *localSockAddr, GSocketAddress *remoteSockAddr);
IpcomConnection *IpcomConnectionGet(IpcomConnection *conn);
void IpcomConnectionPut(IpcomConnection *conn);
gint IpcomConnectionPushOutgoingMessage(IpcomConnection *, IpcomMessage *);
gint IpcomConnectionPushIncomingMessage(IpcomConnection *, IpcomMessage *);

#endif
