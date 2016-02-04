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

#ifndef __IpcomTransport_h__
#define __IpcomTransport_h__


#include <glib.h>
#include <gio/gio.h>
#include <IpcomTypes.h>
#include <IpcomEnums.h>

G_BEGIN_DECLS

typedef gint 				(*_IpcomTransportTransmit)			(IpcomTransport *, IpcomConnection *, IpcomMessage*);
typedef gboolean 			(*_IpcomTransportBind)				(IpcomTransport *, const gchar *ip, guint16 port);
typedef gboolean 			(*_IpcomTransportAddConnection)		(IpcomTransport *, IpcomConnection *, gpointer data);
typedef gboolean 			(*_IpcomTransportOnReadyToReceive)	(IpcomTransport *, GSocket *);
typedef IpcomConnection*	(*_IpcomTransportConnect)			(IpcomTransport *, const gchar *ip, guint16 port);
typedef gboolean			(*_IpcomTransportListen)				(IpcomTransport *, gint backlog);
typedef IpcomConnection*	(*_IpcomTransportLookup)				(IpcomTransport *, const gchar *localIpAddr, guint16 localPort, const gchar *remoteIpAddr, guint16 remotePort);
typedef gboolean			(*IpcomNewConnectionCb)				(IpcomConnection *, gpointer);

struct _IpcomTransport {
	GSocket			*socket;
	IpcomProtocol	*protocol;

	IpcomTransportType		type;
	_IpcomTransportTransmit	transmit;
	_IpcomTransportConnect	connect;
	_IpcomTransportBind		bind;
	_IpcomTransportListen	listen;
	_IpcomTransportLookup	lookup;
	_IpcomTransportOnReadyToReceive	onReadyToReceive;
	IpcomNewConnectionCb	onNewConn;
	gpointer				onNewConn_data;
	//addConnection()
};

IpcomTransport 		*IpcomTransportNew(IpcomTransportType type, GMainContext *mainContext);
/* IpcomTransportConnect:
 * transport	: IpcomTransport
 * localIpAddr	: local IP address, written in dot-decimal notation. If it is NULL,
 * localPort	: port number
 * remoteIpAddr	: remote IP address, written in dot-decimal notation.
 * remotePort	: remote port number
 */
IpcomConnection		*IpcomTransportConnect(IpcomTransport *transport, gchar *localIpAddr, guint16 localPort, gchar *remoteIpAddr, guint16 remotePort);
gboolean 			IpcomTransportListen(IpcomTransport *transport, gchar *localIpAddr, gint localPort, IpcomNewConnectionCb newConnCb, gpointer cb_data);

G_END_DECLS

#endif
