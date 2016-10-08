/*
 * ClientAgentTest.c
 *
 *  Created on: Oct 7, 2016
 *      Author: hyotiger
 */


#include "../include/IpcmdAgent.h"
#include <glib.h>

TransportDescUdpv4 udp_desc = {
		.link_type_ = kIpcmdAgentLinkUdpv4,
		.is_server_ = TRUE,
		.local_addr_ = "192.168.0.13",
		.local_port = 50000,
		.allow_broadcast_ = FALSE,
};

gint main()
{
	IpcmdAgent *agent;
	GMainLoop	*loop = g_main_loop_new (g_main_context_default(), FALSE);

	agent = IpcmdAgentNew (g_main_context_default());

	IpcmdAgentTransportAdd (agent, (TransportDesc*)&udp_desc);

}
