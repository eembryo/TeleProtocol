/*
 * IpcmdTransportTcpv4.h
 *
 *  Created on: Jan 25, 2017
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDTRANSPORTTCPV4_H_
#define INCLUDE_IPCMDTRANSPORTTCPV4_H_

#include "IpcmdDeclare.h"
#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _IpcmdTransportTcpv4 IpcmdTransportTcpv4;

IpcmdTransport*	IpcmdTransportTcpv4New();
void			IpcmdTransportTcpv4Destroy(IpcmdTransport *transport);

G_END_DECLS

#endif /* INCLUDE_IPCMDTRANSPORTTCPV4_H_ */
