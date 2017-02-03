#include "network_link_monitor.h"

#include <iostream>
#include <glib.h>
#include <gio/gio.h>
#include <net/if.h>
#include <sys/socket.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

NetworkLinkMonitor::NetworkLinkMonitor ()
{
	mMainContext = g_main_context_ref_thread_default();
	mSocketReadSource = NULL;
	mSocket = NULL;
	mListener = NULL;
	mSockfd = -1;
	init();
}

NetworkLinkMonitor::~NetworkLinkMonitor ()
{
	if (mSocketReadSource) g_source_destroy (mSocketReadSource);
	if (mSocket) g_object_unref (mSocket);
	if (mMainContext) g_main_context_unref(mMainContext);
	if (mSockfd != -1) {
		(void) close (mSockfd);
		mSockfd = -1;
	}
	// IMPL: free other pointers
}

bool
NetworkLinkMonitor::setListener(const char *ifc_name, NetworkLinkEventListener *listener)
{
	mNetifcName = std::string(ifc_name);
	mListener = listener;

	return true;
}

bool
NetworkLinkMonitor::init()
{
	GError *error = NULL;
	struct sockaddr_nl snl;

	mSockfd = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (mSockfd == -1) {
		XCALLLOGE("[NM] Cannot create socket: %s", g_strerror (errno));
		return FALSE;
	}

	snl.nl_family = AF_NETLINK;
	snl.nl_pid = snl.nl_pad = 0;
	snl.nl_groups = RTNLGRP_LINK;
	if (bind (mSockfd, (struct sockaddr *)&snl, sizeof (snl)) != 0) {
		XCALLLOGE("[NM] Failed to bind socket: %s", g_strerror(errno));
		(void) close (mSockfd);
		return FALSE;
	}

	mSocket = g_socket_new_from_fd (mSockfd, &error);
	if (error) {
		XCALLLOGE("[NM] Failed to GSocket from fd: %s", error->message);
		g_error_free (error);
		(void) close (mSockfd);
		mSockfd = -1;
		return FALSE;
	}
	mSocketReadSource = g_socket_create_source (mSocket, G_IO_IN, NULL);
	g_source_set_callback (mSocketReadSource, (GSourceFunc) NetworkLinkMonitor::readLinkEvent, this, NULL);

	g_source_attach (mSocketReadSource, mMainContext);
	g_source_unref (mSocketReadSource);

	return TRUE;
}

gboolean
NetworkLinkMonitor::readLinkEvent(GSocket *socket, GIOCondition condition, gpointer user_data)
{
	NetworkLinkMonitor *monitor = (NetworkLinkMonitor *)user_data;
	gssize len;
	GError *error = NULL;

	gchar input_buffer[4096];;
	GInputVector iv;
	iv.buffer = input_buffer;
	iv.size = 4096;

	char *ifname = NULL;
	struct ifinfomsg *ifinfomsg;
	struct rtattr *attr;
	gsize attrlen;
	struct nlmsghdr *msg;

	len = g_socket_receive_message (monitor->getGSocket(), NULL, &iv, 1, NULL, NULL, NULL, NULL, &error);
	if (len < 0) {
		XCALLLOGI("Error on reading netlink socket: %s", error->message);
		g_error_free (error);
		goto _readLinkEvent_done;
	}

	msg = (struct nlmsghdr *) iv.buffer;
	for (; len > 0; msg = NLMSG_NEXT (msg, len)) {
		if (!NLMSG_OK (msg, (size_t) len)){
			XCALLLOGI ("netlink message was truncated; shouldn't happen...");
			goto _readLinkEvent_done;
		}

		switch (msg->nlmsg_type) {
		case RTM_DELLINK:
			ifinfomsg = (struct ifinfomsg*)NLMSG_DATA (msg);
			/* NOTE: On removing a network link, several RTM_DELLINK messages may be
			 * generated with various ifi_family values. The RTM_DELLINK message
			 * with AF_UNSPEC in ifi_family always exists.
			 */
			if (ifinfomsg->ifi_family != AF_UNSPEC) break;
			attrlen = NLMSG_PAYLOAD (msg, sizeof(struct ifinfomsg));
			attr = ((struct rtattr*)(((char*)(ifinfomsg)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))));

			while (RTA_OK (attr, attrlen)) {
				if (attr->rta_type == IFLA_IFNAME) {
					ifname = (char *)RTA_DATA(attr);
					break;
				}
				attr = RTA_NEXT (attr, attrlen);
			}
			if (monitor->mListener && (ifname != NULL) && monitor->mNetifcName.compare(ifname) == 0) {
				monitor->mListener->notifyDelLinkEvent();
			}
			break;
		case NLMSG_DONE:
			break;
		case NLMSG_ERROR:
			{
				struct nlmsgerr *e = (struct nlmsgerr *)NLMSG_DATA (msg);
				XCALLLOGI ("netlink error: %s", g_strerror (-e->error));
			}
			break;
		default:
		  break;
		}
  }

	_readLinkEvent_done:
	return G_SOURCE_CONTINUE;
}
