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
	//GHashTable	*pInitiatedOpContextHash;	//Operation Contexts, which are initiated by this application
	//GHashTable	*pAcceptedOpContextHash;	//Operation Contexts, which are came from other application
};

IpcomProtocol*
IpcomProtocolGetInstance()
{
	if (gIpcomProtocolInstance == NULL) {
		gIpcomProtocolInstance = g_malloc0(sizeof(IpcomProtocol));
		if (!gIpcomProtocolInstance) DWARN("Cannot allocate memory for IpcomProtocol.\n");
		gIpcomProtocolInstance->pServiceHash = g_hash_table_new(g_direct_hash, g_direct_equal);
		gIpcomProtocolInstance->pOpContextHash = g_hash_table_new(IpcomOpContextIdHashFunc, IpcomOpContextIdEqual);
		//gIpcomProtocolInstance->pInitiatedOpContextHash = g_hash_table_new(IpcomOpContextIdHashFunc, IpcomOpContextIdEqual);
		//gIpcomProtocolInstance->pAcceptedOpContextHash = g_hash_table_new(IpcomOpContextIdHashFunc, IpcomOpContextIdEqual);
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
	IpcomConnectionPushOutgoingMessage(conn, ackMsg);
	IpcomMessageUnref(ackMsg);

	return TRUE;
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
	IpcomConnectionPushOutgoingMessage(conn, errMsg);
	IpcomMessageUnref(errMsg);

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
_SendERRORFor(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *target, guint8 ecode, guint16 einfo)
{
	return _SendERRORMessage(proto, conn,
			g_htons(target->vccpdu_ptr->serviceID), g_htons(target->vccpdu_ptr->operationID),
			g_htonl(target->vccpdu_ptr->senderHandleId), target->vccpdu_ptr->proto_version,
			ecode, einfo);
}

static gint
_IpcomProtocolHandleREQUEST(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	DFUNCTION_START;

	IpcomService *service;

	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	IpcomOpContextSetMessage(opContext, mesg);

	service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUServiceID(mesg)));
	g_assert(service);
	service->ProcessMessage(service, IpcomOpContextGetContextId(opContext), mesg);

	return 0;
}

static gint
_IpcomProtocolHandleRESPONSE(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	gboolean status;
	IpcomServiceReturn ret;

	DFUNCTION_START;

	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	switch(IpcomOpContextGetStatus(opContext)) {
	case OPCONTEXT_STATUS_ACK_RECV:
	case OPCONTEXT_STATUS_REQUEST_SENT:
		IpcomOpContextSetMessage(opContext, mesg);
		//1. stop WFA/WFR timer if exists
		//2. migrate status to PROCESS_RESPONSE
		IpcomOpContextSetStatus(opContext, OPCONTEXT_STATUS_PROCESS_RESPONSE);
		//3. call callback function
		ret = IpcomOpContextGetRecvCallback(opContext)(IpcomOpContextGetContextId(opContext), mesg, IpcomOpContextGetRecvCallbackData(opContext));
		if (ret != IPCOM_SERVICE_PENDING) IpcomProtocolProcessDone(proto, IpcomOpContextGetContextId(opContext), ret);

		break;
	default:
		DWARN("We got an wrong RESPONSE message at operation status(%d). Silently discard this message.\n", IpcomOpContextGetStatus(opContext));
		break;
	}

	return 0;
}

/**
 * IpcomProtocolProcessDone:
 */
gint
IpcomProtocolProcessDone(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomServiceReturn result)
{
	IpcomOpContext *ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);

	DFUNCTION_START;

	if (!ctx) {
		DWARN("OpContext(%p) does not exist.\n", opContextId);
		return -1;
	}
	//if result is one of errors, send ERROR and return 0
	/*
	if (result == IPCOM_SERVICE_ERR_XXX) {
		sendERROR();
		return 0;
	}
	 */

	switch(IpcomOpContextGetStatus(ctx)) {
	case OPCONTEXT_STATUS_PROCESS_RESPONSE:
	case OPCONTEXT_STATUS_PROCESS_REQUEST:
		g_hash_table_remove(proto->pOpContextHash, opContextId);
		IpcomOpContextDestroy(ctx);
		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		//Should wait for ACK message
		break;
	default:
		DERROR("You should not be here!\n");
		g_assert(FALSE);
	}

	return 0;
}

static gint
_IpcomProtocolHandleACK(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{

	DFUNCTION_START;

	switch(IpcomOpContextGetStatus(opContext)) {
	case OPCONTEXT_STATUS_REQUEST_SENT:
		//1. stop WFA timer
		//2. migrate status to OPCONTEXT_STATUS_ACK_RECV
		IpcomOpContextSetStatus(opContext, OPCONTEXT_STATUS_ACK_RECV);
		//3. start WFR timer

		break;
	case OPCONTEXT_STATUS_RESPONSE_SENT:
		//1. stop WFR timer
		//2. destroy the operation context
		//g_hash_table_remove(proto->pOpContextHash, IpcomOpContextGetContextId(opContext));
		//IpcomOpContextDestroy(opContext);
		break;
	default:
		DWARN("We got an wrong ACK message at operation status (%d). Silently discard this message.\n", IpcomOpContextGetStatus(opContext));
		break;
	}

	return 0;
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
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_SETREQUEST:
		//case IPPROTO_OPTYPE_NOTIFICATION_REQUEST:
		ctx = IpcomOpContextCreate(ctxId.connection, ctxId.senderHandleId, IpcomMessageGetVCCPDUOpType(mesg), recv_cb, userdata);
		IpcomOpContextSetMessage(ctx, mesg);
		IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_REQUEST_SENT);
		g_hash_table_insert(proto->pOpContextHash, (gpointer)IpcomOpContextGetContextId(ctx), ctx);

		//Send it to destination.
		IpcomConnectionPushOutgoingMessage(conn, mesg);
		break;
	default:
		DWARN("Cannot send this opType(%d). Only REQUEST type message is acceptable.\n",IpcomMessageGetVCCPDUOpType(mesg));
		return -1;
	}

	return IpcomMessageGetPacketSize(mesg);
}

gint
IpcomProtocolRepondMessage(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg)
{
	IpcomOpContext *ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);

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
	//1. start WFA timer
	//2. migrate status to OPCONTEXT_STATUS_RESPONSE_SENT
	IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_RESPONSE_SENT);
	//3. send RESPONSE message
	IpcomConnectionPushOutgoingMessage(IpcomOpContextGetConnection(ctx), mesg);

	return IpcomMessageGetPacketSize(mesg);
}

gint
IpcomProtocolSend(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg)
{
	IpcomOpContext *ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
		break;
	case IPCOM_OPTYPE_SETREQUEST:
		break;
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
		break;
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		break;
	case IPCOM_OPTYPE_RESPONSE:
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		break;
	case IPCOM_OPTYPE_ACK:
		break;
	case IPCOM_OPTYPE_ERROR:
		break;
	}

	return 0;

Failed_to_Send:
	return -1;
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomOpContext *ctx;
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
		IpcomOpContextSetStatus(ctx, OPCONTEXT_STATUS_PROCESS_REQUEST);
		_IpcomProtocolHandleREQUEST(proto, ctx, mesg);

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
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		break;
	case IPCOM_OPTYPE_ACK:
		DPRINT("IpcomProtocol received ACK message.\n");
		//IMPLEMENT: check that the "mesg" is really involved in this ctx: compare service ID, Operation ID, Protocol version, DataType and etc.
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

	return 0;

HandleMessage_failed:
	return -1;
}
