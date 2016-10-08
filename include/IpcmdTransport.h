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

//typedef gboolean			(*_IpcomTransportCloseChannel)		(IpcmdTransport*,const IpcmdChannelId channel_id);

struct _IpcmdTransport {
	IpcmdBus					*bus_;	// will be set when attaching to IpcmdBus
	void						(*OnAttachedToBus)(IpcmdTransport *transport, IpcmdBus *bus);
	void						(*OnDetachedFromBus)(IpcmdTransport *transport, IpcmdBus *bus);

	/* following items should be set by inherited class before attaching. */
	GSource						*source_;	// source_ is attached to mainloop at IpcmdBusAttachTransport().
	enum IpcmdTransportType		type_;
	gboolean					(*bind)(IpcmdTransport*,const gchar *ip,const guint16 port);
	gint						(*connect)(IpcmdTransport *transport,const gchar *ip,const guint16 port);
	gboolean					(*listen)(IpcmdTransport *transport, const gint backlog);
	gint						(*transmit)(IpcmdTransport *transport, const IpcmdChannel *channel, IpcmdMessage *message);
	gboolean					(*EnableBroadcast)(IpcmdTransport *transport);
	void						(*DisableBroadcast)(IpcmdTransport *transport);
};

G_END_DECLS

#endif /* INCLUDE_IPCMDTRANSPORT_H_ */
