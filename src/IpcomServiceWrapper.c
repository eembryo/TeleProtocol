#include <glib.h>
#include <IpcomMessage.h>
#include <IpcomService.h>
#include <IpcomServiceWrapper.h>
#include <IpcomConnection.h>
#include <IpcomTransport.h>
#include <IpcomTransportUDPv4.h>
#include <IpcomProtocol.h>
#include <ref_count.h>
#include <dprint.h>

#define GetWrapperFromIpcomService(p) (IpcomServiceWrapper *)(container_of(p, struct _IpcomServiceWrapper, _service))

static gboolean
_AddNewConnection(struct _IpcomServiceWrapper *wrapper, IpcomConnection *conn)
{
	IpcomConnectionRef(conn);
	wrapper->acceptedConns = g_list_append(wrapper->acceptedConns, conn);

	return TRUE;
}

static gboolean
_DelConnectionFromList(struct _IpcomServiceWrapper *wrapper, IpcomConnection *conn)
{
	wrapper->acceptedConns = g_list_remove(wrapper->acceptedConns, conn);
	IpcomConnectionUnref(conn);

	return TRUE;
}

static gboolean
_DenyNewConnections(IpcomService *service, IpcomConnection *conn)
{
	return FALSE;
}
static gboolean
_AllowNewConnections(IpcomService *service, IpcomConnection *conn)
{
	_AddNewConnection(GetWrapperFromIpcomService(service), conn);
	return TRUE;
}

static IpcomServiceReturn
_DefaultProcessMessage(IpcomService *service, const IpcomOpContextId *ctxId, IpcomMessage *mesg)
{
	IpcomServiceWrapper *wrapper = GetWrapperFromIpcomService(service);
	if (!wrapper->process_cb) {
		DWARN("You should set ProcessMessage callback in IpcomService structure.\n");
		return IPCOM_SERVICE_ERR_XXX;
	}

	wrapper->process_cb(wrapper, mesg, ctxId, wrapper->cb_data);

	return  IPCOM_SERVICE_SUCCESS;
}

IpcomMessage *
IpcomServiceWrapperCreateMessage(IpcomServiceWrapper *wrapper,
		guint16 operationId, guint32 senderHandleId, guint8 opType, guint8 dataType, guint8 reserved, gpointer payload, guint32 payloadLen)
{
	IpcomMessage *msg;

	msg = IpcomMessageNew(IPCOM_MESSAGE_MIN_SIZE);

	IpcomMessageSetVCCPDUHeader(msg,
			wrapper->_service.serviceId, operationId,
			IPCOM_MESSAGE_MIN_SIZE,
			senderHandleId,
			IPCOM_PROTOCOL_VERSION, opType, dataType, reserved);
	IpcomMessageSetPayloadBuffer(msg, payload, payloadLen);

	return msg;
}

IpcomServiceWrapper *
IpcomServiceWrapperNew(IpcomServiceMode mode, IpcomServiceId serviceId, IpcomTransportType transType, gchar *address, guint port)
{
	IpcomServiceWrapper *new = g_malloc0(sizeof(struct _IpcomServiceWrapper));

	new->_service.mode = mode;
	if (mode == IPCOM_SERVICE_CLIENT) {
		new->_service.OnRecvNewConn = _DenyNewConnections;
		new->peerAddr = address;
		new->peerPort = port;
		new->localAddr = NULL;
		new->localPort = port;
	}
	else {	//IPCOM_SERVICE_SERVER
		new->_service.OnRecvNewConn = _AllowNewConnections;
		new->peerAddr = NULL;
		new->peerPort = 0;
		new->localAddr = address;
		new->localPort = port;
	}
	new->_service.serviceId = serviceId;
	new->_service.pProto = IpcomProtocolGetInstance();

	switch(transType) {
	case IPCOM_TRANSPORT_UDPV4:
		new->_service.pTransport = IpcomTransportUDPv4New(&new->_service);
		break;
	default:
		DERROR("The TransportType(%d) is not supported.\n", transType);
		g_assert(FALSE);
		break;
	}
	new->_service.ProcessMessage = _DefaultProcessMessage;
	new->status = IPCOM_SERVICE_STATUS_NONE;
	new->seqNum = 0;

	return new;
}

