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
#include <IpcomOpStateMachine.h>

static IpcomProtocol *gIpcomProtocolInstance = NULL;

struct _IpcomProtocol {
	GHashTable	*pServiceHash;
	GHashTable	*pOpContextHash;
	GMainContext *pMainContext;
};

static gboolean _SendERRORFor(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *target, guint8 ecode, guint16 einfo);
static gboolean _SendACKFor(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *target);

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

	/// initialize operation state machine
	IpcomOpStateMachineInit();
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
	IpcomService *service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(g_ntohs(IpcomMessageGetVCCPDUHeader(mesg)->serviceID)));

	/// Check protocol version

	/// service is available?
	if (!service) {
		//send ERROR message
		_SendERRORFor(proto, conn, mesg, IPCOM_MESSAGE_ECODE_SERVICEID_NOT_AVAILABLE,IpcomMessageGetVCCPDUServiceID(mesg));
		return FALSE;
	}

	/// Length comparison is disabled because encoded message has different length from original message.
#if 0
	/// If real length of the message does not equal to one in VCC PDU Header
	if (IpcomMessageGetVCCPDULength(mesg) + VCCPDUHEADER_LENGTH_CORRECTION != IpcomMessageGetLength(mesg)) {
		_SendERRORFor(proto, conn, mesg, IPCOM_ECODE_INVALID_LENGTH, 0);
		return FALSE;
	}
#endif

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
	g_assert(proto && conn && target);
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

IpcomService*
IpcomProtocolLookupService(IpcomProtocol *proto, guint serviceId)
{
	return g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(serviceId));
}

static gint
_IpcomProtocolHandleREQUEST(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	gint err;

	DFUNCTION_START;

	IpcomService *service;

	/// 1. send ACK message
	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	/// 2. Find corresponding service
	service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUServiceID(mesg)));
	g_assert(service);

	if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_REQUEST, GINT_TO_POINTER(0)) < 0) {
		goto HandleREQUEST_failed;
	}
	IpcomOpContextSetMessage(opContext, mesg);
	err = service->ProcessMessage(service, IpcomOpContextGetContextId(opContext), mesg);
	if (err) {
		//!! NEED IMPLEMENTATION
	}
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_PROCESS_DONE, GINT_TO_POINTER(0));
	return 0;

	HandleREQUEST_failed:
	//IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_FINALIZE, GINT_TO_POINTER(OPCONTEXT_FINCODE_UNKNOWN_FAILURE));
	return -1;
}

static gint
_IpcomProtocolHandleRESPONSE(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	DFUNCTION_START;

	_SendACKFor(proto, IpcomOpContextGetConnection(opContext), mesg);

	if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_RESPONSE, GINT_TO_POINTER(0)) < 0) {
		DWARN("failed to trigger OPCONTEXT_TRIGGER_RECV_RESPONSE.\n");
		goto HandleRESPONSE_failed;
	}
	/// call callback function
	IpcomOpContextGetRecvCallback(opContext)(IpcomOpContextGetContextId(opContext), mesg, IpcomOpContextGetRecvCallbackData(opContext));
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_PROCESS_DONE, GINT_TO_POINTER(0));

	return 0;

	HandleRESPONSE_failed:
	return -1;
}

static gint
_IpcomProtocolHandleACK(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{

	DFUNCTION_START;

	if (IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_RECV_ACK, GINT_TO_POINTER(0)) < 0) {
		goto HandleACK_failed;
	}
	return 0;

	HandleACK_failed:
	return -1;
}

static gint
_IpcomProtocolHandleERROR(IpcomProtocol *proto, IpcomOpContext *opContext, IpcomMessage *mesg)
{
	IpcomReceiveMessageCallback cbfn;

	DFUNCTION_START;

	g_assert(opContext);

	cbfn = IpcomOpContextGetRecvCallback(opContext);
	if (cbfn)
		cbfn(IpcomOpContextGetContextId(opContext), mesg, IpcomOpContextGetRecvCallbackData(opContext));

	// IMPLEMENT: Trigger should be TRIGGER_ERROR
	// IMPLEMENT: change FINCODE according to error message
	IpcomOpContextTrigger(opContext, OPCONTEXT_TRIGGER_FINALIZE, GINT_TO_POINTER(OPCONTEXT_FINCODE_PEER_NOT_OK));

	return 0;
}

