#include <glib.h>
#include <IpcomConnection.h>
#include <IpcomProtocol.h>
#include <IpcomTransport.h>
#include <IpcomMessage.h>
#include <ref_count.h>
#include <dprint.h>

gint
IpcomConnectionTrnasmitMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	g_assert(conn->transport);
	conn->transport->transmit(conn->transport, conn, mesg);
	return 0;
}

static void _IpcomConnectionFree(struct ref *r)
{
	IpcomConnection *pConn = container_of(r, IpcomConnection, _ref);
	r->count = -1;
	g_free(pConn);
}

IpcomConnection *
IpcomConnectionGet(IpcomConnection *conn)
{
	ref_inc(&conn->_ref);
	return conn;
}

void
IpcomConnectionPut(IpcomConnection *conn)
{
	ref_dec(&conn->_ref);
}

IpcomConnection *
IpcomConnectionNew(GSocketAddress *localSockAddr, GSocketAddress *remoteSockAddr)
{
	IpcomConnection *conn;

	conn = g_malloc0(sizeof(IpcomConnection));
	if (localSockAddr)
		conn->localSockAddr = g_object_ref(localSockAddr);

	g_assert(remoteSockAddr);
	conn->remoteSockAddr = g_object_ref(remoteSockAddr);

	ref_init(&conn->_ref, _IpcomConnectionFree);

	return IpcomConnectionGet(conn);
}

gint
IpcomConnectionPushOutgoingMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	DFUNCTION_START;

	conn->transport->transmit(conn->transport, conn, mesg);
	return 0;
}

gint
IpcomConnectionPushIncomingMessage(IpcomConnection *conn, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomProtocolHandleMessage(conn->protocol, conn, mesg);
	return 0;
}

#if 0

gint
IpcomConnectionPopOutgoingMessage()
{
	return 0;
}

gint
IpcomConnectionPopIncomingMessage()
{
	return 0;
}
#endif
