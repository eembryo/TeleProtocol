/*
 * IpcmdChannel.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdChannel.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdTransport.h"
#include <glib.h>

gboolean
IpcmdChannelEqualEndpoints(const IpcmdChannel *a, const IpcmdChannel *b)
{
	return	a->local_host_->equal (a->local_host_, b->local_host_) &&
			a->remote_host_->equal (a->remote_host_, b->remote_host_) ? TRUE : FALSE;
}

/* @fn: IpcmdChannelIsConnectionOriented
 *
 * return -1 on error
 * return 0 on connection-less channel.
 * return 1 on connection-oriented channel.
 */
gint
IpcmdChannelIsConnectionOriented(const IpcmdChannel *channel)
{
	switch (channel->transport_->type_) {
	case kIpcmdTransportUdpv4:
		return 0;
	case kIpcmdTransportTcpv4:
		return 1;
	default:
		return -1;
	}
}
