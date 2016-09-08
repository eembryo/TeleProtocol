/*
 * SocketUtils.c
 *
 *  Created on: Sep 8, 2016
 *      Author: hyotiger
 */

#include "../include/SocketUtils.h"
#include <glib.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#define MAX_CMSG_SIZE	256

gint
QueryNetifcForDst(const struct in_addr *target)
{
	int nl_sock;
	union {
		struct nlmsghdr	nlh;
		char 			data[MAX_CMSG_SIZE];
	} nl_req, nl_resp = {0};

	struct rtmsg*	p_rtm;
	struct rtattr*	p_rta;

	if ((nl_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
		perror("socket creation failed:");
		goto _QuerySrcIpv4Addr_failed;
	}

	nl_req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nl_req.nlh.nlmsg_flags = NLM_F_REQUEST;
	nl_req.nlh.nlmsg_type = RTM_GETROUTE;

	/* Set RTM message */
	p_rtm = NLMSG_DATA(&nl_req.nlh);
	p_rtm->rtm_family = AF_INET;

	/* Add RTA */
	p_rta = RTM_RTA(p_rtm);
	p_rta->rta_type = RTA_DST;
	p_rta->rta_len = RTA_LENGTH(sizeof(struct in_addr));
	memcpy(RTA_DATA(p_rta), target, sizeof(struct in_addr));
	nl_req.nlh.nlmsg_len += p_rta->rta_len;

	/* Send the request message */
	if (send(nl_sock, &nl_req, nl_req.nlh.nlmsg_len, 0) < 0) {
		perror("Write To Socket Failed...\n");
		goto _QuerySrcIpv4Addr_failed;
	}

	/* Retrieve result */
	{
		int nll, rtml;
		struct nlmsghdr*	p_nlh;

		p_rtm = NULL;
		p_rta = NULL;

		/* Receive response */
		nll = recv(nl_sock, &nl_resp, sizeof(nl_resp), 0);
		if (nll < 0) {
			perror("Receive From Socket Failed:");
			goto _QuerySrcIpv4Addr_failed;
		}

		for (p_nlh = (struct nlmsghdr *)&nl_resp.nlh; NLMSG_OK(p_nlh,nll); p_nlh = NLMSG_NEXT(p_nlh, nll)) {
			p_rtm = (struct rtmsg *) NLMSG_DATA(p_nlh);
			rtml = RTM_PAYLOAD(p_nlh);
			for (p_rta = RTM_RTA(p_rtm); RTA_OK(p_rta, rtml); p_rta = RTA_NEXT(p_rta, rtml)) {
				//DPRINT("rta_type = %d\n", p_rta->rta_type);
				if (p_rta->rta_type == RTA_OIF) {
					return *(gint *)RTA_DATA(p_rta);
				}
			}
		}
	}
	close(nl_sock);

	return 0;

	_QuerySrcIpv4Addr_failed:
	if (nl_sock != -1) close(nl_sock);

	return -1;
}

guint
QuerySrcIpv4AddrForDst(const struct in_addr *target, struct in_addr *out)
{
	int nl_sock;
	union {
		struct nlmsghdr	nlh;
		char 			data[MAX_CMSG_SIZE];
	} nl_req = {0}, nl_resp = {0};

	struct rtmsg*	p_rtm;
	struct rtattr*	p_rta;

	if ((nl_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
		perror("socket creation failed:");
		goto _QuerySrcIpv4Addr_failed;
	}

	nl_req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nl_req.nlh.nlmsg_flags = NLM_F_REQUEST;
	nl_req.nlh.nlmsg_type = RTM_GETROUTE;

	/* Set RTM message */
	p_rtm = NLMSG_DATA(&nl_req.nlh);
	p_rtm->rtm_family = AF_INET;

	/* Add RTA */
	p_rta = RTM_RTA(p_rtm);
	p_rta->rta_type = RTA_DST;
	p_rta->rta_len = RTA_LENGTH(sizeof(struct in_addr));
	memcpy(RTA_DATA(p_rta), target, sizeof(struct in_addr));
	nl_req.nlh.nlmsg_len += p_rta->rta_len;

	/* Send the request message */
	if (send(nl_sock, &nl_req, nl_req.nlh.nlmsg_len, 0) < 0) {
		perror("Write To Socket Failed...\n");
		goto _QuerySrcIpv4Addr_failed;
	}

	/* Retrieve result */
	{
		int nll, rtml;
		struct nlmsghdr*	p_nlh;

		p_rtm = NULL;
		p_rta = NULL;

		/* Receive response */
		nll = recv(nl_sock, &nl_resp, sizeof(nl_resp), 0);
		if (nll < 0) {
			perror("Receive From Socket Failed:");
			goto _QuerySrcIpv4Addr_failed;
		}

		for (p_nlh = (struct nlmsghdr *)&nl_resp.nlh; NLMSG_OK(p_nlh,nll); p_nlh = NLMSG_NEXT(p_nlh, nll)) {
			p_rtm = (struct rtmsg *) NLMSG_DATA(p_nlh);
			rtml = RTM_PAYLOAD(p_nlh);
			for (p_rta = RTM_RTA(p_rtm); RTA_OK(p_rta, rtml); p_rta = RTA_NEXT(p_rta, rtml)) {
				//DPRINT("rta_type = %d\n", p_rta->rta_type);
				if (p_rta->rta_type == RTA_PREFSRC) {
					memcpy(out, RTA_DATA(p_rta), sizeof(struct in_addr));
				}
			}
		}
	}
	close(nl_sock);

	return 0;

	_QuerySrcIpv4Addr_failed:
	if (nl_sock != -1) close(nl_sock);

	return -1;
}
