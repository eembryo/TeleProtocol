#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include <IpcomSimpleAgent.h>
#include <IpcomProtocol.h>
#include <ref_count.h>
#include <IpcomTransport.h>
#include <IpcomConnection.h>
#include <IpcomService.h>

#include <dprint.h>

#ifdef DEBUG
#include <IpcomOperationContext.h>
#include <IpcomMessage.h>
#endif

#define IPCOM_SIMPLE_AGENT_ERROR IpcomSimpleAgentErrorQuark()

#define CONVERT_AGENT_TO_SIMPLEAGENT(ptr)	container_of(ptr, IpcomSimpleAgent, _agent)
#define CONVERT_SIMPLEAGENT_TO_AGENT(ptr)	(IpcomAgent*)(&ptr->_agent)

static GQuark
IpcomSimpleAgentErrorQuark(void)
{
	return g_quark_from_static_string ("ipcom-simple-agent-error-quark");
}

typedef enum {
	AGENTSOCKET_NULL = 0,
	AGENTSOCKET_CONNECT,
	AGENTSOCKET_LISTEN
} IpcomAgentSocketType;
typedef struct {
	IpcomAgentSocketType	type;
	union {
		IpcomConnection*	conn;
		IpcomTransport*		trans;
	} impl;
} IpcomAgentSocketImpl;

static const IpcomAgentSocket SIMPLEAGENTSOCKET_ERROR = GINT_TO_POINTER(0);
static const IpcomAgentSocket SIMPLEAGENTSOCKET_BOUND_SOCKET = GINT_TO_POINTER(1);

static IpcomAgentSocket
_Bind(IpcomAgent* agent,IpcomTransportType ttype,const gchar *src,guint16 sport,GError** error)
{
	IpcomSimpleAgent* 	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);
	IpcomTransport* 	pNewTransport = NULL;

	if (pSAgent->mode != NOT_INIT_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "IpcomSimpleAgent allows only one binding.");
		goto _Bind_failed;
	}

	pNewTransport = IpcomTransportNew(ttype, pSAgent->pGlibContext);
	if (pNewTransport == NULL) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Cannot create socket.");
		goto _Bind_failed;
	}

	if (!pNewTransport->bind(pNewTransport, src, sport)) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Cannot bind socket.");
		goto _Bind_failed;
	}

	pSAgent->mode = BINDING_MODE;
	pSAgent->pConnection = NULL;
	pSAgent->pTransport = pNewTransport;
	pSAgent->pBroadConnection = pNewTransport->getBroadConnection(pNewTransport);
	IpcomConnectionSetBroadConnectionPort(pSAgent->pBroadConnection, sport);
	///pSAgent->hashSocketTable <--not yet IMPLEMENTED

	return SIMPLEAGENTSOCKET_BOUND_SOCKET;

	_Bind_failed:
	if (pNewTransport) IpcomTransportDestroy(pNewTransport);
	return SIMPLEAGENTSOCKET_ERROR;
}

/**
 * RETURN - 0 on error
 */
static gboolean
_ConnectTo(IpcomAgent* agent,IpcomAgentSocket asock, const gchar *dst, guint16 dport, GError** error)
{
	IpcomSimpleAgent* 	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);
	IpcomConnection*	pConnection = NULL;

	if (asock != SIMPLEAGENTSOCKET_BOUND_SOCKET) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "wrong socket.");
		goto _ConnectTo_failed;
	}

	if (pSAgent->mode == NOT_INIT_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "socket is not bound.");
		goto _ConnectTo_failed;
	}
	else if (pSAgent->mode != BINDING_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "IpcomSimpleAgent is already connected or listening.");
		goto _ConnectTo_failed;
	}

	///[Transport] Create connection with CONNECTION mode
	pConnection = pSAgent->pTransport->connect( pSAgent->pTransport, dst, dport);
	if (pConnection == NULL) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Cannot connect to <%s:%d>", dst, dport);
		goto _ConnectTo_failed;
	}

	pSAgent->mode = CONNECTED_MODE;
	pSAgent->pConnection = pConnection;
	return TRUE;

	_ConnectTo_failed:
	return FALSE;
}

static inline IpcomAgentSocket
_GetAcceptedConnection(IpcomAgent* agent,IpcomAgentSocket asock,const gchar *dst,guint16 dport,GError** error)
{
	DPRINT("Not implemented yet.\n");

	if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Not implemented");

	return SIMPLEAGENTSOCKET_ERROR;
}

