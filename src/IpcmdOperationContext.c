/*
 * IpcmdOperationContext.c
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperationContext.h"
#include <glib.h>

struct _IpcmdOperationContextId {
	IpcmdChannelId	channel_id_;
	guint32			sender_handle_id_;
};

struct _IpcmdOperationContext {
	struct _IpcmdOperationContextId	opctx_id_;
};


