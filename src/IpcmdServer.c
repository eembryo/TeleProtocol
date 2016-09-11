/*
 * IpcmdServer.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdServer.h"
#include "../include/IpcmdServerImpl.h"
#include "../include/IpcmdBus.h"
#include <glib.h>

#define IPCMD_SERVER_FROM_LISTENER(l) (container_of(l, struct _IpcmdServer, listener_)

static void _ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data);
static void _DestroyOpCtx (gpointer opctx);

gint
IpcmdServerCompleteOperation(IpcmdServer *self, IpcmdOpCtxId opctx_id, const IpcmdOperationResult *result)
{
	//1. find opctx
	//2. trigger PROCESSDONE to opctx with result
	return 0;
}

gint
IpcmdServerHandleMessage(IpcmdServer *self, IpcmdChannelId channel_id, IpcmdMessage* mesg)
{
	IpcmdOpCtx 		*opctx = NULL;
	gint			ret;
	IpcmdOpCtxId	opctx_id = {
			.channel_id_ = channel_id,
			.sender_handle_id_ = IpcmdMessageGetVCCPDUSenderHandleID(mesg)
	};

	IpcmdMessageRef(mesg);

	/* validation check */
	// ProtocolVersion
	// ServiceID
	// OperationID
	// OperationType
	// Length
	// Busy
	// Processing

	switch (IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_REQUEST:
		ret = IpcmdCoreAllocOpCtx (self->core_, opctx_id, &opctx);
		if (!ret) { // failed to allocate opctx
			g_warning("failed to allocate operation context: %d", ret);
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-REQUEST to opctx
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		ret = IpcmdCoreAllocOpCtx (self->core_, opctx_id, &opctx);
		if (!ret) { // failed to allocate opctx
			g_warning("failed to allocate operation context: %d", ret);
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-SETNOR to opctx
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		ret = IpcmdCoreAllocOpCtx (self->core_, opctx_id, &opctx);
		if (!ret) { // failed to allocate opctx
			g_warning("failed to allocate operation context: %d", ret);
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-SETREQ to opctx
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		ret = IpcmdCoreAllocOpCtx (self->core_, opctx_id, &opctx);
		if (!ret) { // failed to allocate opctx
			g_warning("failed to allocate operation context: %d", ret);
			goto _HandleMessage_failed;
		}
		//1. initialize opctx
		//2. trigger RECV-NOTREQ to opctx
		break;
	case IPCMD_OPTYPE_ACK:
		//1. find opctx from operations_
		//2. trigger RECV-ACK to opctx
		break;
	case IPCMD_OPTYPE_ERROR:
		//1. find opctx from operations_
		//2. trigger RECV-ERROR to opctx
		break;
	default :
		goto _HandleMessage_failed;
	}

	IpcmdMessageUnref(mesg);
	return 0;

	_HandleMessage_failed:
	IpcmdMessageUnref(mesg);
	return -1;
}

gboolean
IpcmdServerRegisterService(IpcmdServer *self, IpcmdService *service)
{
	GList *l;

	for (l = self->services_; l!=NULL; l=l->next) {
		if (l->data == service) // already registered service
			return FALSE;
	}

	self->services_ = g_list_append (self->services_, service);

	return TRUE;
}

void
IpcmdServiceUnregisterService(IpcmdServer *self, IpcmdService *service)
{
	self->services_ = g_list_remove (self->services_, service);
}

void
IpcmdServerInit(IpcmdServer *self, IpcmdCore *core)
{
	//self->operation_contexts_ = g_hash_table_new (hash_fn, key_equal_fn);
	self->services_ = NULL;
	self->core_ = core;
	self->listener_.OnChannelEvent = _ReceiveChannelEvent;
	self->operation_contexts_ = g_hash_table_new_full (IpcmdOpCtxIdHashfunc, IpcmdOpCtxIdEqual, NULL, _DestroyOpCtx);
	IpcmdBusAddEventListener (IpcmdCoreGetBus(self->core_),self->listener_);
}

void
IpcmdServerFinalize(IpcmdServer *self)
{
	IpcmdBusRemoveEventListener (IpcmdCoreGetBus(self->core_), self->listener_);
	// finalize self->services_
}

static void
_ReceiveChannelEvent(IpcmdBusEventListener *self, IpcmdChannelId id, guint type, gconstpointer data)
{
	IpcmdServer *server = IPCMD_SERVER_FROM_LISTNER(self);

	g_debug("Got channel event: id=%d, type=%d", id, type);
}


static void
_DestroyOpCtx (gpointer opctx)
{
	IpcmdOpCtxUnref(opctx);
}
