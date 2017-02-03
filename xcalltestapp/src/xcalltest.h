/*
 * @@@ LICENSE
 * Copyright (C) 2016-2017, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 # Design/Author : manisha.manisha@lge.com, dhilipkumar.raman@lge.com
 # Date : Oct. 24, 2016
 *
 * XCALL TEST APP Main Implementation
 *
 */

#ifndef __XCALL_TEST_H__
#define __XCALL_TEST_H__

#include "xcallcommonutils.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <cstring>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <queue>

using namespace std;

typedef struct paramsData {
	GValue data;
} paramsData;

typedef struct resultData {
	gchar *result;
	int error_t;
} resultData;

class XCallTest {
public:

	XCallTest();
	~XCallTest();

	static gchar *opt_call_dest ;
	static gchar *opt_call_object_path ;
	static gchar *opt_call_method ;
	static gint opt_call_timeout ;
	static const GOptionEntry call_entries[];

	void TestEvent();
	void TestScenario();
	void ChangeTimer();
	void ConnectedCall (int callType, int event, int eventCount);
	void ConnectedAndDisconnectCall (int callType, int event, int eventCount);
	void IntializeAndDisconnectCall (int callType, int event, int eventCount);
	void StandbyAndDisconnectCall (int callType, int event, int eventCount);
	void ConnectingAndDisconnectcall (int callType, int event, int eventCount);
	void IncomingCall (int callType, int event, int eventCount);
	void IncomingAndDisconnectCall (int callType, int event, int eventCount);
	void AnsweredAndDisconnectCall (int callType, int event, int eventCount);
	void CreateCallScenarioAndSend (int callType, int event, int dataIndex);
	//void ListAPI();

	resultData *SendToDbus(gchar *opt_call_dest1, gchar *opt_call_object_path1,
							gchar *interface_name, gchar *method_name, GVariant **parameters);

	//static CALL_TYPE_T callType;
	static int initializingTimer;
	static int standbyTimer;
	static int connectingTimer;
	static int incomingConnectingTimer;
	static int disconnectingTimer;
	static RescueStatusData_t previousData;
};

#endif // __XCALL_TEST_H__
