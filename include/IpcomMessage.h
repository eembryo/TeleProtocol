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

#ifndef __IpcomMessage_h__
#define __IpcomMessage_h__


#include <glib.h>
#include <gio/gio.h>
#include <ref_count.h>
#include <IpcomTypes.h>

G_BEGIN_DECLS

#define VCCPDUHEADER_SIZE				16
#define VCCPDUHEADER_LENGTH_CORRECTION	8
#define IPCOM_MESSAGE_MIN_SIZE 			VCCPDUHEADER_SIZE
#define IPCOM_MESSAGE_MAX_SIZE 			1472	//MTU - IP_HEADER_SIZE(20) - UDP_HEADER_SIZE(8)
#define BUILD_SENDERHANDLEID(service, operation, optype, seqNR) (guint32)(((service&0xFF)<<24) + ((operation&0xFF)<<16) + ((optype&0xFF)<<8) + (guint8)seqNR)
#define IPCOM_ERROR_MESSAGE_SIZE	VCCPDUHEADER_SIZE + 3
#define IPCOM_ACK_MESSAGE_SIZE		VCCPDUHEADER_SIZE

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

struct _IpcomMessage {
	struct ref				_ref;
	struct _VCCPDUHeader	*vccpdu_ptr;
	gpointer				payload_ptr;
	guint32					actual_size;	//the size of allocated memory for this message. It does not include the size of 'struct _IpcomMessage'
    GSocketAddress*         origin_addr;    //originator of this message
	guint32					mesg_length;
	gchar					message[0];		//the start of raw message.
};

// if size equals to zero, maximum message size is allocated.
IpcomMessage *IpcomMessageNew(guint16 maxSize);
static inline gchar*		IpcomMessageGetRawData(IpcomMessage *mesg)
{ return mesg->message; }
static inline VCCPDUHeader *IpcomMessageGetVCCPDUHeader(IpcomMessage *mesg)
{ return mesg->vccpdu_ptr; }
static inline guint16	IpcomMessageGetVCCPDUServiceID(IpcomMessage *mesg)
{ return g_ntohs(mesg->vccpdu_ptr->serviceID);}
static inline guint16 	IpcomMessageGetVCCPDUOperationID(IpcomMessage *mesg)
{return g_ntohs(mesg->vccpdu_ptr->operationID);}
static inline guint 	IpcomMessageGetVCCPDULength(IpcomMessage *mesg)
{	return g_ntohl(mesg->vccpdu_ptr->length);}
static inline void 		IpcomMessageSetVCCPDULength(IpcomMessage *mesg, guint32 length)
{	mesg->vccpdu_ptr->length = g_htonl(length);}
static inline guint32 	IpcomMessageGetVCCPDUSenderHandleID(IpcomMessage *mesg)
{	return g_ntohl(mesg->vccpdu_ptr->senderHandleId);}
static inline guint8 	IpcomMessageGetVCCPDUProtoVersion(IpcomMessage *mesg)
{	return mesg->vccpdu_ptr->proto_version;}
static inline guint8 	IpcomMessageGetVCCPDUOpType(IpcomMessage *mesg)
{	return mesg->vccpdu_ptr->opType;}
static inline guint8 	IpcomMessageGetVCCPDUDataType(IpcomMessage *mesg)
{	return mesg->vccpdu_ptr->dataType;}
static inline guint8 	IpcomMessageGetVCCPDUReserved(IpcomMessage *mesg)
{	return mesg->vccpdu_ptr->reserved;}
static inline void		IpcomMessageSetLength(IpcomMessage *mesg, guint length)
{	mesg->mesg_length = length; }
static inline guint32	IpcomMessageGetLength(IpcomMessage *mesg)
{	return mesg->mesg_length; }
static inline guint 	IpcomMessageGetPaylodLength(IpcomMessage *mesg)
{	return IpcomMessageGetVCCPDULength(mesg) - 8;}
static inline void 		IpcomMessageSetPayloadLength(IpcomMessage *mesg, guint32 length)
{	IpcomMessageSetVCCPDULength(mesg, length+8); IpcomMessageSetLength(mesg, length + VCCPDUHEADER_SIZE);}
static inline gpointer 	IpcomMessageGetPayload(IpcomMessage *mesg)
{	return mesg->payload_ptr;}
static inline void 		IpcomMessageSetPayloadBuffer(IpcomMessage *mesg, gpointer payload, guint32 length) {
	mesg->payload_ptr = payload;
	IpcomMessageSetPayloadLength(mesg, length);
}

/**
 * - On receiving IP command message, IpcomTransport sets originator of the packet into IpcomMessage.
 */
void        IpcomMessageSetOriginSockAddress(IpcomMessage *mesg, GSocketAddress *paddr);
//gchar*      IpcomMessageGetOriginInetAddressString(IpcomMessage *mesg);
gboolean    IpcomMessageCopyOriginInetAddressString(IpcomMessage *mesg, gpointer buf, gsize buf_len);
guint16     IpcomMessageGetOriginInetPort(IpcomMessage *mesg);

gboolean IpcomMessageCopyToPayloadBuffer(IpcomMessage *mesg, gpointer src, guint32 length);

static inline void IpcomMessageInitVCCPDUHeader(IpcomMessage *mesg,
		guint16 serviceId, guint16 operationId,
		guint32	senderhandleid,
		guint8 version, guint8 opType, guint8 dataType, guint8 reserved) {
	mesg->vccpdu_ptr->serviceID = g_htons(serviceId);
	mesg->vccpdu_ptr->operationID = g_htons(operationId);
	mesg->vccpdu_ptr->length = g_htonl(8);
	mesg->vccpdu_ptr->senderHandleId = g_htonl(senderhandleid);
	mesg->vccpdu_ptr->proto_version = version;
	mesg->vccpdu_ptr->opType = opType;
	mesg->vccpdu_ptr->dataType = dataType;
	mesg->vccpdu_ptr->reserved = reserved;
}

static inline guint IpcomMessageGetPacketSize(IpcomMessage *mesg) {
	return IpcomMessageGetVCCPDULength(mesg) + 8;
}

static inline gboolean IpcomMessageIsRequestMessage(IpcomMessage *mesg) {
	switch(IpcomMessageGetVCCPDUOpType(mesg)) {
	case IPCOM_OPTYPE_REQUEST:
	case IPCOM_OPTYPE_SETREQUEST_NORETURN:
	case IPCOM_OPTYPE_SETREQUEST:
	case IPCOM_OPTYPE_NOTIFICATION_REQUEST:
		return TRUE;
	default:
		return FALSE;
	}
}

IpcomMessage *IpcomMessageRef(IpcomMessage *message);
void IpcomMessageUnref(IpcomMessage *message);

G_END_DECLS

#endif
