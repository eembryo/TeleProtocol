/*
 * IpcmdAgent.c
 *
 *  Created on: Oct 6, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdAgent.h"
#include "../include/IpcmdOperation.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdClient.h"
#include "../include/IpcmdService.h"
#include "../include/IpcmdServer.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdHost.h"
#include "../include/IpcmdTransportUdpv4.h"
#include "../include/IpcmdTransport.h"
#include "../include/IpcmdOperationContext.h"

typedef struct _IpcmdAgent {
	GHashTable	*clients_;			//key: service_id and link_type, value: IpcmdClient*
	GHashTable	*server_services_;	//key: service id, value: IpcmdService*
	IpcmdCore	*core_;
	GHashTable 	*transports_;
	guint16		last_used_transport_id_;
} IpcmdAgent;

struct _IpcmdAgentClientHashkey {
	enum IpcmdHostLinkType link_type;
	guint16 service_id;
};

static guint _ClientHashfunc (gconstpointer key)
{
	struct _IpcmdAgentClientHashkey *client_key = (struct _IpcmdAgentClientHashkey*)key;

	return client_key->service_id + ((client_key->link_type & 0xFF)<<16);
}

static gboolean _ClientHahskkeyEqual (gconstpointer a, gconstpointer b)
{
	struct _IpcmdAgentClientHashkey *key_a = (struct _IpcmdAgentClientHashkey*)a;
	struct _IpcmdAgentClientHashkey *key_b = (struct _IpcmdAgentClientHashkey*)b;

	return key_a->link_type == key_b->link_type && key_a->service_id == key_b->service_id ? TRUE : FALSE;
}

IpcmdAgent *
IpcmdAgentNew(GMainContext *main_context)
{
	IpcmdAgent *agent = g_malloc0(sizeof(struct _IpcmdAgent));

	agent->core_ = IpcmdCoreNew (main_context);
	agent->last_used_transport_id_ = 0;
	agent->transports_ = g_hash_table_new (g_direct_hash, g_direct_equal);
	agent->clients_ = g_hash_table_new_full (_ClientHashfunc, _ClientHahskkeyEqual, g_free, NULL);
	agent->server_services_ = g_hash_table_new (g_direct_hash, g_direct_equal);

	return agent;
}

void
IpcmdAgentFree(IpcmdAgent *agent)
{
	g_hash_table_destroy(agent->clients_);
	g_hash_table_destroy(agent->server_services_);
	g_hash_table_destroy(agent->transports_);
	g_free(agent);
}

/* Transport */
/* @fn: IpcmdAgentTransportAdd
 * Create udp/tcp transport and register it to IP command protocol core.
 *
 * return : positive integer on success
 * 			-1 on not enough memory
 * 			-2 when can not use local address and port
 * 			-3 when maximum number of transports are registered
 */
gint
IpcmdAgentTransportAdd (IpcmdAgent *agent, const TransportDesc *transport_desc)
{
	IpcmdTransport	*transport;
	guint16			index;
	gint			error;

	switch (transport_desc->link_type_) {
	case IPCMD_HOSTLINK_UDPv4:
	{
		TransportDescUdpv4 *desc_udpv4 = (TransportDescUdpv4 *)transport_desc;

		transport = IpcmdTransportUdpv4New();
		if (!transport) {
			error = -1;
			goto _IpcmdAgentTransportAdd_failed;
		}
		IpcmdBusAttachTransport (IpcmdCoreGetBus(agent->core_), transport);
		if (!transport->bind(transport,desc_udpv4->local_addr_,desc_udpv4->local_port_)) {
			error = -2;
			goto _IpcmdAgentTransportAdd_failed;
		}

		if (desc_udpv4->is_server_) {
			transport->listen(transport,10);
			if (desc_udpv4->allow_broadcast_) transport->EnableBroadcast(transport);
		}
		else {
			transport->connect(transport, desc_udpv4->remote_addr_, desc_udpv4->remote_port_);
		}
	}
		break;
	default:
		g_error("Not implemented.");
	}

	for (index = agent->last_used_transport_id_+1; index!=agent->last_used_transport_id_; index++) {
		if (index == 0) continue;
		if (g_hash_table_contains(agent->transports_, GINT_TO_POINTER(index))) continue;
		g_hash_table_insert (agent->transports_, GINT_TO_POINTER(index), transport);
		break;
	}
	if (index == agent->last_used_transport_id_) {
		error = -3;
		goto _IpcmdAgentTransportAdd_failed;
	}
	agent->last_used_transport_id_ = index;

	return index;

	_IpcmdAgentTransportAdd_failed:
	if (transport) {
		IpcmdBusDetachTransport (IpcmdCoreGetBus(agent->core_), transport);
		IpcmdTransportUdpv4Destroy (transport);
	}
	return error;
}

