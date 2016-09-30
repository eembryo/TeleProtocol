/*
 * IpcmdService.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDSERVICE_H_
#define INCLUDE_IPCMDSERVICE_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

struct _IpcmdService {
	guint16		service_id_;
	IpcmdServer	*server_;

	/* @fn: ExeOperation
	 * virtual function, which should be implemented by each IpcmdService. After ExecOperation() is invoked,
	 * the callee should call IpcmdServiceCompleteOperation() to complete the operation.
	 */
	ExecuteOperation	exec_;
	//void		(*ExecOperation)(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *operation);
};

gint		IpcmdServiceCompleteOperation(IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *result);
gboolean	IpcmdServiceAddSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdHost *subscriber, gboolean is_static_member);
void		IpcmdServiceInformNotification(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdOperationInfo *operation);
void		IpcmdServiceRemoveSubscriber(IpcmdService *self, guint16 operation_id, gboolean is_cyclic, const IpcmdHost *subscriber);

G_END_DECLS

#endif /* INCLUDE_IPCMDSERVICE_H_ */
