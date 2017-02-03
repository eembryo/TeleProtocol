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

#ifndef __XCALL_COMMON_UTILS_H__
#define __XCALL_COMMON_UTILS_H__

#include<map>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstring>
#include <gio/gio.h>
#include <glib/gi18n.h>


typedef struct CallStatus {
	guint status;
	guint callId;
	guint voiceStatus;
	guint voiceSource;
	guint messageStatus;
	guint buttonStatus;
	guint psapConfirmStatus;
	guint sbStatus;
} CallStatus_t;

static std::map<std::string, CallStatus_t> CALL_SCENARIO_MAPPING = {
	{"Initializing",   		   {1, 1, 0, 0, 0, 1, 1, 0}},
	{"Standby",				   {1, 1, 0, 0, 0, 2, 1, 1}},
	{"Connecting",	 		   {3, 1, 1, 0, 1, 2, 1, 0}},
	{"Connected",	  		   {3, 1, 3, 0, 2, 2, 1, 0}},
	{"Disconnected",   		   {3, 1, 8, 0, 2, 2, 1, 0}},
	{"Idle",		 		   {1, 2, 0, 0, 0, 3, 1, 0}},
	{"Incoming",	  		   {3, 1, 5, 0, 2, 2, 1, 0}},
	{"StandByCancel",	  	   {3, 2, 0, 0, 2, 2, 1, 2}},
	{"InitializingCancel",	   {3, 2, 0, 0, 2, 2, 1, 0}},
};


typedef struct RescueStatusData {
CallStatus_t ecallStatus;
CallStatus_t bcallStatus;
CallStatus_t icallStatus;
CallStatus_t sdncallStatus;
guint backupAudioStatus;
} RescueStatusData_t;

typedef struct CallEvent_t {
	char CallEventName[1024];
	RescueStatusData_t RescueStatusData;
} CallEvent_t;

typedef enum TEST_CALL_TYPE
{
	TEST_CALL_TYPE_NONE,
	TEST_CALL_TYPE_EMERGENCY_CALL,
	TEST_CALL_TYPE_BREAKDOWN_CALL,
	TEST_CALL_TYPE_INFORMATION_CALL,
	TEST_CALL_TYPE_SDN_CALL,
	TEST_CALL_TYPE_MAX,
} TEST_CALL_TYPE_T;

static std::map<int, std::string> TEST_CALL_TYPE_MAPPING = {
		{TEST_CALL_TYPE_NONE,"NoCall"},
		{TEST_CALL_TYPE_EMERGENCY_CALL,"EmergencyCall"},
		{TEST_CALL_TYPE_BREAKDOWN_CALL,"BreakdownCall"},
		{TEST_CALL_TYPE_INFORMATION_CALL,"InformationCall"},
		{TEST_CALL_TYPE_SDN_CALL,"SdnCall"},
		{TEST_CALL_TYPE_MAX,"NoCall"}
};

typedef struct TestEventCaseDetails {
	char callType[100];
	CallEvent_t CallEvent[20];
} TestEventCaseDetails_t;

static TestEventCaseDetails_t EventCaseInfo[] {

		// call Type Name
		"EmergencyCall",
		{

		{"Initializing", 	   CALL_SCENARIO_MAPPING["Initializing"], 		CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Standby",	 	   	   CALL_SCENARIO_MAPPING["Standby"],	 		 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connecting",  	   CALL_SCENARIO_MAPPING["Connecting"],  		 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Incoming",	 	   CALL_SCENARIO_MAPPING["Incoming"],	  		CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connected",		   CALL_SCENARIO_MAPPING["Connected"],	  		CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Disconnected", 	   CALL_SCENARIO_MAPPING["Disconnected"], 		CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"StandByCancel",	   CALL_SCENARIO_MAPPING["StandByCancel"],		CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"InitializingCancel", CALL_SCENARIO_MAPPING["InitializingCancel"], CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		},

		"BreakdownCall",
		{

		{"Initializing", 	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"],	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Standby",			   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"],			   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connecting",   	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],		   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Incoming",		   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],		   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connected",		   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],		   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Disconnected", 	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"],	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"StandByCancel", 	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["StandByCancel"],	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"InitializingCancel", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["InitializingCancel"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		},
		"InformationCall",
		{

		//{"Initializing", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		//{"Standby", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connecting",   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Incoming",	 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],	  CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Connected",	 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],	  CALL_SCENARIO_MAPPING["Idle"], 1 },
		{"Disconnected", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
		},

		"SdnCall",
		{

		//{"Initializing", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], 1 },
		//{"Standby", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"], 1 },
		{"Connecting",   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   1 },
		{"Incoming",	 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],     1 },
		{"Connected",	 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],    1 },
		{"Disconnected", CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], 1 },
		},

};

