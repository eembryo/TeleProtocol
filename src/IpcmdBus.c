/*
 * IpcmdBus.c
 *
 *  Created on: Aug 29, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdBus.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdTransport.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdChannel.h"
#include "../include/reference.h"
#include <glib.h>

#define MAX_N_OF_CHANNELS	1024
#define INVALID_CHANNEL_ID	0	//channel number '0' is used for error.

static inline IpcmdCore *IPCMD_BUS_TO_CORE(IpcmdBus *b) {
	return container_of (b, IpcmdCore, bus_);
}

/* _GetSpareChannelId :
 * Return unique channel id unless the number of allocated channel id does not exceed MAX_N_OF_CHANNELS.
 *
 * return 0 if the number of allocated id reaches to MAX_N_OF_CHANNELS.
 * return positive integer if success.
 */
static guint16
_GetSpareChannelId(struct _IpcmdBus *self)
{
	guint i;

	if (g_hash_table_size(self->channels_) > MAX_N_OF_CHANNELS) return 0;

	for (i=self->last_alloc_channel_id_+1; i != self->last_alloc_channel_id_; i++) {
		if (i == 0) i++;
		if (!g_hash_table_contains(self->channels_, GINT_TO_POINTER(i))) {
			self->last_alloc_channel_id_ = i;
			return i;
		}
	}
	return 0;
}

static void
_OnChannelRemovedFromHashtable(IpcmdChannel *channel)
{
	channel->channel_id_ = 0;
}

/* IpcmdBusInit :
 * initialize IpcmdBus.
 */
void
IpcmdBusInit(IpcmdBus *self)
{
	self->channels_ = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)_OnChannelRemovedFromHashtable);
	self->last_alloc_channel_id_ = 0;
	self->transports_ = NULL;
}

/* IpcmdBusClear :
 * Clear IpcmdBus memory.
 */
void
IpcmdBusClear(struct _IpcmdBus *self)
{
	GList *iter;

	g_hash_table_destroy (self->channels_);
	self->last_alloc_channel_id_ = 0;

	for (iter=self->transports_; iter!=NULL; iter=self->transports_->next) {
		IpcmdBusDetachTransport(self, (IpcmdTransport*)iter->data);
	}
	g_list_free (self->transports_);
}

IpcmdChannel *
IpcmdBusFindChannelById(IpcmdBus *self, IpcmdChannelId id)
{
	GHashTableIter	iter;
	gpointer		key, value;
	IpcmdChannel	*channel = NULL;

	g_hash_table_iter_init (&iter, self->channels_);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (((IpcmdChannel *)value)->channel_id_ == id) {
			channel = value;
			break;
		}
	}

	return channel;
}

/* IpcmdBusRegisterChannel:
 * register a new channel. An unique channel id is assigned to "channel".
 *
 * return 0 if failed to register
 * return a channel id(positive integer) if success
 */
guint16
IpcmdBusRegisterChannel(IpcmdBus *self, IpcmdChannel *channel)
{
	if (channel->channel_id_ != INVALID_CHANNEL_ID) { // already registered channel
		return channel->channel_id_;
	}
	channel->channel_id_ = _GetSpareChannelId(self);
	if (channel->channel_id_ == INVALID_CHANNEL_ID)	return 0;

	g_hash_table_insert (self->channels_, GINT_TO_POINTER(channel->channel_id_), channel);

	return channel->channel_id_;
}

/* IpcmdBusUnregisterChannel:
 * unregister a channel.
 */
void
IpcmdBusUnregisterChannel(IpcmdBus *self, IpcmdChannel *channel)
{
	if (channel->channel_id_ == INVALID_CHANNEL_ID) return;

	/* NOTE: the hash key with 'channel->channel_id_' should exist in self->channels_ hashtable.
	 * And its data should be 'channel'.
	 */
	g_hash_table_remove (self->channels_, GINT_TO_POINTER(channel->channel_id_));

	// IMPL: notify an unregistering event to IpcmdServer and IpcmdClients
}

/* IpcmdBusFindChannelsByPeerHost:
 * Find channel ids for remote host. The caller should free returned GList.
 *
 * return NULL if failed.
 */
GList *
IpcmdBusFindChannelIdsByPeerHost(IpcmdBus *self, IpcmdHost *remote)
{
	GHashTableIter	iter;
	gpointer		key,value;
	GList			*ret_list = NULL;

	g_hash_table_iter_init (&iter, self->channels_);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (remote->equal (remote, ((IpcmdChannel *)value)->remote_host_)) {
			ret_list = g_list_append(ret_list, GINT_TO_POINTER(((IpcmdChannel *)value)->channel_id_));
		}
	}

	return ret_list;
}

/* IpcmdBusAttachTransport :
 * Attach transport to bus.
 *
 */
gboolean
IpcmdBusAttachTransport(IpcmdBus *self, IpcmdTransport *transport)
{
	GList *l = g_list_find (self->transports_, transport);

	if (l) { //transport is already registered
		g_list_free(l);
		return TRUE;
	}

	self->transports_ = g_list_append (self->transports_, transport);
	transport->bus_ = self;

	g_source_attach (transport->source_, IpcmdCoreGetGMainContext(IPCMD_BUS_TO_CORE(self)));

	return TRUE;
}

void
IpcmdBusDetachTransport(IpcmdBus *self, IpcmdTransport *transport)
{
	if (!g_source_is_destroyed(transport->source_)) g_source_destroy (transport->source_);
	self->transports_ = g_list_remove (self->transports_, transport);
	transport->bus_ = NULL;
}

/* IpcmdBusTx :
 * Transmit the message to transport layer.
 *
 * return -1 if channel_id is invalid
 * return amount of sent bytes
 */
gint
IpcmdBusTx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	IpcmdChannel *channel= g_hash_table_lookup (self->channels_, GINT_TO_POINTER (channel_id));

	return channel ? channel->transport_->transmit (channel->transport_, channel, mesg) : -1;
}

/* IpcmdBusRx :
 * Deliver the message to IpcmdCore.
 *
 * return -1 on error
 * return 0 on success
 */
gint
IpcmdBusRx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	IpcmdCoreDispatch (IPCMD_BUS_TO_CORE(self), channel_id, mesg);

	return 0;
}
