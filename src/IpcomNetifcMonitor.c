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
	gboolean		isprimary;
	gchar*			pIfcName;
	GInetAddress*	pAddr;
	GInetAddress*	pBroadAddr;
};

typedef struct _IpcomNetifc		IpcomNetifc;
struct _IpcomNetifc {
	GList*			listIfcIpv4Addrs;
	guint8			nIfcNum;
};

static IpcomNetifcMonitor* pNetifcMonitor = NULL;

static void
_NetifcDump(IpcomNetifc* netifc)
{
	gchar*	str, str1;
	GList* listIter;
	struct _IpcomNetifcAddress* pNetifcAddr;

	DPRINT("\t Interface number: %d\n", netifc->nIfcNum);
	if (netifc->listIfcIpv4Addrs) {
		for (listIter = g_list_first(netifc->listIfcIpv4Addrs); listIter != NULL; listIter = g_list_next(listIter)) {
			pNetifcAddr = (struct _IpcomNetifcAddress*)listIter->data;
			str = g_inet_address_to_string(pNetifcAddr->pAddr);
			if (pNetifcAddr->pBroadAddr) {
				str1 = g_inet_address_to_string(pNetifcAddr->pBroadAddr);
				DPRINT("\t\t[%s][%s] Address: %s, BroadcastAddress: %s", pNetifcAddr->pIfcName, pNetifcAddr->isprimary ? "PRI", "SEC", str, str1);
				g_free(str1);
			} else {
				DPRINT("\t\t[%s][%s] Address: %s", pNetifcAddr->pIfcName, pNetifcAddr->isprimary ? "PRI", "SEC", str);
			}
			g_free(str);
		}
	}
}

static gboolean
IpcomIfcAddressCompare(const IpcomIfcAddress* a, const IpcomIfcAddress* b)
{
	if (a->isprimary != b->isprimary || !g_inet_address_equal(a->pAddr, b->pAddr) || strcmp(a->pIfcName, b->pIfcName))
		return FALSE;

	if (a->pBroadAddr && b->pBroadAddr && g_inet_address_equal(a->pBroadAddr,b->pBroadAddr)) return TRUE;
	else if (!a->pBroadAddr && !b->pBroadAddr) return TRUE;

	return FALSE;
}

static IpcomIfcAddress*
IpcomIfcAddressNew(const gchar* name, GInetAddress* uniAddr, GInetAddress* broadAddr, gboolean isPrimary)
{
	IpcomIfcAddress* new = g_malloc0(sizeof(IpcomIfcAddress));
	if (!new) return NULL;

	new->isprimary = isPrimary;
	new->pAddr = g_object_ref(uniAddr);
	if (broadAddr)
		new->pBroadAddr = g_object_ref(broadAddr);
	else
		new->pBroadAddr = NULL;
	new->pIfcName = g_strdup(name);

	return new;
}

static void
IpcomIfcAddressDestroy(IpcomIfcAddress *addr)
{
	if (addr->pAddr) g_object_unref(addr->pAddr);
	if (addr->pBroadAddr) g_object_unref(addr->pBroadAddr);
	if (addr->pIfcName) g_free(addr->pIfcName);
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


static gboolean
IpcomNetifcIsAddrExist(IpcomNetifc* netifc, const GInetAddress *pUnicatAddr)
{
	GList*	iter;

	for (iter = g_list_first(netifc->listIfcIpv4Addrs); iter != NULL; iter = g_list_next(iter)) {
		if (g_inet_address_equal((GInetAddress*)pUnicatAddr, ((IpcomIfcAddress*)iter->data)->pAddr)) {
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
IpcomNetifcIsIfcAddrExist(IpcomNetifc* netifc, const IpcomIfcAddress *pIfcAddr)
{
	GList*	iter;

	for (iter = g_list_first(netifc->listIfcIpv4Addrs); iter != NULL; iter = g_list_next(iter)) {
		if (IpcomIfcAddressCompare(pIfcAddr, (IpcomIfcAddress*)iter->data))
			return TRUE;
	}
}

static gboolean
IpcomNetifcAddIfcAddress(IpcomNetifc* netifc, const IpcomIfcAddress *pIfcAddr)
{
	IpcomIfcAddress* new;

	if (IpcomNetifcIsIfcAddrExist(netifc, pIfcAddr)) {
		return FALSE;
	}

	new = IpcomIfcAddressNew(pIfcAddr->pIfcName, pIfcAddr->pAddr, pIfcAddr->pBroadAddr, pIfcAddr->isprimary);
	g_assert(new);

	netifc->listIfcIpv4Addrs = g_list_append(netifc->listIfcIpv4Addrs, new);

	return TRUE;
}

IpcomNetifcMonitor*
IpcomNetifcMonitorNew()
{
	if (pNetifcMonitor) return NULL;

	pNetifcMonitor = g_malloc(sizeof(IpcomNetifcMonitor));
	pNetifcMonitor->hashIpcomNetIfcs = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, IpcomNetifcDestroy);

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
	IpcomNetifc*		pNetifc;

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
		struct rtattr	*rtap;
		GInetAddress	*addr;

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

			for (rtap = IFA_RTA(ifAddrMsg); RTA_OK(rtap, rtml); rtap = RTA_NEXT(rtap, rtml)) {
				DPRINT("rta_type = %d\n", rtap->rta_type);
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
					IpcomNetifcAddAddress(pNetifc, addr);
					g_object_unref(addr);
				}
				else if (rtap->rta_type == IFA_BROADCAST) {
					pNetifc->pIfcBroadAddr = g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
				}
				else if (rtap->rta_type == IFA_LABEL) {
					pNetifc->pIfcName = g_strdup((gchar *)RTA_DATA(rtap));
				}
			}
		}
	}
	close (sock);

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
		if (IpcomNetifcIsAddrExist(pNetifc, target)) {
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
	GHashTableIter iter;
	gpointer key, value;
	GInetAddress*	subject;

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		subject = ((IpcomNetifc*)value)->pIfcBroadAddr;
		if (g_inet_address_equal((GInetAddress*)addr, subject)) {
			return TRUE;
		}
	}

	return FALSE;
}

