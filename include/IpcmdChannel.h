/*
 * IpcmdChannel.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCHANNEL_H_
#define INCLUDE_IPCMDCHANNEL_H_

#include "IpcmdHost.h"
#include <glib.h>

typedef guint16					IpcmdChannelId;
typedef struct _IpcmdChannel	IpcmdChannel;
struct _IpcmdChannel {
	IpcmdChannelId		channel_id_;		// channel_id_ is fixed when channel is registered at IpcmdBus
	enum IpcmdChannelStatus	status_;
	IpcmdHost			*local_host_;
	IpcmdHost			*remote_host_;
	IpcmdTransport		*transport_;
	gchar				priv_data_[0];		// Each transport has different private data structure
};

enum IpcmdChannelStatus {
	kChannelClosed,
	kChannelOpening,
	kChannelEstablished,
	kChannelClosing,
};

static inline gboolean
IpcmdChannelFlowEqual(const IpcmdChannel *a, const IpcmdChannel *b)
{
	return	a->local_host_->equal (a->local_host_, b->local_host_) &&
			a->remote_host_->equal (a->remote_host_, b->remote_host_) ? TRUE : FALSE;
}

#endif /* INCLUDE_IPCMDCHANNEL_H_ */
