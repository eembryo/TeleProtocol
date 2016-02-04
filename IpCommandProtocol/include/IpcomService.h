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

#ifndef __IpcomService_h__
#define __IpcomService_h__


#include <glib.h>
#include <IpcomTypes.h>
#include <IpcomEnums.h>

G_BEGIN_DECLS

struct _IpcomService {
	guint16				serviceId;
	IpcomProtocol		*pProto;

	IpcomServiceProcessMessage	ProcessMessage;
	gchar				priv_data[0];
};

static inline gpointer IpcomServiceGetPrivData(IpcomService *service) {
	return (gpointer)(service->priv_data);
}

IpcomService 	*IpcomServiceNew(guint16 serviceId, guint priv_size);
void			IpcomServiceDestroy(IpcomService *service);
gboolean		IpcomServiceRegister(IpcomService *service);
G_END_DECLS

#endif
