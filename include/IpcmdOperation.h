/*
 * IpcmdOperation.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPERATION_H_
#define INCLUDE_IPCMDOPERATION_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

enum IpcmdOperationResultType {
	OPERATION_RESULT_OK,
	OPERATION_RESULT_ERROR,
	OPERATION_RESULT_RESPONSE,
	OPERATION_RESULT_FAIL,
	OPERATION_RESULT_NOTIFICATION,
};

struct _IpcmdPayloadData {
	guint8		type_;	//'data_' encoding type: 0 on encoded message, 1 on Normal message
	guint32		length_;
	gpointer	data_;
};

struct _IpcmdOperation {
	guint16	service_id_;
	guint16 operation_id_;
	guint32 sender_handle_id_;
	guint8	op_type_;
	guint8	flags_;
	IpcmdPayloadData	*payload_data_;
};

struct _IpcmdOperationResult {
//	OpHandle						handle_;
	enum IpcmdOperationResultType	result_type_;
};

struct _IpcmdOperationResultError {
	struct _IpcmdOperationResult	parent_;
	guint8							error_code_;
	guint16							error_info_;
};

struct _IpcmdOperationResultResponse {
	struct _IpcmdOperationResult	parent_;
	struct _IpcmdPayloadData		response_;
};

struct _IpcmdOperationResultFail {
	struct _IpcmdOperationResult	parent_;
	guint							reason_;
};

struct _IpcmdOperationResultNotification {
	struct _IpcmdOperationResult	parent_;
	IpcmdOperation					notification_;
};

struct _IpcmdOperationResultCallback {
	gint		(*cb_func)(OpHandle handle, const IpcmdOperationResult *result, gpointer cb_data);
	/* cb_destroy :
	 * called to release cb_data memory when IpcmdOperationResultCallback is destroyed. If it is NULL, 'cb_data' is not released.
	 */
	gint		(*cb_destroy)(gpointer cb_data);
	gpointer	cb_data;
};
static inline void IpcmdOperationResultCallbackClear(IpcmdOperationResultCallback *cb) {
	if (cb->cb_destroy)	cb->cb_destroy (cb->cb_data);
	cb->cb_func = NULL;
	cb->cb_destroy = NULL;
	cb->cb_data = NULL;
}
static inline void IpcmdOperationResultCallbackFree(IpcmdOperationResultCallback *cb) {
	if (cb->cb_destroy)	cb->cb_destroy (cb->cb_data);
	g_free(cb);
}

G_END_DECLS

#endif /* INCLUDE_IPCMDOPERATION_H_ */
