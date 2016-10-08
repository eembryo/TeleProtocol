/*
 * ServerTestNoti.c
 *
 *  Created on: Sep 28, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdClient.h"
#include "../include/IpcmdOperation.h"
#include "../include/IpcmdService.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdTransportUdpv4.h"
#include "../include/IpcmdTransport.h"
#include "../include/IpcmdMessage.h"

#define TRANSPORT_LOCAL_UDP_ADDRESS	"192.168.0.13"
#define TRANSPORT_LOCAL_UDP_PORT	50000
#define TRANSPORT_REMOTE_UDP_ADDRESS "192.168.0.13"
#define TRANSPORT_REMOTE_UDP_PORT	40000

#define CLIENT_REMOTE_SERVICE_ID	0x00A3
#define CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS	"192.168.0.13"
#define CLIENT_REMOTE_SERVICE_HOST_UDP_PORT		40000

#define SERVER_PROVIDE_SERVICE_ID	0x00A1
#define SERVER_NOTIFICATION_OPERATION_ID	0x0107
/*
struct _IpcmdOperationPayload {
	guint8		type_;	//'data_' encoding type: 0 on encoded message, 1 on Normal message
	guint32		length_;
	gpointer	data_;
};
struct _IpcmdOperationInfoReplyMessage {
	struct _IpcmdOperationInfo parent_;
	guint8							op_type_;
	struct _IpcmdOperationPayload	payload_;
};
*/
static void
server_service_exec_callback (OpHandle handle, const IpcmdOperationInfo *operation, gpointer cb_data)
{
	IpcmdService *service = (IpcmdService*)cb_data;
	gchar payload_data[10] = {0xff, 0xff, 0xff,0xff, 0xff, 0xff,0xff, 0xff, 0xff,0xff};
	const IpcmdOperationInfoReceivedMessage *recv_mesg = (const IpcmdOperationInfoReceivedMessage*)operation;
	IpcmdMessage *raw_mesg = (IpcmdMessage*)recv_mesg->raw_message_;
	IpcmdOperationInfoReplyMessage reply_info;
	IpcmdOperationInfoOk ok_info;

	IpcmdOperationInfoReplyMessageInit(&reply_info);
	reply_info.op_type_ = IPCMD_OPTYPE_RESPONSE;
	reply_info.payload_.type_ = IPCMD_PAYLOAD_NOTENCODED;
	reply_info.payload_.length_ = 10;
	reply_info.payload_.data_ = payload_data;

	IpcmdOperationInfoOkInit(&ok_info);

	g_usleep (100*1000);
	switch (IpcmdMessageGetVCCPDUOpType(raw_mesg)) {
	case IPCMD_OPTYPE_REQUEST:
		IpcmdServiceCompleteOperation (service, handle, (IpcmdOperationInfo*)&reply_info);
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		IpcmdServiceCompleteOperation (service, handle, (IpcmdOperationInfo*)&ok_info);
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		IpcmdServiceCompleteOperation (service, handle, (IpcmdOperationInfo*)&reply_info);
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		IpcmdServiceCompleteOperation (service, handle, (IpcmdOperationInfo*)&ok_info);
		break;
	default:
		g_error("will not happened");
		break;
	}
}

static void
op_callback (OpHandle handle, const IpcmdOperationInfo *result, gpointer cb_data)
{
	switch (result->type_) {
	case kOperationInfoOk:
		g_message("Got kOpeartionInfoOk.");
		break;
	case kOperationInfoFail:
	{
		IpcmdOperationInfoFail *info = (IpcmdOperationInfoFail*)result;
		g_message("Got kOpeartionInfoFail: reason = %d", info->reason_);
	}
		break;
	case kOperationInfoReceivedMessage:
		g_message("Got kOpeartionInfoReceivedMessage.");
		break;
	default:
		g_error("What is this type? %d", result->type_);
	}
}

static gboolean
ApplicationLifetime(gpointer data)
{
	gchar payload_data[10] = {0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0,0xf0, 0xf0};
	IpcmdService *service = (IpcmdService *)data;
	IpcmdOperationInfoReplyMessage reply_info;
	IpcmdOperationInfoReplyMessageInit(&reply_info);

	reply_info.op_type_ = IPCMD_OPTYPE_NOTIFICATION;
	reply_info.payload_.data_ = payload_data;
	reply_info.payload_.length_ = 10;
	reply_info.payload_.type_ = IPCMD_PAYLOAD_NOTENCODED;

	IpcmdServiceInformNotification (service, SERVER_NOTIFICATION_OPERATION_ID, &reply_info);
	return G_SOURCE_CONTINUE;
}

int main()
{
	IpcmdClient	*client;
	//IpcmdServer *server;
	IpcmdCore 	*core;
	IpcmdBus	*bus;
	IpcmdTransport	*transport;
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);

	core = IpcmdCoreNew (g_main_context_default());
	bus = IpcmdCoreGetBus (core);

	/* Setup UDP server */
	transport = IpcmdTransportUdpv4New();
	IpcmdBusAttachTransport (bus, transport);
	transport->bind(transport,TRANSPORT_LOCAL_UDP_ADDRESS,TRANSPORT_LOCAL_UDP_PORT);
	transport->listen(transport,10);

	/* Connect to service (0x00A1)*/
	{
		IpcmdHost	*peer_host;

		peer_host = IpcmdUdpv4HostNew3 (CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS, CLIENT_REMOTE_SERVICE_HOST_UDP_PORT);
		client = IpcmdClientNew (core, CLIENT_REMOTE_SERVICE_ID, peer_host);
		IpcmdHostUnref(peer_host);
		IpcmdCoreRegisterClient(core, client);
	}
	/* Provide server service (0x00A3) */
	{
		IpcmdService	*server_service = g_malloc(sizeof(struct _IpcmdService));
		IpcmdOperationCallback	cbs = {
				.cb_func = server_service_exec_callback,
				.cb_data = server_service,
				.cb_destroy = NULL
		};
		server_service->server_ = IpcmdCoreGetServer (core);
		server_service->service_id_ = SERVER_PROVIDE_SERVICE_ID;
		server_service->exec_ = cbs;
		IpcmdServerRegisterService (IpcmdCoreGetServer (core), server_service);

		// Add static subscriber
		//IpcmdService *self, guint16 operation_id, gboolean is_cyclic, IpcmdHost *subscriber, gboolean is_static_member);
		{
			IpcmdHost *static_host = IpcmdUdpv4HostNew3 (CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS, CLIENT_REMOTE_SERVICE_HOST_UDP_PORT);
			IpcmdServiceAddSubscriber (server_service, SERVER_NOTIFICATION_OPERATION_ID, FALSE, static_host, TRUE);
			IpcmdHostUnref (static_host);
		}
		g_timeout_add(3000, ApplicationLifetime, server_service); //send notification every 3 seconds
	}

#if 0
	/* Invoke Operation */
	cbs.cb_data = client;
	cbs.cb_func = op_callback;
	cbs.cb_destroy = NULL;

	IpcmdClientInvokeOperation(client, 0x0104, 0x00, 0x00, NULL, &cbs);

	//g_timeout_add(10000, ApplicationLifetime, loop); //exit after 10 seconds
#endif
	g_main_loop_run (loop);

	g_message("Exit!");
}
