/*
 * IpcmdOperation.c
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOperation.h"
#include <glib.h>

struct _IpcmdPayloadData {
	guint8		type_;	//ENCODING TYPE
	guint32		length_;
	gpointer	data_;
};

struct _IpcmdOperation {
	guint16	service_id_;
	guint16 operation_id_;
	guint32 sender_handle_id_;
	guint8	operation_type_;
	guint8	flags_;
	struct _IpcmdPayloadData	*payload_data_;
};

enum IpcmdOperationResultType {
	OPERATION_RESULT_OK,
	OPERATION_RESULT_ERROR,
	OPERATION_RESULT_RESPONSE,
	OPERATION_RESULT_FAIL,
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
