/*
 * NetifcMonitor.c
 *
 *  Created on: Sep 7, 2016
 *      Author: hyotiger
 */


#include "../include/NetifcMonitor.h"
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <SocketUtils.h>

/// For netlink
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

typedef struct _IfcAddress IfcAddress;
struct _IfcAddress {
	/// @pAddr: a unique address involved in the interface.
	GInetAddress*	pAddr;
	/// @pIfcName:	interface name such as "eth0", "eth0:0".
	gchar*			pIfcName;
	/// @pBroadAddr:	broadcast address. It may be duplicate address, if interface has addresses more than one.
	GInetAddress*	pBroadAddr;
};

typedef struct _Netifc		Netifc;
struct _Netifc {
	GList*			listIfcIpv4Addrs;
	//GList*		listIfcIpv6Addrs;
	guint8			nIfcNum;
};

static NetifcMonitor* pNetifcMonitor = NULL;

static void
_NetifcDump(Netifc* netifc)
{
	gchar*				str;
	gchar*				str1;
	GList* 				listIter;
	IfcAddress* 	pIfcAddr;

	g_debug("\t Interface number: %d", netifc->nIfcNum);
	if (netifc->listIfcIpv4Addrs) {
		for (listIter = g_list_first(netifc->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			pIfcAddr = (IfcAddress*)listIter->data;
			str = g_inet_address_to_string(pIfcAddr->pAddr);
			if (pIfcAddr->pBroadAddr) {
				str1 = g_inet_address_to_string(pIfcAddr->pBroadAddr);
				g_debug("\t\t[%s] Address: %s, BroadcastAddress: %s", pIfcAddr->pIfcName, str, str1);
				g_free(str1);
			} else {
				g_debug("\t\t[%s] Address: %s", pIfcAddr->pIfcName, str);
			}
			g_free(str);
		}
	}
}

static IfcAddress*
IfcAddressNew()
{
	IfcAddress* new = g_malloc(sizeof(IfcAddress));
	if (!new) return NULL;

	new->pAddr = NULL;
	new->pBroadAddr = NULL;
	new->pIfcName = NULL;

	return new;
}

static inline void
IfcAddressSetAddress(IfcAddress* pIfcAddr, GInetAddress* pAddr)
{
	if (pIfcAddr->pAddr) g_object_unref(pIfcAddr->pAddr);
	pIfcAddr->pAddr = pAddr ? g_object_ref(pAddr) : NULL;
}

static inline void
IfcAddressSetBroadcast(IfcAddress* pIfcAddr, GInetAddress* broadAddr)
{
	if (pIfcAddr->pBroadAddr) g_object_unref(pIfcAddr->pBroadAddr);
	pIfcAddr->pBroadAddr = broadAddr ? g_object_ref(broadAddr) : NULL;
}

static inline void
IfcAddressSetName(IfcAddress* pIfcAddr, const gchar* name)
{
	if (pIfcAddr->pIfcName) g_free(pIfcAddr->pIfcName);
	pIfcAddr->pIfcName = name ? g_strdup(name) : NULL;
}
static void
IfcAddressDestroy(IfcAddress *addr)
{
	if (addr->pAddr) g_object_unref(addr->pAddr);
	if (addr->pBroadAddr) g_object_unref(addr->pBroadAddr);
	if (addr->pIfcName) g_free(addr->pIfcName);

	g_free(addr);
}
static Netifc*
NetifcNew()
{
	Netifc *new = g_malloc0(sizeof(Netifc));
	if (!new) return NULL;

	new->nIfcNum = 0;
	new->listIfcIpv4Addrs = NULL;

	return new;
}
static void
NetifcDestroy(gpointer data)
{
	Netifc *ifc = (Netifc *)data;
	GList*	iter;

	if (ifc->listIfcIpv4Addrs) {
		for (iter = g_list_first(ifc->listIfcIpv4Addrs); iter != NULL; iter = g_list_next(iter)) {
			IfcAddressDestroy((IfcAddress*)iter->data);
		}
		g_list_free(ifc->listIfcIpv4Addrs);
	}
	g_free(ifc);
}
static IfcAddress*
NetifcLookupAddress(Netifc* netifc, const GInetAddress *pAddr)
{
	GList*	listIter;

	switch(g_inet_address_get_family((GInetAddress*)pAddr)) {
	case G_SOCKET_FAMILY_IPV4:
		for (listIter = g_list_first(netifc->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			if (g_inet_address_equal((GInetAddress*)pAddr, ((IfcAddress*)listIter->data)->pAddr)) {
				return listIter->data;
			}
		}
		break;
	case G_SOCKET_FAMILY_IPV6:
		g_warning("Ipv6 is not supported yet.\n");
		break;
	default:
		g_warning("Only inet address is supported.\n");
		break;
	}

	return NULL;
}

static gboolean
NetifcAddIfcAddress(Netifc* netifc, IfcAddress* pIfcAddr)
{
	IfcAddress* 		pOldIfcAddr;
	GList**				ppList;

	///check whether interface address with pIfcAddr->pAddr exists
	g_assert(pIfcAddr->pAddr);
	switch(g_inet_address_get_family(pIfcAddr->pAddr)) {
	case G_SOCKET_FAMILY_IPV4:
		ppList = &netifc->listIfcIpv4Addrs;
		break;
	case G_SOCKET_FAMILY_IPV6:
		g_error("Ipv6 is not supported yet.");
		g_assert(FALSE);
		break;
	default:
		g_error("Only inet address is supported.");
		g_assert(FALSE);
	}
	pOldIfcAddr = NetifcLookupAddress(netifc, pIfcAddr->pAddr);
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
		g_error("Only Inet address is supported.\n");
		g_assert(FALSE);
	}

	return hashValue;
}


/*************************************
 * NetifcMonitor functions
 *************************************/
static void
NetifcMonitorClearBroadAddrCache(NetifcMonitor* monitor)
{
	g_hash_table_remove_all(monitor->hashIpv4BroadAddrCache);
}

static void
NetifcMonitorUpdateBroadAddrCache(NetifcMonitor* monitor)
{
	GHashTableIter	iter;
	gpointer 		key, value;
	GList*			listIter;
	GInetAddress	*pBroadAddr;

	NetifcMonitorClearBroadAddrCache(monitor);

	g_hash_table_iter_init (&iter, monitor->hashNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		for (listIter = g_list_first(((Netifc*)value)->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			pBroadAddr = ((IfcAddress*)listIter->data)->pBroadAddr;
			if (!pBroadAddr) continue;
			else if (!g_hash_table_contains(monitor->hashIpv4BroadAddrCache, pBroadAddr)) {
				g_object_ref(pBroadAddr);
				g_hash_table_insert(monitor->hashIpv4BroadAddrCache, pBroadAddr, NULL);
			}
		}
	}
}

NetifcMonitor*
NetifcMonitorNew()
{
	if (pNetifcMonitor) return NULL;

	pNetifcMonitor = g_malloc(sizeof(NetifcMonitor));
	pNetifcMonitor->hashNetIfcs = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NetifcDestroy);
	pNetifcMonitor->hashIpv4BroadAddrCache = g_hash_table_new_full(GInetAddressHashFunc, (GEqualFunc)g_inet_address_equal , g_object_unref, NULL);
	NetifcMonitorUpdate(pNetifcMonitor);

	return pNetifcMonitor;
}

NetifcMonitor*
NetifcGetInstance()
{
	return pNetifcMonitor ? pNetifcMonitor : NetifcMonitorNew();
}

void
NetifcMonitorDump(NetifcMonitor* monitor)
{
	GHashTableIter iter;
	gpointer key, value;
	Netifc* pNetifc;

	g_message("*********** NetifcMonitor Dump *********\n");
	g_hash_table_iter_init (&iter, monitor->hashNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		g_message("---\n");
		pNetifc = (Netifc*)value;
		_NetifcDump(pNetifc);
	}
	g_message("***********************************\n");
}

#define NETLINK_BUFSIZE 1024
void
NetifcMonitorUpdate(NetifcMonitor *monitor)
{
	int sock;
	struct nlmsghdr*	nlMsg;
	struct ifaddrmsg*	ifAddrMsg;
	gchar msgBuf[NETLINK_BUFSIZE] = {0};
	gchar reply[NETLINK_BUFSIZE] = {0};

	g_hash_table_remove_all(pNetifcMonitor->hashNetIfcs);

	if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		g_warning("socket creation:");

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
		g_warning("Write To Socket Failed...\n");
		return;
	}

	{
		int nll, rtml;
		struct rtattr*		rtap;
		GInetAddress*		addr;
		Netifc*		pNetifc;
		IfcAddress*	pThisIfcAddress;

		/* Receive response */
		nll = recv(sock, reply, sizeof(reply), 0);
		if (nll < 0) {
			g_warning("Receive From Socket Failed:");
			return;
		}

		for (nlMsg = (struct nlmsghdr *)reply; NLMSG_OK(nlMsg,nll); nlMsg = NLMSG_NEXT(nlMsg, nll)) {
			ifAddrMsg = (struct ifaddrmsg*) NLMSG_DATA(nlMsg);
			rtml = IFA_PAYLOAD(nlMsg);
			pNetifc = g_hash_table_lookup(pNetifcMonitor->hashNetIfcs, GINT_TO_POINTER(ifAddrMsg->ifa_index));
			if (!pNetifc) {
				pNetifc = NetifcNew();
				pNetifc->nIfcNum = ifAddrMsg->ifa_index;
				g_hash_table_insert(monitor->hashNetIfcs, GINT_TO_POINTER(ifAddrMsg->ifa_index), pNetifc);
			}
			pThisIfcAddress = IfcAddressNew();

			for (rtap = IFA_RTA(ifAddrMsg); RTA_OK(rtap, rtml); rtap = RTA_NEXT(rtap, rtml)) {
				//g_message("rta_type = %d\n", rtap->rta_type);
				if (rtap->rta_type == IFA_ADDRESS) {
					addr =  g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
#if DEBUG
					{
						gchar* addrstr;
						addrstr = g_inet_address_to_string(addr);
						g_message("address is %s\n", addrstr);
						g_free(addrstr);
					}
#endif
					IfcAddressSetAddress(pThisIfcAddress, addr);
					g_object_unref(addr);
				}
				else if (rtap->rta_type == IFA_BROADCAST) {
					addr = g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
					IfcAddressSetBroadcast(pThisIfcAddress, addr);
					g_object_unref(addr);
				}
				else if (rtap->rta_type == IFA_LABEL) {
					IfcAddressSetName(pThisIfcAddress, (gchar *)RTA_DATA(rtap));
				}
			} // for(...)
			/// register pThisIfcAddress to pNetifc
			NetifcAddIfcAddress(pNetifc, pThisIfcAddress);
		}
	}
	close (sock);

	NetifcMonitorUpdateBroadAddrCache(monitor);
	NetifcMonitorDump(monitor);
}

