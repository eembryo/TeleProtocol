#ifndef __IpcomTransportUDPv4_h__
#define __IpcomTransportUDPv4_h__

#include <IpcomTypes.h>
#include <ref_count.h>

#define UDPV4TRANSPORT_FROM_TRANSPORT(ptr)	container_of(ptr, IpcomTransportUDPv4, _transport)
#define TRANSPORT_FROM_UDPv4TRANSPORT(ptr)	(IpcomTransport *)(&ptr->_transport)

typedef struct _IpcomTransportUDPv4 IpcomTransportUDPv4;

IpcomTransport *IpcomTransportUDPv4New();
void IpcomTransportUDPv4Destroy(IpcomTransport *transport);
gboolean IpcomTransportUDPv4AttachGlibContext(IpcomTransportUDPv4 *transport, GMainContext *ctx);

#endif
