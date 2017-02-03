
/*
 * @@@ LICENSE
 * Copyright (C) 2015-2016, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 * Design/Author : dhilipkumar.raman@lge.com, jinheung.tark@lgepartner.com
 * Date : 02/04/2016
 *
 * <IHU XCallService> Detailed note is as follows.
 *
 *
 */


#include <gio/gio.h>
#include<iostream>
#include<cstring>

#ifdef TIME_MEASURE
#include <time.h>
#endif

#include <xcall_manager.h>
#include <xcall_service_client.h>
#include <xcall_service_server.h>

#include <xcallservice_logger.h>

#ifdef SYSTEMD_NOTIFY
#include <systemd/sd-daemon.h>
#endif //SYSTEMD_NOTIFY

#ifdef DEBUG_ENABLED
#include "xcall_debug.h"
#endif

/**
 * ================================================================================
 * @fn : Main function
 * @brief : program startup function
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Register dlt logging context
 * - Create instances for various temxcallservice components
 * - Initialize IP, port, encoding details
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : Integer value returns the program status
 * ================================================================================
 */
gint main (gint argc, gchar *argv[])
{

#ifdef TIME_MEASURE
	clock_t tic = clock();
#endif

#ifdef DLT_LOG_ENABLE
	xCallLog::RegisterLogger(new xCallDLTLogger("TXCS", "telematics xcall service", "TXCS", "telematics xcall service context"));
#endif
	XCALLLOGI("[xCall] main ------ start \n");
	temxcall::XcallManager *callManager = new temxcall::XcallManager;
	temxcall::XCallServiceServer *svcServer = new temxcall::XCallServiceServer(callManager);
	temxcall::XCallServiceClient *svcClient = new temxcall::XCallServiceClient(callManager);

#ifdef DEBUG_ENABLED
	temxcall::xcall_debug_input input = {"198.18.50.97",50012,"198.18.50.1",50010,"PCMA"};
	XCALLLOGD("number of parameters = %d", argc);
	if (argc < 6) {
		XCALLLOGD("number of parameters are not enough. Default parameters used \n");
		input.destIP.assign("198.18.50.97");
		input.destPort1 = 50012;

		input.srcIP.assign("198.18.50.1");
		input.srcPort1 = 50010;
		input.encScheme.assign("PCMA");
		XCALLLOGI("ip details are Destination: %s %u & source: %s %u coding Scheme: %s \n", input.destIP.c_str(), input.destPort1, input.srcIP.c_str(), input.srcPort1, input.encScheme.c_str());
		callManager->setSourceInformation(input);
	} else {
		input.destIP.assign(argv[1]);
		input.destPort1 = atoi(argv[2]);
		input.srcIP.assign(argv[3]);
		input.srcPort1 = atoi(argv[4]);

		input.encScheme.assign(argv[5]);
		XCALLLOGI("ip details are Destination: %s %u & source: %s %u coding Scheme: %s \n", input.destIP.c_str(), input.destPort1, input.srcIP.c_str(), input.srcPort1, input.encScheme.c_str());
		callManager->setSourceInformation(input);
	}
#endif

	XCALLLOGI("[xCall] main ------ registerSkeletons mgr(%p) | server(%p) | client(%p) \n",callManager,svcServer,svcClient);
	svcServer->registerSkeletons();

	XCALLLOGD("[xCall] main ------ registerProxys \n");
	svcClient->registerProxys();

	XCALLLOGD("[xCall] main ------ registerServices \n");
	callManager->registerServices(svcServer,svcClient);

	XCALLLOGI("[xCall] main ------ mainServiceStart \n");

#ifdef SYSTEMD_NOTIFY
	int ret = sd_notify(0, "READY=1");
	XCALLLOGI("sd_notify with READY=1\n");
#endif //SYSTEMD_NOTIFY

#ifdef TIME_MEASURE
	XCALLLOGI("[TIME] set StartTime\n");
	callManager->setStartTime(tic);
#endif

	callManager->mainServiceStart();

	XCALLLOGI("[xCall] main ------ end \n");

  return 0;
}