gint
NetifcMonitorQueryNetifcWithAddr(NetifcMonitor* monitor, const GInetAddress *target)
{
	GHashTableIter iter;
	gpointer key, value;
	Netifc* pNetifc;

	g_hash_table_iter_init (&iter, monitor->hashNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		pNetifc = (Netifc*)value;
		if (NetifcLookupAddress(pNetifc, target)) {
			return pNetifc->nIfcNum;
		}
	}
	return 0;
}

GInetAddress*
NetifcMonitorQueryPrefSrcForDest(NetifcMonitor* monitor, const GInetAddress *target)
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
		g_warning("only IPv4 is supported.\n");
		return NULL;
	}
	else {
		g_warning("only IPv4 is supported.\n");
		return NULL;
	}

	return newAddr;
}

gboolean
NetifcMonitorIsBroadcastAddress(NetifcMonitor* monitor, const GInetAddress *addr)
{
	switch(g_inet_address_get_family((GInetAddress*)addr)) {
	case G_SOCKET_FAMILY_IPV4:
		return g_hash_table_contains (monitor->hashIpv4BroadAddrCache, addr);
	case G_SOCKET_FAMILY_IPV6:
		//return g_hash_table_contains (monitor->hashIpv6BroadAddrCache, addr);
		break;
	default:
		g_warning("Only inet address is supported\n");
		g_assert(FALSE);
	}

	return FALSE;
}

