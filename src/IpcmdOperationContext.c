/*
 * IpcmdOperationContext.c
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachine.h"
#include <glib.h>

struct _IpcmdOperationContextId {
	IpcmdChannelId	channel_id_;
	guint32			sender_handle_id_;
} __attribute__ ((packed));

struct _IpcmdOperationContext {
	struct _IpcmdOperationContextId		opctx_id_;

	/// operation information
	guint16								serviceId;
	guint16								operationId;
	guint8								protoVersion;
	guint8								opType;
	//gboolean							procflag;
	IpcmdMessage						*message;

	/// operation state
	IpcmdOpState						mOpState;

	/// timer-related
	GSource								*timer;
	gint								numberOfRetries;
	gint								nWFAMaxRetries;
	gint								nWFRMaxRetries;
	gint 								nWFABaseTimeout;
	gint 								nWFRBaseTimeout;
	gfloat 								nWFAIncreaseTimeout;
	gfloat 								nWFRIncreaseTimeout;
	gboolean							(*OnWFAExpired)(gpointer data);
	gboolean							(*OnWFRExpired)(gpointer data);

	///callback functions
	//IpcomOpCtxDestroyNotify			NotifyDestroyed;
	//IpcomReceiveMessageCallback		recvCallback;
	//void 								*cb_data;

	struct ref							_ref;
};


