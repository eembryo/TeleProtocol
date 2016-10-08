/*
 * IpcmdService.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDSERVICE_H_
#define INCLUDE_IPCMDSERVICE_H_

#include "IpcmdDeclare.h"
#include "IpcmdOperation.h"
#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdNotifSubscriber {
	IpcmdHost	*host_;
	gboolean	is_static_;
	guint8		last_seq_num;	//last used sequence number
} IpcmdNotifSubscriber;

typedef struct _IpcmdNotifGroup {
	guint16	service_id_;
	guint16	operation_id_;
	GList	*subscriber_;
} IpcmdNotifGroup;

struct _IpcmdService {
	guint16		service_id_;
	IpcmdServer	*server_;
	GList		*notif_groups_;
	GList		*notif_cylic_groups_;

	/* @callback: exec_
	 * a callback structure, which involves callback function, callback data and destroying function.
	 * IpcmdService calls the registered callback function, when it receives an IP command message. The callee
	 * should process the received message and notify the end of processing by calling IpcmdServiceCompleteOperation().
	 *
	 * struct _IpcmdOperationCallback {
	 * 		void		(*cb_func)(OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data);
	 * 		gint		(*cb_destroy)(gpointer cb_data);
	 * 		gpointer	cb_data;
	 * }
	 */
	IpcmdOperationCallback exec_;
	/* @fn: ExeOperation
	 * virtual function, which should be implemented by each IpcmdService. After ExecOperation() is invoked,
	 * the callee should call IpcmdServiceCompleteOperation() to complete the operation.
	 * Possible IpcmdOperationInfo : IpcmdOperationInfoReceivedMessage
	 */
	//ExecuteOperation	exec_;
};

void	IpcmdServiceInit(IpcmdService *self, IpcmdServer *server, guint16 service_id, const IpcmdOperationCallback *exec);
/* @fn: IpcmdServiceCompleteOperation
 * The callee which was called by ExecOperation(), should notify that requested operation was finished by
 * calling IpcmdServiceCompleteOperation().
 *
 * The "result" should be one of following IpcmdOperationInfo type:
 * 	IpcmdOperationOk (if it does not have to send no RESPONSE message)
 * 	IpcmdOperationReplyMessage (in case that RESPONSE or ERROR need to be sent)
 */
gint		IpcmdServiceCompleteOperation(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *result);
gboolean	IpcmdServiceAddSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, IpcmdHost *subscriber, gboolean is_static_member);
gboolean	IpcmdServiceInformNotification(IpcmdService *self, guint16 operation_id, const IpcmdOperationInfoReplyMessage *info);
void		IpcmdServiceRemoveSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, IpcmdHost *subscriber);

G_END_DECLS

#endif /* INCLUDE_IPCMDSERVICE_H_ */
