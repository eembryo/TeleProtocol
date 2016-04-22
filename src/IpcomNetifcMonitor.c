#include <IpcomNetifcMonitor.h>
#include <glib.h>
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

static IpcomNetifcMonitor* pNetifcMonitor = NULL;

static void
_NetifcDump(IpcomNetifc* netifc)
{
	gchar*	str;
	GList* listIter;

	DPRINT("\t Interface number: %d\n", netifc->nIfcNum);
	DPRINT("\t Interface name: %s\n", netifc->pIfcName);
	if (netifc->pIfcBroadAddr) {
		str = g_inet_address_to_string(netifc->pIfcBroadAddr);
		DPRINT("\t BroadcastAddress: %s\n", str);
		g_free(str);
	}
	else {
		DPRINT("\t BroadcastAddress: NA\n");
	}
	DPRINT("\t UnicastAddresses: \n");
	if (netifc->listIfcAddrs) {
		for (listIter = g_list_first(netifc->listIfcAddrs); listIter != NULL; listIter = g_list_next(listIter)) {
			str = g_inet_address_to_string((GInetAddress*)listIter->data);
			DPRINT("\t\t%s\n", str);
			g_free(str);
		}
	}
}

static IpcomNetifc*
IpcomNetifcNew()
{
	IpcomNetifc *new = g_malloc0(sizeof(IpcomNetifc));
	if (!new) return NULL;

	new->nIfcNum = 0;
	new->listIfcAddrs = NULL;
	new->pIfcBroadAddr = NULL;
	new->pIfcName = NULL;

	return new;
}

static void
IpcomNetifcDestroy(gpointer data)
{
	IpcomNetifc *ifc = (IpcomNetifc *)data;
	GList*	iter;

	if (ifc->listIfcAddrs) {
		for (iter = g_list_first(ifc->listIfcAddrs); iter != NULL; iter = g_list_next(iter)) {
			g_object_unref((GInetAddress*)iter->data);
		}
		g_list_free(ifc->listIfcAddrs);
	}
	if (ifc->pIfcBroadAddr) {
		g_object_unref(ifc->pIfcBroadAddr);
	}
	if (ifc->pIfcName) {
		g_free(ifc->pIfcName);
	}
	g_free(ifc);
}


static gboolean
IpcomNetifcIsAddrExist(IpcomNetifc* netifc, const GInetAddress *addr)
{
	GList*	iter;

	for (iter = g_list_first(netifc->listIfcAddrs); iter != NULL; iter = g_list_next(iter)) {
		if (g_inet_address_equal((GInetAddress*)addr, (GInetAddress*)iter->data)) {
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
IpcomNetifcAddAddress(IpcomNetifc* netifc, GInetAddress *addr)
{
	if (IpcomNetifcIsAddrExist(netifc, addr)) {
		return FALSE;
	}

	g_object_ref(addr);
	netifc->listIfcAddrs = g_list_append(netifc->listIfcAddrs, addr);

	return TRUE;
}

#if 0
static const GList *
IpcomNetifcGetAddrs(IpcomNetifc* netifc)
{
	return netifc->listIfcAddrs;
}
#endif

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
				//DPRINT("rta_type = %d\n", rtap->rta_type);
				if (rtap->rta_type == IFA_ADDRESS || rtap->rta_type == IFA_LOCAL) {
					addr =  g_inet_address_new_from_bytes(RTA_DATA(rtap), G_SOCKET_FAMILY_IPV4);
					IpcomNetifcAddAddress(pNetifc, addr);
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
		ret_list = g_list_append(ret_list, brAddr);
	}

	return ret_list;
}
