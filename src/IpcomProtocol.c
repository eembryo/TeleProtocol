// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#include <glib.h>
#include <string.h>

#include <IpcomProtocol.h>
#include <IpcomService.h>
#include <IpcomConnection.h>
#include <IpcomMessage.h>
#include <IpcomOperationContext.h>
#include <IpcomEnums.h>
#include <dprint.h>
#include <math.h>

static IpcomProtocol *gIpcomProtocolInstance = NULL;

struct _IpcomProtocol {
	GHashTable	*pServiceHash;
	GHashTable	*pOpContextHash;
	GMainContext *pMainContext;
};

static gboolean _WFRTimerExpired(gpointer data);
static gboolean _WFATimerExpired(gpointer data);
static gint _GetWFRTimeout(gint nOfRetries);
static gint _GetWFATimeout(gint nOfRetries);

#define IPCOM_PROTOCOL_ERROR IpcomProtocolErrorQuark()

static GQuark
IpcomProtocolErrorQuark(void)
{
  return g_quark_from_static_string ("ipcom-protocol-error-quark");
}

IpcomProtocol*
IpcomProtocolInit(GMainContext *gcontext)
{
	if (gIpcomProtocolInstance != NULL) {
		DERROR("IpcomProtocol is already initialized. Use IpcomProtocolGetInstance().\n");
		return NULL;
	}

	gIpcomProtocolInstance = g_malloc0(sizeof(IpcomProtocol));
	if (!gIpcomProtocolInstance) {
		DERROR("Cannot allocate memory for IpcomProtocol.\n");
		return NULL;
	}

	gIpcomProtocolInstance->pMainContext = gcontext;
	gIpcomProtocolInstance->pServiceHash = g_hash_table_new(g_direct_hash, g_direct_equal);
	gIpcomProtocolInstance->pOpContextHash = g_hash_table_new_full(IpcomOpContextIdHashFunc, IpcomOpContextIdEqual, NULL, NULL);

	return gIpcomProtocolInstance;
}

IpcomProtocol*
IpcomProtocolGetInstance()
{
	if (gIpcomProtocolInstance == NULL) {
		DPRINT("IpcomProtocol will be initialized with default glib main context.\n");
		DPRINT("If you want to use specific main context. Call IpcomProtocolInit() at the beginning of your program.\n");
		IpcomProtocolInit(g_main_context_default());
	}

	return gIpcomProtocolInstance;
}

GMainContext *
IpcomProtocolGetMainContext()
{
	return IpcomProtocolGetInstance()->pMainContext;
}

gboolean
IpcomProtocolRegisterService(IpcomProtocol *proto, IpcomService *service)
{
	DFUNCTION_START;

	if (g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(service->serviceId))) {
		DWARN("The service(%d) is already registered in IpcomProtocol.\n", service->serviceId);
		return FALSE;
	}

	g_hash_table_insert(proto->pServiceHash, GINT_TO_POINTER(service->serviceId), service);

	return TRUE;
}

static gboolean
_ValidateReceivedMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	/*
	IpcomService *service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(g_ntohs(IpcomMessageGetVCCPDUHeader(mesg)->serviceID)));

	if (!service) {
		//send ERROR message
		return FALSE;
	}
	 */
	return TRUE;
}

static gboolean
_SendAckMessage(IpcomProtocol *proto, IpcomConnection *conn,
		guint16 serviceID, guint16 operationID,
		guint32 senderHandleId, guint8 proto_version)
{
	IpcomMessage *ackMsg = IpcomMessageNew(IPCOM_ACK_MESSAGE_SIZE);

	g_assert(ackMsg);

	IpcomMessageInitVCCPDUHeader(ackMsg, serviceID, operationID, senderHandleId, proto_version, IPCOM_OPTYPE_ACK, 0x00, 0);
	IpcomConnectionTransmitMessage(conn, ackMsg);
	IpcomMessageUnref(ackMsg);

	return TRUE;
}

static gboolean
_SendACKFor(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *target)
{
	return _SendAckMessage(proto, conn,
			g_htons(target->vccpdu_ptr->serviceID), g_htons(target->vccpdu_ptr->operationID),
			g_htonl(target->vccpdu_ptr->senderHandleId), target->vccpdu_ptr->proto_version);
}