static inline gboolean
_AcceptAllNewConnections(IpcomConnection *conn, gpointer data)
{
	return TRUE;
}

static gboolean
_ListenAt(IpcomAgent* agent,IpcomAgentSocket asock, GError** error)
{
	IpcomSimpleAgent* pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);

	if (asock != SIMPLEAGENTSOCKET_BOUND_SOCKET) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "wrong socket.");
		goto _ListenAt_failed;
	}
	if (pSAgent->mode == NOT_INIT_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "socket is not bound.");
		goto _ListenAt_failed;
	}
	else if (pSAgent->mode != BINDING_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "IpcomSimpleAgent is already connected or listening.");
		goto _ListenAt_failed;
	}

	///[Transport] Create connection with LISTEN mode
	if (!pSAgent->pTransport->listen(pSAgent->pTransport, 10)) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Failed to listen.");
		goto _ListenAt_failed;
	}
	pSAgent->pTransport->onNewConn = _AcceptAllNewConnections;
	pSAgent->pTransport->onNewConn_data = (gpointer)agent;

	pSAgent->mode = LISTEN_MODE;
	pSAgent->pConnection = NULL;
	return TRUE;

	_ListenAt_failed:
	return FALSE;
}

static void
_CloseAgentSocket(IpcomAgent* agent,IpcomAgentSocket asock)
{
	IpcomSimpleAgent* pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);

	if (asock == SIMPLEAGENTSOCKET_BOUND_SOCKET) {
		if (pSAgent->pConnection) {
			IpcomConnectionUnref(pSAgent->pConnection);
			pSAgent->pConnection = NULL;
		}
		if (pSAgent->pTransport) {
			IpcomTransportDestroy(pSAgent->pTransport);
			pSAgent->pTransport = NULL;
		}
		pSAgent->mode = NOT_INIT_MODE;
	}
	else return;

	pSAgent->mode = NOT_INIT_MODE;
}

static gboolean
_Broadcast(IpcomAgent* agent,IpcomAgentSocket asock,IpcomMessage* mesg,GError** error)
{
	IpcomSimpleAgent* pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);

	if (asock != SIMPLEAGENTSOCKET_BOUND_SOCKET) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "wrong socket.");
		goto _Broadcast_failed;
	}

	if (pSAgent->mode == NOT_INIT_MODE) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "socket is not bound.");
		goto _Broadcast_failed;
	}

	if (!pSAgent->pBroadConnection) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Transport does not support broadcasting.");
		goto _Broadcast_failed;
	}

#if DEBUG
	DINFO("Operation will be broadcasted: (OpType:0x%.02x, SHID:0x%.04x)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg));
#endif
	IpcomProtocolSendMessageFull(pSAgent->pProtocol, pSAgent->pBroadConnection, mesg, NULL, NULL, NULL, NULL);
	return TRUE;

	_Broadcast_failed:
	return FALSE;
}

/**
 * _OperationSend
 */
struct opcb_data {
	IpcomAgent* agent;
	IpcomAgentOpCallbacks	cbs;
};

IpcomServiceReturn
_AgentOnResponse(const IpcomOpContextId *opContextId, IpcomMessage *mesg, gpointer userdata)
{
	struct opcb_data *cb_data = userdata;
	IpcomOpHandle handle = (IpcomOpHandle)opContextId;

#if DEBUG
	DINFO("Operation Response has been received: (OpType:0x%.02x, SHID:0x%.04x, CONN:%p)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), opContextId->connection);
#endif

	if (cb_data && cb_data->cbs.OnOpResponse)
	    cb_data->cbs.OnOpResponse(cb_data->agent, handle, mesg, cb_data->cbs.userdata);
	return IPCOM_SERVICE_SUCCESS;
}

static void
_AgentOnOpCtxDestroy(IpcomOpContextId *opContextId, gint code, gpointer userdata)
{
	struct opcb_data *cb_data = userdata;
	IpcomOpHandle handle = (IpcomOpHandle)opContextId;

	if (!cb_data)
		return;
	else if(cb_data->cbs.OnOpDestroyed)
		cb_data->cbs.OnOpDestroyed(cb_data->agent, handle, code, cb_data->cbs.userdata);

#if DEBUG
	DINFO("Operation has been destroyed: (SHID:0x%.04x, CONN:%p)\n", opContextId->senderHandleId, opContextId->connection);
#endif
	g_free(cb_data);
}

static IpcomOpHandle
_OperationSend(IpcomAgent* agent, IpcomAgentSocket asock, IpcomMessage* mesg,const IpcomAgentOpCallbacks* cbs, GError** error)
{
	IpcomSimpleAgent*	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);
	GError*				gerror = NULL;
	IpcomOpHandle		handle;
	struct opcb_data	*udata = NULL;

	if (asock != GINT_TO_POINTER(1)) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "IpcomAgentSocket(%x) does not exist.", GPOINTER_TO_INT(asock));
		goto _OperationSend_failed;
	}

	udata = (struct opcb_data*)g_malloc0(sizeof(struct opcb_data));
	if (!udata) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Not enough memory.");
		goto _OperationSend_failed;
	}
	udata->agent = agent;
	if (cbs) udata->cbs = *cbs;

