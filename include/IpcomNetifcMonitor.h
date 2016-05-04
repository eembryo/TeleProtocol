#ifndef __IpcomNetifcMonitor_h_
#define __IpcomNetifcMonitor_h_

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _IpcomNetifcMonitor IpcomNetifcMonitor;

#define MAX_NUM_OF_ADDRESS	16

struct _IpcomNetifcMonitor {
	GHashTable*		hashIpcomNetIfcs;
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
GInetAddress*		IpcomNetifcMonitorQueryBroadcastAddress(IpcomNetifcMonitor*, guint8 inum);
GInetAddress*		IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcMonitor*, const GInetAddress *addr);
GInetAddress*		IpcomNetifcMonitorQueryIpv4PrefSrcForDest(IpcomNetifcMonitor*, const GInetAddress *target);
gboolean			IpcomNetifcMonitorIsBroadcastAddress(IpcomNetifcMonitor*, const GInetAddress *addr);
IpcomNetifcMonitorAddressType	IpcomNetifcMonitorQueryAddressType(IpcomNetifcMonitor*,const GInetAddress *addr);
IpcomNetifcMonitorAddressType	IpcomNetifcMonitorQueryAddressType2(IpcomNetifcMonitor*,const GInetAddress *addr,guint8 ifnum);

/* IpcomNetifMonitorGetAllBroadcastAddress
 *
 * - need to free GList memory with g_list_free()
 */
GList*				IpcomNetifMonitorGetAllBroadcastAddress(IpcomNetifcMonitor*);

G_END_DECLS

#endif //__IpcomNetifcMonitor_h_
