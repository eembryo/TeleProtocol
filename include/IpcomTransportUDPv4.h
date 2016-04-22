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

#ifndef __IpcomTransportUDPv4_h__
#define __IpcomTransportUDPv4_h__


#include <IpcomTypes.h>
#include <ref_count.h>
#include <glib.h>

G_BEGIN_DECLS

IpcomTransport*	IpcomTransportUDPv4New();
void 			IpcomTransportUDPv4Destroy(IpcomTransport *transport);
//gint IpcomTransportGetIpv4Address(IpcomTransport *transport, const char *ifcName, guint *oAddr);
gboolean 		IpcomTransportUDPv4AttachGlibContext(IpcomTransport *transport, GMainContext *ctx);
gboolean 		IpcomTransportUDPv4DetachGlibContext(IpcomTransport *transport);

G_END_DECLS

#endif
