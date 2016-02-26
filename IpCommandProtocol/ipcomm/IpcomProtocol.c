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

static IpcomProtocol *gIpcomProtocolInstance = NULL;

struct _IpcomProtocol {
	GHashTable	*pServiceHash;
	GHashTable	*pOpContextHash;
	GMainContext *pMainContext;
};

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
	gIpcomProtocolInstance->pOpContextHash = g_hash_table_new_full(IpcomOpContextIdHashFunc, IpcomOpContextIdEqual, NULL, (GDestroyNotify)IpcomOpContextDestroy);

	return gIpcomProtocolInstance;
}

IpcomProtocol*
IpcomProtocolGetInstance()
{
	if (gIpcomProtocolInstance == NULL) {
		DPRINT("IpcomProtocol will be initialized with default glib main context.\n");
		DPRINT("If you want to use specific main context. Call IpcomProtocolInit() before calling this function.\n");
		IpcomProtocolInit(g_main_context_default());
	}

	return gIpcomProtocolInstance;
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

	if (!IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_REQUEST)) {
		goto HandleREQUEST_failed;
	}
	service->ProcessMessage(service, IpcomOpContextGetContextId(opContext), mesg);
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_PROCESS_DONE);

	return 0;

	HandleREQUEST_failed:
	return -1;
}

/*
#define IPCOM_DEFAULT_WFA  1000	//miliseconds
#define IPCOM_DEFAULT_WFR  1000

static gboolean
_WFATimerExpired(gpointer data)
{
	IpcomOpContext *ctx = (IpcomOpContext *)data;


	return G_SOURCE_CONTINUE;
}
*/
static gint
_IpcomProtocolHandleRESPONSE(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	gboolean status;
	IpcomServiceReturn ret;
	GSource *timer;

	DFUNCTION_START;

	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	if (!IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_RESPONSE)) {
		DERROR("failed to trigger OPCONTEXT_TRIGGER_RECV_RESPONSE.\n");
		goto HandleRESPONSE_failed;
	}
	//1. stop WFA/WFR timer if exists
	//IpcomOpContextClearTimer(opContext);
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
		if (!IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK)) {
			goto HandleACK_failed;
		}
		///2. stop WFA timer
		//IpcomOpContextClearTimer(opContext);
		///3. start WFR timer

		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		/// 1. trigger OpContext
		if (!IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK)) {
			goto HandleACK_failed;
		}
		/// 2. stop WFR timer
		//IpcomOpContextClearTimer(opContext);
		break;
	case OPCONTEXT_STATUS_NOTIFICATION_SENT:
		/// 1. trigger OpContext
		if (!IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK)) {
			goto HandleACK_failed;
		}
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

gint
IpcomProtocolSendMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, gpointer userdata)
{
	IpcomOpContextId ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};
	IpcomOpContext *ctx = NULL;

	DFUNCTION_START;

	if (g_hash_table_contains(proto->pOpContextHash, &ctxId)) {
		DERROR("Failed to send a message: Already registered context ID.\n");
		return -1;
	}

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST:
		//case IPPROTO_OPTYPE_NOTIFICATION_REQUEST:
		ctx = IpcomOpContextCreate(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), recv_cb, userdata);
		if (!IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_REQUEST)) {
			goto SendMessage_failed;
		}
		IpcomOpContextSetMessage(ctx, mesg);
		g_hash_table_insert(proto->pOpContextHash, (gpointer)IpcomOpContextGetContextId(ctx), ctx);
		///<---- start WFA timer
		IpcomConnectionTransmitMessage(conn, mesg);
		break;
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_NOTIFICATION:
		ctx = IpcomOpContextCreate(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), NULL, NULL);
		if (!IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_NOTIFICATION)) {
			goto SendMessage_failed;
		}
		IpcomOpContextSetMessage(ctx, mesg);
		g_hash_table_insert(proto->pOpContextHash, (gpointer)IpcomOpContextGetContextId(ctx), ctx);
		///<---- start WFA timer
		IpcomConnectionTransmitMessage(conn, mesg);
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		IpcomConnectionTransmitMessage(conn, mesg);
		break;
	default:
		DWARN("Cannot send this opType(%d). Only REQUEST type message is acceptable.\n", IpcomMessageGetVCCPDUOpType(mesg));
		goto SendMessage_failed;
	}

	if (IpcomOpContextGetStatus(ctx) == OPCONTEXT_STATUS_FINALIZE) g_hash_table_remove(proto->pOpContextHash, IpcomOpContextGetContextId(ctx));
	return IpcomMessageGetPacketSize(mesg);

	SendMessage_failed:
	if (ctx) g_hash_table_remove(proto->pOpContextHash, (gconstpointer)IpcomOpContextGetContextId(ctx));
	return -1;
}

