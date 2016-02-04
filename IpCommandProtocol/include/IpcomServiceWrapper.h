// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#ifndef __IpcomServiceWrapper_h__
#define __IpcomServiceWrapper_h__

#include <glib.h>
#include <IpcomTypes.h>
#include <IpcomEnums.h>
#include <IpcomService.h>

G_BEGIN_DECLS

typedef gint (*IpcomServiceWrapperProcessMessage)(IpcomServiceWrapper *wrapper, IpcomMessage *mesg, const IpcomOpContextId *ctxId, gpointer cb_data);

struct _IpcomServiceWrapper {
	struct _IpcomService	_service;
	gchar					*localAddr;
	guint16					localPort;
	gchar					*peerAddr;
	guint16					peerPort;
	GMainContext			*glibContext;
	IpcomConnection			*connected;
	GList					*acceptedConns;
	guint8					seqNum;
	IpcomServiceWrapperStatus	status;
	IpcomServiceWrapperProcessMessage	process_cb;
	gpointer							cb_data;
};

/**
 * IpcomServiceWrapperNew:
 * @mode: IPCOM_SERVICE_SERVER or IPCOM_SERVICE_CLIENT
 * @serviceId:
 * @transType: type of transport. For UDP over IPv4, use IPCOM_TRANSPORT_UDPV4
 * @address: dot-decimal notation of IP address.
 * 				if mode is IPCOM_SERVICE_SERVER, this is local address to bind.
 * 			 	if not, this is peer address to connect.
 * @port: if mode is IPCOM_SERVICE_SERVER, this is loca port.
 * 			if not, this is peer port to connect
 *
 * Allocate memory and setup for IpcomServiceWrapper
 */
IpcomServiceWrapper *IpcomServiceWrapperNew(IpcomServiceMode mode, IpcomServiceId serviceId, IpcomTransportType transType, gchar *address, guint port);

/**
 * IpcomServiceWrapperSetLocalAddress:
 * @address: dot-decimal notation of local address. eg. "127.0.0.1"
 * @port:
 *
 * If mode of wrapper is IPCOM_SERVICE_SERVER, you do not have to use this: local address and port is already set by IpcomServiceWrapperNew().
 * If mode is IPCOM_SERVICE_CLIENT, you may use this to explicitly set local address and port to connect to peer.
 */
void IpcomServiceWrapperSetLocalAddress(IpcomServiceWrapper *wrapper, gchar *address, guint port);
/**
 * IpcomServiceWrapperStart:
 * @wrapper:
 * @context:
 *
 * Attach sockets, which is involved in wrapper, to context to send and receive packets
 */
gint IpcomServiceWrapperStart(IpcomServiceWrapper *wrapper, GMainContext *context);
/**
 * IpcomServiceWrapperSend:
 * @wrapper:
 * @mesg:
 * @cb: callback function. It is called when there is packets to receive
 * @userdata: userdata is passed to callback function
 *
 * low-level send function.
 */
gint IpcomServiceWrapperSend(IpcomServiceWrapper *wrapper, IpcomMessage *mesg, IpcomReceiveMessageCallback cb, gpointer userdata);
/**
 * IpcomServiceWrapperSendRequest:
 * @wrapper: IpcomServiceWrapper
 * @operationId: Operation ID, which is used in VCC PDU header
 * @opType: Operation Type, which is used in VCC PDU header. This should be one of REQUEST, SETREQUEST_NORETURN, SETREQUEST and NOTIFICATION_REQUEST
 * @dataType:
 * @reserved:
 * @payload: pointer of payload. This pointer will be freed when IpcomMessage is destroyed.
 * @length: payload length
 */
gint IpcomServiceWrapperSendRequest(IpcomServiceWrapper *wrapper,
		guint16 operationId, guint8 opType, guint8 dataType, guint8 reserved, gpointer payload, guint32 length,
		IpcomReceiveMessageCallback cb, gpointer userdata);

gint IpcomServiceWrapperRespondMessage(IpcomServiceWrapper *wrapper, const IpcomOpContextId *opCtxId, IpcomMessage *mesg);

IpcomMessage *IpcomServiceWrapperCreateMessage(IpcomServiceWrapper *wrapper,
		guint16 operationId, guint32 senderHandleId, guint8 opType, guint8 dataType, guint8 reserved, gpointer payload, guint32 payloadLen);

static inline IpcomService *IpcomServiceWrapperGetService(IpcomServiceWrapper *wrapper)
{ return &wrapper->_service; }
static inline void IpcomServiceWrapperSetProcessMessage(IpcomServiceWrapper *wrapper, IpcomServiceWrapperProcessMessage cb, gpointer cb_data)
{ wrapper->process_cb = cb; wrapper->cb_data = cb_data; }

G_END_DECLS

#endif
