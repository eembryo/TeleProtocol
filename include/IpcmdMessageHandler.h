/*
 * IpcmdMessageHandler.h
 *
 *  Created on: Oct 20, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDMESSAGEHANDLER_H_
#define INCLUDE_IPCMDMESSAGEHANDLER_H_

typedef struct _IpcmdMessageHandlerInterface {
	gint (*handle)(struct _IpcmdMessageHandlerInterface *self, IpcmdChannelId channel_id, IpcmdMessage *mesg);
} IpcmdMessageHandlerInterface;


#endif /* INCLUDE_IPCMDMESSAGEHANDLER_H_ */
