/*
 * NetifcMonitor.h
 *
 *  Created on: Sep 7, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_NETIFCMONITOR_H_
#define INCLUDE_NETIFCMONITOR_H_

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _NetifcMonitor NetifcMonitor;

#define MAX_NUM_OF_ADDRESS	16

struct _NetifcMonitor {
	GHashTable*		hashNetIfcs;
	GHashTable* 	hashIpv4BroadAddrCache;
	/// GList* listIpv6BroadAddrCache;
};

typedef enum {
	ADDRESSTYPE_UNKNOWN = 0,
	ADDRESSTYPE_IPV4_ANY,
	ADDRESSTYPE_IPV4_UNICAST,
	ADDRESSTYPE_IPV4_BROADCAST,
	ADDRESSTYPE_IPV6_ANY,
	ADDRESSTYPE_IPV6_UNICAST,
	ADDRESSTYPE_IPV6_BROADCAST,
} NetifcMonitorAddressType;

NetifcMonitor*			NetifcGetInstance();
void					NetifcMonitorUpdate(NetifcMonitor *);
GInetAddress*			NetifcMonitorQueryBroadcastAddressWithSrc(NetifcMonitor*, const GInetAddress *addr);
GInetAddress*			NetifcMonitorQueryIpv4PrefSrcForDest(NetifcMonitor*, const GInetAddress *target);
gboolean				NetifcMonitorIsBroadcastAddress(NetifcMonitor*, const GInetAddress *addr);
NetifcMonitorAddressType	NetifcMonitorQueryAddressType(NetifcMonitor*,const GInetAddress *addr);

/* NetifcMonitorGetAllIpv4Addr
 * Return ipv4 broadcast addresses. caller should free returned list with g_list_free().
 *
 * return:	a list of GInetAddress*
 * 			NULL on no broadcast addresses
 *
 *
 */
GList*              NetifcMonitorGetAllIpv4Addr(NetifcMonitor*);
/* NetifcMonitorGetAllIpv4BroadAddr
 *
 * - need to free returned value with g_list_free()
 */
GList*				NetifcMonitorGetAllIpv4BroadAddr(NetifcMonitor*);

G_END_DECLS

#endif /* INCLUDE_NETIFCMONITOR_H_ */
