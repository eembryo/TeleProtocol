/*
 * IpcmdClient.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdClient.h"
#include "../include/IpcmdChannel.h"
#include "../include/IpcmdMessage.h"
#include "../include/IpcmdOperation.h"
#include <glib.h>

struct _IpcmdClient {
	IpcmdChannelId	channel_id_;
	GHashTable		*operation_contexts_;
	GList			*subscribed_notifications_;
	guint16			service_id_;
	IpcmdCore		*core_;
};

/*
struct _SubscribedNotification {
	guint16					operation_id_;
	IpcmdOperationCallback	*cb_;
};
*/

void
IpcmdOperationCallbackFree(IpcmdClientOperationCallback *cb)
{
	if (cb->cb_destroy)	cb->cb_destroy (cb->cb_data);
	else g_free(cb->cb_data);

	g_free(cb);
}

static gint
_CompareSubscribedNotification(gconstpointer a, gconstpointer b)
{
	return 	((struct _SubscribedNotification *)a)->service_id_ == ((struct _SubscribedNotification *)b)->service_id_ &&
			((struct _SubscribedNotification *)a)->operation_id_ == ((struct _SubscribedNotification *)b)->operation_id_ ? 0 : 1;
}

gint
IpcmdClientHandleMessage(IpcmdClient *self, IpcmdChannelId channel_id, IpcmdMessage *mesg)
{
	// if mesg is NOTIFICATION or NOTIFICATION_CYCLIC, look up in subscribed notification list

	// if mesg is not notification,

	return -1;
}

/* IpmdClientInvokeOperation :
 * create IP COMMAND message and send it to bus.
 */
OpHandle
IpcmdClientInvokeOperation(struct _IpcmdClient *self, const IpcmdOperation *operation, const IpcmdOperationResultCallback *cb)
{

	return NULL;
}

/* IpcmdClientSubscribeNotification :
 * subscribe to IpcmdBus to get notification messages, which are for service_id and operation_id.
 *
 * type should be one of NOTIFICATION or NOTIFICATION_CYCLIC
 */
gint
IpcmdClientSubscribeNotification(struct _IpcmdClient *self, guint16 service_id, guint16 operation_id, guint8 type, const IpcmdOperationCallback *cb)
{
	struct _SubscribedNotification *new_sn;

	// if service_id and operation_id is already registered, it fails
	if (g_list_find_custom (self->subscribed_notifications_, _CompareSubscribedNotification)) {
		return -1;
	}

	new_sn = g_malloc(sizeof(struct _SubscribedNotification));
	if (!new_sn) return -1;	// not enough memory

	new_sn->service_id_ = service_id;
	new_sn->operation_id_ = operation_id;
	*(new_sn->cb_) = *cb;

	self->subscribed_notifications_ = g_list_append(self->subscribed_notifications_, new_sn);

	return 0;
}

/* IpcmdClientUnsubscribeNotification :
 * stop to receive notification message, which are for service_id and operation_id.
 */
void
IpcmdClientUnsubscribeNotification(struct _IpcmdClient *self, guint16 service_id, guint16 operation_id)
{
	GList 	*l;
	struct _SubscribedNotification *sn;

	l = g_list_find_custom (self->subscribed_notifications_, _CompareSubscribedNotification);
	self->subscribed_notifications_ = g_list_remove_link (self->subscribed_notifications_, l);
	g_list_free_full (l, (GDestroyNotify)IpcmdOperationCallbackFree);
}

