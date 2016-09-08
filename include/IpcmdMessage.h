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
// date   : 08/31/2016
// Desc   :
//
// LICENSE@@@

#ifndef INCLUDE_IPCMDMESSAGE_H_
#define INCLUDE_IPCMDMESSAGE_H_

#include <glib.h>
#include <gio/gio.h>
#include "IpcmdDeclare.h"
#include "reference.h"

G_BEGIN_DECLS

#define VCCPDUHEADER_SIZE				16
#define VCCPDUHEADER_LENGTH_CORRECTION	8
#define IPCMD_MESSAGE_MIN_SIZE 			VCCPDUHEADER_SIZE
#define IPCMD_MESSAGE_MAX_SIZE 			1472	//MTU - IP_HEADER_SIZE(20) - UDP_HEADER_SIZE(8)
#define IPCMD_ERROR_MESSAGE_SIZE		VCCPDUHEADER_SIZE + 3
#define IPCMD_ACK_MESSAGE_SIZE			VCCPDUHEADER_SIZE
#define IPCMD_MESSAGE_FLAGS_PROC		0x80

#define BUILD_SENDERHANDLEID(service, operation, optype, seqNR) (guint32)(((service&0xFF)<<24) + ((operation&0xFF)<<16) + ((optype&0xFF)<<8) + (guint8)seqNR)

typedef struct _VCCPDUHeader VCCPDUHeader;
struct _VCCPDUHeader {
	guint16		serviceID;
	guint16		operationID;
	guint32		length;
	guint32 	senderHandleId;
	guint8		proto_version;
	guint8		opType;
	guint8		dataType;
	guint8		reserved;
} __attribute__ ((packed));

struct _ErrorPayload {
	guint8		errorCode;
	guint16		errorInformation;
} __attribute__ ((packed));

struct _IpcmdMessage {
	struct ref				_ref;
	struct _VCCPDUHeader	*vccpdu_ptr;
	gpointer				payload_ptr;
	guint32					actual_size;	//the amount of allocated memory for the following raw message. It does not include the size of 'struct _IpcmdMessage'
    GSocketAddress*         origin_addr;    //originator of this message
	guint32					mesg_length;
	gchar					message[0];		//the start of raw message.
};

enum IPCMD_OPTYPE {
	IPCMD_OPTYPE_REQUEST 				= 0x00,
	IPCMD_OPTYPE_SETREQUEST_NORETURN	= 0x01,
	IPCMD_OPTYPE_SETREQUEST				= 0x02,
	IPCMD_OPTYPE_NOTIFICATION_REQUEST	= 0x03,
	IPCMD_OPTYPE_RESPONSE				= 0x04,
	IPCMD_OPTYPE_NOTIFICATION			= 0x05,
	IPCMD_OPTYPE_NOTIFICATION_CYCLIC	= 0x06,
	IPCMD_OPTYPE_ACK 					= 0x70,
	IPCMD_OPTYPE_ERROR 					= 0xE0,
};

