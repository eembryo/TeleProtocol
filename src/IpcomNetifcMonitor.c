#include <IpcomNetifcMonitor.h>
#include <glib.h>
#include <gio/gio.h>
#include <dprint.h>
#include <string.h>
#include <SocketUtils.h>

/// For netlink
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

typedef struct _IpcomIfcAddress IpcomIfcAddress;
struct _IpcomIfcAddress {
	/// @pAddr: a unique address involved in the interface.
	GInetAddress*	pAddr;
	/// @pIfcName:	interface name such as "eth0", "eth0:0".
	gchar*			pIfcName;
	/// @pBroadAddr:	broadcast address. It may be duplicate address, if interface has addresses more than one.
	GInetAddress*	pBroadAddr;
};

typedef struct _IpcomNetifc		IpcomNetifc;
struct _IpcomNetifc {
	GList*			listIfcIpv4Addrs;
	//GList*		listIfcIpv6Addrs;
	guint8			nIfcNum;
};

static IpcomNetifcMonitor* pNetifcMonitor = NULL;

static void
_NetifcDump(IpcomNetifc* netifc)
{
	gchar*				str;
	gchar*				str1;
	GList* 				listIter;
	IpcomIfcAddress* 	pIfcAddr;

	DPRINT("\t Interface number: %d\n", netifc->nIfcNum);
	if (netifc->listIfcIpv4Addrs) {
		for (listIter = g_list_first(netifc->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			pIfcAddr = (IpcomIfcAddress*)listIter->data;
			str = g_inet_address_to_string(pIfcAddr->pAddr);
			if (pIfcAddr->pBroadAddr) {
				str1 = g_inet_address_to_string(pIfcAddr->pBroadAddr);
				DPRINT("\t\t[%s] Address: %s, BroadcastAddress: %s\n", pIfcAddr->pIfcName, str, str1);
				g_free(str1);
			} else {
				DPRINT("\t\t[%s] Address: %s\n", pIfcAddr->pIfcName, str);
			}
			g_free(str);
		}
	}
}
/*
static gboolean
IpcomIfcAddressCompare(const IpcomIfcAddress* a, const IpcomIfcAddress* b)
{
	if (a->isprimary == b->isprimary &&
			g_inet_address_equal(a->pAddr, b->pAddr) &&
			((a->pBroadAddr && b->pBroadAddr) ? g_inet_address_equal(a->pBroadAddr,b->pBroadAddr) :
					((!a->pBroadAddr && !b->pBroadAddr) ? TRUE : FALSE))
	) return TRUE;
	else return FALSE;
}
*/
static IpcomIfcAddress*
IpcomIfcAddressNew()
{
	IpcomIfcAddress* new = g_malloc(sizeof(IpcomIfcAddress));
	if (!new) return NULL;

	new->pAddr = NULL;
	new->pBroadAddr = NULL;
	new->pIfcName = NULL;

	return new;
}

static inline void
IpcomIfcAddressSetAddress(IpcomIfcAddress* pIfcAddr, GInetAddress* pAddr)
{
	if (pIfcAddr->pAddr) g_object_unref(pIfcAddr->pAddr);
	pIfcAddr->pAddr = pAddr ? g_object_ref(pAddr) : NULL;
}

static inline void
IpcomIfcAddressSetBroadcast(IpcomIfcAddress* pIfcAddr, GInetAddress* broadAddr)
{
	if (pIfcAddr->pBroadAddr) g_object_unref(pIfcAddr->pBroadAddr);
	pIfcAddr->pBroadAddr = broadAddr ? g_object_ref(broadAddr) : NULL;
}

static inline void
IpcomIfcAddressSetName(IpcomIfcAddress* pIfcAddr, const gchar* name)
{
	if (pIfcAddr->pIfcName) g_free(pIfcAddr->pIfcName);
	pIfcAddr->pIfcName = name ? g_strdup(name) : NULL;
}
static void
IpcomIfcAddressDestroy(IpcomIfcAddress *addr)
{
	if (addr->pAddr) g_object_unref(addr->pAddr);
	if (addr->pBroadAddr) g_object_unref(addr->pBroadAddr);
	if (addr->pIfcName) g_free(addr->pIfcName);

	g_free(addr);
}
static IpcomNetifc*
IpcomNetifcNew()
{
	IpcomNetifc *new = g_malloc0(sizeof(IpcomNetifc));
	if (!new) return NULL;

	new->nIfcNum = 0;
	new->listIfcIpv4Addrs = NULL;

	return new;
}
static void
IpcomNetifcDestroy(gpointer data)
{
	IpcomNetifc *ifc = (IpcomNetifc *)data;
	GList*	iter;

	if (ifc->listIfcIpv4Addrs) {
		for (iter = g_list_first(ifc->listIfcIpv4Addrs); iter != NULL; iter = g_list_next(iter)) {
			IpcomIfcAddressDestroy((IpcomIfcAddress*)iter->data);
		}
		g_list_free(ifc->listIfcIpv4Addrs);
	}
	g_free(ifc);
}
static IpcomIfcAddress*
IpcomNetifcLookupAddress(IpcomNetifc* netifc, const GInetAddress *pAddr)
{
	GList*	listIter;

	switch(g_inet_address_get_family((GInetAddress*)pAddr)) {
	case G_SOCKET_FAMILY_IPV4:
		for (listIter = g_list_first(netifc->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			if (g_inet_address_equal((GInetAddress*)pAddr, ((IpcomIfcAddress*)listIter->data)->pAddr)) {
				return listIter->data;
			}
		}
		break;
	case G_SOCKET_FAMILY_IPV6:
		DERROR("Ipv6 is not supported yet.\n");
		break;
	default:
		DERROR("Only inet address is supported.\n");
		break;
	}

	return NULL;
}

static gboolean
IpcomNetifcAddIfcAddress(IpcomNetifc* netifc, IpcomIfcAddress* pIfcAddr)
{
	IpcomIfcAddress* 	pOldIfcAddr;
	GList**				ppList;

	///check whether interface address with pIfcAddr->pAddr exists
	g_assert(pIfcAddr->pAddr);
	switch(g_inet_address_get_family(pIfcAddr->pAddr)) {
	case G_SOCKET_FAMILY_IPV4:
		ppList = &netifc->listIfcIpv4Addrs;
		break;
	case G_SOCKET_FAMILY_IPV6:
		DERROR("Ipv6 is not supported yet.\n");
		g_assert(FALSE);
		break;
	default:
		DERROR("Only inet address is supported.\n");
		g_assert(FALSE);
	}
	pOldIfcAddr = IpcomNetifcLookupAddress(netifc, pIfcAddr->pAddr);
	if (pOldIfcAddr) *ppList = g_list_remove(*ppList, pOldIfcAddr);
	*ppList = g_list_append(*ppList, pIfcAddr);

	return TRUE;
}


static guint
GInetAddressHashFunc(gconstpointer key)
{
	GInetAddress*	pAddr = (GInetAddress*)key;
	const guint8* 	pRawData = g_inet_address_to_bytes(pAddr);
	guint			hashValue;

	/// we consider last 4 bytes as hash value.
	switch (g_inet_address_get_family(pAddr)) {
	case G_SOCKET_FAMILY_IPV4:
		memcpy(&hashValue, pRawData, 4);
		break;
	case G_SOCKET_FAMILY_IPV6:
		memcpy(&hashValue, pRawData + 12, 4);
		break;
	default:
		DERROR("Only Inet address is supported.\n");
		g_assert(FALSE);
	}

	return hashValue;
}


/*************************************
 * IpcomNetifcMonitor functions
 *************************************/
static void
IpcomNetifcMonitorClearBroadAddrCache(IpcomNetifcMonitor* monitor)
{
	g_hash_table_remove_all(monitor->hashIpv4BroadAddrCache);
}

static void
IpcomNetifcMonitorUpdateBroadAddrCache(IpcomNetifcMonitor* monitor)
{
	GHashTableIter	iter;
	gpointer 		key, value;
	GList*			listIter;
	GInetAddress	*pBroadAddr;

	IpcomNetifcMonitorClearBroadAddrCache(monitor);

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		for (listIter = g_list_first(((IpcomNetifc*)value)->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			pBroadAddr = ((IpcomIfcAddress*)listIter->data)->pBroadAddr;
			if (!pBroadAddr) continue;
			else if (!g_hash_table_contains(monitor->hashIpv4BroadAddrCache, pBroadAddr)) {
				g_object_ref(pBroadAddr);
				g_hash_table_insert(monitor->hashIpv4BroadAddrCache, pBroadAddr, NULL);
			}
		}
	}
}

IpcomNetifcMonitor*
IpcomNetifcMonitorNew()
{
	if (pNetifcMonitor) return NULL;

	pNetifcMonitor = g_malloc(sizeof(IpcomNetifcMonitor));
	pNetifcMonitor->hashIpcomNetIfcs = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, IpcomNetifcDestroy);
	pNetifcMonitor->hashIpv4BroadAddrCache = g_hash_table_new_full(GInetAddressHashFunc, (GEqualFunc)g_inet_address_equal , g_object_unref, NULL);
	IpcomNetifcMonitorUpdate(pNetifcMonitor);

	return pNetifcMonitor;
}

IpcomNetifcMonitor*
IpcomNetifcGetInstance()
{
	return pNetifcMonitor ? pNetifcMonitor : IpcomNetifcMonitorNew();
}

void
IpcomNetifcMonitorDump(IpcomNetifcMonitor* monitor)
{
	GHashTableIter iter;
	gpointer key, value;
	IpcomNetifc* pNetifc;

	DPRINT("*********** NetifcMonitor Dump *********\n");
	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		DPRINT("---\n");
		pNetifc = (IpcomNetifc*)value;
		_NetifcDump(pNetifc);
	}
	DPRINT("***********************************\n");
}

