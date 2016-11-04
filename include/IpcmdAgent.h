/*
 * IpcmdAgent.h
 *
 *  Created on: Oct 6, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDAGENT_H_
#define INCLUDE_IPCMDAGENT_H_

#include "IpcmdDeclare.h"
#include "IpcmdOperation.h"
#include "IpcmdHost.h"
#include "IpcmdMessage.h"
#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdAgent IpcmdAgent;

typedef struct _TransportDesc {
	enum IpcmdHostLinkType	link_type_;
} TransportDesc;

typedef struct _TransportDescUdpv4 {
	TransportDesc				parent_;
	gboolean					is_server_;
	gchar						*local_addr_;
	guint16						local_port_;

	gboolean					allow_broadcast_;	//used in case of kIpcmdAgentLinkUdpv4
	guint16						broadcast_port_;	//port number for broadcasting. if 0, local_port_ is used instead.

	gchar						*remote_addr_;		//used in case of ClientTransport
	guint16						remote_port_;		//used in case of ClientTransport
} TransportDescUdpv4;

IpcmdAgent *IpcmdAgentNew(GMainContext *main_context);
void	IpcmdAgentFree(IpcmdAgent *agent);

/* Transport */
/**
 * @fn IpcmdAgentTransportAdd
 * @brief Add transport to IP command protocol core
 * @param[in] agent:
 * @param[in] transport_desc: transport configuration to be created
 * @return transport id if it successfully added
 * @retval positive integer on success
 * @retval -1 Error not enough memory
 * @retval -2 Error binding to address: Cannot assign requested address or port
 * @retval -3 Error maximum number of transports are added
 */
gint IpcmdAgentTransportAdd (IpcmdAgent *agent, const TransportDesc *transport_desc);
gint IpcmdAgentTransportAddUdpv4Server (IpcmdAgent *agent, gchar *local_addr, guint16 local_port);
gint IpcmdAgentTransportAddUdpv4Client (IpcmdAgent *agent, gchar *local_addr, guint16 local_port, gchar *remote_addr, guint16 remote_port);
/**
 * @fn IpcmdAgentTransportRemove
 * @brief Remove the transport from IP command protocol core
 * @param[in] agent:
 * @param[in] transport_id: transport id, which was returned in IpcmdAgentTransportAdd()
 */
void IpcmdAgentTransportRemove (IpcmdAgent *agent, gint transport_id);

gint IpcmdAgentTransportEnableBroadcast (IpcmdAgent *agent, gint transport_id, guint16 dest_port);

/**
 * @fn IpcmdAgentTransportAddUdpv4ManualChannel
 * @brief Force the transport to create specific channel. DONOT use this function if you not sure
 */
gint IpcmdAgentTransportAddUdpv4ManualChannel (IpcmdAgent *agent, gint transport_id, gchar *local_addr, guint16 local_port, gchar *remote_addr, guint16 remote_port);
/***********************************
 * IP COMMAND PROTOCOL CLIENT */
/***********************************
 * @fn IpcmdAgentClientOpen
 * @brief connect to remote IP command server
 * @param[in] agent:
 * @param[in] remote_host: a remote host
 * @return client id
 * @retval 0 on successfully connected
 * @retval -1 on not enough memory
 * @retval -2 on already connected 'service_id'
 */
gint IpcmdAgentClientOpen (IpcmdAgent *agent, const IpcmdHost *remote_host);
gint IpcmdAgentClientOpenUdpv4 (IpcmdAgent *agent, gchar *remote_addr, guint16 remote_port);

/**
 * @fn IpcmdAgentClientCloseService
 * @brief close connected remote service
 */
void IpcmdAgentClientClose (IpcmdAgent *agent, guint16 client_id);
//void IpcmdAgentClientCloseUdpv4 (IpcmdAgent *agent, gchar *remote_addr, guint16 remote_port);

/**
 * @fn IpcmdAgentClientInvokeOperation
 * @brief Create and send an IP command operation.
 * @param[in] agent:
 * @param[in] link_type:
 * @param[in] service_id:
 * @param[in] op_id:
 * @param[in] op_type:
 * @param[in] flags:
 * @param[in] cb:
 * @return unique handle for the operation
 */
OpHandle IpcmdAgentClientInvokeOperation (IpcmdAgent *agent, guint16 client_id, guint16 service_id, guint16 op_id, guint8 op_type, guint8 flags,
		const IpcmdOperationPayload *payload,
		const IpcmdOperationCallback *cb);

/**
 * @fn IpcmdAgentClientListenNoti
 * @brief Start to listen on notification group, which is identified by 'service_id','op_id' and 'is_cyclic'.
 * @param[in] agent:
 * @param[in] link_type:
 * @param[in] service_id: specific service id
 * @param[in] op_id: '0' means all operation id
 * @param[in] is_cyclic:
 * @param[in] cb: callback function, which is called when proper notification message is arrived.
 * @return
 * @retval 0 on success for listening
 * @retval -1 on wrong 'service_id'
 * @retval -2 when 'op_id' is already used
 */
gint IpcmdAgentClientListenNoti (IpcmdAgent *agent, guint16 client_id, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdOperationCallback *cb);
//gint IpcmdAgentClientListenNotiUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic, const IpcmdOperationCallback *cb);
/**
 * @fn IpcmdAgentClientIgnoreNoti
 * @brief Stop listening notification
 * @param[in] agent:
 * @param[in] link_type:
 * @param[in] service_id:
 * @param[in] op_id:
 * @param[in] is_cyclic:
 */
