#ifndef _IpcomConnectionManager_h_
#define _IpcomConnectionManager_h_

#include <glib.h>

typedef struct _IpcomConnMgr IpcomConnMgr;

struct _IpcomConnMgr {
	GHashTable*	pID2Conn;
	GList*		pTransportList;
	GList*		pConnList;
	GQueue*		pIdleConnQueue;

	IpcomProtocol	*pIpcomProtocol;
	/*
	guint	nTransportIdleTimeout;
	guint	nConnectionIdleTimeout;
	*/
};

IpcomConnMgr*	IpcomConnMgrInit(IpcomProtocol*);
void			IpcomConnMgrDestroy(IpcomConnMgr*);

gint	IpcomConnMgrListenAt(IpcomConnMgr*, guint16 proto, GInetAddress* pLocalAddr, guint16 nLocalport);
void	IpcomConnMgrStopListening(IpcomConnMgr*, guint16 proto, GInetAddress* pLocalAddr, guint16 nLocalport);

IpcomConnection*	IpcomConnMgrConnectTo(IpcomConnMgr*, guint16 proto, GInetAddress* pRemoteAddr, guint16 nRemotePort);
void				IpcomConnMgrCloseConn(IpcomConnMgr*, IpcomConnection *);


#endif