#define NETLINK_BUFSIZE 1024
void
IpcomNetifcMonitorUpdate(IpcomNetifcMonitor *monitor)
{
	int sock;
	struct nlmsghdr*	nlMsg;
	struct ifaddrmsg*	ifAddrMsg;
	gchar msgBuf[NETLINK_BUFSIZE] = {0};
	gchar reply[NETLINK_BUFSIZE] = {0};

	g_hash_table_remove_all(pNetifcMonitor->hashIpcomNetIfcs);

	if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		DERROR("socket creation:");

	/* point the header and the msg structure pointers into the buffer */
	nlMsg = (struct nlmsghdr*) msgBuf;
	ifAddrMsg = (struct ifaddrmsg*) NLMSG_DATA(nlMsg);

	/* Fill in the nlmsg header*/
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));  // Length of message.
	nlMsg->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	nlMsg->nlmsg_type = RTM_GETADDR;

	ifAddrMsg->ifa_family = AF_INET;

	/* Send the request */
	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
		DERROR("Write To Socket Failed...\n");
		return;
	}

	{
		int nll, rtml;
		struct rtattr*		rtap;
		GInetAddress*		addr;
		IpcomNetifc*		pNetifc;
		IpcomIfcAddress*	pThisIfcAddress;

		/* Receive response */
		nll = recv(sock, reply, sizeof(reply), 0);
		if (nll < 0) {
			DERROR("Receive From Socket Failed:");
			return;
		}

		for (nlMsg = (struct nlmsghdr *)reply; NLMSG_OK(nlMsg,nll); nlMsg = NLMSG_NEXT(nlMsg, nll)) {
			ifAddrMsg = (struct ifaddrmsg*) NLMSG_DATA(nlMsg);
			rtml = IFA_PAYLOAD(nlMsg);
			pNetifc = g_hash_table_lookup(pNetifcMonitor->hashIpcomNetIfcs, GINT_TO_POINTER(ifAddrMsg->ifa_index));
			if (!pNetifc) {
				pNetifc = IpcomNetifcNew();
				pNetifc->nIfcNum = ifAddrMsg->ifa_index;
				g_hash_table_insert(monitor->hashIpcomNetIfcs, GINT_TO_POINTER(ifAddrMsg->ifa_index), pNetifc);
			}
			pThisIfcAddress = IpcomIfcAddressNew();

			for (rtap = IFA_RTA(ifAddrMsg); RTA_OK(rtap, rtml); rtap = RTA_NEXT(rtap, rtml)) {
				//DPRINT("rta_type = %d\n", rtap->rta_type);
				if (rtap->rta_type == IFA_ADDRESS) {
					addr =  g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
#if DEBUG
					{
						gchar* addrstr;
						addrstr = g_inet_address_to_string(addr);
						DPRINT("address is %s\n", addrstr);
						g_free(addrstr);
					}
#endif
					IpcomIfcAddressSetAddress(pThisIfcAddress, addr);
					g_object_unref(addr);
				}
				else if (rtap->rta_type == IFA_BROADCAST) {
					addr = g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
					IpcomIfcAddressSetBroadcast(pThisIfcAddress, addr);
					g_object_unref(addr);
				}
				else if (rtap->rta_type == IFA_LABEL) {
					IpcomIfcAddressSetName(pThisIfcAddress, (gchar *)RTA_DATA(rtap));
				}
			} // for(...)
			/// register pThisIfcAddress to pNetifc
			IpcomNetifcAddIfcAddress(pNetifc, pThisIfcAddress);
		}
	}
	close (sock);

	IpcomNetifcMonitorUpdateBroadAddrCache(monitor);
	IpcomNetifcMonitorDump(monitor);
}