static gboolean
_SendERRORMessage(IpcomProtocol *proto, IpcomConnection *conn,
		guint16 serviceID, guint16 operationID,
		guint32 senderHandleId, guint8 proto_version,
		guint8 ecode, guint16 einfo)
{
	struct _ErrorPayload *payload;
	IpcomMessage *errMsg = IpcomMessageNew(IPCOM_ERROR_MESSAGE_SIZE);

	g_assert(errMsg);

	IpcomMessageInitVCCPDUHeader(errMsg, serviceID, operationID, senderHandleId, proto_version, IPCOM_OPTYPE_ERROR, 0x00, 0);
	IpcomMessageSetPayloadLength(errMsg, sizeof(struct _ErrorPayload));
	payload = (struct _ErrorPayload *)IpcomMessageGetPayload(errMsg);
	payload->errorCode = ecode;
	payload->errorInformation = g_htons(einfo);
	IpcomConnectionTransmitMessage(conn, errMsg);
	IpcomMessageUnref(errMsg);

	return TRUE;
}
static gboolean
_SendERRORFor(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *target, guint8 ecode, guint16 einfo)
{
	return _SendERRORMessage(proto, conn,
			g_htons(target->vccpdu_ptr->serviceID), g_htons(target->vccpdu_ptr->operationID),
			g_htonl(target->vccpdu_ptr->senderHandleId), target->vccpdu_ptr->proto_version,
			ecode, einfo);
}

static gint
_IpcomProtocolHandleNOTIFICATION(IpcomProtocol *proto, gpointer data, IpcomMessage *mesg)
{
	DFUNCTION_START;
	IpcomService *service;
	IpcomConnection *conn = (IpcomConnection *)data;

	/// 1. Find corresponding service
	service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUServiceID(mesg)));

	/// 2. send ACK or ERROR message
	if (service)
		_SendACKFor(proto, conn, mesg);
	else {
		DERROR("Got notification message for unknown service ID(%x)", IpcomMessageGetVCCPDUServiceID(mesg));
		return -1;
	}

	/// 3. deliver to Application layer
	service->ProcessNotification(service, conn, mesg);

	return 0;
}

static gint
_IpcomProtocolHandleNOTIFICATION_CYCLIC(IpcomProtocol *proto, gpointer data, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomService *service;
	IpcomConnection *conn = (IpcomConnection *)data;

	/// 1. Find corresponding service
	service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUServiceID(mesg)));

	/// 2. deliver to Application layer
	if (service)
		service->ProcessNotification(service, conn, mesg);
	else
		return -1;

	return 0;
}

static gint
_IpcomProtocolHandleREQUEST(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomService *service;

	/// 1. send ACK message
	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	/// 2. Find corresponding service
	service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUServiceID(mesg)));
	g_assert(service);

	if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_REQUEST) < 0) {
		goto HandleREQUEST_failed;
	}
	service->ProcessMessage(service, IpcomOpContextGetContextId(opContext), mesg);
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_PROCESS_DONE);
	return 0;

	HandleREQUEST_failed:
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_FINALIZE);
	return -1;
}

static gint
_IpcomProtocolHandleRESPONSE(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	gboolean status;
	IpcomServiceReturn ret;
	GSource *timer;
	gint nextStatus;

	DFUNCTION_START;

	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	nextStatus = IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_RESPONSE);
	if (nextStatus < 0) {
		DWARN("failed to trigger OPCONTEXT_TRIGGER_RECV_RESPONSE.\n");
		goto HandleRESPONSE_failed;
	}
	//1. stop WFA/WFR timer if exists
	IpcomOpContextCancelTimer(opContext);
	//2. call callback function
	IpcomOpContextGetRecvCallback(opContext)(IpcomOpContextGetContextId(opContext), mesg, IpcomOpContextGetRecvCallbackData(opContext));
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_PROCESS_DONE);

	return 0;

	HandleRESPONSE_failed:
	return -1;
}

static gint
_IpcomProtocolHandleACK(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{

	DFUNCTION_START;

	switch(IpcomOpContextGetStatus(opContext)) {
	case OPCONTEXT_STATUS_REQUEST_SENT:
		/// 1. trigger OpContext
		if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK) < 0) {
			goto HandleACK_failed;
		}
		///2. stop WFA timer
		IpcomOpContextCancelTimer(opContext);
		///3. start WFR timer
		IpcomOpContextSetTimer(opContext, _GetWFRTimeout(opContext->numberOfRetries), _WFRTimerExpired);
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		/// 1. trigger OpContext
		if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK) < 0) {
			goto HandleACK_failed;
		}
		/// 2. stop WFR timer
		IpcomOpContextCancelTimer(opContext);
		break;
	case OPCONTEXT_STATUS_NOTIFICATION_SENT:
		/// 1. trigger OpContext
		if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK) < 0) {
			goto HandleACK_failed;
		}
		///2. stop WFA timer
		IpcomOpContextCancelTimer(opContext);
		break;
	default:
		DWARN("We got an wrong ACK message at operation status (%d). Silently discard this message.\n", IpcomOpContextGetStatus(opContext));
		goto HandleACK_failed;

		break;
	}
	return 0;

	HandleACK_failed:
	return -1;
}

