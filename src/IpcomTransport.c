// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#include <glib.h>
#include <IpcomTransport.h>
#include <IpcomTransportUDPv4.h>
#include <dprint.h>

IpcomTransport *
IpcomTransportNew(IpcomTransportType type, GMainContext *mainContext)
{
	IpcomTransport *newTransport = NULL;

	g_assert(mainContext);

	switch(type) {
	case IPCOM_TRANSPORT_UDPV4:
		newTransport = IpcomTransportUDPv4New(mainContext);
		if (newTransport)
			IpcomTransportUDPv4AttachGlibContext(newTransport, mainContext);
		break;
	default:
		DERROR("This protocol type(%d) is not supported.\n", type);
		goto _CreateError;
	}

	return newTransport;

	_CreateError:
	return NULL;
}

IpcomConnection *
IpcomTransportConnect(IpcomTransport *transport, gchar *localIpAddr, guint16 localPort, gchar *remoteIpAddr, guint16 remotePort)
{
	//IMPLEMENT: transport->status == IPCOM_TRANSPORT_NONE
	guint16	lport = (localPort == 0 ? remotePort : localPort);

	g_assert(transport && remoteIpAddr);

	if (!transport->bind(transport, localIpAddr, lport))
		goto _ConnectError;

	return transport->connect( transport, remoteIpAddr, remotePort);

	_ConnectError:
	return NULL;
}

gboolean
IpcomTransportListen(IpcomTransport *transport, gchar *localIpAddr, gint localPort, IpcomNewConnectionCb newConnCb, gpointer cb_data)
{
	//IMPLEMENT: transport->status == IPCOM_TRANSPORT_NONE

	g_assert(transport && localIpAddr);

	transport->onNewConn = newConnCb;
	transport->onNewConn_data = cb_data;

	if (!transport->bind(transport, localIpAddr, localPort)) {
		goto _ListenError;
	}

	return transport->listen(transport, 10);

	_ListenError:
	return FALSE;
}

gboolean
IpcomTransportDestroy(IpcomTransport *transport)
{
	switch(transport->type) {
	case IPCOM_TRANSPORT_UDPV4:
		IpcomTransportUDPv4Destroy(transport);
		break;
	default:
		DERROR("This protocol type(%d) is not supported for destroying.\n", transport->type);
		goto _DestroyError;
	}

	return TRUE;

	_DestroyError:
	return FALSE;
}
