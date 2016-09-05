/*
 * IpcmdTransport.h
 *
 *  Created on: Aug 29, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDTRANSPORT_H_
#define INCLUDE_IPCMDTRANSPORT_H_

#include "IpcmdBus.h"
#include <glib.h>

typedef gint 				(*_IpcmdTransportTransmit)			(IpcmdTransport*,const IpcmdChannel*,const IpcmdMessage*);
//typedef IpcomConnection*	(*_IpcomTransportGetBroadConnection)(IpcomTransport*);
typedef gboolean 			(*_IpcmdTransportBind)				(IpcmdTransport*,const gchar *ip,const guint16 port);
typedef gboolean 			(*_IpcomTransportOnReadyToReceive)	(IpcmdTransport*,GSocket *);
typedef IpcmdChannelId		(*_IpcmdTransportConnect)			(IpcmdTransport*,const gchar *ip,const guint16 port);
typedef gboolean			(*_IpcmdTransportListen)			(IpcmdTransport*,const gint backlog);
typedef gboolean			(*_IpcomTransportCloseChannel)		(IpcmdTransport*,const IpcmdChannelId channel_id);

typedef enum {
	kIpcmdTransportUnknown = 0,
	kIpcmdTransportUdpv4,
} IpcmdTransportType;

typedef struct _IpcmdTransport IpcmdTransport;
struct _IpcmdTransport {
	GSocket			*socket_;
	IpcmdBus		*bus_;

	IpcmdTransportType			type_;
	_IpcmdTransportConnect		connect;
	_IpcmdTransportBind			bind;
	_IpcmdTransportTransmit		transmit;
	_IpcmdTransportListen		listen;
//	_IpcomTransportGetBroadConnection	getBroadConnection;
	_IpcomTransportOnReadyToReceive		onReadyToReceive;
};

#endif /* INCLUDE_IPCMDTRANSPORT_H_ */