gint
IpcomNetifcMonitorQueryNetifcWithAddr(IpcomNetifcMonitor* monitor, const GInetAddress *target)
{
	GHashTableIter iter;
	gpointer key, value;
	IpcomNetifc* pNetifc;

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		pNetifc = (IpcomNetifc*)value;
		if (IpcomNetifcLookupAddress(pNetifc, target)) {
			return pNetifc->nIfcNum;
		}
	}
	return 0;
}

GInetAddress*
IpcomNetifcMonitorQueryPrefSrcForDest(IpcomNetifcMonitor* monitor, const GInetAddress *target)
{
	struct in_addr	dst;
	struct in_addr	result;
	GSocketFamily	family = g_inet_address_get_family((GInetAddress*)target);
	GInetAddress*	newAddr = NULL;

	if (family == G_SOCKET_FAMILY_IPV4) {
		memcpy(&dst, g_inet_address_to_bytes((GInetAddress*)target), sizeof(struct in_addr));
		QuerySrcIpv4AddrForDst(&dst, &result);
		newAddr = g_inet_address_new_from_bytes((const guint8 *)&result, G_SOCKET_FAMILY_IPV4);
	}
	else if(family == G_SOCKET_FAMILY_IPV6) {
		DERROR("only IPv4 is supported.\n");
		return NULL;
	}
	else {
		DERROR("only IPv4 is supported.\n");
		return NULL;
	}

	return newAddr;
}