typedef struct CallScenario_t {
	char CallScenarioName[1024];
	int eventCount;
	RescueStatusData_t RescueStatusData[10];
} CallScenario_t;

typedef struct TestScenarioCaseDetails {
	char callType[100];
	CallScenario_t CallScenario[20];
} TestScenarioCaseDetails_t;

static TestScenarioCaseDetails_t ScenarioCaseInfo[] {
	// call Type Name
	"EmergencyCall",
	{
		"Connected->continuous", //call Scenario Name
			4, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Standby"],	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connected"],    CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connected->Disconnected",	//call Scenario Name
			5, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Standby"],	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connected"],    CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },

			},
		"Intitializing->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Initializing"], 		CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["InitializingCancel"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },

			},
		"StandBy->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Initializing"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Standby"],	   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["StandByCancel"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connecting->Disconnected",	//call Scenario Name
			4, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Standby"],	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },

			},
		"Incoming->continuous",	//call Scenario Name
			2, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Incoming"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incoming->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Incoming"],	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },

			},
		"Incomingconnected->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				 { CALL_SCENARIO_MAPPING["Incoming"], 	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Connected"],	CALL_SCENARIO_MAPPING["Idle"], 	 CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				 { CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },

			},
	},

	// call Type Name
	"BreakdownCall",
	{
		"Connected->continuous",	//call Scenario Name
			4,	//eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"],	  	CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],	CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connected->Disconnected",	//call Scenario Name
			5, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"],	  	CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],	CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Intitializing->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], 	  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["InitializingCancel"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"StandBy->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"],	    CALL_SCENARIO_MAPPING["Idle"],  CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["StandByCancel"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connecting->Disconnected",	//call Scenario Name
			4, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Initializing"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Standby"],	    CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incoming->continuous",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],  CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incoming->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],	    CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incomingconnected->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],     CALL_SCENARIO_MAPPING["Idle"],    CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],	CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
	},

	// call Type Name
	"InformationCall",
	{
		"Connected->continuous",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],    CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connected->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],    CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Connecting->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incoming->continuous",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],  CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incoming->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],	   CALL_SCENARIO_MAPPING["Idle"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
		"Incomingconnected->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],	 CALL_SCENARIO_MAPPING["Idle"],   1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],	CALL_SCENARIO_MAPPING["Idle"],    1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], CALL_SCENARIO_MAPPING["Idle"], 1 },
			},
	},
	// call Type Name
	"SdnCall",
	{
		"Connected->continuous",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"], 1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],  1 },
			},
		"Connected->Disconnected",	//call Scenario Name
			3, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],    1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], 1 },
			},
		"Connecting->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connecting"],   1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], 1 },
			},
		"Incoming->continuous",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],  1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"], 1 },
			},
		"Incoming->Disconnected",	//call Scenario Name
			2, //eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],     1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], 1 },
			},
		"Incomingconnected->Disconnected",	//call Scenario Name
			3,	//eventCount
			{
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Incoming"],     1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Connected"],    1 },
				{ CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Idle"], CALL_SCENARIO_MAPPING["Disconnected"], 1 },
			},
	},
};

#endif // __XCALL_COMMON_UTILS_H__
