#include "../include/IpcmdClient.h"
#include "../include/IpcmdOperation.h"
#include "../include/IpcmdService.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdTransportUdpv4.h"
#include "../include/IpcmdTransport.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdMessage.h"

#define TRANSPORT_LOCAL_UDP_ADDRESS	"192.168.0.10"
#define TRANSPORT_LOCAL_UDP_PORT	40000
#define TRANSPORT_REMOTE_UDP_ADDRESS "192.168.0.10"
#define TRANSPORT_REMOTE_UDP_PORT	50000

#define CLIENT_REMOTE_SERVICE_ID	0x00A1
#define CLIENT_REMOTE_SERVICE_HOST_UDP_ADDRESS	"192.168.0.10"
#define CLIENT_REMOTE_SERVICE_HOST_UDP_PORT		50000

#define SERVER_PROVIDE_SERVICE_ID	0x00A3

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
	IpcmdClient *client = (IpcmdClient*)data;
	IpcmdOperationCallback	cbs;
	static count = 0;

	/* Invoke Operation */
	cbs.cb_data = client;
	cbs.cb_func = op_callback;
	cbs.cb_destroy = NULL;

	IpcmdClientInvokeOperation(client, 0x0104, count%4, 0x00, NULL, &cbs);
	count++;

	return TRUE;
}

int main()
{
	IpcmdClient	*client;
	IpcmdServer *server;
	IpcmdCore 	*core;
	IpcmdBus	*bus;
	IpcmdTransport	*transport;
	//IpcmdOperationCallback	cbs;
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);

	core = IpcmdCoreNew (g_main_context_default());
	bus = IpcmdCoreGetBus (core);

	/* Setup UDP client */
	transport = IpcmdTransportUdpv4New();
	IpcmdBusAttachTransport (bus, transport);
	transport->bind(transport,TRANSPORT_LOCAL_UDP_ADDRESS,TRANSPORT_LOCAL_UDP_PORT);
	transport->connect(transport, TRANSPORT_REMOTE_UDP_ADDRESS, TRANSPORT_REMOTE_UDP_PORT);

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

	g_timeout_add(1000, ApplicationLifetime, client); //request every second
	g_main_loop_run (loop);

	g_message("Exit!");
}
