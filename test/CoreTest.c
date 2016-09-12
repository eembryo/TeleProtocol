/*
 * CoreTest.c
 *
 *  Created on: Sep 8, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdCore.h"
#include "../include/IpcmdBus.h"
#include "../include/IpcmdTransportUdpv4.h"
#include "../include/IpcmdTransport.h"
#include <glib.h>

int main ()
{
	IpcmdCore 	*core;
	IpcmdBus	*bus;
	IpcmdTransport	*transport;
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);

	//IpcmdCoreInit (core, g_main_context_default());
	core = IpcmdCoreNew (g_main_context_default());
	bus = IpcmdCoreGetBus (core);

	transport = IpcmdTransportUdpv4New();
	transport->bind(transport,"192.168.0.10",50000);
	transport->listen(transport, 10);

	IpcmdBusAttachTransport (bus, transport);

	g_main_loop_run (loop);

}
