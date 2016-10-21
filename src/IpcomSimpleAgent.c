/*
 * IpcomSimpleAgent.c
 *
 *  Created on: Oct 17, 2016
 *      Author: hyotiger
 */

#include "../include/IpcomSimpleAgent.h"
#include "../include/IpcmdAgent.h"

struct _FakeIpcmdAgent {
	GHashTable	*clients_;			//key: service_id and link_type, value: IpcmdClient*
	GHashTable	*server_services_;	//key: service id, value: IpcmdService*
	IpcmdCore	*core_;
	GHashTable 	*transports_;
	guint16		last_used_transport_id_;
};

struct _IpcomSimpleAgent {
	IpcomAgent				_agent;
	IpcomSimpleAgentMode 	mode;		//0 - NOT initialized, 1 - connect mode, 2 - listen mode
	gchar					*bound_addr;
	guint16					bound_port;
	struct _FakeIpcmdAgent	*ipcmd_agent;

	IpcomConnection*		pConnection;
	IpcomConnection*		pBroadConnection;
	IpcomTransport*			pTransport;
	IpcomProtocol*			pProtocol;
	GHashTable*				hashSocketTable;
};

IpcomAgentSocket
_SimpleAgentBind (IpcomAgent *agent,IpcomTransportType type,const gchar *src,guint16 sport,GError **gerror)
{
	struct _IpcomSimpleAgent *simple_agent = (struct _IpcomSimpleAgent*)agent;
	IpcmdTransport	*transport;

	transport = IpcmdTransportUdpv4New();
	if (!transport) {
		return -1;
	}
	IpcmdBusAttachTransport (simple_agent->ipcmd_agent->core_, transport);
	if (!transport->bind(transport,src,sport)) {
		return -1;
	}

	g_hash_table_insert (agent->transports_, GINT_TO_POINTER(index), transport);
	simple_agent->
	return GINT_TO_POINTER(1);
}

gboolean
_SimpleAgentConnectTo (IpcomAgent *agent,IpcomAgentSocket type,const gchar *dst,guint16 dport,GError**)
{
	struct _IpcomSimpleAgent *simple_agent = (struct _IpcomSimpleAgent*)agent;

	IpcmdAgentTransportAddUdpv4Client (simple_agent->ipcmd_agent, simple_agent->bound_addr, simple_agent->bound_port, dst, dport);

	return TRUE;
}

gboolean
_SimpleAgentListenAt (IpcomAgent *agent,IpcomAgentSocket,GError**)
{
	struct _IpcomSimpleAgent *simple_agent = (struct _IpcomSimpleAgent*)agent;

	IpcmdAgentTransportAddUdpv4Server (simple_agent->ipcmd_agent, simple_agent->bound_addr, simple_agent->bound_port, FALSE);
}

gboolean
_SimpleAgentBroadcast (IpcomAgent*,IpcomAgentSocket,IpcomMessage*,GError**)
{
	return FALSE;
}
IpcomAgentSocket
_SimpleAgentGetAcceptedConnection (IpcomAgent*,IpcomAgentSocket,const gchar *dst,guint16 dport,GError**)
{

}

void
_SimpleAgentCloseAgentSocket (IpcomAgent*,IpcomAgentSocket)
{

}

IpcomOpHandle
_SimpleAgentOperationSend (IpcomAgent*,IpcomAgentSocket,IpcomMessage*,const IpcomAgentOpCallbacks*,GError**)
{

}

gboolean
_SimpleAgentOperationRespond (IpcomAgent*,IpcomOpHandle,IpcomMessage*,const IpcomAgentOpCallbacks*,GError**)
{

}

static struct _IpcomAgent agent_ops = {
		.Bind = _SimpleAgentBind,
		.ConnectTo = _SimpleAgentConnectTo,
		.Broadcast = _SimpleAgentBroadcast,
		.ListenAt = _SimpleAgentListenAt,
		.GetAcceptedConnection = _SimpleAgentGetAcceptedConnection,
		.CloseAgentSocket = _SimpleAgentCloseAgentSocket,
		.OperationSend = _SimpleAgentOperationSend,
		.OperationRespond = _SimpleAgentOperationRespond,
};

IpcomAgent*
IpcomSimpleAgentCreate(GMainContext* main_context)
{
	IpcomSimpleAgent *simple_agent = g_malloc0(sizeof(struct _IpcomSimpleAgent));
	IpcmdAgent *ipcmd_agent;

	ipcmd_agent = IpcmdAgentNew (main_context);

	simple_agent->ipcmd_agent = (struct _FakeIpcmdAgent*)ipcmd_agent;
	simple_agent->_agent = agent_ops;

	return simple_agent;
}
