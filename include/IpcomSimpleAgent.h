#ifndef __IpcomSimpleAgent_h_
#define __IpcomSimpleAgent_h_

#include <glib.h>
#include <IpcomTypes.h>

typedef struct _IpcomSimpleAgent IpcomSimpleAgent;
typedef struct _IpcomAgent IpcomAgent;

typedef const IpcomOpContextId*						IpcomOpHandle;
typedef const IpcomConnection*						IpcomConnectionHandle;
typedef const gpointer	IpcomAgentSocket;

typedef IpcomServiceReturn 		(*IpcomAgentOnRecvResponse)(IpcomAgent*,IpcomOpHandle,IpcomMessage*,gpointer userdata);
typedef IpcomServiceReturn 		(*IpcomAgentOnOpCtxDestroy)(IpcomAgent*,IpcomOpContextId *opContextId, gint code, gpointer userdata);

struct _IpcomAgent {
	IpcomAgentSocket	(*ConnectTo)			(IpcomAgent*,IpcomTransportType, const gchar *dst, guint16 dport, GError **gerror);
	IpcomAgentSocket	(*ListenAt)				(IpcomAgent*,IpcomTransportType, const gchar *src, guint16 sport, GError **gerror);
	void				(*CloseAgentSocket)		(IpcomAgent*,IpcomAgentSocket asock);
	IpcomOpHandle		(*SendMessage)			(IpcomAgent*,IpcomMessage*,IpcomReceiveMessageCallback, IpcomOpCtxDestroyNotify, gpointer userdata, GError **gerror);
	gint				(*RespondMessage)		(IpcomAgent*,IpcomOpHandle, IpcomMessage*,IpcomOpCtxDestroyNotify);
	void				(*CancelOperation)		(IpcomAgent*,IpcomOpHandle);
	gboolean			(*RegisterMessageHandler)(IpcomAgent*,guint16 serviceId,IMsgHandler msgHandler, GError **gerror);
};

struct _IpcomSimpleAgent {
	guint8			mode;		//0 - NOT initialized, 1 - connect mode, 2 - listen mode
	IpcomConnection*	conn;
	IpcomTransport*	pTransport;
	IpcomProtocol*	pProtocol;
	GMainContext*	pGlibContext;
};

typedef struct _IMsgHandler IMsgHandler;

struct _IMsgHandler {
	IpcomServiceProcessMessage	OnReceiveMesg;
	IpcomServiceProcessNoti 	OnReceiveNoti;
};

IpcomSimpleAgent*	IpcomSimpleAgentCreate();
void				IpcomSimpleAgentDestroy(IpcomSimpleAgent*);
gboolean			IpcomSimpleAgentConnectTo(IpcomSimpleAgent* agent, IpcomTransportType proto, const gchar *dst, guint16 dport, GError **gerror);
gboolean			IpcomSimpleAgentListenAt(IpcomSimpleAgent* agent,IpcomTransportType proto, const gchar *src, guint16 sport, GError **gerror);
gboolean			IpcomSimpleAgentRegisterMessageHandler(IpcomSimpleAgent* agent, guint16 serviceId, IMsgHandler msgHandler, GError **gerror);
IpcomOpHandle		IpcomSimpleAgentSendMessage(IpcomSimpleAgent*, IpcomMessage*, IpcomReceiveMessageCallback, IpcomOpCtxDestroyNotify, gpointer, GError **gerror);
gint				IpcomSimpleAgentBroadcast(IpcomSimpleAgent*, IpcomMessage*, GError **gerror);
gint				IpcomSimpleAgentRespondMessage(IpcomSimpleAgent*, IpcomOpHandle, IpcomMessage*, IpcomOpCtxDestroyNotify, gpointer, GError **gerror);

#endif
