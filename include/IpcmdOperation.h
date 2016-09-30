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
	kOperationInfoOk,		// the message is successfully processed. It may sent from IpcmdCore to application or vice versa.
	kOperationInfoFail,		// the message is not processed properly. It may sent from IpcmdCore to application or vice versa.
	kOperationInfoReceivedMessage,	// the received message. It is sent from IpcmdCore to application.
	kOperationInfoReplyMessage,		// the response message from application. It is sent from application to IpcmdCore.
	kOperationInfoInvokeMessage,	// invocation message from application. It is sent from application to IpcmdCore.
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

static inline void IpcmdOperationInfoOkInit (IpcmdOperationInfoOk *info) { info->parent_.type_ = kOperationInfoOk;}
static inline void IpcmdOperationInfoFailInit (IpcmdOperationInfoFail *info) { info->parent_.type_ = kOperationInfoFail;}
static inline void IpcmdOperationInfoReceivedMessageInit (IpcmdOperationInfoReceivedMessage *info) { info->parent_.type_ = kOperationInfoReceivedMessage;}
static inline void IpcmdOperationInfoReplyMessageInit (IpcmdOperationInfoReplyMessage *info) { info->parent_.type_ = kOperationInfoReplyMessage;}
static inline void IpcmdOperationInfoInvokeMessageInit (IpcmdOperationInfoInvokeMessage *info) { info->parent_.type_ = kOperationInfoInvokeMessage;}

struct _IpcmdOperationCallback {
	void		(*cb_func)(OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data);
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