IpcomOpContextId *
IpcomProtocolSendMessageFull(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg,
		IpcomReceiveMessageCallback OnRecvMessage, gpointer userdata,
		IpcomOpCtxDestroyNotify OnOpCtxDestroy,
		GError **error)
{
	IpcomOpContextId ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};
	IpcomOpContext *ctx = NULL;
	gint			nxtOpCtxStatus;
	gint			ret;

	DFUNCTION_START;

	if (g_hash_table_contains(proto->pOpContextHash, &ctxId)) {
		if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 0, "Failed to send a message: Already registered context ID.");
		goto _SendMessage_failed;
	}

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST:
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), OnRecvMessage, userdata);
		if (!ctx)
			goto _SendMessage_failed;

		nxtOpCtxStatus = IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_REQUEST);
		if (nxtOpCtxStatus == OPCONTEXT_TRIGGER_SEND_REQUEST) {
			if (IpcomConnectionTransmitMessage(conn, mesg) < 0) {
				if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to transmit in transport.");
				goto _SendMessage_failed;
			}
			///start WFA timer
			IpcomOpContextCancelTimer(ctx);
			g_assert(IpcomOpContextSetTimer(ctx, _GetWFATimeout(ctx->numberOfRetries), _WFATimerExpired) >= 0);

			///OpContext is created successfully, do rest things
			IpcomOpContextSetMessage(ctx, mesg);
		}
		else
			goto _SendMessage_failed;
		break;
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
		ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), OnRecvMessage, userdata);
		if (!ctx)
			goto _SendMessage_failed;

		nxtOpCtxStatus = IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_REQUEST);
		if (nxtOpCtxStatus == OPCONTEXT_TRIGGER_SEND_REQUEST) {
			if (IpcomConnectionTransmitMessage(conn, mesg) < 0) {
				if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to transmit in transport.");
				goto _SendMessage_failed;
			}
			///start WFA timer
			IpcomOpContextCancelTimer(ctx);
			g_assert(IpcomOpContextSetTimer(ctx, _GetWFATimeout(ctx->numberOfRetries), _WFATimerExpired));

			///OpContext is created successfully, do rest things
			IpcomOpContextSetMessage(ctx, mesg);
		}
		else
			goto _SendMessage_failed;
		break;
	case IPCOM_OPTYPE_NOTIFICATION:
		ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), NULL, NULL);
		if (!ctx)
			goto _SendMessage_failed;

		nxtOpCtxStatus = IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_NOTIFICATION);
		if (nxtOpCtxStatus == OPCONTEXT_STATUS_NOTIFICATION_SENT) {
			if (IpcomConnectionTransmitMessage(conn, mesg) < 0) {
				if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to transmit in transport.");
				goto _SendMessage_failed;
			}
			///start WFA timer
			IpcomOpContextCancelTimer(ctx);
			g_assert(IpcomOpContextSetTimer(ctx, _GetWFATimeout(ctx->numberOfRetries), _WFATimerExpired));

			///OpContext is created successfully, do rest things
			IpcomOpContextSetMessage(ctx, mesg);
		}
		else
			goto _SendMessage_failed;
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		if (IpcomConnectionTransmitMessage(conn, mesg)) {
			if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to transmit in transport.");
			goto _SendMessage_failed;
		}
		break;
	default:
		DERROR("Cannot send this opType(%d). Only REQUEST/NOTIFICATION type message is acceptable.\n", IpcomMessageGetVCCPDUOpType(mesg));
		g_assert(FALSE);
	}

	if (ctx && OnOpCtxDestroy)
		IpcomOpContextSetOnDestroy(ctx, OnOpCtxDestroy);

	_SendMessage_success:
	ret = IpcomOpContextGetStatus(ctx);
	IpcomOpContextUnref(ctx);
	/// if OpContext is OPCONTEXT_STATUS_FINALIZE, IpcomOpContextUnref() frees ctx, and return NULL.
	return ret == OPCONTEXT_STATUS_FINALIZE ? NULL : IpcomOpContextGetContextId(ctx);

	_SendMessage_failed:
	if (ctx) IpcomOpContextUnref(ctx);
	return NULL;
}