IpcomOpContextId *
IpcomProtocolSendMessageFull(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg,
		IpcomReceiveMessageCallback OnRecvMessage, IpcomOpCtxDestroyNotify OnOpCtxDestroy,
		gpointer userdata,
		GError **error)
{
	IpcomOpContextId	ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};
	IpcomOpContext		*ctx = NULL;
	IpcomOpContextId	*ret;
	gint				trigger;

	if (g_hash_table_contains(proto->pOpContextHash, &ctxId)) {
		if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 0, "Failed to send a message: Already registered context ID.");
		goto _SendMessage_failed;
	}

	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST:
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_NOTIFICATION:
		trigger = IpcomMessageGetVCCPDUOpType(mesg) == IPCOM_OPTYPE_NOTIFICATION ? OPCONTEXT_TRIGGER_SEND_NOTIFICATION : OPCONTEXT_TRIGGER_SEND_REQUEST;

		ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId,
				IpcomMessageGetVCCPDUServiceID(mesg), IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUProtoVersion(mesg), IpcomMessageGetVCCPDUOpType(mesg));
		if (!ctx) goto _SendMessage_failed;

		if (IpcomOpContextTrigger(ctx, trigger, mesg) < 0) {
			if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to Transmit.");
			goto _SendMessage_failed;
		}
		break;
	case IPCOM_OPTYPE_NOTIFICATION_CYCLIC:
		if (IpcomConnectionTransmitMessage(conn, mesg) < 0) {
			if (error) g_set_error(error, IPCOM_PROTOCOL_ERROR, 1, "Failed to transmit in transport.");
			goto _SendMessage_failed;
		}
		break;
	default:
		DERROR("Cannot send this opType(%d). Only REQUEST/NOTIFICATION type message is acceptable.\n", IpcomMessageGetVCCPDUOpType(mesg));
		g_assert(FALSE);
	}

	//_SendMessage_success:
	ret = NULL;
	if (ctx && IpcomOpContextGetState(ctx) != OPCONTEXT_STATUS_FINALIZE) {
		IpcomOpContextSetCallbacks(ctx, OnRecvMessage, OnOpCtxDestroy, userdata);
		ret = IpcomOpContextGetContextId(ctx);
	}
	if (ctx) IpcomOpContextUnref(ctx);
	return ret;

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

void
IpcomProtocolCancelOpContext(IpcomProtocol *proto, const IpcomOpContextId *pOpContextId)
{
	IpcomOpContext *ctx = IpcomProtocolLookupAndGetOpContext(proto, pOpContextId);
	if (!ctx) return;
	IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE, GINT_TO_POINTER(OPCONTEXT_FINCODE_CANCELLED));
}

IpcomOpContext *
IpcomProtocolLookupAndGetOpContext(IpcomProtocol *proto, const IpcomOpContextId *opContextId)
{
	IpcomOpContext *ctx;
	ctx = g_hash_table_lookup(proto->pOpContextHash, opContextId);
	return ctx ? IpcomOpContextRef(ctx) : NULL;
}

gint
IpcomProtocolRespondError(IpcomProtocol *proto, const IpcomOpContextId *opContextId, guint8 ecode, guint16 einfo)
{
	IpcomOpContext *ctx;

	g_assert(opContextId);

	ctx = IpcomProtocolLookupAndGetOpContext(proto, opContextId);
	if (!ctx) g_error("Cannot find context ID: %p\n", opContextId);

	/// We donot care whether error message is delivered successfully or not.
	_SendERRORMessage(proto, IpcomOpContextGetConnection(ctx), IpcomOpContextGetServiceID(ctx), IpcomOpContextGetOperationID(ctx), IpcomOpContextGetSenderHandleID(ctx), IpcomOpContextGetProtoVersion(ctx), ecode, einfo);
	IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_FINALIZE, GINT_TO_POINTER(OPCONTEXT_FINCODE_APP_ERROR));

	IpcomOpContextUnref(ctx);
	return 0;
}

