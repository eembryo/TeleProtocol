#include <IpcomSimpleAgent.h>
#include <IpcomProtocol.h>

#define IPCOM_SIMPLE_AGENT_ERROR IpcomProtocolErrorQuark()

static GQuark
IpcomSimpleAgentErrorQuark(void)
{
	return g_quark_from_static_string ("ipcom-simple-agent-error-quark");
}

IpcomSimpleAgent*
IpcomSimpleAgentNew()
{
	IpcomSimpleAgent* newAgent;

	newAgent = (IpcomSimpleAgent*)malloc(sizeof(IpcomSimpleAgent));
	if (!newAgent)
	newAgent->mode = 0;
	newAgent->conn = NULL;
	newAgent->pProtocol = IpcomProtocolGetInstance();
	newAgent->pTransport = NULL;
	newAgent->pGlibContext = g_main_context_default();

	return newAgent;
}

void
IpcomSimpleAgentDestroy(IpcomSimpleAgent* agent)
{

}

gboolean
IpcomSimpleAgentConnectTo(IpcomSimpleAgent* agent, IpcomTransportType proto, const gchar *dst, guint16 dport, GError **error)
{
	if (agent->mode != 0) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 0, "Agent is already connected or listening.");
		return FALSE;
	}

	//[Transport] Create connection with CONNECT mode
	agent->pTransport = IpcomTransportNew(IPCOM_TRANSPORT_UDPV4, agent->pGlibContext);
	if (!agent->pTransport) {
		if (error) g_set_error(error, IPCOM_SIMPLE_AGENT_ERROR, 0, "Cannot get new IpcomTransport.");
		return FALSE;
	}

	//[Transport] connect to peer
	connection = IpcomTransportConnect(transport, "127.0.0.1", 50001, "127.0.0.1", 50000);

}
gboolean
IpcomSimpleAgentListenAt(IpcomSimpleAgent* agent,IpcomTransportType proto, const gchar *src, guint16 sport, GError **gerror)
{

}
gboolean
IpcomSimpleAgentRegisterMessageHandler(IpcomSimpleAgent* agent, guint16 serviceId, IMsgHandler msgHandler, GError **gerror)
{

}
IpcomOpHandle
IpcomSimpleAgentSendMessage(IpcomSimpleAgent*, IpcomMessage*, IpcomReceiveMessageCallback, IpcomOpCtxDestroyNotify, gpointer, GError **gerror)
{

}

gboolean
IpcomSimpleAgentBroadcast(IpcomSimpleAgent* agent, IpcomMessage*, GError **gerror)
{
	if (agent->pTransport)
}
gint
IpcomSimpleAgentRespondMessage(IpcomSimpleAgent*, IpcomOpHandle, IpcomMessage*, IpcomOpCtxDestroyNotify, gpointer, GError **gerror)
{

}
