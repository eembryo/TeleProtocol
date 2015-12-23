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
	//timer
};

static gboolean
_OpContextId_equal(gconstpointer aOpContextId, gconstpointer bOpContextId)
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
		gIpcomProtocolInstance->pOpContextHash = g_hash_table_new(g_direct_hash, _OpContextId_equal);
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

	//IpcomProtocolOpContextNew()
	return IpcomConnectionPushOutgoingMessage(conn, mesg);
}

gint
IpcomProtocolHandleMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg)
{
}
