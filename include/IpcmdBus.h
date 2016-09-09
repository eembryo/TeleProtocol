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

typedef struct {
	void (*OnChannelEvent)(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
} IpcmdBusEventListener;

gboolean 	IpcmdBusAddEventListener(IpcmdBus *self, const IpcmdBusEventListener *listener);
void		IpcmdBusRemoveEventListener(IpcmdBus *self, const IpcmdBusEventListener *listener);
void 		IpcmdBusNotifyChannelEvent(IpcmdBus *self, IpcmdChannelId id, const IpcmdBusEventType ev_type, gconstpointer ev_data);

void 		IpcmdBusInit(IpcmdBus *self);
void 		IpcmdBusFinalize(struct _IpcmdBus *self);
IpcmdChannel*	IpcmdBusFindChannelById(IpcmdBus *self, IpcmdChannelId id);
guint16 	IpcmdBusRegisterChannel(IpcmdBus *self, IpcmdChannel *channel);
void 		IpcmdBusUnregisterChannel(IpcmdBus *self, IpcmdChannel *channel);
GList*		IpcmdBusFindChannelIdsByPeerHost(IpcmdBus *self, IpcmdHost *remote);
gboolean 	IpcmdBusAttachTransport(IpcmdBus *self, IpcmdTransport *transport);
void 		IpcmdBusDetachTransport(IpcmdBus *self, IpcmdTransport *transport);
gint		IpcmdBusTx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
gint		IpcmdBusRx(IpcmdBus *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);

G_END_DECLS

#endif /* INCLUDE_IPCMDBUS_H_ */
