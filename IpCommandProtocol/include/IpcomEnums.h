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

#ifndef _IpcomEnums_h_
#define _IpcomEnums_h_

G_BEGIN_DECLS

typedef enum {
	IPCOM_SERVICEID_TELEMATICS				= 0xA1,
	IPCOM_SERVICEID_PHONE					= 0xA2,
	IPCOM_SERVICEID_CONNECTIVITY			= 0xA3,
	IPCOM_SERVICEID_COMMON_PHONE_TELEMATICS	= 0xA7,
	IPCOM_SERVICEID_COMMON_ALL				= 0xA8,
	IPCOM_SERVICEID_POSITIONING				= 0xA9,
	IPCOM_SERVICEID_DIAGNOSTIC_MANAGEMENT	= 0xAA
} IpcomServiceId;

typedef enum {
	IPCOM_OPTYPE_REQUEST 				= 0x00,
	IPCOM_OPTYPE_SETREQUEST_NORETURN	= 0x01,
	IPCOM_OPTYPE_SETREQUEST				= 0x02,
	IPCOM_OPTYPE_NOTIFICATION_REQUEST	= 0x03,
	IPCOM_OPTYPE_RESPONSE				= 0x04,
	IPCOM_OPTYPE_NOTIFICATION			= 0x05,
	IPCOM_OPTYPE_NOTIFICATION_CYCLIC	= 0x06,
	IPCOM_OPTYPE_ACK					= 0x70,
	IPCOM_OPTYPE_ERROR					= 0xe0,
} IpcomOpType;

typedef enum {
	IPCOM_SERVICE_SUCCESS		= 0x00,
	IPCOM_SERVICE_PENDING		= 0x10,		//If application returns IPCOM_SERVICE_PENDING in callback function, it should call IpcomProtocolProcessDone() to notify a result for post processing.
	IPCOM_SERVICE_ERR_START		= 0xA0,
	IPCOM_SERVICE_ERR_XXX,
	IPCOM_SERVICE_ERR_END		= 0xFF,
} IpcomServiceReturn;

typedef enum {
	IPCOM_TRANSPORT_UNKNOWN		= 0,
	IPCOM_TRANSPORT_UDPV4,
} IpcomTransportType;

typedef enum {
	IPCOM_SERVICE_STATUS_NONE	= 0,
	IPCOM_SERVICE_STATUS_RUNNING,
	IPCOM_SERVICE_STATUS_STOPPING,
} IpcomServiceWrapperStatus;

typedef enum {
	IPCOM_ECODE_NOT_OK						= 0x00,
	IPCOM_ECODE_SERVICEID_NOT_AVAILABLE,
	IPCOM_ECODE_OPERATIONID_NOT_AVAILABLE,
	IPCOM_ECODE_OPERATIONTYPE_NOT_AVAILABLE,
	IPCOM_ECODE_INVALID_PROTOCOL_VERSION,
	IPCOM_ECODE_PROCESSING,
	IPCOM_ECODE_INVALID_LENGTH,
	IPCOM_ECODE_APPLICATION_ERROR,	//application not ready
	IPCOM_ECODE_TIMEOUT,
	IPCOM_ECODE_BUSY,
} IpcomGeneralErrorCodes;


typedef enum {
	OPCONTEXT_STATUS_NONE = 0,
	OPCONTEXT_STATUS_REQUEST_SENT,
	OPCONTEXT_STATUS_ACK_RECV,
	OPCONTEXT_STATUS_PROCESS_RESPONSE,
	OPCONTEXT_STATUS_PROCESS_REQUEST,
	OPCONTEXT_STATUS_RESPONSE_SENT,
	OPCONTEXT_STATUS_NOTIFICATION_SENT,
	OPCONTEXT_STATUS_FINALIZE,		//will be destroyed
} IpcomProtocolOpContextStatus;

typedef enum {
	OPCONTEXT_TRIGGER_SEND_REQUEST = 1,
	OPCONTEXT_TRIGGER_SEND_RESPONSE,
	OPCONTEXT_TRIGGER_RECV_REQUEST,
	OPCONTEXT_TRIGGER_RECV_RESPONSE,
	OPCONTEXT_TRIGGER_RECV_ACK,
	OPCONTEXT_TRIGGER_EXPIRE_WFA,
	OPCONTEXT_TRIGGER_EXPIRE_WFR,
	OPCONTEXT_TRIGGER_PROCESS_DONE,
	OPCONTEXT_TRIGGER_SEND_NOTIFICATION,
	OPCONTEXT_TRIGGER_FINALIZE
} IpcomProtocolOpContextTriggers;

/// configurable parameters
extern gint defaultTimeoutWFA;
extern gint defaultTimeoutWFR;
extern gint incraseTimerValueWFA;
extern gint incraseTimerValueWFR;
extern gint numberOfRetriesWFA;
extern gint numberOfRetriesWFR;

G_END_DECLS

#endif
