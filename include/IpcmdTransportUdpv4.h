/*
 * IpcmdTransportUdpv4.h
 *
 *  Created on: Sep 5, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDTRANSPORTUDPV4_H_
#define INCLUDE_IPCMDTRANSPORTUDPV4_H_

#include "IpcmdDeclare.h"
#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdTransportUdpv4 IpcmdTransportUdpv4;

IpcmdTransport*	IpcmdTransportUdpv4New();
void			IpcmdTransportUdpv4Destroy(IpcmdTransport *transport);
gboolean		IpcmdUdpv4EnableBroadcast(IpcmdTransport *transport, guint16 dst_port);
void			IpcmdUdpv4DisableBroadcast(IpcmdTransport *transport);

G_END_DECLS

#endif /* INCLUDE_IPCMDTRANSPORTUDPV4_H_ */
