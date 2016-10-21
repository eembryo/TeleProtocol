/*
 * IpcomSimpleAgent.h
 *
 *  Created on: Oct 17, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCOMSIMPLEAGENT_H_
#define INCLUDE_IPCOMSIMPLEAGENT_H_


#ifndef __IpcomSimpleAgent_h_
#define __IpcomSimpleAgent_h_

#include <glib.h>
#include "IpcmdDeclare.h"
#include "IpcmdOperation.h"
#include "IpcmdHost.h"

G_BEGIN_DECLS

/**************
 * IpcomAgent
 **************/
typedef OpHandle					IpcomOpHandle;
typedef IpcmdMessage				IpcomMessage;
typedef gint						IpcomConnectionHandle;
typedef gpointer					IpcomAgentSocket;
typedef struct _IpcomAgent			IpcomAgent;
typedef struct _AgentMsgHandlers	IpcomAgentMsgHandlers;
typedef struct _AgentOpCallbacks	IpcomAgentOpCallbacks;
typedef gint						IpcomServiceReturn;

typedef enum {
	IPCOM_TRANSPORT_UNKNOWN         = 0,
	IPCOM_TRANSPORT_UDPV4,
} IpcomTransportType;

struct _AgentOpCallbacks {
	void					(*OnOpResponse)(IpcomAgent*,IpcomOpHandle,IpcomMessage*,gpointer userdata);
	void					(*OnOpDestroyed)(IpcomAgent*,IpcomOpHandle,IpcomOpContextFinCode,gpointer userdata);
	gpointer				userdata;
};
struct _AgentMsgHandlers {
	IpcomServiceReturn 		(*OnReceiveMesg)(IpcomAgent*,IpcomOpHandle,IpcomMessage*,gpointer userdata);
	IpcomServiceReturn 		(*OnReceiveNoti)(IpcomAgent*,IpcomMessage*,gpointer userdata);
	gpointer				userdata;
};

struct _IpcomAgent {
	IpcomAgentSocket	(*Bind)							(IpcomAgent*,IpcomTransportType,const gchar *src,guint16 sport,GError**);
	gboolean			(*ConnectTo)					(IpcomAgent*,IpcomAgentSocket,const gchar *dst,guint16 dport,GError**);
	gboolean			(*ListenAt)						(IpcomAgent*,IpcomAgentSocket,GError**);
	gboolean			(*Broadcast)					(IpcomAgent*,IpcomAgentSocket,IpcomMessage*,GError**);
	IpcomAgentSocket	(*GetAcceptedConnection)		(IpcomAgent*,IpcomAgentSocket,const gchar *dst,guint16 dport,GError**);
	void				(*CloseAgentSocket)				(IpcomAgent*,IpcomAgentSocket);

	IpcomOpHandle		(*OperationSend)				(IpcomAgent*,IpcomAgentSocket,IpcomMessage*,const IpcomAgentOpCallbacks*,GError**);
	gboolean			(*OperationRespond)				(IpcomAgent*,IpcomOpHandle,IpcomMessage*,const IpcomAgentOpCallbacks*,GError**);
	/**
	 * Generate error message and respond to operation requester with it
	 *
	 * IpcomOpHandle: indicate the operation, to which error message should be sent.
	 * ecode: indicate specific errors of the operation. It should be range from 0x20 to 0x3f
	 * einfo: auxiliary information for the ecode
	 * GError: error code is described in "IpcomErrorCode"
	 */
	gboolean			(*OperationRespondError)		(IpcomAgent*,IpcomOpHandle,guint8 ecode,guint16 einfo,GError**);
	void				(*OperationCancel)				(IpcomAgent*,IpcomOpHandle);

	gboolean			(*RegisterMessageHandlers)		(IpcomAgent*,guint16 serviceId,const IpcomAgentMsgHandlers*, GError**);
	void				(*UnRegisterMessageHandlers)	(IpcomAgent*,guint16 serviceId);
};

/*****************
 * IpcomSimpleAgent
 *****************/
typedef struct _IpcomSimpleAgent	IpcomSimpleAgent;
typedef enum {
	NOT_INIT_MODE = 0,
	BINDING_MODE,
	CONNECTED_MODE,
	LISTEN_MODE
} IpcomSimpleAgentMode;

IpcomAgent*			IpcomSimpleAgentCreate(GMainContext* pMainContext);
void				IpcomSimpleAgentDestroy(IpcomAgent*);

G_END_DECLS

#endif

#endif /* INCLUDE_IPCOMSIMPLEAGENT_H_ */