gint
IpcmdAgentTransportAddUdpv4Server (IpcmdAgent *agent, gchar *local_addr, guint16 local_port, gboolean allow_broadcast)
{
	struct _TransportDescUdpv4 desc = {
			.parent_.link_type_ = IPCMD_HOSTLINK_UDPv4,
			.is_server_ = TRUE,
			.local_addr_ = local_addr,
			.local_port_ = local_port,
			.allow_broadcast_ = allow_broadcast,
			.remote_addr_ = NULL,
			.remote_port_ = 0,
	};
	return IpcmdAgentTransportAdd (agent, (TransportDesc*)&desc);
}

gint
IpcmdAgentTransportAddUdpv4Client (IpcmdAgent *agent, gchar *local_addr, guint16 local_port, gchar *remote_addr, guint16 remote_port)
{
	struct _TransportDescUdpv4 desc = {
			.parent_.link_type_ = IPCMD_HOSTLINK_UDPv4,
			.is_server_ = FALSE,
			.local_addr_ = local_addr,
			.local_port_ = local_port,
			.allow_broadcast_ = FALSE,
			.remote_addr_ = remote_addr,
			.remote_port_ = remote_port,
	};
	return IpcmdAgentTransportAdd (agent, (TransportDesc*)&desc);
}

void
IpcmdAgentTransportRemove(IpcmdAgent *agent, gint transport_id)
{
	//IMPL:
	IpcmdTransport *transport = g_hash_table_lookup (agent->transports_, GINT_TO_POINTER(transport_id));

	if (!transport) return;
	IpcmdBusDetachTransport (IpcmdCoreGetBus(agent->core_), transport);

	g_hash_table_remove (agent->transports_, GINT_TO_POINTER(transport_id));
	switch (transport->type_) {
	case IPCMD_HOSTLINK_UDPv4:
		IpcmdTransportUdpv4Destroy (transport);
		break;
	default:
		g_error("Not implemented");
	}
}

/* IP COMMAND PROTOCOL CLIENT */

/* @fn: IpcmdAgentClientConnectToService
 *
 * return:	0 on success
 * 			-1 on not enough memory
 * 			-2 on already connected with service_id and link_type
 */
gint
IpcmdAgentClientOpenService (IpcmdAgent *agent, guint16 service_id, const IpcmdHost *remote_host)
{
	struct _IpcmdAgentClientHashkey *client_hashkey;
	IpcmdClient	*client;

	client_hashkey = g_malloc(sizeof(struct _IpcmdAgentClientHashkey));
	if (!client_hashkey) return -1;
	client_hashkey->link_type = IpcmdHostType (remote_host);
	client_hashkey->service_id = service_id;
	if (g_hash_table_contains (agent->clients_, client_hashkey)) {
		g_free(client_hashkey);
		return -2;
	}

	client = IpcmdClientNew (agent->core_, service_id, remote_host->duplicate(remote_host));
	IpcmdCoreRegisterClient(agent->core_, client);

	g_hash_table_insert (agent->clients_, client_hashkey, client);

	return 0;
}

gint
IpcmdAgentClientOpenServiceUdpv4 (IpcmdAgent *agent, guint16 service_id, gchar *remote_addr, guint16 remote_port)
{
	IpcmdHost	*remote_host;
	gint		ret;

	remote_host = IpcmdUdpv4HostNew3 (remote_addr, remote_port);
	ret = IpcmdAgentClientOpenService (agent, service_id, remote_host);
	IpcmdHostUnref(remote_host);

	return ret;
}