GInetAddress*
IpcomNetifcMonitorQueryBroadcastAddressWithSrc(IpcomNetifcMonitor* monitor, const GInetAddress *addr)
{
	int num = IpcomNetifcMonitorQueryNetifcWithAddr(monitor, addr);
	IpcomNetifc* pNetifc;

	if (!num) {
		gchar *str = g_inet_address_to_string((GInetAddress*)addr);
		DWARN("Failed to find address : %s \n", str);
		g_free(str);
		return NULL;
	}

	pNetifc = (IpcomNetifc *)g_hash_table_lookup(monitor->hashIpcomNetIfcs, GINT_TO_POINTER(num));
	g_assert(pNetifc);

	return pNetifc->pIfcBroadAddr;
}

GList*
IpcomNetifMonitorGetAllBroadcastAddress(IpcomNetifcMonitor* monitor)
{
	GList*	ret_list = NULL;
	GInetAddress*	brAddr;
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		brAddr = ((IpcomNetifc*)value)->pIfcBroadAddr;
		if (brAddr)
			ret_list = g_list_append(ret_list, brAddr);
	}

	return ret_list;
}

static inline IpcomNetifcMonitorAddressType
_QueryAddressType(IpcomNetifcMonitor* monitor,const GInetAddress *addr, IpcomNetifc* netifc)
{
	switch(g_inet_address_get_family ((GInetAddress*)addr)) {
	case G_SOCKET_FAMILY_IPV4:
		if (netifc->pIfcBroadAddr && g_inet_address_equal((GInetAddress*)addr, netifc->pIfcBroadAddr)) return ADDRESSTYPE_IPV4_BROADCAST;
		else if (IpcomNetifcIsAddrExist(netifc, addr)) return ADDRESSTYPE_IPV4_UNICAST;
		break;
	case G_SOCKET_FAMILY_IPV6:
		DWARN("Not implemented yet.");
		break;
	default :
		break;
	}

	return ADDRESSTYPE_UNKNOWN;
}

IpcomNetifcMonitorAddressType
IpcomNetifcMonitorQueryAddressType(IpcomNetifcMonitor* monitor,const GInetAddress *addr)
{
	GHashTableIter iter;
	gpointer key, value;
	IpcomNetifcMonitorAddressType ret = ADDRESSTYPE_UNKNOWN;

	g_hash_table_iter_init (&iter, monitor->hashIpcomNetIfcs);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		ret = _QueryAddressType(monitor, addr, (IpcomNetifc*)value);
		if (ret != ADDRESSTYPE_UNKNOWN) break;
	}

	return ret;
}

IpcomNetifcMonitorAddressType
IpcomNetifcMonitorQueryAddressType2(IpcomNetifcMonitor* monitor,const GInetAddress *addr,guint8 ifnum)
{
	IpcomNetifc* pNetifc;

	pNetifc = g_hash_table_lookup(monitor->hashIpcomNetIfcs, GINT_TO_POINTER(ifnum));

	if (!pNetifc) return ADDRESSTYPE_UNKNOWN;

	return _QueryAddressType(monitor, addr, pNetifc);
}