gint
IpcomProtocolRespondMessageFull(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg,
		IpcomOpCtxDestroyNotify OnOpCtxDestroy, gpointer userdata)
{
	IpcomOpContext *ctx;
	gint			ret;

	DFUNCTION_START;

	g_assert(opContextId);

	ctx = IpcomProtocolLookupAndGetOpContext(proto, opContextId);
	if (!ctx) g_error("Cannot find context ID: %p\n", opContextId);

	if (IpcomOpContextTrigger(ctx, OPCONTEXT_TRIGGER_SEND_RESPONSE, mesg) < 0) {
		goto _RespondMessageFull_failed;
	}

	if (OnOpCtxDestroy) IpcomOpContextSetCallbacks(ctx, NULL, OnOpCtxDestroy, userdata);

	ret = IpcomMessageGetPacketSize(mesg);
	IpcomOpContextUnref(ctx);
	return ret;

	_RespondMessageFull_failed:
	/// send ERROR
	/// NEED IMPLEMENTATION
	if (ctx) IpcomOpContextUnref(ctx);
	return 0;
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomOpContext *ctx = NULL;
	IpcomOpContextId ctxId = {
			.connection = conn,
			.senderHandleId = IpcomMessageGetVCCPDUSenderHandleID(mesg)
	};

	//DFUNCTION_START;
	DPRINT("Handle Message (SenderHandleID: %x)\n", IpcomMessageGetVCCPDUSenderHandleID(mesg));

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
			ctx = IpcomOpContextNewAndRegister(ctxId.connection, ctxId.senderHandleId,
					IpcomMessageGetVCCPDUServiceID(mesg), IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUProtoVersion(mesg), IpcomMessageGetVCCPDUOpType(mesg));
			if (!ctx) goto _HandleMessage_failed;
		}
		DPRINT("IpcomProtocol received one of REQUEST,SETREQUEST,SETREQUEST_NORETURN or NOTIFICATION_REQUEST message.\n");
		_IpcomProtocolHandleREQUEST(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_RESPONSE:
		if (!ctx) {
			DWARN("We got wrong RESPONSE message. Sending Error.\n");
			_SendERRORFor(proto, conn, mesg, IPCOM_MESSAGE_ECODE_OPERATIONTYPE_NOT_AVAILABLE, IPCOM_OPTYPE_RESPONSE);
			goto _HandleMessage_failed;
		}
		DPRINT("IpcomProtocol received RESPONSE message.\n");
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
			_SendERRORFor(proto, conn, mesg, IPCOM_MESSAGE_ECODE_OPERATIONTYPE_NOT_AVAILABLE, IPCOM_OPTYPE_ACK);
			goto _HandleMessage_failed;
		}
		_IpcomProtocolHandleACK(proto, ctx, mesg);
		break;
	case IPCOM_OPTYPE_ERROR:
		if (ctx) _IpcomProtocolHandleERROR(proto, ctx, mesg);
		break;
	default:
		break;
	}
	IpcomMessageUnref(mesg);
	if (ctx) IpcomOpContextUnref(ctx);
	return 0;

_HandleMessage_failed:
	IpcomMessageUnref(mesg);
	return -1;
}

/**
 * deprecated functions
 */
#if 0
gint
IpcomProtocolSendMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg,
		IpcomReceiveMessageCallback recv_cb, gpointer userdata)
{
	IpcomOpContextId *ctxid;

	ctxid = IpcomProtocolSendMessageFull(proto, conn, mesg, recv_cb, userdata, NULL, NULL);
	return ctxid ? 0 : -1;
}

gint
IpcomProtocolRepondMessage(IpcomProtocol *proto, const IpcomOpContextId *opContextId, IpcomMessage *mesg)
{
	return IpcomProtocolRespondMessageFull(proto, opContextId, mesg, NULL, NULL);
}
#endif
