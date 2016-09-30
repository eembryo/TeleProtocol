/*
 * IpcmdBus.h
 *
 *  Created on: Aug 29, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDBUS_H_
#define INCLUDE_IPCMDBUS_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	kBusEventChannelAdd,
	kBusEventChannelRemove,
	kBusEventChannelStatusChange,
} IpcmdBusEventType;

struct _IpcmdBus {
	GHashTable		*channels_;				// key: positive integer, value: IpcmdChannel*
	GList			*transports_;			// data: IpcmdTransport*
	guint16			last_alloc_channel_id_;	// the last allocated channel id
	GList			*event_listeners_;		// data: IpcmdBusEventListener*
	IpcmdCore		*core_;
};

struct _IpcmdBusEventListener{
	void (*OnChannelEvent)(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
};

gboolean 	IpcmdBusAddEventListener(IpcmdBus *self, IpcmdBusEventListener *listener);
void		IpcmdBusRemoveEventListener(IpcmdBus *self, IpcmdBusEventListener *listener);
void 		IpcmdBusNotifyChannelEvent(IpcmdBus *self, IpcmdChannelId id, const IpcmdBusEventType ev_type, gconstpointer ev_data);

void 		IpcmdBusInit(IpcmdBus *self,IpcmdCore *core);
void 		IpcmdBusFinalize(struct _IpcmdBus *self);
guint16 	IpcmdBusRegisterChannel(IpcmdBus *self, IpcmdChannel *channel);
void 		IpcmdBusUnregisterChannel(IpcmdBus *self, IpcmdChannel *channel);
GList*		IpcmdBusFindChannelIdsByPeerHost(IpcmdBus *self, IpcmdHost *remote);
gboolean 	IpcmdBusAttachTransport(IpcmdBus *self, IpcmdTransport *transport);
void 		IpcmdBusDetachTransport(IpcmdBus *self, IpcmdTransport *transport);
gint		IpcmdBusTx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
gint		IpcmdBusRx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
IpcmdChannel*	IpcmdBusFindChannelById(IpcmdBus *self, IpcmdChannelId id);

G_END_DECLS

#endif /* INCLUDE_IPCMDBUS_H_ */