gint
IpcomProtocolRepondMessage(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg)
{
	IpcomOpContext *ctx;

	if (!opContextId) {
		DWARN("opContextId is NULL.\n");
		return -1;
	}

	ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);

	if (!ctx) {
		DWARN("Cannot find context ID: %p\n", opContextId);
		return -1;
	}

	//ctx status should be IPCOM_STATUS_PROCESS_REQUEST
	if (IpcomOpContextGetStatus(ctx) != OPCONTEXT_STATUS_PROCESS_REQUEST) {
		DWARN("OpContext status should be OPCONTEXT_STATUS_PROCESS_REQUEST(%d) to send a response message: But current status is %d\n", OPCONTEXT_STATUS_PROCESS_REQUEST, IpcomOpContextGetStatus(ctx));
		return -1;
	}

	//mesg should be RESPONSE message
	if (IpcomMessageGetVCCPDUOpType(mesg) != IPCOM_OPTYPE_RESPONSE) {
		DWARN("This is not RESPONSE message:%d\n", IpcomMessageGetVCCPDUOpType(mesg));
		return -1;
	}

	IpcomOpContextSetMessage(ctx, mesg);
	//1. migrate status to OPCONTEXT_STATUS_RESPONSE_SENT
	IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_RESPONSE);
	//2. start WFA timer
	//3. send RESPONSE message
	IpcomConnectionTransmitMessage(IpcomOpContextGetConnection(ctx), mesg);

	if (IpcomOpContextGetStatus(ctx) == OPCONTEXT_STATUS_FINALIZE) g_hash_table_remove(proto->pOpContextHash, IpcomOpContextGetContextId(ctx));
	return IpcomMessageGetPacketSize(mesg);
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomOpContext *ctx = NULL;
	IpcomOpContextId ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};

	if(!_ValidateReceivedMessage(proto, conn, mesg)) {
		goto HandleMessage_failed;
	}

	ctx = g_hash_table_lookup(proto->pOpContextHash, &ctxId);

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_SETREQUEST:
		//case IPPROTO_OPTYPE_NOTIFICATION_REQUEST:
		if (!ctx) {
			ctx = IpcomOpContextCreate(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), NULL, NULL);
			g_hash_table_insert(proto->pOpContextHash, (gpointer)IpcomOpContextGetContextId(ctx), ctx);
		}
		if (!_IpcomProtocolHandleREQUEST(proto, ctx, mesg)) {
			g_hash_table_remove(proto->pOpContextHash, IpcomOpContextGetContextId(ctx));
			goto HandleMessage_failed;
		}
		break;
	case IPCOM_OPTYPE_RESPONSE:
		DPRINT("IpcomProtocol received RESPONSE message.\n");
		if (!ctx) {
			DWARN("We got wrong RESPONSE message. Sending Error.\n");
			_SendERRORFor(proto, conn, mesg, 0,0);
			goto HandleMessage_failed;
		}
		_IpcomProtocolHandleRESPONSE(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_NOTIFICATION:
		_IpcomProtocolHandleNOTIFICATION(proto, (gpointer)conn, mesg);
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		_IpcomProtocolHandleNOTIFICATION_CYCLIC(proto, (gpointer)conn, mesg);
		break;
	case IPCOM_OPTYPE_ACK:
		DPRINT("IpcomProtocol received ACK message.\n");
		if (!ctx) {
			DWARN("We got wrong ACK message. Sending error.\n");
			_SendERRORFor(proto, conn, mesg, 0,0);
			goto HandleMessage_failed;
		}
		_IpcomProtocolHandleACK(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_ERROR:
		break;
	default:
		break;
	}

	IpcomMessageUnref(mesg);

	if (ctx && IpcomOpContextGetStatus(ctx) == OPCONTEXT_STATUS_FINALIZE) g_hash_table_remove(proto->pOpContextHash, IpcomOpContextGetContextId(ctx));
	return 0;

HandleMessage_failed:
	IpcomMessageUnref(mesg);
	return -1;
}
