#ifndef __IpcomSimpleAgent_h_
#define __IpcomSimpleAgent_h_

#include <glib.h>
#include <IpcomEnums.h>
#include <IpcomTypes.h>

G_BEGIN_DECLS

/**************
 * IpcomAgent
 **************/
typedef IpcomOpContextId*			IpcomOpHandle;
typedef IpcomConnection*			IpcomConnectionHandle;
typedef gpointer					IpcomAgentSocket;
typedef struct _IpcomAgent			IpcomAgent;
typedef struct _AgentMsgHandlers	IpcomAgentMsgHandlers;
typedef struct _AgentOpCallbacks	IpcomAgentOpCallbacks;
struct _AgentOpCallbacks {
	void             		(*OnOpResponse)(IpcomAgent*,IpcomOpHandle,IpcomMessage*,gpointer userdata);
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
	gboolean			(*OperationRespond)				(IpcomAgent*,IpcomOpHandle,   IpcomMessage*,const IpcomAgentOpCallbacks*,GError**);
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

struct _IpcomSimpleAgent {
	IpcomAgent				_agent;
	IpcomSimpleAgentMode 	mode;		//0 - NOT initialized, 1 - connect mode, 2 - listen mode
	IpcomConnection*		pConnection;
	IpcomConnection*		pBroadConnection;
	IpcomTransport*			pTransport;
	IpcomProtocol*			pProtocol;
	GMainContext*			pGlibContext;
	GHashTable*				hashSocketTable;
};

IpcomAgent*			IpcomSimpleAgentCreate(GMainContext* pMainContext);
void				IpcomSimpleAgentDestroy(IpcomAgent*);

G_END_DECLS

#endif
