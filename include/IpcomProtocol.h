#ifndef __IpcomProtocolh__
#define __IpcomProtocolh__

#include <glib.h>
#include <IpcomTypes.h>

typedef enum {
	IPPROTO_OPTYPE_REQUEST 			= 0x00,
	IPPROTO_OPTYPE_SETREQUEST_NORETURN	= 0x01,
	IPPROTO_OPTYPE_SETREQUEST			= 0x02,
	IPPROTO_OPTYPE_NOTIFICATION_REQUEST	= 0x03,
	IPPROTO_OPTYPE_RESPONSE			= 0x04,
	IPPROTO_OPTYPE_NOTIFICATION		= 0x05,
	IPPROTO_OPTYPE_NOTIFICATION_CYCLIC	= 0x06,
	IPPROTO_OPTYPE_ACK					= 0x70,
	IPPROTO_OPTYPE_ERROR				= 0xe0,
} IpcomProtocolOpType;

typedef gint (*IpcomReceiveMessageCallback)(void *opContextId, IpcomMessage *mesg, void *userdata);

IpcomProtocol	*IpcomProtocolGetInstance();
gboolean	IpcomProtocolRegisterService(IpcomProtocol *, IpcomService *);
//gboolean	IpcomProtocolUnregisterService(IpcomProtocol *, guint serviceId);
gint		IpcomProtocolSendMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg, IpcomReceiveMessageCallback recv_cb, void *userdata);
//gint		IpcomProtocolRepondMessage(IpcomProtocol *, void *opContextId, IpcomMessage *mesg);
gint		IpcomProtocolHandleMessage(IpcomProtocol *, IpcomConnection *conn, IpcomMessage *mesg);

IpcomProtocolOpContext *IpcomProtocolOpContextNew(IpcomConnection *conn, guint32 senderHandleId, guint8 opType, guint16 serviceId);
#endif
