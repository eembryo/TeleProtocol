/*
 * ServerTest.c
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

#define TRANSPORT_LOCAL_UDP_ADDRESS	"192.168.0.10"
#define TRANSPORT_LOCAL_UDP_PORT	50000
#define TRANSPORT_REMOTE_UDP_ADDRESS "192.168.0.10"
#define TRANSPORT_REMOTE_UDP_PORT	40000

#define CLIENT_REMOTE_SERVICE_ID	0x00A3
#define CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS	"192.168.0.10"
#define CLIENT_REMOTE_SERVICE_HOST_UDP_PORT		40000

#define SERVER_PROVIDE_SERVICE_ID	0x00A1

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
server_service_exec_callback (IpcmdService *self, OpHandle handle, const IpcmdOperationInfo *operation)
{
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

	switch (IpcmdMessageGetVCCPDUOpType(raw_mesg)) {
	case IPCMD_OPTYPE_REQUEST:
		IpcmdServiceCompleteOperation (self, handle, (IpcmdOperationInfo*)&reply_info);
		break;
	case IPCMD_OPTYPE_SETREQUEST_NORETURN:
		IpcmdServiceCompleteOperation (self, handle, (IpcmdOperationInfo*)&ok_info);
		break;
	case IPCMD_OPTYPE_SETREQUEST:
		IpcmdServiceCompleteOperation (self, handle, (IpcmdOperationInfo*)&reply_info);
		break;
	case IPCMD_OPTYPE_NOTIFICATION_REQUEST:
		IpcmdServiceCompleteOperation (self, handle, (IpcmdOperationInfo*)&ok_info);
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
	GMainLoop *loop = (GMainLoop*)data;
	g_main_loop_quit(loop);
	return G_SOURCE_REMOVE;
}

int main()
{
	IpcmdClient	*client;
	IpcmdServer *server;
	IpcmdCore 	*core;
	IpcmdBus	*bus;
	IpcmdTransport	*transport;
	IpcmdOperationCallback	cbs;
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
		IpcmdHost	*server_host;

		server_host = IpcmdUdpv4HostNew3 (CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS, CLIENT_REMOTE_SERVICE_HOST_UDP_PORT);
		client = IpcmdClientNew (core, CLIENT_REMOTE_SERVICE_ID, server_host);
		IpcmdHostUnref(server_host);
		IpcmdCoreRegisterClient(core, client);
	}
	/* Provide server service (0x00A3) */
	{
		IpcmdService	*server_service = g_malloc(sizeof(struct _IpcmdService));
		server_service->server_ = IpcmdCoreGetServer (core);
		server_service->service_id_ = SERVER_PROVIDE_SERVICE_ID;
		server_service->exec_ = server_service_exec_callback;
		IpcmdServerRegisterService (IpcmdCoreGetServer (core), server_service);
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
