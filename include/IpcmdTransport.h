/*
 * IpcmdTransport.h
 *
 *  Created on: Aug 29, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDTRANSPORT_H_
#define INCLUDE_IPCMDTRANSPORT_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

enum IpcmdTransportType {
	kIpcmdTransportUnknown = 0,
	kIpcmdTransportUdpv4,
	kIpcmdTransportTcpv4,
};

typedef gint 				(*IpcmdTransportTransmit)			(IpcmdTransport*,const IpcmdChannel*,IpcmdMessage*);
//typedef IpcomConnection*	(*_IpcomTransportGetBroadConnection)(IpcomTransport*);
typedef gboolean 			(*IpcmdTransportBind)				(IpcmdTransport*,const gchar *ip,const guint16 port);
typedef gint				(*IpcmdTransportConnect)			(IpcmdTransport*,const gchar *ip,const guint16 port);
typedef gboolean			(*IpcmdTransportListen)			(IpcmdTransport*,const gint backlog);
//typedef gboolean			(*_IpcomTransportCloseChannel)		(IpcmdTransport*,const IpcmdChannelId channel_id);

struct _IpcmdTransport {
	IpcmdBus					*bus_;	// will be set when attaching to IpcmdBus

	/* following items should be set by inherited class before attaching. */
	GSource						*source_;	// source_ is attached to mainloop at IpcmdBusAttachTransport().
	enum IpcmdTransportType		type_;
	IpcmdTransportBind			bind;
	IpcmdTransportConnect		connect;
	IpcmdTransportListen		listen;
	IpcmdTransportTransmit		transmit;
};

G_END_DECLS

#endif /* INCLUDE_IPCMDTRANSPORT_H_ */
