/*
 * ServerAgentTest.c
 *
 *  Created on: Oct 7, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdAgent.h"
#include "../include/IpcmdMessage.h"
#include <glib.h>

#define TRANSPORT_LOCAL_ADDRESS "192.168.0.13"
#define TRANSPORT_LOCAL_PORT 50000
#define TRANSPORT_ALLOW_BROADCAST TRUE

#define SERVER_SERVICE_ID	0x00A1
#define SERVER_NOTIFICATION_OPERATION_ID 0x0107

#define SUBSCRIBER_UDP_ADDRESS "192.168.0.13"
#define SUBSCRIBER_UDP_PORT 40000

#define CLIENT_REMOTE_SERVICE_ID 0x00A7
#define CLIENT_REMOTE_OPERATION_ID 0x0702
#define CLIENT_REMOTE_OPERATION_TYPE 0x02	//SetRequest
#define CLIENT_REMOTE_SERVICE_UDP_ADDRESS	"192.168.0.13"
#define CLIENT_REMOTE_SERVICE_UDP_PORT		40000
/*
TransportDescUdpv4 udp_desc = {
		.link_type_ = IPCMD_HOSTLINK_UDPv4,
		.is_server_ = TRUE,
		.local_addr_ = TRANSPORT_LOCAL_ADDRESS,
		.local_port_ = TRANSPORT_LOCAL_PORT,
		.allow_broadcast_ = FALSE,
};
*/
static void
client_service_operation_callback (OpHandle handle, const IpcmdOperationInfo *info, gpointer cb_data)
{
	IpcmdAgent *agent = (IpcmdAgent*)cb_data;

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

	IpcmdAgentServerInformNoti (agent, SERVER_SERVICE_ID, SERVER_NOTIFICATION_OPERATION_ID, &reply_info);

	return G_SOURCE_CONTINUE;
}

static gboolean
RequestOperation(gpointer data)
{
	IpcmdAgent *agent = (IpcmdAgent*)data;
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
	IpcmdAgentClientInvokeOperationUdpv4 (agent, CLIENT_REMOTE_SERVICE_ID, CLIENT_REMOTE_OPERATION_ID, CLIENT_REMOTE_OPERATION_TYPE, 0x0, &payload, &op_cb);

	return G_SOURCE_CONTINUE;
}
gint main()
{
	IpcmdAgent *agent;
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);

	agent = IpcmdAgentNew (g_main_context_default());

	IpcmdAgentTransportAddUdpv4Server (agent, TRANSPORT_LOCAL_ADDRESS, TRANSPORT_LOCAL_PORT, FALSE);

	IpcmdAgentServerOpenService (agent, SERVER_SERVICE_ID, server_service_exec_callback, agent, NULL);
	IpcmdAgentServerAddNotiSubscriberUdpv4 (agent,
			SERVER_SERVICE_ID,	//service id
			SERVER_NOTIFICATION_OPERATION_ID, //operation id
			FALSE,	//NOTIFICATION_CYCLIC ?
			SUBSCRIBER_UDP_ADDRESS,			//address of subscriber
			SUBSCRIBER_UDP_PORT,					//port of subscriber
			TRUE	//is the subscriber static ?
			);

	g_timeout_add(10000, SendNotification, agent); //send notification to clients every 10 seconds

	IpcmdAgentClientOpenServiceUdpv4 (agent, CLIENT_REMOTE_SERVICE_ID, CLIENT_REMOTE_SERVICE_UDP_ADDRESS, CLIENT_REMOTE_SERVICE_UDP_PORT);

	g_timeout_add(5000, RequestOperation, agent);	//send REQUEST message to server every 5 seconds

	g_main_loop_run (loop);
}
