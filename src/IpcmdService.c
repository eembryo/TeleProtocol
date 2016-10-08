/*
 * IpcmdService.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdMessage.h"
#include "../include/IpcmdService.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdOperation.h"
#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdOperationContext.h"
#include <glib.h>

static gboolean	_IsSubscribed (IpcmdNotifGroup *notif_group, IpcmdHost *subscriber);
//static void		_UnsubscribeFromNotifGroup (IpcmdNotifGroup *group, IpcmdNotifSubscriber *subscriber);
static void		_RemoveNotifGroup (GList **group_list, IpcmdNotifGroup *group);
static void		_FreeNotifSubscriber (IpcmdNotifSubscriber *subscriber);

void
IpcmdServiceInit(IpcmdService *self, IpcmdServer *server, guint16 service_id, const IpcmdOperationCallback *exec)
{
	self->server_ = server;
	self->service_id_ = service_id;
	self->exec_ = *exec;
	self->notif_groups_ = NULL;
	self->notif_cylic_groups_ = NULL;
}
gint
IpcmdServiceCompleteOperation(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *reply)
{
	IpcmdServerCompleteOperation (self->server_, &((IpcmdOpCtx *)handle)->opctx_id_, reply);
	return 0;
}

gboolean
IpcmdServiceAddSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, IpcmdHost *subscriber, gboolean is_static_member)
{
	GList *l;
	GList **groups = is_cyclic ? &self->notif_cylic_groups_ : &self->notif_groups_;
	IpcmdNotifGroup 		*group = NULL;
	IpcmdNotifSubscriber	*notif_subscriber = NULL;

	// find notification group in this service
	for (l=*groups; l!=NULL; l=l->next) {
		if (((IpcmdNotifGroup*)l->data)->service_id_ == self->service_id_ && ((IpcmdNotifGroup*)l->data)->operation_id_ == operation_id) {
			group = l->data;
			break;
		}
	}
	if (!group) {	//if no group exists for this subscription, make new one.
		group = g_malloc0(sizeof(IpcmdNotifGroup));
		group->service_id_ = self->service_id_;
		group->operation_id_ = operation_id;
		group->subscriber_ = NULL;
		*groups = g_list_append(*groups, group);
	}
	if (_IsSubscribed(group, subscriber)) { //already subscribed
		return TRUE;
	}

	notif_subscriber = g_malloc0(sizeof(IpcmdNotifSubscriber));
	notif_subscriber->host_ = IpcmdHostRef(subscriber);
	notif_subscriber->is_static_ = is_static_member;
	notif_subscriber->last_seq_num = 0;
	group->subscriber_ = g_list_append (group->subscriber_, notif_subscriber);

	return TRUE;
}

gboolean
IpcmdServiceInformNotification(IpcmdService *self, guint16 operation_id, const IpcmdOperationInfoReplyMessage *info)
{
	GList				*groups;
	IpcmdNotifGroup 	*group = NULL;
	IpcmdNotifSubscriber	*subscriber;
	GList 	*l;
	gint	ret;

	switch (info->op_type_) {
	case IPCMD_OPTYPE_NOTIFICATION:
		groups = self->notif_groups_;
		break;
	case IPCMD_OPTYPE_NOTIFICATION_CYCLIC:
		groups = self->notif_cylic_groups_;
		break;
	default:
		g_warning("Only notification type allowed.");
		return FALSE;
	}
	// find notification group
	for (l=groups; l!=NULL; l=l->next) {
		if (((IpcmdNotifGroup*)l->data)->service_id_ == self->service_id_ && ((IpcmdNotifGroup*)l->data)->operation_id_ == operation_id) {
			group = l->data;
			break;
		}
	}
	if (!group) { // no group exists
		g_warning("Tried to inform notification with not existing notification group.");
		return FALSE;
	}

	/* 5.7.1.1.2.3.3.8 OperationType - NotificationRequest
	 * If a notification request is successful, the client has started to subscribe on a certain service/operation and
	 * it's expected that notification messages will be received. If connection to the serving node for any reason is
	 * cut off the subscriber needs to request to subscribe again to resume the information flow.
	 */
	l=group->subscriber_;
	while (l!=NULL) {
		subscriber = (IpcmdNotifSubscriber*)l->data;
		ret = IpcmdServerSendNotification (self->server_, self->service_id_, operation_id, &subscriber->last_seq_num, subscriber->host_, (const IpcmdOperationInfo*)info);
		if (ret == -2 && !subscriber->is_static_) { // subscriber is dynamic host and unreachable,
			GList *pl = l;
			l=l->next;
			_FreeNotifSubscriber ((IpcmdNotifSubscriber*)pl->data);
			group->subscriber_ = g_list_delete_link (group->subscriber_, pl);
		}
		else l=l->next;
	}
	// check that notification group is empty
	if (group->subscriber_ == NULL) {	//if empty, remove the notification group
		_RemoveNotifGroup (&groups, group);
	}
	return TRUE;
}

void
IpcmdServiceRemoveSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, IpcmdHost *subscriber)
{
}

static void
_FreeNotifSubscriber (IpcmdNotifSubscriber *subscriber)
{
	if (subscriber->host_) IpcmdHostUnref(subscriber->host_);
	g_free (subscriber);
}

/*
static void
_UnsubscribeFromNotifGroup (IpcmdNotifGroup *group, IpcmdNotifSubscriber *subscriber)
{
	GList *l;

	for (l=group->subscriber_; l!=NULL; l=l->next) {
		if (l->data == subscriber) {
			_FreeNotifSubscriber (subscriber);
			group->subscriber_ = g_list_delete_link (group->subscriber_, l);
			break;
		}
	}
}
*/
static void
_RemoveNotifGroup (GList **group_list, IpcmdNotifGroup *group)
{
	GList *l;
	GList *pl;

	l = group->subscriber_;
	while (l!=NULL) {
		pl = l;
		l=l->next;
		_FreeNotifSubscriber (pl->data);
		group->subscriber_ = g_list_delete_link (group->subscriber_, pl);
	}

	*group_list = g_list_remove (*group_list, group);
}
static gboolean
_IsSubscribed (IpcmdNotifGroup *notif_group, IpcmdHost *subscriber)
{
	GList *l;

	for (l=notif_group->subscriber_; l!=NULL; l=l->next) {
		if (subscriber->equal(subscriber, ((IpcmdNotifSubscriber*)l->data)->host_)) {
			return TRUE;
		}
	}
	return FALSE;
}
