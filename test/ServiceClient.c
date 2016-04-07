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
#include <dprint.h>
#include <Ipcom.h>

static IpcomTransport *transport;
static IpcomConnection *connection;

static IpcomServiceReturn
_ProcessMessage(IpcomService *service, const IpcomOpContextId *ctxId, IpcomMessage *mesg)
{
	IpcomMessage 	*newMsg;
	gpointer		payload_buf;

	DFUNCTION_START;

	/// You can get service specific data
	/// priv = IpcomServiceGetPrivData(service);
	/// Generate a response message
	newMsg = IpcomMessageNew(IPCOM_MESSAGE_MIN_SIZE);
	IpcomMessageInitVCCPDUHeader(newMsg,
			IPCOM_SERVICEID_TELEMATICS, IpcomMessageGetVCCPDUOperationID(mesg),
			IpcomMessageGetVCCPDUSenderHandleID(mesg),
			IPCOM_PROTOCOL_VERSION, IPCOM_OPTYPE_RESPONSE, 0, 0);
	payload_buf = g_malloc0(16);
	IpcomMessageSetPayloadBuffer(newMsg, payload_buf, 16);

	/// Send the response message
	IpcomProtocolRespondMessageFull(service->pProto, ctxId, newMsg, NULL, NULL);
	IpcomMessageUnref(newMsg);

	return IPCOM_SERVICE_SUCCESS;
}

static IpcomServiceReturn
_ProcessNoti(IpcomService *service, IpcomConnection *conn, IpcomMessage *mesg)
{
	DFUNCTION_START;

	return IPCOM_SERVICE_SUCCESS;
}

static IpcomServiceReturn
_RecvedCallback(const IpcomOpContextId *opContextId, IpcomMessage *mesg, void *userdata)
{
	DFUNCTION_START;

	return 1;
}

static void
_OnOpCtxDestroy(IpcomOpContextId *opContextId, gint code, gpointer data)
{
	DPRINT("data = %d\n", code);
}

static gpointer
SendingThread(gpointer data)
{
	IpcomMessage 	*mesg = NULL;
	gpointer		payload_buf;
	guint				count = 0;
	GError			*gerror;


	while(1) {
		gerror = NULL;
		//DPRINT("count = %d.\n", count);
		if (count%7 == 4) {
			count += 2;
		}
		mesg = IpcomMessageNew(IPCOM_MESSAGE_MIN_SIZE);
		IpcomMessageInitVCCPDUHeader(mesg,
				IPCOM_SERVICEID_TELEMATICS, 0x0104,
				BUILD_SENDERHANDLEID(IPCOM_SERVICEID_TELEMATICS, 0x0104, count%7, count),
				IPCOM_PROTOCOL_VERSION, count%7, 0, 0);
		payload_buf = g_malloc0(16);
		IpcomMessageSetPayloadBuffer(mesg, payload_buf, 16);
		/// Send it to IpcomProtocol
		IpcomProtocolSendMessageFull(IpcomProtocolGetInstance(), connection, mesg, _RecvedCallback, _OnOpCtxDestroy, NULL, &gerror);
		if (gerror) {
			DWARN("%s\n", gerror->message);
			g_error_free(gerror);
		}
		IpcomMessageUnref(mesg);

		g_usleep(1000000);
		count++;
	}
	return NULL;
}

gint main()
{
	GMainContext *context;
	GMainLoop	*main_loop;
	//GThread		*pSendingThread;

	IpcomService *service;

	context = g_main_context_new();
	main_loop = g_main_loop_new(context, FALSE);
	g_main_context_unref(context);

	//initialize IpcomProtocol with custom context
	IpcomProtocolInit(context);
	
	///[Application] register service ID
	service = IpcomServiceNew(IPCOM_SERVICEID_TELEMATICS, 0);
	service->ProcessMessage = _ProcessMessage;
	service->ProcessNotification = _ProcessNoti;
	IpcomServiceRegister(service);

	//[Transport] Create connection with CONNECT mode
	transport = IpcomTransportNew(IPCOM_TRANSPORT_UDPV4, context);
	transport->onNewConn = NULL;	//If you are in LISTEN mode, set this up.
	transport->onNewConn_data = NULL;

	//[Transport] connect to peer
	connection = IpcomTransportConnect(transport, "127.0.0.1", 50001, "127.0.0.1", 50000);

	//start thread for sending messages
	//pSendingThread = g_thread_new("Sending Thread", &SendingThread, NULL);
	g_thread_new("Sending Thread", &SendingThread, NULL);
	g_main_loop_run (main_loop);

	return 0;
}
