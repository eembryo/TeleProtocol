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

#include <glib.h>
#include <IpcomService.h>
#include <IpcomEnums.h>
#include <IpcomProtocol.h>
#include <dprint.h>
#include <IpcomMessage.h>


static IpcomServiceReturn
_DefaultProcessMessage(IpcomService *service, const IpcomOpContextId *ctxId, IpcomMessage *mesg)
{
	DWARN("You cannot process REQUEST message before setting ProcessMessage callback.\n");

	IpcomMessageUnref(mesg);

	return  IPCOM_SERVICE_SUCCESS;
}


IpcomService *
IpcomServiceNew(guint16 serviceId, guint priv_size)
{
	IpcomService *service;

	service = g_malloc0(sizeof(struct _IpcomService) + priv_size);
	g_assert(service);

	service->pProto = IpcomProtocolGetInstance();
	service->serviceId = serviceId;
	service->ProcessMessage = _DefaultProcessMessage;

	return service;
}

void
IpcomServiceDestroy(IpcomService *service)
{
	g_free(service);
}

gboolean
IpcomServiceRegister(IpcomService *service)
{
	return IpcomProtocolRegisterService(service->pProto, service);
}
