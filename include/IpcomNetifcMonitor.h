#ifndef __IpcomNetifcMonitor_h_
#define __IpcomNetifcMonitor_h_

#include <glib.h>
#include <gio/gio.h>

typedef struct _IpcomNetifcMonitor IpcomNetifcMonitor;
typedef struct _IpcomNetifc		IpcomNetifc;

#define MAX_NUM_OF_ADDRESS	16

struct _IpcomNetifc {
	GList*			listIfcAddrs;
	GInetAddress*	pIfcBroadAddr;
	guint8			nIfcNum;
	gchar*			pIfcName;
};

struct _IpcomNetifcMonitor {
	GHashTable*		hashIpcomNetIfcs;
};

IpcomNetifcMonitor*	IpcomNetifcGetInstance();
void				IpcomNetifcMonitorUpdate(IpcomNetifcMonitor *);
GInetAddress*		IpcomNetifcMonitorQueryBroadcastAddress(IpcomNetifcMonitor*, guint8 inum);
GInetAddress*		IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcMonitor*, const GInetAddress *addr);
GInetAddress*		IpcomNetifcMonitorQueryIpv4PrefSrcForDest(IpcomNetifcMonitor*, const GInetAddress *target);
gboolean			IpcomNetifcMonitorIsBroadcastAddress(IpcomNetifcMonitor*, const GInetAddress *addr);

/* IpcomNetifMonitorGetAllBroadcastAddress
 *
 * - need to free GList memory with g_list_free()
 */
GList*				IpcomNetifMonitorGetAllBroadcastAddress(IpcomNetifcMonitor*);

#endif //__IpcomNetifcMonitor_h_