void
IpcmdAgentClientCloseService (IpcmdAgent *agent, enum IpcmdHostLinkType link_type, guint16 service_id)
{
	struct _IpcmdAgentClientHashkey hashkey = {
			.link_type = link_type,
			.service_id = service_id,
	};
	IpcmdClient *client;

	client = g_hash_table_lookup (agent->clients_, &hashkey);
	if (!client) return;

	IpcmdCoreUnregisterClient(agent->core_, client);
	g_hash_table_remove (agent->clients_, &hashkey);
	IpcmdClientDestroy(client);
}

void
IpcmdAgentClientCloseServiceUdpv4 (IpcmdAgent *agent, guint16 service_id)
{
	IpcmdAgentClientCloseService (agent, IPCMD_HOSTLINK_UDPv4, service_id);
}

OpHandle
IpcmdAgentClientInvokeOperation(IpcmdAgent *agent, enum IpcmdHostLinkType link_type, guint16 service_id, guint16 op_id, guint8 op_type, guint8 flags, const IpcmdOperationPayload *payload,const IpcmdOperationCallback *cb)
{
	struct _IpcmdAgentClientHashkey hashkey = {
			.link_type = link_type,
			.service_id = service_id,
	};
	IpcmdClient *client = g_hash_table_lookup (agent->clients_, &hashkey);

	if (!client) return NULL;

	return IpcmdClientInvokeOperation(client, op_id, op_type, flags, payload, cb);
}

OpHandle
IpcmdAgentClientInvokeOperationUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, guint8 op_type, guint8 flags, const IpcmdOperationPayload *payload, const IpcmdOperationCallback *cb)
{
	return IpcmdAgentClientInvokeOperation (agent, IPCMD_HOSTLINK_UDPv4, service_id, op_id, op_type, flags, payload, cb);
}
/*
 * return:	0 on success
 * 			-1 on not registered service
 */
gint
IpcmdAgentClientListenNoti (IpcmdAgent *agent, enum IpcmdHostLinkType link_type, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdOperationCallback *cb)
{
	struct _IpcmdAgentClientHashkey hashkey = {
			.link_type = link_type,
			.service_id = service_id,
	};
	IpcmdClient *client = g_hash_table_lookup (agent->clients_, &hashkey);
	if (!client) return -1;

	IpcmdClientSubscribeNotification(client, op_id, is_cyclic, cb);
	return 0;
}
gint
IpcmdAgentClientListenNotiUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdOperationCallback *cb)
{
	return IpcmdAgentClientListenNoti (agent, IPCMD_HOSTLINK_UDPv4, service_id, op_id, is_cyclic, cb);
}

void
IpcmdAgentClientIgnoreNoti (IpcmdAgent *agent, enum IpcmdHostLinkType link_type, guint16 service_id, guint16 op_id, gboolean is_cyclic)
{
	struct _IpcmdAgentClientHashkey hashkey = {
			.link_type = link_type,
			.service_id = service_id,
	};
	IpcmdClient *client = g_hash_table_lookup (agent->clients_, &hashkey);
	if (!client) return;

	//IMPL: need to consider is_cyclic
	IpcmdClientUnsubscribeNotification(client, op_id);
}

void
IpcmdAgentClientIgnoreNotiUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic)
{
	IpcmdAgentClientIgnoreNoti (agent, IPCMD_HOSTLINK_UDPv4, service_id, op_id, is_cyclic);
}

/* IP COMMAND PROTOCOL SERVER */

/*
 * return:	service_id on success
 * 			-1 on not enough memory
 * 			-2 on already opened service
 */
