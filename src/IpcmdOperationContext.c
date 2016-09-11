/*
 * IpcmdOperationContext.c
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperationContext.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOpStateMachine.h"
#include "IpcmdDeclare.h"
#include <glib.h>

IpcmdOpCtx*
IpcmdOpCtxNew()
{

}

void
IpcmdOpCtxSetMessage(IpcmdOpCtx *self, IpcmdMessage *mesg)
{

}

guint
IpcmdOpCtxIdHashfunc(gconstpointer key) {
	IpcmdOpCtxId *opctx_id = (IpcmdOpCtxId*)key;
	return (guint)(opctx_id->channel_id_ + opctx_id->sender_handle_id_);
}

gboolean
IpcmdOpCtxIdEqual(gconstpointer a, gconstpointer b) {
	return !memcmp(a,b,sizeof(IpcmdOpCtxId)) ? TRUE : FALSE;
}

gint
IpcomOpCtxTrigger(IpcmdOpCtx *self, gint trigger, gpointer data)
{

}
//gboolean									IpcomOpContextSetCallbacks(IpcomOpContext *opContext, IpcomReceiveMessageCallback recv_cb, IpcomOpCtxDestroyNotify OnNotify, void *userdata);

gboolean
IpcmdOpCtxStartWFATimer(IpcmdOpCtx *self)
{

}
gboolean
IpcmdOpCtxStartWFRTimer(IpcmdOpCtx *self)
{
}
gboolean
IpcmdOpCtxSetTimer(IpcmdOpCtx *self, gint milliseconds, GSourceFunc func);
gboolean
IpcmdOpCtxUnsetTimer(IpcmdOpCtx *self);
gboolean
IpcmdOpCtxCancelTimer(IpcmdOpCtx *self);

IpcmdOpCtx*
IpcmmOpCtxRef(IpcmdOpCtx *self)
{

}
void
IpcmdOpCtxUnref(IpcmdOpCtx *self)
{

}



