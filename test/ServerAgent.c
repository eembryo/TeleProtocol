#include <IpcomSimpleAgent.h>
#include <dprint.h>
#include <Ipcom.h>

IpcomAgent* 		agent = NULL;
IpcomAgentSocket	asock = 0;

IpcomMessage *
GenRESPONSEFor(IpcomMessage *mesg)
{
	IpcomMessage	*newMsg;
	gpointer		payload_buf;

	newMsg = IpcomMessageNew(IPCOM_MESSAGE_MIN_SIZE);
	IpcomMessageInitVCCPDUHeader(newMsg,
			IPCOM_SERVICEID_TELEMATICS, IpcomMessageGetVCCPDUOperationID(mesg),
			IpcomMessageGetVCCPDUSenderHandleID(mesg),
			IPCOM_PROTOCOL_VERSION, IPCOM_OPTYPE_RESPONSE, 0, 0);

	payload_buf = g_malloc0(16);

	IpcomMessageSetPayloadBuffer(newMsg, payload_buf, 16);

	return newMsg;
}

IpcomMessage *
GenDummyMessage(IpcomServiceId serviceid, guint opid, IpcomOpType optype, guint seqnum)
{
	IpcomMessage	*newMsg;
	gpointer		payload_buf;

	newMsg = IpcomMessageNew(IPCOM_MESSAGE_MIN_SIZE);
	IpcomMessageInitVCCPDUHeader(newMsg,
			serviceid, opid,
			BUILD_SENDERHANDLEID(serviceid, opid, optype, seqnum),
			IPCOM_PROTOCOL_VERSION, optype, 0, 0);

	payload_buf = g_malloc0(16);

	IpcomMessageSetPayloadBuffer(newMsg, payload_buf, 16);

	return newMsg;
}

static gpointer
BroadcastingThread(gpointer data)
{
	IpcomMessage 	*mesg = NULL;
	GError			*gerror;

	mesg = GenDummyMessage(IPCOM_SERVICEID_IP_ACTIVITY, 0x01, IPCOM_OPTYPE_NOTIFICATION_CYCLIC, 0x01);

	while(1) {
		gerror = NULL;
		agent->Broadcast(agent, asock, mesg, &gerror);
		if (gerror) {
			DWARN("%s\n", gerror->message);
			g_error_free(gerror);
			return NULL;
		}
		g_usleep(1000000);
	}
	return NULL;
}

static IpcomServiceReturn
ServerProccessMessage(IpcomAgent* agent, IpcomOpHandle handle, IpcomMessage *mesg, gpointer userdata)
{
	IpcomMessage 	*newMsg;

	DPRINT("Got message to process (OperationID=0x%.04x, SenderHandleID=0x%.08x, 0xOpType=%.02x\n",
			IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), IpcomMessageGetVCCPDUOpType(mesg));

	if (IpcomMessageGetVCCPDUOpType(mesg) == IPCOM_OPTYPE_REQUEST ||
			IpcomMessageGetVCCPDUOpType(mesg) == IPCOM_OPTYPE_SETREQUEST) {
		/// generate RESPONSE message
		newMsg = GenRESPONSEFor(mesg);
		//Send the RESPONSE message
		agent->OperationRespond(agent, handle, newMsg, NULL, NULL);
		/// destroy the RESPONSE message
		IpcomMessageUnref(newMsg);
	}

	return IPCOM_SERVICE_SUCCESS;
}

static IpcomServiceReturn
ServerProcessNoti(IpcomAgent* agent, IpcomMessage *mesg, gpointer userdata)
{
	DPRINT("Got message to process (OperationID=0x%.04x, SenderHandleID=0x%.08x, OpType=0x%.02x\n",
			IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), IpcomMessageGetVCCPDUOpType(mesg));

	return IPCOM_SERVICE_SUCCESS;
}

int
main()
{
	GMainLoop	*main_loop = g_main_loop_new(NULL, FALSE);
	GError*		gerror = NULL;
	IpcomAgentMsgHandlers	msgHandler;

	agent = IpcomSimpleAgentCreate(NULL);
	asock = 0;

	/// register message handler
	msgHandler.OnReceiveMesg = ServerProccessMessage;
	msgHandler.OnReceiveNoti = ServerProcessNoti;
	msgHandler.userdata = NULL;
	agent->RegisterMessageHandlers(agent, IPCOM_SERVICEID_TELEMATICS, &msgHandler, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		return 0;
	}

	asock = agent->Bind(agent, IPCOM_TRANSPORT_UDPV4, "0.0.0.0", 50000, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		return 0;
	}
	/// start to listen on UDPv4 socket
	agent->ListenAt(agent, asock, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		return 0;
	}

	g_thread_new("Broadcasting Thread", &BroadcastingThread, NULL);
	g_main_loop_run(main_loop);
}