gboolean
IpcomNetifcMonitorIsBroadcastAddress(IpcomNetifcMonitor* monitor, const GInetAddress *addr)
{
	switch(g_inet_address_get_family((GInetAddress*)addr)) {
	case G_SOCKET_FAMILY_IPV4:
		return g_hash_table_contains (monitor->hashIpv4BroadAddrCache, addr);
	case G_SOCKET_FAMILY_IPV6:
		//return g_hash_table_contains (monitor->hashIpv6BroadAddrCache, addr);
		break;
	default:
		DERROR("Only inet address is supported\n");
		g_assert(FALSE);
	}

	return FALSE;
}

GInetAddress*
IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcMonitor* monitor, const GInetAddress *addr)
{
	GHashTableIter 		iter;
	gpointer 			key, value;
	IpcomNetifc* 		pNetifc;
	IpcomIfcAddress*	pIfcAddr;

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		pNetifc = (IpcomNetifc*)value;
		pIfcAddr = IpcomNetifcLookupAddress(pNetifc, addr);
		return pIfcAddr->pBroadAddr;
	}

	return NULL;
}

IpcomNetifcMonitorAddressType
IpcomNetifcMonitorQueryAddressType(IpcomNetifcMonitor* monitor,const GInetAddress *addr)
{
	GHashTableIter hashIter;
	gpointer key, value;

	switch(g_inet_address_get_family((GInetAddress*)addr)) {
	case G_SOCKET_FAMILY_IPV4:
		if (g_inet_address_get_is_any((GInetAddress*)addr)) return ADDRESSTYPE_IPV4_ANY;
		else if (IpcomNetifcMonitorIsBroadcastAddress(monitor, addr)) return ADDRESSTYPE_IPV4_BROADCAST;
		else {
			g_hash_table_iter_init (&hashIter, monitor->hashIpcomNetIfcs);
			while (g_hash_table_iter_next (&hashIter, &key, &value)) {
				if (IpcomNetifcLookupAddress((IpcomNetifc*)value, addr)) {
					return ADDRESSTYPE_IPV4_UNICAST;
				}
			}
		}
		break;
	case G_SOCKET_FAMILY_IPV6:
		break;
	default:
		break;
	}
	return ADDRESSTYPE_UNKNOWN;
}

GList*
IpcomNetifcMonitorGetAllIpv4BroadAddr(IpcomNetifcMonitor* monitor)
{
	GList*	list = NULL;
	GHashTableIter	iter;
	gpointer 		key;

	g_hash_table_iter_init (&iter, monitor->hashIpv4BroadAddrCache);

	while (g_hash_table_iter_next (&iter, &key, NULL)) {
		list = g_list_append(list, (GInetAddress*)key);
	}
	return list;
}

GList*
IpcomNetifcMonitorGetAllIpv4Addr(IpcomNetifcMonitor* monitor)
{
    GHashTableIter  iter;
    gpointer        key, value;
    GList*          listIter = NULL;
    GList*          retList = NULL;

    IpcomNetifcMonitorClearBroadAddrCache(monitor);

    g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        for (listIter = g_list_first(((IpcomNetifc*)value)->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
            retList = g_list_append(retList, ((IpcomIfcAddress*)listIter->data)->pAddr);
        }
    }

    return retList;
}