#if DEBUG
	DINFO("Operation will be sent: (OpType:0x%.02x, SHID:0x%.04x, CONN:%p)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), pSAgent->pConnection);
#endif
	handle = IpcomProtocolSendMessageFull(pSAgent->pProtocol, pSAgent->pConnection, mesg, _AgentOnResponse, _AgentOnOpCtxDestroy, (gpointer)udata, &gerror);
	if (gerror) {
		g_error_free(gerror);
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Failed to transmit the operation.");
		goto _OperationSend_failed;
	}
	if (!handle) { //this does not mean error. In case of NOTIFICATION_CYCLIC, no handle is returned.
		//free udata, because OpCtxDestroy callback function will not be called.
		g_free(udata);
		return 0;
	}

	return handle;

	_OperationSend_failed:
	if (udata) g_free(udata);
	return 0;
}
static gboolean
_OperationRespond(IpcomAgent* agent,IpcomOpHandle handle, IpcomMessage* mesg,const IpcomAgentOpCallbacks* cbs, GError** error)
{
	IpcomSimpleAgent*	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);
	struct opcb_data	*udata = NULL;

	udata = (struct opcb_data*)g_malloc0(sizeof(struct opcb_data));
	if (!udata) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "Not enough memory.");
		goto _OperationRespond_failed;
	}
	udata->agent = agent;
	if (cbs) udata->cbs = *cbs;

	IpcomProtocolRespondMessageFull(pSAgent->pProtocol, handle, mesg, _AgentOnOpCtxDestroy, udata);

#if DEBUG
	DINFO("Operation Response has been sent: (OpType:0x%.02x, SHID:0x%.04x, CONN:%p)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), handle->connection);
#endif
	return TRUE;

	_OperationRespond_failed:
	if (udata) g_free(udata);
	return FALSE;
}

static void
_OperationCancel(IpcomAgent* agent,IpcomOpHandle handle)
{
	IpcomSimpleAgent*	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);

	IpcomProtocolCancelOpContext(pSAgent->pProtocol, handle);
}

/**
 * _RegisterMessageHandlers
 */
struct service_priv {
	IpcomAgent*		agent;
	IpcomAgentMsgHandlers	handlers;
};

static IpcomServiceReturn
_AgentProcessMessage(IpcomService *service, const IpcomOpContextId *ctxId, IpcomMessage *mesg)
{
	struct service_priv *priv = IpcomServiceGetPrivData(service);
	IpcomOpHandle handle = (IpcomOpHandle)ctxId;

#if DEBUG
	DINFO("Operation has been received: (OpType:0x%.02x, SHID:0x%.04x, CONN:%p)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), ctxId->connection);
#endif

	return priv->handlers.OnReceiveMesg(priv->agent, handle, mesg, priv->handlers.userdata);
}

static IpcomServiceReturn
_AgentProcessNoti(IpcomService *service, IpcomConnection *conn, IpcomMessage *mesg)
{
	struct service_priv *priv = IpcomServiceGetPrivData(service);

#if DEBUG
	DINFO("Operation has been received: (OpType:0x%.02x, SHID:0x%.04x, CONN:%p)\n", IpcomMessageGetVCCPDUOpType(mesg), IpcomMessageGetVCCPDUSenderHandleID(mesg), conn);
#endif
	return priv->handlers.OnReceiveNoti(priv->agent, mesg, priv->handlers.userdata);
}

static gboolean
_RegisterMessageHandlers(IpcomAgent* agent, guint16 serviceId, const IpcomAgentMsgHandlers* msgHandlers, GError** error)
{
	//IpcomSimpleAgent*	pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);
	IpcomService*		service = NULL;
	struct service_priv	priv = {0};

	///[Application] register service ID
	service = IpcomServiceNew(serviceId, sizeof(struct service_priv));
	priv.agent = agent;
	if (msgHandlers) priv.handlers = *msgHandlers;
	memcpy(IpcomServiceGetPrivData(service), &priv, sizeof(struct service_priv));

	service->ProcessMessage = _AgentProcessMessage;
	service->ProcessNotification = _AgentProcessNoti;
	if (!IpcomServiceRegister(service)) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 1, "ServiceID(%d) is already registered.", serviceId);
		goto _RegisterMessageHandlers_failed;
	}

	return TRUE;

	_RegisterMessageHandlers_failed:
	if (service) IpcomServiceDestroy(service);
	return FALSE;
}