// if size equals to zero, maximum message size is allocated.
IpcmdMessage *IpcmdMessageNew(guint16 maxSize);
static inline gchar*		IpcmdMessageGetRawData(IpcmdMessage *mesg)
{ return mesg->message; }
static inline VCCPDUHeader *IpcmdMessageGetVCCPDUHeader(IpcmdMessage *mesg)
{ return mesg->vccpdu_ptr; }
static inline guint16	IpcmdMessageGetVCCPDUServiceID(IpcmdMessage *mesg)
{ return g_ntohs(mesg->vccpdu_ptr->serviceID);}
static inline guint16 	IpcmdMessageGetVCCPDUOperationID(IpcmdMessage *mesg)
{return g_ntohs(mesg->vccpdu_ptr->operationID);}
static inline guint 	IpcmdMessageGetVCCPDULength(IpcmdMessage *mesg)
{	return g_ntohl(mesg->vccpdu_ptr->length);}
static inline void 		IpcmdMessageSetVCCPDULength(IpcmdMessage *mesg, guint32 length)
{	mesg->vccpdu_ptr->length = g_htonl(length);}
static inline guint32 	IpcmdMessageGetVCCPDUSenderHandleID(IpcmdMessage *mesg)
{	return g_ntohl(mesg->vccpdu_ptr->senderHandleId);}
static inline guint8 	IpcmdMessageGetVCCPDUProtoVersion(IpcmdMessage *mesg)
{	return mesg->vccpdu_ptr->proto_version;}
static inline guint8 	IpcmdMessageGetVCCPDUOpType(IpcmdMessage *mesg)
{	return mesg->vccpdu_ptr->opType;}
static inline guint8 	IpcmdMessageGetVCCPDUDataType(IpcmdMessage *mesg)
{	return mesg->vccpdu_ptr->dataType;}
static inline guint8 	IpcmdMessageGetVCCPDUReserved(IpcmdMessage *mesg)
{	return mesg->vccpdu_ptr->reserved;}
static inline void		IpcmdMessageSetLength(IpcmdMessage *mesg, guint length)
{	mesg->mesg_length = length; }
static inline guint32	IpcmdMessageGetLength(IpcmdMessage *mesg)
{	return mesg->mesg_length; }
static inline guint 	IpcmdMessageGetPaylodLength(IpcmdMessage *mesg)
{	return IpcmdMessageGetVCCPDULength(mesg) - 8;}
static inline void 		IpcmdMessageSetPayloadLength(IpcmdMessage *mesg, guint32 length)
{	IpcmdMessageSetVCCPDULength(mesg, length+8); IpcmdMessageSetLength(mesg, length + VCCPDUHEADER_SIZE);}
static inline gpointer 	IpcmdMessageGetPayload(IpcmdMessage *mesg)
{	return mesg->payload_ptr;}
static inline void 		IpcmdMessageSetPayloadBuffer(IpcmdMessage *mesg, gpointer payload, guint32 length) {
	mesg->payload_ptr = payload;
	IpcmdMessageSetPayloadLength(mesg, length);
}

/**
 * - On receiving IP command message, IpcomTransport sets originator of the packet into IpcmdMessage.
 */
void        IpcmdMessageSetOriginSockAddress(IpcmdMessage *mesg, GSocketAddress *paddr);
gboolean    IpcmdMessageCopyOriginInetAddressString(IpcmdMessage *mesg, gpointer buf, gsize buf_len);
guint16     IpcmdMessageGetOriginInetPort(IpcmdMessage *mesg);

gboolean IpcmdMessageCopyToPayloadBuffer(IpcmdMessage *mesg, gpointer src, guint32 length);

static inline void IpcmdMessageInitVCCPDUHeader(IpcmdMessage *mesg,
		guint16 serviceId, guint16 operationId,
		guint32	senderhandleid,
		guint8 version, guint8 opType, guint8 dataType, guint8 flags) {
	mesg->vccpdu_ptr->serviceID = g_htons(serviceId);
	mesg->vccpdu_ptr->operationID = g_htons(operationId);
	mesg->vccpdu_ptr->length = g_htonl(8);
	mesg->vccpdu_ptr->senderHandleId = g_htonl(senderhandleid);
	mesg->vccpdu_ptr->proto_version = version;
	mesg->vccpdu_ptr->opType = opType;
	mesg->vccpdu_ptr->dataType = dataType;
	mesg->vccpdu_ptr->reserved = flags;
}

static inline guint IpcmdMessageGetPacketSize(IpcmdMessage *mesg) {
	return IpcmdMessageGetVCCPDULength(mesg) + 8;
}

static inline gboolean IpcmdMessageIsNotification(IpcmdMessage *mesg) {
	switch(IpcmdMessageGetVCCPDUOpType(mesg)) {
	case IPCMD_OPTYPE_NOTIFICATION:
	case IPCMD_OPTYPE_NOTIFICATION_CYCLIC:
		return TRUE;
	default:
		return FALSE;
	}
}

IpcmdMessage*	IpcmdMessageRef(IpcmdMessage *message);
void 			IpcmdMessageUnref(IpcmdMessage *message);

G_END_DECLS

#endif /* INCLUDE_IPCMDMESSAGE_H_ */