GInetAddress*
NetifcMonitorQueryBroadcastAddressWithSrc(NetifcMonitor* monitor, const GInetAddress *addr)
{
	GHashTableIter 		iter;
	gpointer 			key, value;
	Netifc* 		pNetifc;
	IfcAddress*	pIfcAddr;

	g_hash_table_iter_init (&iter, monitor->hashNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		pNetifc = (Netifc*)value;
		pIfcAddr = NetifcLookupAddress(pNetifc, addr);
		if (pIfcAddr) return pIfcAddr->pBroadAddr;
	}

	return NULL;
}

NetifcMonitorAddressType
NetifcMonitorQueryAddressType(NetifcMonitor* monitor,const GInetAddress *addr)
{
	GHashTableIter hashIter;
	gpointer key, value;

	switch(g_inet_address_get_family((GInetAddress*)addr)) {
	case G_SOCKET_FAMILY_IPV4:
		if (g_inet_address_get_is_any((GInetAddress*)addr)) return ADDRESSTYPE_IPV4_ANY;
		else if (NetifcMonitorIsBroadcastAddress(monitor, addr)) return ADDRESSTYPE_IPV4_BROADCAST;
		else {
			g_hash_table_iter_init (&hashIter, monitor->hashNetIfcs);
			while (g_hash_table_iter_next (&hashIter, &key, &value)) {
				if (NetifcLookupAddress((Netifc*)value, addr)) {
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
NetifcMonitorGetAllIpv4BroadAddr(NetifcMonitor* monitor)
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
NetifcMonitorGetAllIpv4Addr(NetifcMonitor* monitor)
{
    GHashTableIter  iter;
    gpointer        key, value;
    GList*          listIter = NULL;
    GList*          retList = NULL;

    NetifcMonitorClearBroadAddrCache(monitor);

    g_hash_table_iter_init (&iter, monitor->hashNetIfcs);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        for (listIter = g_list_first(((Netifc*)value)->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
            retList = g_list_append(retList, ((IfcAddress*)listIter->data)->pAddr);
        }
    }

    return retList;
}
