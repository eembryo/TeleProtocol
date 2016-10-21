/*
 * IpcmdClientImpl.h
 *
 *  Created on: Oct 7, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCLIENTIMPL_H_
#define INCLUDE_IPCMDCLIENTIMPL_H_

#include <glib.h>
#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdMessageHandler.h"

G_BEGIN_DECLS

struct _IpcmdClient {
	IpcmdMessageHandlerInterface	msg_handler_;
	IpcmdHost		*server_host_;
	IpcmdChannelId	channel_id_;
	GHashTable		*operation_contexts_;
	GList			*subscribed_notifications_;
	//guint16			service_id_;
	guint8			seq_num_;
	IpcmdCore		*core_;
	IpcmdBusEventListener	listener_;
	void			(*OnRegisteredToCore)(IpcmdClient *self, IpcmdCore *core);
	void			(*OnUnregisteredFromCore)(IpcmdClient *self, IpcmdCore *core);
};

G_END_DECLS

#endif /* INCLUDE_IPCMDCLIENTIMPL_H_ */
