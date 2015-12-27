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
  guint32		senderHandleId;
  IpcomConnection	*connection;
};

struct _IpcomProtocolOpContext
{
  struct _IpcomProtocolOpContextId	ctxId;
  IpcomReceiveMessageCallback		recvCallback;
  void								*cb_data;
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

typedef enum {
	OPCONTEXT_STATUS_REQUEST_SENT,
	OPCONTEXT_STATUS_REQUEST_RECV,
	OPCONTEXT_STATUS_ACK_SENT,
	OPCONTEXT_STATUS_ACK_RECV,
	OPCONTEXT_STATUS_RESPONSE_SENT,
	OPCONTEXT_STATUS_RESPONSE_RECV,
	OPCONTEXT_STATUS_NOTI_SENT,
	OPCONTEXT_STATUS_NOTI_RECV,
	OPCONTEXT_STATUS_ERROR_SENT,
	OPCONTEXT_STATUS_ERROR_RECV
};

gint
IpcomProtocolSendMessage(IpcomProtocol *proto, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata)
{
  VCCPDUHeader *hdr = IpcomMessageGetVCCPDUHeader(mesg);

  switch(hdr->opType) {
  case IPPROTO_OPTYPE_REQUEST:
  case IPPROTO_OPTYPE_SETREQUEST_NORETURN:
  case IPPROTO_OPTYPE_SETREQUEST:
  case IPPROTO_OPTYPE_NOTIFICATION_REQUEST:
    {
      IpcomProtocolOpContext *ctx = IpcomProtocolOpContextCreate(conn, mesg, recv_cb, userdata);
      ctx->status = OPCONTEXT_STATUS_REQUEST_SENT;
      IpcomConnectionPushOutgoingMessage(conn, mesg);
    }
    break;
  default:
    DWARN("Cannot send this opType(%d)",hdr->opType);
    return -1;
  }
  //IpcomProtocolOpContextNew()
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
  ctx->recvCallback = recv_cb;
  ctx->cb_data = userdata;
  ctx->serviceId = IpcomMessageGetVCCPDUHeader(mesg)->serviceID;
  ctx->opType = IpcomMessageGetVCCPDUHeader(mesg)->opType;
  ctx->message = IpcomMessageRef(mesg);

  return ctx;
}

void
IpcomProtocolOpContextDestroy(IpcomProtocolOpContext *ctx)
{
	g_free(ctx);
}
