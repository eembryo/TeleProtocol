#include <SocketUtils.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <dprint.h>

uint32_t
GetIpv4AddressForNetInterface(int sockfd, const char *ifcname)
{
	struct ifreq ifr;
	uint32_t	ret;

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, ifcname, IFNAMSIZ-1);

	ioctl(sockfd, SIOCGIFADDR, &ifr);

	ret = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

	DPRINT("This address( %s ) is used for %s interface.\n", inet_ntoa(ret), ifcname);

	return ret;
}
