/*
 * IpcmdChannel.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdChannel.h"
#include "../include/IpcmdHost.h"
#include <glib.h>

gboolean
IpcmdChannelEqualEndpoints(const IpcmdChannel *a, const IpcmdChannel *b)
{
	return	a->local_host_->equal (a->local_host_, b->local_host_) &&
			a->remote_host_->equal (a->remote_host_, b->remote_host_) ? TRUE : FALSE;
}