void
IpcomServiceWrapperSetLocalAddress(IpcomServiceWrapper *wrapper, gchar *address, guint port)
{
	if (wrapper->status == IPCOM_SERVICE_STATUS_RUNNING) return;
	if(address)	wrapper->localAddr = address;
	wrapper->localPort = port;
}

gint
IpcomServiceWrapperSend(IpcomServiceWrapper *wrapper, IpcomMessage *mesg, IpcomReceiveMessageCallback cb, void *userdata)
{
	gint status = 0;

	if (wrapper->_service.mode == IPCOM_SERVICE_CLIENT) {
		status = IpcomProtocolSendMessage(wrapper->_service.pProto, wrapper->connected, mesg, cb, userdata);
	}
	else {
		DERROR("Server wrapper for IP command is not supported.\n");
		g_assert(FALSE);
	}

	return status;
}

gint
IpcomServiceWrapperSendRequest(IpcomServiceWrapper *wrapper,
		guint16 operationId, guint8 opType, guint8 dataType, guint8 reserved,
		gpointer payload, guint32 length,
		IpcomReceiveMessageCallback cb, gpointer userdata)
{
	IpcomMessage *msg;
	gint status = 0;

	if (wrapper->_service.mode == IPCOM_SERVICE_SERVER) {
		DERROR("Server should not use this function.\n");
		return -1;
	}

	if (opType != IPCOM_OPTYPE_REQUEST &&
			opType != IPCOM_OPTYPE_SETREQUEST &&
			opType != IPCOM_OPTYPE_SETREQUEST_NORETURN &&
			opType != IPCOM_OPTYPE_NOTIFICATION_REQUEST) {
		DERROR("opType should be one of IPCOM_OPTYPE_REQUEST, IPCOM_OPTYPE_SETREQUEST, IPCOM_OPTYPE_SETREQUEST_NORETURN and IPCOM_OPTYPE_NOTIFICATION_REQUEST\n");
		return -1;
	}

	msg = IpcomServiceWrapperCreateMessage(wrapper, operationId, BUILD_SENDERHANDLEID(wrapper->_service.serviceId, operationId, opType, wrapper->seqNum++), opType, dataType, reserved, payload, length);

	status = IpcomServiceWrapperSend(wrapper, msg, cb, userdata);
	IpcomMessageUnref(msg);

	return status;
}

gint
IpcomServiceWrapperRespondMessage(IpcomServiceWrapper *wrapper, const IpcomOpContextId *opCtxId, IpcomMessage *mesg)
{
	IpcomProtocolRepondMessage(wrapper->_service.pProto, opCtxId, mesg);
}

gint
IpcomServiceWrapperStart(IpcomServiceWrapper *wrapper, GMainContext *context)
{
	//Register this service to IP Command Protocol core.
	if (!IpcomProtocolRegisterService(wrapper->_service.pProto, &wrapper->_service)) {
		DERROR("Cannot register protocol (0x%x)\n.", wrapper->_service.serviceId);
		return -1;
	}

	IpcomTransportUDPv4AttachGlibContext(wrapper->_service.pTransport, context);
	wrapper->_service.pTransport->bind(wrapper->_service.pTransport, wrapper->localAddr, wrapper->localPort);

	if (wrapper->_service.mode == IPCOM_SERVICE_CLIENT) {
		wrapper->connected = wrapper->_service.pTransport->connect( wrapper->_service.pTransport, wrapper->peerAddr, wrapper->peerPort);
	}
	else { //IPCOM_SERVICE_SERVER
		wrapper->_service.pTransport->listen(wrapper->_service.pTransport, 10);
	}

	wrapper->status = IPCOM_SERVICE_STATUS_RUNNING;

	return 0;

service_start_failed:
	//IpcomProtocolUnregisterService(wrapper->_service);
	//... <--IMPLEMENT
	return -1;
}