gint
IpcmdAgentServerOpenService (IpcmdAgent *agent, guint16 service_id, ExecOperation exec, gpointer exec_data, DestroyExecData destroy)
{
	IpcmdService	*server_service;
	IpcmdOperationCallback	cbs = {
			.cb_func = (void (*)(OpHandle, const IpcmdOperationInfo*, gpointer))exec,
			.cb_data = exec_data,
			.cb_destroy = destroy
	};

	if (g_hash_table_contains (agent->server_services_, GINT_TO_POINTER(service_id))) {	//already opened service
		return -2;
	}
	server_service = g_malloc(sizeof(struct _IpcmdService));
	if (!server_service) return -1;

	IpcmdServiceInit (server_service, IpcmdCoreGetServer(agent->core_), service_id, &cbs);
	IpcmdServerRegisterService (IpcmdCoreGetServer(agent->core_), server_service);

	g_hash_table_insert (agent->server_services_, GINT_TO_POINTER(service_id), server_service);

	return service_id;
}

void
IpcmdAgentServerCloseService (IpcmdAgent *agent, guint16 service_id)
{
	IpcmdService	*server_service = g_hash_table_lookup (agent->server_services_, GINT_TO_POINTER(service_id));

	if (!server_service) return;
	IpcmdServerUnregisterService (IpcmdCoreGetServer(agent->core_), server_service);
	g_hash_table_remove (agent->server_services_, GINT_TO_POINTER(service_id));
	g_free (server_service);
}

/*
 *
 */
gint
IpcmdAgentServerCompleteOperation (IpcmdAgent *agent, OpHandle handle, const IpcmdOperationInfo *result)
{
	IpcmdService	*server_service;
	const IpcmdOpCtx	*opctx = (const IpcmdOpCtx*)handle;

	server_service = g_hash_table_lookup (agent->server_services_, GINT_TO_POINTER(opctx->serviceId));

	if (!server_service) return -1;

	return IpcmdServiceCompleteOperation (server_service, handle, result);
}

/*
 * return:	0 on success
 */
gint
IpcmdAgentServerAddNotiSubscriber (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdHost *subscriber, gboolean is_static_member)
{
	IpcmdService	*server_service = g_hash_table_lookup (agent->server_services_, GINT_TO_POINTER(service_id));
	IpcmdHost		*subscrib_host;

	subscrib_host = subscriber->duplicate(subscriber);
	IpcmdServiceAddSubscriber (server_service, op_id, is_cyclic, subscrib_host, is_static_member);
	IpcmdHostUnref (subscrib_host);

	return 0;
}

gint
IpcmdAgentServerAddNotiSubscriberUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, gchar *subscriber_addr, guint16 subscriber_port, gboolean is_static_member)
{
	IpcmdHost	*subscriber;
	gint		ret;

	subscriber = IpcmdUdpv4HostNew3 (subscriber_addr, subscriber_port);
	ret = IpcmdAgentServerAddNotiSubscriber (agent, service_id, op_id, is_cyclic, subscriber, is_static_member);
	IpcmdHostUnref (subscriber);

	return ret;
}

/*
 * return:	0 on success
 * 			-1 on not enough memory
 * 			-2 on wrong op_id
 * 			-3 on wrong noti_info
 */
gint
IpcmdAgentServerInformNoti (IpcmdAgent *agent, guint16 service_id, guint16 op_id, const IpcmdOperationInfoReplyMessage *noti_info)
{
	IpcmdService	*server_service = g_hash_table_lookup (agent->server_services_, GINT_TO_POINTER(service_id));

	IpcmdServiceInformNotification (server_service, op_id, noti_info);

	return 0;
}

void
IpcmdAgentServerRemoveNotiSubscriber (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdHost *subscriber)
{
	IpcmdService	*server_service = g_hash_table_lookup (agent->server_services_, GINT_TO_POINTER(service_id));
	IpcmdHost		*subscrib;

	subscrib = subscriber->duplicate(subscriber);
	IpcmdServiceRemoveSubscriber(server_service, op_id, is_cyclic, subscrib);
	IpcmdHostUnref(subscrib);
}

void IpcmdAgentServerRemoveNotiSubscriberUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, const gchar *subscriber_addr, guint16 subscriber_port)
{
	IpcmdHost	*subscriber;

	subscriber = IpcmdUdpv4HostNew3 (subscriber_addr, subscriber_port);
	IpcmdAgentServerRemoveNotiSubscriber (agent, service_id, op_id, is_cyclic, subscriber);
	IpcmdHostUnref (subscriber);
}
