/*
 * SocketUtils.h
 *
 *  Created on: Sep 8, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_SOCKETUTILS_H_
#define INCLUDE_SOCKETUTILS_H_

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

#endif /* INCLUDE_SOCKETUTILS_H_ */