gboolean
IpcomProtocolRegisterOpContext(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomOpContext *opContext)
{
	return g_hash_table_insert(proto->pOpContextHash, (gpointer)opContextId, opContext);
}

gboolean
IpcomProtocolUnregisterOpContext(IpcomProtocol *proto, const IpcomOpContextId *opContextId)
{
	return g_hash_table_remove(proto->pOpContextHash, opContextId);
}

IpcomOpContext *
IpcomProtocolLookupAndGetOpContext(IpcomProtocol *proto, const IpcomOpContextId *opContextId)
{
	IpcomOpContext *ctx;
	ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);
	return ctx ? IpcomOpContextRef(ctx) : NULL;
}

gint
IpcomProtocolRespondMessageFull(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg,
		IpcomOpCtxDestroyNotify OnOpCtxDestroy)
{
	IpcomOpContext *ctx;
	gint			nxtOpCtxStatus;
	gint			ret;

	DFUNCTION_START;

	if (!opContextId) {
		DWARN("opContextId is NULL.\n");
		return -1;
	}

	ctx = IpcomProtocolLookupAndGetOpContext(proto, opContextId);
	if (!ctx) g_error("Cannot find context ID: %p\n", opContextId);

	//ctx status should be IPCOM_STATUS_PROCESS_REQUEST
	if (IpcomOpContextGetStatus(ctx) != OPCONTEXT_STATUS_PROCESS_REQUEST) {
		DWARN("OpContext status should be OPCONTEXT_STATUS_PROCESS_REQUEST(%d) to send a response message: But current status is %d\n", OPCONTEXT_STATUS_PROCESS_REQUEST, IpcomOpContextGetStatus(ctx));
		return -1;
	}
	//mesg should be RESPONSE message
	if (IpcomMessageGetVCCPDUOpType(mesg) != IPCOM_OPTYPE_RESPONSE)
		g_error("This is not RESPONSE message:%d\n", IpcomMessageGetVCCPDUOpType(mesg));

	if (OnOpCtxDestroy) IpcomOpContextSetOnDestroy(ctx, OnOpCtxDestroy);
	IpcomOpContextSetMessage(ctx, mesg);

	//1. migrate status to OPCONTEXT_STATUS_RESPONSE_SENT
	nxtOpCtxStatus = IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_RESPONSE);
	if (nxtOpCtxStatus < 0)
		goto _RespondMessageFull_failed;
	//2. send RESPONSE message
	if (IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), mesg) < 0) {
		IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE_TRANSMIT_FAILED);
		goto _RespondMessageFull_failed;
	}
	if (nxtOpCtxStatus == OPCONTEXT_STATUS_RESPONSE_SENT) {
		//3. start WFA timer
		IpcomOpContextCancelTimer(ctx);
		if (!IpcomOpContextSetTimer(ctx, _GetWFATimeout(ctx->numberOfRetries), _WFATimerExpired)) {
			IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE_UNKNOWN_FAILED);
			goto _RespondMessageFull_failed;
		}
	}
	else if (nxtOpCtxStatus == OPCONTEXT_STATUS_FINALIZE) {
		IpcomOpContextCancelTimer(ctx);
	}
	else
		g_error("Tried to respond with wrong status.");

	ret = IpcomMessageGetPacketSize(mesg);
	IpcomOpContextUnref(ctx);
	return ret;

	_RespondMessageFull_failed:
	/// send ERROR
	if (ctx) IpcomOpContextUnref(ctx);
	return 0;
}

