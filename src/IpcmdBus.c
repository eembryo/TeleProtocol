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

static void _NotifyChannelEvent(IpcmdBus *self, IpcmdChannelId id, const IpcmdBusEventType ev_type, gconstpointer ev_data);
static guint16 _GetSpareChannelId(struct _IpcmdBus *self);
static void _OnRemoveChannelFromHashtable(IpcmdChannel *channel);

/* @fn: _GetSpareChannelId :
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
_OnRemoveChannelFromHashtable(IpcmdChannel *channel)
{
	channel->channel_id_ = 0;
}

/* IpcmdBusInit :
 * initialize IpcmdBus.
 */
void
IpcmdBusInit(IpcmdBus *self, IpcmdCore *core)
{
	self->channels_ = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)_OnRemoveChannelFromHashtable);
	self->last_alloc_channel_id_ = 0;
	self->transports_ = NULL;
	self->core_ = core;
}

/* IpcmdBusClear :
 * Clear IpcmdBus memory.
 */
void
IpcmdBusFinalize(struct _IpcmdBus *self)
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

	// Notify new channel is added
	_NotifyChannelEvent(self, channel->channel_id_, kBusEventChannelAdd, NULL);

	return channel->channel_id_;
}

/* IpcmdBusUnregisterChannel:
 * unregister a channel.
 *
 * @self:
 * @channel: IpcmdChannel*
 */
void
IpcmdBusUnregisterChannel(IpcmdBus *self, IpcmdChannel *channel)
{
	if (channel->channel_id_ == INVALID_CHANNEL_ID) return;

	// Notify a channel is removed
	_NotifyChannelEvent(self, channel->channel_id_, kBusEventChannelRemove, NULL);

	/* NOTE: the hash key with 'channel->channel_id_' should exist in self->channels_ hashtable.
	 * And its data should be 'channel'.
	 */
	g_hash_table_remove (self->channels_, GINT_TO_POINTER(channel->channel_id_));
}

/* IpcmdBusFindChannelsByPeerHost:
 * Find channel ids for remote host. The caller should free returned GList with g_list_free().
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

gboolean
IpcmdBusIsChannelIdForPeerHost(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdHost *remote)
{
	IpcmdChannel *channel = IpcmdBusFindChannelById (self, channel_id);

	if (!channel) return FALSE;
	if (remote->equal(remote, channel->remote_host_)) return TRUE;

	return FALSE;
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
	transport->OnAttachedToBus (transport, self);
	g_source_attach (transport->source_, IpcmdCoreGetGMainContext(self->core_));

	return TRUE;
}

void
IpcmdBusDetachTransport(IpcmdBus *self, IpcmdTransport *transport)
{
	GList *l = g_list_find (self->transports_, transport);

	if (l==NULL) return;

	// stop GSource in the transport
	if (!g_source_is_destroyed(transport->source_)) g_source_destroy (transport->source_);
	transport->OnDetachedFromBus (transport, self);
	// detach from the list
	self->transports_ = g_list_delete_link (self->transports_, l);
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
	IpcmdCoreDispatch (self->core_, channel_id, mesg);

	return 0;
}

/* IpcmdBusAddEventListener :
 * Add listener for bus event
 *
 * return TRUE on success
 * return FALSE on failure
 */
gboolean
IpcmdBusAddEventListener(IpcmdBus *self, IpcmdBusEventListener *listener)
{
	GList *l = g_list_find (self->event_listeners_, listener);
	if (l) {
		g_list_free (l);
		return FALSE;
	}

	self->event_listeners_ = g_list_append (self->event_listeners_, listener);
	return TRUE;
}

/* IpcmdBusRemoveEventListener :
 * Stop the listener to get bus events
 *
 * @self:
 * @listener: event listener
 */
void
IpcmdBusRemoveEventListener(IpcmdBus *self, IpcmdBusEventListener *listener)
{
	self->event_listeners_ = g_list_remove (self->event_listeners_, listener);
}

/* @fn: IpcmdBusIsChannelConnectionOriented
 * return -1 on error
 * return 0 on connection-less channel.
 * return 1 on connection-oriented channel.
 */
gint
IpcmdBusIsChannelConnectionOriented(IpcmdBus *self, IpcmdChannelId id)
{
	IpcmdChannel *channel;
	channel = IpcmdBusFindChannelById (self, id);
	return IpcmdChannelIsConnectionOriented (channel);
}

/* _NotifyChannelEvent :
 * Inform a channel event to all subscribers
 *
 * @self : IpcmdBus
 * @id : channel id which is updated
 * @ev_type : event type
 * @ev_data : event data, depended on 'ev_type'
 */
static void
_NotifyChannelEvent(IpcmdBus *self, IpcmdChannelId id, const IpcmdBusEventType ev_type, gconstpointer ev_data)
{
	GList *l;

	g_debug("%s: ChannelId=%d, type=%d", __func__, id, ev_type);
	for (l = self->event_listeners_; l != NULL; l = l->next) {
		((IpcmdBusEventListener*)(l->data))->OnChannelEvent (l->data, id, ev_type, ev_data);
	}
}
