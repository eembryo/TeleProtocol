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

enum IpcmdOperationInfoType {
	kOperationInfoOk,		// the message is successfully processed. Application receives this type of operation info.
	kOperationInfoFail,		// the message is not processed properly. Application receives this type of operation info.
	kOperationInfoReceivedMessage,
	kOperationInfoReplyMessage,
	kOperationInfoInvokeMessage,
};

struct _IpcmdOperationPayload {
	guint8		type_;	//'data_' encoding type: 0 on encoded message, 1 on Normal message
	guint32		length_;
	gpointer	data_;
};

struct _IpcmdOperationHeader {
	guint16	service_id_;
	guint16 operation_id_;
	guint32 sender_handle_id_;
	guint8	op_type_;
	guint8	flags_;
};

struct _IpcmdOperationInfo {
	enum IpcmdOperationInfoType		type_;
};

struct _IpcmdOperationInfoInvokeMessage {
	struct _IpcmdOperationInfo parent_;
	struct _IpcmdOperationHeader	header_;
	struct _IpcmdOperationPayload	payload_;
};

struct _IpcmdOperationInfoReplyMessage {
	struct _IpcmdOperationInfo parent_;
	guint8							op_type_;
	struct _IpcmdOperationPayload	payload_;
};

struct _IpcmdOperationInfoReceivedMessage {
	struct _IpcmdOperationInfo		parent_;
	//struct _IpcmdOperationHeader	header_;
	//struct _IpcmdOperationPayload	payload_;
	IpcmdMessage					*raw_message_;
	IpcmdHost	*sender_;
};

struct _IpcmdOperationInfoFail {
	struct _IpcmdOperationInfo		parent_;
	guint							reason_;
};

struct _IpcmdOperationInfoOk {
	struct _IpcmdOperationInfo		parent_;
};


struct _IpcmdOperationCallback {
	gint		(*cb_func)(OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data);
	/* cb_destroy :
	 * called to release cb_data memory when IpcmdOperationResultCallback is destroyed. If it is NULL, 'cb_data' is not released.
	 */
	gint		(*cb_destroy)(gpointer cb_data);
	gpointer	cb_data;
};

static inline void IpcmdOperationCallbackClear(IpcmdOperationCallback *cb) {
	if (cb->cb_destroy)	cb->cb_destroy (cb->cb_data);
	cb->cb_func = NULL;
	cb->cb_destroy = NULL;
	cb->cb_data = NULL;
}
static inline void IpcmdOperationCallbackFree(IpcmdOperationCallback *cb) {
	if (cb->cb_destroy)	cb->cb_destroy (cb->cb_data);
	g_free(cb);
}

G_END_DECLS

#endif /* INCLUDE_IPCMDOPERATION_H_ */
