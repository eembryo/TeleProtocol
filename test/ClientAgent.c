#include <IpcomSimpleAgent.h>
#include <dprint.h>
#include <Ipcom.h>


IpcomAgent* 		agent = NULL;
IpcomAgentSocket	asock = 0;

static IpcomServiceReturn
ClientOnResponse(IpcomAgent* agent,IpcomOpHandle handle,IpcomMessage *mesg,gpointer userdata)
{
	DFUNCTION_START;

	return 1;
}

static void
ClientOnOpCtxDestroy(IpcomAgent* agent,IpcomOpHandle handle,IpcomOpContextFinCode code,gpointer userdata)
{
	DFUNCTION_START;
	DPRINT("FIN code = %d\n", code);
}

static gboolean
SendingTask(gpointer data)
{
	static guint				count = 0;
	IpcomMessage 	*mesg = NULL;
	gpointer		payload_buf;
	GError			*gerror;
	IpcomAgentOpCallbacks cbs;

	cbs.OnOpDestroyed = ClientOnOpCtxDestroy;
	cbs.OnOpResponse = ClientOnResponse;
	cbs.userdata = NULL;

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
	agent->OperationSend(agent, asock, mesg, &cbs, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		return G_SOURCE_REMOVE ;
	}
	IpcomMessageUnref(mesg);

	count++;

	return G_SOURCE_CONTINUE;
}

#if 0
static gpointer
SendingThread(gpointer data)
{
	IpcomMessage 	*mesg = NULL;
	gpointer		payload_buf;
	guint				count = 0;
	GError			*gerror;
	IpcomAgentOpCallbacks cbs;

	cbs.OnOpDestroyed = ClientOnOpCtxDestroy;
	cbs.OnOpResponse = ClientOnResponse;
	cbs.userdata = NULL;

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
		agent->OperationSend(agent, asock, mesg, &cbs, &gerror);
		if (gerror) {
			DWARN("%s\n", gerror->message);
			g_error_free(gerror);
		}
		IpcomMessageUnref(mesg);

		g_usleep(1000);
		count++;
	}
	return NULL;
}
#endif

IpcomMessage *GetRESPONSEFor(IpcomMessage *mesg)
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

static IpcomServiceReturn
ServerProccessMessage(IpcomAgent* agent, IpcomOpHandle handle, IpcomMessage *mesg, gpointer userdata)
{
	IpcomMessage 	*newMsg;

	DFUNCTION_START;
	DPRINT("Got message to process (OperationID=%.04x, SenderHandleID=%.08x, OpType=%.02x",
			IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), IpcomMessageGetVCCPDUOpType(mesg));

	if (IpcomMessageGetVCCPDUOpType(mesg) == IPCOM_OPTYPE_REQUEST ||
			IpcomMessageGetVCCPDUOpType(mesg) == IPCOM_OPTYPE_SETREQUEST) {
		/// generate RESPONSE message
		newMsg = GetRESPONSEFor(mesg);
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
	DFUNCTION_START;
	DPRINT("Got message to process (OperationID=%.04x, SenderHandleID=%.08x, OpType=%.02x",
			IpcomMessageGetVCCPDUOperationID(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), IpcomMessageGetVCCPDUOpType(mesg));

	return IPCOM_SERVICE_SUCCESS;
}


int
main()
{
	GMainLoop*	main_loop = g_main_loop_new(NULL, FALSE);
	GError*		gerror = NULL;
	IpcomAgentMsgHandlers	msgHandler;

	agent = IpcomSimpleAgentCreate(NULL);
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

	/// bind
	//asock = agent->Bind(agent, IPCOM_TRANSPORT_UDPV4, "192.168.0.4", 40000, &gerror);
	asock = agent->Bind(agent, IPCOM_TRANSPORT_UDPV4, "0.0.0.0", 40000, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		return 0;
	}
	/// connect to ...
	agent->ConnectTo(agent, asock, "192.168.0.4", 50000, &gerror);
	if (gerror) {
		DERROR("%s\n", gerror->message);
		g_error_free(gerror);
		return 0;
	}

	//start thread for sending messages
	//pSendingThread = g_thread_new("Sending Thread", &SendingThread, NULL);
	g_timeout_add(1000,SendingTask, NULL);
	//g_thread_new("Sending Thread", &SendingThread, NULL);
	g_main_loop_run (main_loop);
}
