#ifndef __IpcomTransport_h__
#define __IpcomTransport_h__

#include <glib.h>
#include <gio/gio.h>
#include <IpcomTypes.h>

typedef gint 				(*IpcomTransportTransmit)			(IpcomTransport *, IpcomConnection *, IpcomMessage*);
typedef gboolean 			(*IpcomTransportBind)				(IpcomTransport *, const gchar *ip, guint16 port);
typedef gboolean 			(*IpcomTransportAddConnection)		(IpcomTransport *, IpcomConnection *, gpointer data);
typedef gboolean 			(*IpcomTransportOnReadyToReceive)	(IpcomTransport *, GSocket *);
typedef IpcomConnection*	(*IpcomTransportConnect)			(IpcomTransport *, const gchar *ip, guint16 port);

struct _IpcomTransport {
	GSocket			*socket;
	IpcomProtocol	*protocol;

	IpcomTransportTransmit	transmit;
	IpcomTransportConnect	connect;
	IpcomTransportBind		bind;
	IpcomTransportOnReadyToReceive	onReadyToReceive;
	//listen
	//addConnection()
};

#endif
