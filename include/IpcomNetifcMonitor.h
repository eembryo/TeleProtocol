#ifndef __IpcomNetifcMonitor_h_
#define __IpcomNetifcMonitor_h_

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _IpcomNetifcMonitor IpcomNetifcMonitor;

#define MAX_NUM_OF_ADDRESS	16

struct _IpcomNetifcMonitor {
	GHashTable*		hashIpcomNetIfcs;
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
} IpcomNetifcMonitorAddressType;

IpcomNetifcMonitor*	IpcomNetifcGetInstance();
void				IpcomNetifcMonitorUpdate(IpcomNetifcMonitor *);
GInetAddress*		IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcMonitor*, const GInetAddress *addr);
GInetAddress*		IpcomNetifcMonitorQueryIpv4PrefSrcForDest(IpcomNetifcMonitor*, const GInetAddress *target);
gboolean			IpcomNetifcMonitorIsBroadcastAddress(IpcomNetifcMonitor*, const GInetAddress *addr);
IpcomNetifcMonitorAddressType	IpcomNetifcMonitorQueryAddressType(IpcomNetifcMonitor*,const GInetAddress *addr);

/* IpcomNetifcMonitorGetAllIpv4Addr
 *
 * - need to free returned value with g_list_free()
 */
GList*              IpcomNetifcMonitorGetAllIpv4Addr(IpcomNetifcMonitor*);
/* IpcomNetifcMonitorGetAllIpv4BroadAddr
 *
 * - need to free returned value with g_list_free()
 */
GList*				IpcomNetifcMonitorGetAllIpv4BroadAddr(IpcomNetifcMonitor*);

G_END_DECLS

#endif //__IpcomNetifcMonitor_h_