static void
_UnRegisterMessageHandlers(IpcomAgent* agent, guint16 serviceId)
{

}

/* simple_agent : default IpcomAgent functions
 *
 	IpcomAgentSocket	(*Bind)							(IpcomAgent*,IpcomTransportType,const gchar *src,guint16 sport,GError**);
	gboolean			(*ConnectTo)					(IpcomAgent*,IpcomAgentSocket,const gchar *dst,guint16 dport,GError**);
	gboolean			(*ListenAt)						(IpcomAgent*,IpcomAgentSocket,GError**);
	gboolean			(*Broadcast)					(IpcomAgent*,IpcomAgentSocket,IpcomMessage*,GError**);
	IpcomAgentSocket	(*GetAcceptedConnection)		(IpcomAgent*,IpcomAgentSocket,const gchar *dst,guint16 dport,GError**);
	void				(*CloseAgentSocket)				(IpcomAgent*,IpcomAgentSocket);

	IpcomOpHandle		(*OperationSend)				(IpcomAgent*,IpcomAgentSocket,IpcomMessage*,const IpcomAgentOpCallbacks*,GError**);
	gboolean			(*OperationRespond)				(IpcomAgent*,IpcomOpHandle,   IpcomMessage*,const IpcomAgentOpCallbacks*,GError**);
	void				(*OperationCancel)				(IpcomAgent*,IpcomOpHandle);

	gboolean			(*RegisterMessageHandlers)		(IpcomAgent*,guint16 serviceId,const IpcomAgentMsgHandlers*, GError**);
	void				(*UnRegisterMessageHandlers)	(IpcomAgent*,guint16 serviceId);
 */


static IpcomAgent simple_agent = {
		.Bind = _Bind,
		.ConnectTo = _ConnectTo,
		.ListenAt = _ListenAt,
		.Broadcast = _Broadcast,
		.GetAcceptedConnection = _GetAcceptedConnection,
		.CloseAgentSocket = _CloseAgentSocket,

		.OperationSend = _OperationSend,
		.OperationRespond = _OperationRespond,
		.OperationCancel = _OperationCancel,

		.RegisterMessageHandlers = _RegisterMessageHandlers,
		.UnRegisterMessageHandlers = _UnRegisterMessageHandlers,
};

IpcomAgent*
IpcomSimpleAgentCreate(GMainContext *pMainContext)
{
	IpcomSimpleAgent* newAgent;

	newAgent = (IpcomSimpleAgent*)g_malloc(sizeof(IpcomSimpleAgent));
	if (!newAgent) return NULL;

	newAgent->mode = 0;
	newAgent->pConnection = NULL;
	newAgent->pBroadConnection = NULL;
	newAgent->pTransport = NULL;
	newAgent->pProtocol = IpcomProtocolGetInstance();
	newAgent->pGlibContext = pMainContext ? pMainContext : g_main_context_default();
	newAgent->_agent = simple_agent;

	return CONVERT_SIMPLEAGENT_TO_AGENT(newAgent);
}

void
IpcomSimpleAgentDestroy(IpcomAgent* agent)
{
	IpcomSimpleAgent* pSAgent = CONVERT_AGENT_TO_SIMPLEAGENT(agent);

	pSAgent->_agent.CloseAgentSocket(agent, GINT_TO_POINTER(1));
}
