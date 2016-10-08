/*
 * IpcmdChannel.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCHANNEL_H_
#define INCLUDE_IPCMDCHANNEL_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

enum IpcmdChannelStatus {
	kChannelClosed,
	kChannelOpening,
	kChannelConnecting,
	kChannelEstablished,
	kChannelClosing,
};

struct _IpcmdChannel {
	IpcmdChannelId		channel_id_;		// channel_id_ is assigned when channel is registered at IpcmdBus
	enum IpcmdChannelStatus	status_;
	IpcmdHost			*local_host_;
	IpcmdHost			*remote_host_;
	IpcmdTransport		*transport_;
	gchar				priv_data_[0];		// Each transport has different private data structure
};

static inline IpcmdChannelId	IpcmdChannelGetId(const IpcmdChannel *channel) { return channel->channel_id_;}
gboolean	IpcmdChannelEqualEndpoints(const IpcmdChannel *a, const IpcmdChannel *b);
gint		IpcmdChannelIsConnectionOriented(const IpcmdChannel *channel);
void		IpcmdChannelInit (IpcmdChannel *channel, IpcmdHost *local_host, IpcmdHost *remote_host, IpcmdTransport *transport, enum IpcmdChannelStatus init_status);

G_END_DECLS

#endif /* INCLUDE_IPCMDCHANNEL_H_ */
