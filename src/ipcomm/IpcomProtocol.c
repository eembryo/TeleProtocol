#include <glib.h>
#include <string.h>

#include <IpcomProtocol.h>
#include <IpcomService.h>
#include <IpcomConnection.h>
#include <IpcomMessage.h>
#include <dprint.h>

static IpcomProtocol *gIpcomProtocolInstance = NULL;

struct _IpcomProtocol {
	GHashTable	*pServiceHash;
	GHashTable	*pOpContextHash;
};

struct _IpcomProtocolOpContextId
{
	guint32			senderHandleId;
	IpcomConnection	*connection;
};

struct _IpcomProtocolOpContext
{
	struct _IpcomProtocolOpContextId	ctxId;
	IpcomReceiveMessageCallback			recvCallback;
	gint								opType;
	gint								status;
	gint								serviceId;
	IpcomMessage						*message;
	//timer
};

static guint
_OpContextIdHashFunc(gconstpointer key)
{
	struct _IpcomProtocolOpContextId *ctxId = (struct _IpcomProtocolOpContextId *)key;
	return ctxId->senderHandleId;
}

static gboolean
_OpContextIdEqual(gconstpointer aOpContextId, gconstpointer bOpContextId)
{
	return memcmp(aOpContextId, bOpContextId, sizeof(IpcomProtocolOpContextId)) ? FALSE : TRUE;
}

IpcomProtocol*
IpcomProtocolGetInstance()
{
	if (gIpcomProtocolInstance == NULL) {
		gIpcomProtocolInstance = g_malloc0(sizeof(IpcomProtocol));
		if (!gIpcomProtocolInstance) DWARN("Cannot allocate memory for IpcomProtocol.\n");
		gIpcomProtocolInstance->pServiceHash = g_hash_table_new(g_direct_hash, g_direct_equal);
		gIpcomProtocolInstance->pOpContextHash = g_hash_table_new(_OpContextIdHashFunc, _OpContextIdEqual);
	}

	return gIpcomProtocolInstance;
}

gboolean
IpcomProtocolRegisterService(IpcomProtocol *proto, IpcomService *service)
{
	if (g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(service->serviceId))) {
		DWARN("The service(%d) is already registered in IpcomProtocol.\n", service->serviceId);
		return FALSE;
	}

	g_hash_table_insert(proto->pServiceHash, GINT_TO_POINTER(service->serviceId), service);

	return TRUE;
}

gint
IpcomProtocolSendMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata)
{
	VCCPDUHeader *hdr = IpcomMessageGetVCCPDUHeader(mesg);

	switch(hdr->opType) {
	case IPPROTO_OPTYPE_REQUEST:
	case IPPROTO_OPTYPE_SETREQUEST_NORETURN:
	case IPPROTO_OPTYPE_SETREQUEST:
	case IPPROTO_OPTYPE_NOTIFICATION_REQUEST:
	case IPPROTO_OPTYPE_RESPONSE:
	case IPPROTO_OPTYPE_NOTIFICATION:
	case IPPROTO_OPTYPE_NOTIFICATION_CYCLIC:
	{
		IpcomProtocolOpContext *ctx = IpcomProtocolOpContextCreate(ctxId, hdr->opType, );
	}
	break;
	default:
		DWARN("Cannot send this opType(%d)",hdr->opType);
		return -1;
	}
	//IpcomProtocolOpContextNew()
	return IpcomConnectionPushOutgoingMessage(conn, mesg);
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
	IpcomService *service = g_hash_table_lookup(proto->pServiceHash, GINT_TO_POINTER(IpcomMessageGetVCCPDUHeader(mesg)->serviceID));

	if (!service) {
		//discard message
		IpcomMessagePut(mesg);
		return -1;
	}

	//expected optypes = {REQUEST, SETREQUEST_NORETURN, SETREQUEST
	return 0;
}

IpcomProtocolOpContext *
IpcomProtocolOpContextCreate(IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata)
{
	IpcomProtocolOpContextId ctxId;
	IpcomProtocolOpContext *ctx;

	ctx = g_malloc0(sizeof(IpcomProtocolOpContext));
	ctx->ctxId.connection = conn;
	ctx->ctxId.senderHandleId = IpcomMessageGetVCCPDUHeader(mesg)->senderHandleId;
	ctx->message = mesg;
	return ctx;
}
