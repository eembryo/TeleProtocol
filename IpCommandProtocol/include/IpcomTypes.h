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

#ifndef __IpcomTypes_h__
#define __IpcomTypes_h__

#include <glib.h>
#include <IpcomEnums.h>

G_BEGIN_DECLS

#define IPCOM_PROTOCOL_VERSION 0x03

typedef struct _IpcomConnection 							IpcomConnection;
typedef struct _IpcomMessage 								IpcomMessage;
typedef struct _VCCPDUHeader 								VCCPDUHeader;
typedef struct _IpcomProtocol								IpcomProtocol;
typedef struct _IpcomOpContext								IpcomOpContext;
typedef struct _IpcomOpContextId							IpcomOpContextId;
typedef struct _IpcomService 								IpcomService;
typedef struct _IpcomTransport 								IpcomTransport;
typedef struct _IpcomTransportUDPv4 						IpcomTransportUDPv4;

/* IpcomConnection.h */

/* IpcomMessage.h */

/* IpcomProtocol.h */

/* IpcomService.h */
/**
 * IpcomReceiveMessageCallback:
 * @opContextId: Operation Context is used to keep track of specific operation in IpcomProtocol. Each Operation Context has unique ID,opContextId.
 * @mesg: is a message came from peer.
 * @userdata: data which was registered with this callback function.
 *
 * IpcomProtocol will call this callback function
 */
typedef IpcomServiceReturn 		(*IpcomReceiveMessageCallback)(const IpcomOpContextId *opContextId, IpcomMessage *mesg, gpointer userdata);
typedef gboolean				(*IpcomServiceOnRecvNewConn)(IpcomService *service, IpcomConnection *conn);
typedef IpcomServiceReturn		(*IpcomServiceProcessMessage)(IpcomService *service, const IpcomOpContextId *ctxId, IpcomMessage *mesg);

/* IpcomTransport.h */

/* IpcomOperationContext.h */

G_END_DECLS

#endif
