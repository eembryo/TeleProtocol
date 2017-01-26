/*
 * ClientAgentTest.c
 *
 *  Created on: Oct 7, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdAgent.h"
#include "../include/IpcmdMessage.h"
#include <glib.h>

#define TRANSPORT_LOCAL_ADDRESS "127.0.0.1"
#define TRANSPORT_LOCAL_PORT 40000
#define TRANSPORT_REMOTE_ADDRESS "127.0.0.1"
#define TRANSPORT_REMOTE_PORT 50000

#define SERVER_SERVICE_ID	0x00A7
//#define SERVER_NOTIFICATION_OPERATION_ID 0x0903

//#define SUBSCRIBER_UDP_ADDRESS "192.168.0.13"
//#define SUBSCRIBER_UDP_PORT 50000

#define CLIENT_REMOTE_SERVICE_UDP_ADDRESS	"127.0.0.1"
#define CLIENT_REMOTE_SERVICE_UDP_PORT		50000
#define CLIENT_REMOTE_SERVICE_ID 0x00A1
#define CLIENT_REMOTE_OPERATION_ID 0x0104	//TelematicSettings
#define CLIENT_REMOTE_OPERATION_TYPE 0x02	//SetRequest

IpcmdAgent *agent;

static void
client_service_operation_callback (OpHandle handle, const IpcmdOperationInfo *info, gpointer cb_data)
{
	//IpcmdAgent *agent = (IpcmdAgent*)cb_data;

	//info is one of following types
	switch (info->type_) {
	case kOperationInfoOk:
		g_message("%s: Operation OK",__func__);
		break;
	case kOperationInfoFail:
	{
		IpcmdOperationInfoFail *fail_info = (IpcmdOperationInfoFail*)info;
		g_message("%s: Operation Fail: reason=%d", __func__, fail_info->reason_);
	}
		break;
	case kOperationInfoReceivedMessage:
	{
		IpcmdOperationInfoReceivedMessage *recv_info = (IpcmdOperationInfoReceivedMessage*)info;
		g_message("%s: Operation received a packet. op_type=%d",__func__, IpcmdMessageGetVCCPDUOpType(recv_info->raw_message_));
	}
		break;
	default:
		g_error("cannot be happen.");
		break;
	}
}

static void
server_service_exec_callback (OpHandle handle, const IpcmdOperationInfoReceivedMessage *operation, gpointer cb_data)
{
	IpcmdAgent *agent = (IpcmdAgent*)cb_data;
	const IpcmdMessage *raw_mesg = operation->raw_message_;

	gchar payload_data[10] = {0xff, 0xff, 0xff,0xff, 0xff, 0xff,0xff, 0xff, 0xff,0xff};

	switch (IpcmdMessageGetVCCPDUOpType(raw_mesg)) {
	case IPCMD_OPTYPE_REQUEST:
	{
		IpcmdOperationInfoReplyMessage reply_info;
		IpcmdOperationInfoReplyMessageInit(&reply_info);
		reply_info.op_type_ = IPCMD_OPTYPE_RESPONSE;
		reply_info.payload_.type_ = IPCMD_PAYLOAD_NOTENCODED;
		reply_info.payload_.length_ = 10;
		reply_info.payload_.data_ = payload_data;

		IpcmdAgentServerCompleteOperation (agent, handle, (IpcmdOperationInfo*)&reply_info);
	}
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
	{
		IpcmdOperationInfoOk ok_info;
		IpcmdOperationInfoOkInit(&ok_info);

		IpcmdAgentServerCompleteOperation (agent, handle, (IpcmdOperationInfo*)&ok_info);
	}
		break;
	case IPCMD_OPTYPE_SETREQUEST:
	{
		IpcmdOperationInfoReplyMessage reply_info;
		IpcmdOperationInfoReplyMessageInit(&reply_info);
		reply_info.op_type_ = IPCMD_OPTYPE_RESPONSE;
		reply_info.payload_.type_ = IPCMD_PAYLOAD_NOTENCODED;
		reply_info.payload_.length_ = 10;
		reply_info.payload_.data_ = payload_data;

		IpcmdAgentServerCompleteOperation (agent, handle, (IpcmdOperationInfo*)&reply_info);
	}
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
	{
		IpcmdOperationInfoOk ok_info;
		IpcmdOperationInfoOkInit(&ok_info);

		IpcmdAgentServerCompleteOperation (agent, handle, (IpcmdOperationInfo*)&ok_info);
	}
		break;
	default:
		g_error("will not happened");
		break;
	}
}
#if 0
static gboolean
SendNotification(gpointer data)
{
	IpcmdAgent *agent = (IpcmdAgent*)data;
	gchar payload_data[10] = {0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0};
	IpcmdOperationInfoReplyMessage reply_info;
	IpcmdOperationInfoReplyMessageInit(&reply_info);

	reply_info.op_type_ = IPCMD_OPTYPE_NOTIFICATION;
	reply_info.payload_.data_ = payload_data;
	reply_info.payload_.length_ = 10;
	reply_info.payload_.type_ = IPCMD_PAYLOAD_NOTENCODED;

	// The notification message will be generated and sent to each subscribers by IP command protocol.
	IpcmdAgentServerInformNoti (agent, SERVER_SERVICE_ID, SERVER_NOTIFICATION_OPERATION_ID, &reply_info);

	return G_SOURCE_CONTINUE;
}
#endif
static gboolean
RequestOperation(gpointer data)
{
	gint client_id = GPOINTER_TO_INT(data);

	IpcmdOperationCallback	op_cb = {
			.cb_func = client_service_operation_callback,
			.cb_destroy = NULL,
			.cb_data = agent,
	};
	gchar payload_data[10] = {0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0};
	IpcmdOperationPayload payload = {
			.data_ = payload_data,
			.length_ = 10,
			.type_ = IPCMD_PAYLOAD_NOTENCODED,
	};
	IpcmdAgentClientInvokeOperation (agent, client_id, CLIENT_REMOTE_SERVICE_ID, CLIENT_REMOTE_OPERATION_ID, CLIENT_REMOTE_OPERATION_TYPE, 0x0, &payload, &op_cb);

	return G_SOURCE_CONTINUE;
}

gint main()
{
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);
	gint	client_id;

	agent = IpcmdAgentNew (g_main_context_default());

	// Setup Transport Layer
	IpcmdAgentTransportAddUdpv4Client (agent, TRANSPORT_LOCAL_ADDRESS, TRANSPORT_LOCAL_PORT, TRANSPORT_REMOTE_ADDRESS, TRANSPORT_REMOTE_PORT);

	// Create new service to process operations, requested by remote clients
	IpcmdAgentServerOpenService (agent, SERVER_SERVICE_ID, server_service_exec_callback, agent, NULL);
	// You may create multiple services to serve.
	// IpcmdAgentServerOpenService (agent, SERVER_SERVICE_ID2, server_service_exec_callback2, agent, NULL);
	// IpcmdAgentServerOpenService (agent, SERVER_SERVICE_ID3, server_service_exec_callback3, agent, NULL);
	// ...

#if 0
	// Add a specific client to notification group.
	// You may invoke multiple times to add clients to the notification group.
	IpcmdAgentServerAddNotiSubscriberUdpv4 (agent,
			SERVER_SERVICE_ID,	//service id
			SERVER_NOTIFICATION_OPERATION_ID, //operation id
			FALSE,	//NOTIFICATION_CYCLIC ?
			SUBSCRIBER_UDP_ADDRESS,			//address of subscriber
			SUBSCRIBER_UDP_PORT,			//port of subscriber
			TRUE	//is the subscriber static ?
			);
	g_timeout_add(10*1000, SendNotification, agent); //send notification to clients every 10 seconds
#endif
	// Create new client to request operations
	client_id = IpcmdAgentClientOpenUdpv4 (agent, CLIENT_REMOTE_SERVICE_UDP_ADDRESS, CLIENT_REMOTE_SERVICE_UDP_PORT);
	{
		IpcmdOperationCallback	op_cb = {
				.cb_func = client_service_operation_callback,
				.cb_destroy = NULL,
				.cb_data = agent,
		};
		IpcmdAgentClientListenNoti (agent, client_id, 0, 0, FALSE, &op_cb);
	}
	// You may create multiple clients to access multiple remote services.
	// NOTE: Each service ID should be different, while udp address and port may be same.
	// IpcmdAgentClientOpenServiceUdpv4 (agent, CLIENT_REMOTE_SERVICE_UDP_ADDRESS2, CLIENT_REMOTE_SERVICE_UDP_PORT2);
	// IpcmdAgentClientOpenServiceUdpv4 (agent, CLIENT_REMOTE_SERVICE_UDP_ADDRESS3, CLIENT_REMOTE_SERVICE_UDP_PORT3);
	// ...
	//g_timeout_add(5*1000, RequestOperation, GINT_TO_POINTER(client_id));	//send REQUEST message to server every 5 seconds

	g_main_loop_run (loop);
}
