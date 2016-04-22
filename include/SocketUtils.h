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

#ifndef _SocketUtils_H_
#define _SocketUtils_H_

#include <glib.h>
#include <netinet/ip.h>

G_BEGIN_DECLS

/* @QuerySrcIpv4AddrForDst
 * target: target IPv4 address, which is network-byte ordered address.
 *
 * RETURN VALUE: return 0 and *out has network-byte ordered IPv4 address on success, -1 on error.
 */
guint QuerySrcIpv4AddrForDst(const struct in_addr *target, struct in_addr *out);

G_END_DECLS
#endif