void IpcmdAgentClientIgnoreNoti (IpcmdAgent *agent, guint16 client_id, guint16 service_id, guint16 op_id, gboolean is_cyclic);
//void IpcmdAgentClientIgnoreNotiUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic);

/********************************
 *  IP COMMAND PROTOCOL SERVER *
 *******************************/
/* If ExecOperation() has been called, IpcmdAgentServerCompleteOperation() should be also called
 * after processing the operation.
 *
 * For example, when a server get new operation from client,
 *  1. IP command protocol calls ExecOperation() callback with 'OpHandle' and received message.
 *  2. In ExecOperation(), you may do something and return the function.
 *  3. After some time, server may call IpcmdAgentServerServiceCompleteOperation() with 'OpHandle' and result.
 */
typedef void (*ExecOperation) (OpHandle handle, const IpcmdOperationInfoReceivedMessage *result, gpointer cb_data);
typedef void (*DestroyExecData) (gpointer data);
/**
 * @fn IpcmdAgentServerOpenService
 * @brief Create new server service
 * @param[in] agent: IpcmdAgent
 * @param[in] service_id: service id. This should be unique in the agent.
 * @param[in] exec: This callback function will be called when any IP command messages with 'service_id' are arrived.
 */
gint IpcmdAgentServerOpenService (IpcmdAgent *agent, guint16 service_id, ExecOperation exec, gpointer exec_data, DestroyExecData destroy);
/**
 * @fn IpcmdAgentServerCloseService
 * @brief Destroy service
 * @param[in] agent: IpcmdAgent
 * @param[in] service_id: service id
 */
void IpcmdAgentServerCloseService (IpcmdAgent *agent, guint16 service_id);
/**
 * @fn IpcmdAgentServerCompleteOperation
 * @brief This function notifies that an application at server side finished to process the requested operation.
 * IP command protocol core will not free memory for the operation until this function is called.
 * @param[in] agent: IpcmdAgent
 * @param[in] handle: operation handle, which was passed in ExecOperation().
 * @param[in] result: should be IpcmdOperationInfoReplyMessage or IpcmdOperationInfoOk.
 * If the operation is REQUEST or SETREQUEST type, server should send back RESPONSE or ERROR message to client by using IpcmdOperationInfoReplyMessage.
 * In other cases(NOTIFICATION_REQUEST, SETREQUEST_NORETURN), use IpcmdOperationInfoOk.
 * @return
 * @retval 0 on success
 * @retval -1 if handle is not valid anymore
 */
gint IpcmdAgentServerCompleteOperation (IpcmdAgent *agent, OpHandle handle, const IpcmdOperationInfo *result);
/**
 * @fn IpcmdAgentServerAddNotiSubscriber
 * @brief Add a subscriber for specific notification group, which is identified by 'service_id','op_id' and 'is_cyclic'.
 * @param[in] agent: IpcmdAgent
 * @param[in] service_id: service id
 * @param[in] op_id: operation id
 * @param[in] is_cyclic: FALSE for NOTIFICATION, TRUE for NOTIFICATION_CYCLIC
 * @param[in] subscriber: a host to be added to notification group
 * @param[in] is_static_member: TRUE for static subscriber.
 * @return
 * @retval 0 on success
 * @retval -1 on not enough memory
 */
gint IpcmdAgentServerAddNotiSubscriber (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic,
		const IpcmdHost *subscriber, gboolean is_static_member);
gint IpcmdAgentServerAddNotiSubscriberUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic,
		gchar *subscriber_addr, guint16 subscriber_port, gboolean is_static_member);
/**
 * @fn IpcmdAgentServerInformNoti
 * @brief Send a NOTIFICATION/NOTIFICATION_CYCLIC message to subscribers in the notification group
 * @param[in] agent:
 * @param[in] service_id: service id
 * @param[in] op_id: operation id
 * @param[in] noti_info: should include notification payload.
 * @return
 * @retval 0 on success
 * @retval -1 on not enough memory
 * @retval -2 on wrong operation id
 * @retval -3 on wront 'noti_info'
 */
gint IpcmdAgentServerInformNoti (IpcmdAgent *agent, guint16 service_id, guint16 op_id, const IpcmdOperationInfoReplyMessage *noti_info);
/**
 * @fn IpcmdAgentServerRemoveNotiSubscriber
 * @brief Remove a subscriber from the notification group.
 * @param[in] agent: IpcmdAgent
 * @param[in] service_id: service id
 * @param[in] op_id: operation id
 * @param[in] is_cyclic: FALSE for NOTIFICATION, TRUE for NOTIFICATION_CYCLIC
 * @param[in] subscriber: a host to be removed from notification group
 */
void IpcmdAgentServerRemoveNotiSubscriber (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic,
		const IpcmdHost *subscriber);
void IpcmdAgentServerRemoveNotiSubscriberUdpv4 (IpcmdAgent *agent, guint16 service_id, guint16 op_id, gboolean is_cyclic,
		const gchar *subscriber_addr, guint16 subscriber_port);

G_END_DECLS

#endif /* INCLUDE_IPCMDAGENT_H_ */
