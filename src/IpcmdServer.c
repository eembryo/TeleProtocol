/*
 * IpcmdServer.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdServer.h"
#include <glib.h>

struct _IpcmdServer {
	GHashTable	*operation_contexts_;
	GList		*services_;
	IpcmdCore	*core_;
};

gint
IpcmdServerHandleMessage(struct _IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg)
{

}

gboolean
IpcmdServerRegisterService(struct _IpcmdServer *self, IpcmdService *service)
{

	return FALSE;
}

void
IpcmdServiceunregisterService(struct _IpcmdServer *self, IpcmdService *service)
{

}