gint
IpcomProtocolRepondMessage(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg)
{
	return IpcomProtocolRespondMessageFull(proto, opContextId, mesg, NULL);
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomOpContext *ctx = NULL;
	IpcomOpContextId ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};

	DFUNCTION_START;

	if(!_ValidateReceivedMessage(proto, conn, mesg)) {
		goto _HandleMessage_failed;
	}

	ctx = IpcomProtocolLookupAndGetOpContext(proto, &ctxId);

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_SETREQUEST:
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		if (!ctx) {
			ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), NULL, NULL);
			if (!ctx) goto _HandleMessage_failed;
		}
		_IpcomProtocolHandleREQUEST(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_RESPONSE:
		DPRINT("IpcomProtocol received RESPONSE message.\n");
		if (!ctx) {
			DWARN("We got wrong RESPONSE message. Sending Error.\n");
			_SendERRORFor(proto, conn, mesg, 0,0);
			goto _HandleMessage_failed;
		}
		_IpcomProtocolHandleRESPONSE(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_NOTIFICATION:
	  	  DPRINT("IpcomProtocol received NOTIFICATION message.\n");
		_IpcomProtocolHandleNOTIFICATION(proto, (gpointer)conn, mesg);
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		 DPRINT("IpcomProtocol received NOTIFICATION_CYCLIC message.\n");
		_IpcomProtocolHandleNOTIFICATION_CYCLIC(proto, (gpointer)conn, mesg);
		break;
	case IPCOM_OPTYPE_ACK:
		DPRINT("IpcomProtocol received ACK message.\n");
		if (!ctx) {
			DWARN("We got wrong ACK message. Sending error.\n");
			_SendERRORFor(proto, conn, mesg, 0,0);
			goto _HandleMessage_failed;
		}
		_IpcomProtocolHandleACK(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_ERROR:
		break;
	default:
		break;
	}

	IpcomMessageUnref(mesg);

	if (ctx)
		IpcomOpContextUnref(ctx);
	return 0;

_HandleMessage_failed:
	IpcomMessageUnref(mesg);
	return -1;
}

static gint
_GetWFATimeout(gint nOfRetries)
{
	gint usedTimeoutWFA = 0;

	if (nOfRetries > numberOfRetriesWFA) {
		DWARN("The number of retries(%d) should be lower than maximum value(%d)\n.", nOfRetries, numberOfRetriesWFA);
		return -1;
	}

	usedTimeoutWFA = defaultTimeoutWFA * powf(increaseTimerValueWFA,nOfRetries);

	return usedTimeoutWFA;
}

static gint
_GetWFRTimeout(gint nOfRetries)
{
	gint usedTimeoutWFR = 0;

	if (nOfRetries > numberOfRetriesWFR) {
		DWARN("The number of retries(%d) should be lower than maximum value(%d)\n.", nOfRetries, numberOfRetriesWFA);
		return -1;
	}

	usedTimeoutWFR = defaultTimeoutWFR * powf(increaseTimerValueWFR,nOfRetries);

	return usedTimeoutWFR;
}

static gboolean
_WFATimerExpired(gpointer data)
{
	IpcomOpContext *ctx = (IpcomOpContext *)data;
	gint usedTimeoutWFA;

	DFUNCTION_START;

	IpcomOpContextRef(ctx);
	///check whether retransmission is needed or not
	ctx->numberOfRetries++;
	if (ctx->numberOfRetries > numberOfRetriesWFA) {
		/// We give up to transmit this message.
		DWARN("Give up to transmit. Destroying this operation (%p).\n", IpcomOpContextGetContextId(ctx));
		//IpcomOpContextUnsetTimer(ctx);
		IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE_WFA_EXPIRED);
		IpcomOpContextUnref(ctx);
		return G_SOURCE_REMOVE;
	}
	///retransmit IpcomMessage
	g_assert(ctx->message);
	IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), ctx->message);
	///Reset timer
	usedTimeoutWFA = _GetWFATimeout(ctx->numberOfRetries);	g_assert(usedTimeoutWFA > 0);
	IpcomOpContextSetTimer(ctx, usedTimeoutWFA, _WFATimerExpired);

	IpcomOpContextUnref(ctx);
	return G_SOURCE_REMOVE;
}

static gboolean
_WFRTimerExpired(gpointer data)
{
	IpcomOpContext *ctx = (IpcomOpContext *)data;
	gint usedTimeoutWFR;

	DFUNCTION_START;

	IpcomOpContextRef(ctx);
	///check whether retransmission is needed or not
	ctx->numberOfRetries++;
	if (ctx->numberOfRetries > numberOfRetriesWFR) {
		/// We give up to transmit this message.
		DWARN("Give up to transmit. Destroying this operation.\n");
		IpcomOpContextUnsetTimer(ctx);
		IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE_WFR_EXPIRED);
		IpcomOpContextUnref(ctx);
		return G_SOURCE_REMOVE;
	}
	///retransmit IpcomMessage
	g_assert(ctx->message);
	IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), ctx->message);
	///Reset timer
	usedTimeoutWFR = _GetWFRTimeout(ctx->numberOfRetries);	g_assert(usedTimeoutWFR > 0);
	IpcomOpContextSetTimer(ctx, usedTimeoutWFR, _WFRTimerExpired);

	IpcomOpContextUnref(ctx);
	return G_SOURCE_REMOVE;
}

/**
 * deprecated functions
 */
gint
IpcomProtocolSendMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg,
		IpcomReceiveMessageCallback recv_cb, gpointer userdata)
{
	IpcomOpContextId *ctxid;

	ctxid = IpcomProtocolSendMessageFull(proto, conn, mesg, recv_cb, userdata, NULL, NULL);
	return ctxid ? 0 : -1;
}
