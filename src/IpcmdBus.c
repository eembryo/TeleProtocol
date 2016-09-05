/*
 * IpcmdBus.c
 *
 *  Created on: Aug 29, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdBus.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdTransport.h"
#include <glib.h>

#define MAX_N_OF_CHANNELS	1024
#define UNAVILABLE_CHANNEL	0	//channel number '0' is used for error.

struct _IpcmdBus {
	GHashTable		*channels_;
	guint			next_available_channel_index_;
	GList			*transports_;
};

typedef struct _IpcmdBus IpcmdBus;

const static IpcmdChannel kUnavailableChannel = {
		.channel_id_ = 0,
		.local_host_ = NULL,
		.remote_host_ = NULL,
		.transport_ = NULL,
		.status = 0,
};

void
IpcmdBusInit(IpcmdBus *self)
{
	self->channels_ = NULL;
	self->next_available_channel_index_ = 1;
	self->transports_ = NULL;
}

void
IpcmdBusClear(struct _IpcmdBus *self)
{
	GList *iter;

	g_list_free (self->channels_);
	self->next_available_channel_index_ = 1;

	for (iter=self->transports_; iter!=NULL; iter=self->transports_->next) {
		IpcmdBusDettachTransport(self, (IpcmdTransport*)iter->data);
	}
	g_list_free (self->transports_);
}

IpcmdChannel *
IpcmdBusFindChannelById(IpcmdBus *self, IpcmdChannelId id)
{
	GList 			*iter;
	IpcmdChannel	*channel = NULL;

	for (iter = self->channels_; iter != NULL; iter = iter->next) {
		if (((IpcmdChannel *)iter->data)->channel_id_ == id) {
			channel = iter->data;
			break;
		}
	}

	return channel;
}

gint
IpcmdBusIncreaseChannelId(IpcmdBus *self)
{
	GList *iter;
	GList *old_iter=NULL;

	for (iter=self->channels_;iter!=NULL;iter=iter->next) {
		if (old_iter == NULL) {
			old_iter = iter;
			continue;
		}
		else if(((IpcmdChannel*)iter->data)->channel_id_ == ((IpcmdChannel*)old_iter->data)->channel_id_+1) {

		}
	}
}

/* IpcmdBusFindSpareChannelId:
 *
 * return negative value if no spare id exists.
 * return IpcmdChannelId if found spare id
 */
gint
IpcmdBusFindSpareChannelId(IpcmdBus *self)
{
	if (!IpcmdBusFindChannelById(self, self->next_available_channel_index_))
		return self->next_available_channel_index_;
	if (g_list_length (self->channels_) >= MAX_N_OF_CHANNELS)
		return -1;
	// find spare id
}

gint
IpcmdBusInsertChannel(IpcmdBus *self, IpcmdChannel *channel)
{
	if (g_list_length(self->channels_) < MAX_N_OF_CHANNELS) {

	}
	return -1;
	IpcmdBusFindChannelById(self, self->next_available_channel_index_)
}

/* IpcmdBusRegisterChannel:
 * register a new channel. An unique channel id is assigned to "channel".
 *
 */
gint
IpcmdBusRegisterChannel(IpcmdBus *self, IpcmdChannel *channel)
{

}

/* IpcmdBusUnregisterChannel:
 * unregister a channel.
 */
void
IpcmdBusUnregisterChannel(IpcmdBus *self, IpcmdChannel *channel)
{

}

/* IpcmdBusFindChannelsByPeerHost:
 * return channels for remote host
 *
 */
GList *
IpcmdBusFindChannelsByPeerHost(struct _IpcmdBus *self, IpcmdHost *remote)
{

}

gboolean
IpcmdBusAttachTransport(struct _IpcmdBus *self, IpcmdTransport *transport)
{

}

void
IpcmdBusDettachTransport(struct _IpcmdBus *self, IpcmdTransport *transport)
{

}
