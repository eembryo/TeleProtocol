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

#include <iostream>
#include <string>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>

#include <xcall_service_server.h>
#include "xcall_error.h"


 namespace temxcall {

#define XCALL_PATH_NAME "com.lge.xcallsvc"
#define XCALL_OBJECT_NAME "com/lge/xcallsvc"

#ifdef GEELY_HARDWARE
	#define DBUS_TYPE G_BUS_TYPE_SYSTEM
#else
	#define DBUS_TYPE G_BUS_TYPE_SESSION
#endif

std::vector<std::pair<const char*, GCallback>> XCallServiceServer::testIfaceHandler_table = {
	{ "handle-calling",			G_CALLBACK(XCallServiceServer::Calling) },
	{ "handle-connected",		G_CALLBACK(XCallServiceServer::Connected) },
	{ "handle-end",				G_CALLBACK(XCallServiceServer::End) },
	{ "handle-unittest",		G_CALLBACK(XCallServiceServer::UnitTest) },
	{ "handle-send-err-info-notification",	G_CALLBACK(XCallServiceServer::sendErrorInfoNotification) },
	{ "handle_set_rescue_status",	G_CALLBACK(XCallServiceServer::HandleSetRescueStatus) }
};

std::vector<std::pair<const char*, GCallback>> XCallServiceServer::callFeatureHandler_table = {
	{ "handle-call-handling-set-request",		G_CALLBACK(XCallServiceServer::CallHandlingSetRequest) },
	{ "handle-service-activation-set-request",	G_CALLBACK(XCallServiceServer::ServiceActivationSetRequest) }
};

std::vector<std::pair<const char*,	GCallback>> XCallServiceServer::premiumAudioHandler_table = {
	{ "handle-set-request",			G_CALLBACK(XCallServiceServer::PremiumAudioSetRequest) },
	{ "handle-request",				G_CALLBACK(XCallServiceServer::PremiumAudioRequest) }
};

ComLgeXcallserviceTestInterface* XCallServiceServer::testIfaceSkel_obj = nullptr;
ComLgeXcallserviceCallFeature* XCallServiceServer::callFeatureSkel_Obj = nullptr;
ComLgeXcallservicePremiumAudio* XCallServiceServer::premiumAudioeSkel_Obj = nullptr;

int XCallServiceServer::mCallType = -1;
int XCallServiceServer::mCallId = -1;

/**
 * ================================================================================
 * @fn : XCallServiceServer
 * @brief : This is constructor function for XCallServiceServer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceServer::XCallServiceServer()
{
	XCALLLOGI("XCallServiceServer creator with no param is called \n");
	callManager_ = NULL;
	mainLoopId =0;
}

/**
 * ================================================================================
 * @fn : XCallServiceServer
 * @brief : This is constructor function for XCallServiceServer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @param[in] XcallManager : XcallManager instance
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceServer::XCallServiceServer(XcallManager *mgr)
{
	XCALLLOGI("XCallServiceServer creator with mgr param is called \n");
	callManager_ = mgr;
	mainLoopId =0;
	mCallType = -1;
	mCallId = -1;
}

/**
 * ================================================================================
 * @fn : ~XCallServiceServer
 * @brief : This is destructor function for XCallServiceServer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceServer::~XCallServiceServer()
{

}

/**
 * ================================================================================
 * @fn : Calling
 * @brief : This is a test stub function to simulate call initializing event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes callId and call Type by in parameters
 * - Emits the CallStatusQuery Signal with call type and call status
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] callinfo   : Call status
 * @param[in] callname   : Type of the call
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::Calling(ComLgeXcallserviceTestInterface *skeleton,
							 GDBusMethodInvocation *invocation,
							 char *callinfo, char *callname,
							 gpointer user_data)
{
	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;

	//call internal func

	XCALLLOGI("[xCall] DBUS API] Calling is called [%s : %s]\n",callinfo,callname);

	result = service->callManager_->startCall("Connecting");

	XCALLLOGD("callManager_->startCall() called \n");

	com_lge_xcallservice_test_interface_complete_calling(skeleton,invocation,result);

#ifdef DEBUG_ENABLED
	XCALLLOGD("callManager_->emit call status query() called \n");

	if (!strcmp(callname, "error")) {

		/* Error Test */
		if (!strcmp(callinfo, "100")) {
			service->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: No response");
		}
	} else {

		/* Call Test */
		if (!strcmp(callname, "emergencycall")) {
			mCallType = 1;
			mCallId = 1;
		} else if (!strcmp(callname, "breakdowncall")) {
			mCallType = 2;
			mCallId = 2;
		} else if (!strcmp(callname, "informationcall")) {
			mCallType = 3;
			mCallId = 3;
		} else if (!strcmp(callname, "sndcall")) {
			mCallType = 1;
			mCallId = 0;
		}

		if (!strcmp(callinfo, "incoming")) {
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Incoming");
		} else if (!strcmp(callinfo, "initial")) {
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Initializing");
		} else if (!strcmp(callinfo, "prepare")) {
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "StandBy");
		} else if (!strcmp(callinfo, "calling")) {
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Connecting");
		} else if (!strcmp(callinfo, "connected")) {
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Connected");
		} else {
			/* Disconnected */
			com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Disconnected");
		}
	}
#endif

	return true;
}

/**
 * ================================================================================
 * @fn : Connected
 * @brief : This is a test stub function to simulate call connected event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Emits the CallStatusQuery Signal with call type and call status
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::Connected(ComLgeXcallserviceTestInterface *skeleton,
							 GDBusMethodInvocation *invocation,
							 gpointer user_data)
{

	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;

	//call internal func

	XCALLLOGI("[xCall] DBUS API] Connected is called \n");

	com_lge_xcallservice_test_interface_complete_connected(skeleton,invocation,result);

#ifdef DEBUG_ENABLED
	com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Connected");
#endif

	XCALLLOGD("callManager_->conntectedCall() called \n");
	if (service->callManager_->conntectedCall()) {
		//com_lge_xcallsvc_emit_call_status_query(skeleton_, 0, 0, "connected");
	}

	return true;
}

/**
 * ================================================================================
 * @fn : End
 * @brief : This is a test stub function to simulate call disconnected event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Emits the CallStatusQuery Signal with call type and call status
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::End(ComLgeXcallserviceTestInterface *skeleton,
							 GDBusMethodInvocation *invocation,
							 gpointer user_data)
{
	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;

#ifdef ENABLE_UNITTEST_ASK_MMUS_API

	g_print("[xCall] DBUS API] End is called \n");
	service->callManager_->endCall();

	com_lge_xcallservice_test_interface_complete_end(skeleton,invocation,result);
#else
	//call internal func

	XCALLLOGI("[xCall] DBUS API] End is called \n");

	g_fprintf(stderr, "fprintf --> [xCall] DBUS API] End is called \n");

	service->callManager_->endCall();
	XCALLLOGD("callManager_->endCall() called \n");

	com_lge_xcallservice_test_interface_complete_end(skeleton,invocation,result);
#ifdef DEBUG_ENABLED
	com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj, mCallId, mCallType, "Disconnected");
	mCallType = -1;
	mCallId = -1;
#endif
#endif
	return true;
}

/**
 * ================================================================================
 * @fn : UnitTest
 * @brief : This is a test stub function to simulate unit test cases
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::UnitTest(ComLgeXcallserviceTestInterface *skeleton,
							 GDBusMethodInvocation *invocation, char *testName, char *param1, char *param2,
							 gpointer user_data)
{
	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;

	XCALLLOGI("[xCall-Server] UnitTest is called \n");

	com_lge_xcallservice_test_interface_complete_end(skeleton,invocation,result);

	service->callManager_->unitTest(testName, param1, param2);
	return true;
}

/**
 * ================================================================================
 * @fn : sendErrorInfoNotification
 * @brief : This is a test stub function to simulate error id and error cases
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] errId      : error value id (integer)
 * @param[in] errMsg     : error value string information
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::sendErrorInfoNotification(ComLgeXcallserviceTestInterface *skeleton,
							 GDBusMethodInvocation *invocation, guint errId, char *errMsg,
							 gpointer user_data)
{
	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;
	XCALLLOGI("[xCall] DBUS API] sendErrorInfo is called \n");

	service->NotifyErrorInformation(errMsg, errId);

	com_lge_xcallservice_test_interface_complete_send_err_info_notification(skeleton,invocation,result);

	return true;
}

/**
 * ================================================================================
 * @fn : HandleSetRescueStatus
 * @brief : This is a test stub function to simulate rescue status and simluate the call sequence
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::HandleSetRescueStatus (ComLgeXcallserviceTestInterface *skeleton,
		 GDBusMethodInvocation *invocation,
		 guint arg_eRescueStatus, guint arg_eXCallID, guint arg_eVoiceStatus, guint arg_eSourceStatus, guint arg_eMessageStatus, guint arg_eButtonStatus, guint arg_ePSAPStatus, guint arg_eStandbyStatus,
		 guint arg_bRescueStatus, guint arg_bXCallID, guint arg_bVoiceStatus, guint arg_bSourceStatus, guint arg_bMessageStatus, guint arg_bButtonStatus, guint arg_bPSAPStatus, guint arg_bStandbyStatus,
		 guint arg_iRescueStatus, guint arg_iXCallID, guint arg_iVoiceStatus, guint arg_iSourceStatus, guint arg_iMessageStatus, guint arg_iButtonStatus, guint arg_iPSAPStatus, guint arg_iStandbyStatus,
		 guint arg_sRescueStatus, guint arg_sXCallID, guint arg_sVoiceStatus, guint arg_sSourceStatus, guint arg_sMessageStatus, guint arg_sButtonStatus, guint arg_sPSAPStatus, guint arg_sStandbyStatus,
		 guint arg_backupAudioStatus,
		 gpointer user_data)
{
	XCallServiceServer *service = (XCallServiceServer *)user_data;
	bool result = true;
	XCALLLOGI("[xCall] DBUS API] sendErrorInfo is called \n");
	uint8_t arg_eCallsessionNum = 0, arg_bCallsessionNum = 0, arg_iCallsessionNum = 0, arg_sCallsessionNum = 0;

	GVariant *arg_eCallStatus, *arg_bCallStatus, *arg_iCallStatus, *arg_sdnStatus;
	arg_eCallStatus = g_variant_new("(uuyuuuuuu)", arg_eRescueStatus, arg_eXCallID, arg_eCallsessionNum, arg_eVoiceStatus, arg_eSourceStatus, arg_eMessageStatus, arg_eButtonStatus, arg_ePSAPStatus, arg_eStandbyStatus);
	arg_bCallStatus = g_variant_new("(uuyuuuuuu)", arg_bRescueStatus, arg_bXCallID, arg_bCallsessionNum, arg_bVoiceStatus, arg_bSourceStatus, arg_bMessageStatus, arg_bButtonStatus, arg_bPSAPStatus, arg_bStandbyStatus);
	arg_iCallStatus = g_variant_new("(uuyuuuuuu)", arg_iRescueStatus, arg_iXCallID, arg_iCallsessionNum, arg_iVoiceStatus, arg_iSourceStatus, arg_iMessageStatus, arg_iButtonStatus, arg_iPSAPStatus, arg_iStandbyStatus);
	arg_sdnStatus = g_variant_new("(uuyuuuuuu)", arg_sRescueStatus, arg_sXCallID, arg_sCallsessionNum, arg_sVoiceStatus, arg_sSourceStatus, arg_sMessageStatus, arg_sButtonStatus, arg_sPSAPStatus, arg_sStandbyStatus);

	XCALLLOGD("E-Call: %d, %d, %d, %d, %d, %d, %d, %d, %d \n", arg_eRescueStatus, arg_eXCallID, arg_eCallsessionNum, arg_eVoiceStatus, arg_eSourceStatus, arg_eMessageStatus, arg_eButtonStatus, arg_ePSAPStatus, arg_eStandbyStatus);
	XCALLLOGD("B-Call: %d, %d, %d, %d, %d, %d, %d, %d, %d \n", arg_bRescueStatus, arg_bXCallID, arg_bCallsessionNum, arg_bVoiceStatus, arg_bSourceStatus, arg_bMessageStatus, arg_bButtonStatus, arg_bPSAPStatus, arg_bStandbyStatus);
	XCALLLOGD("I-Call: %d, %d, %d, %d, %d, %d, %d, %d, %d \n", arg_iRescueStatus, arg_iXCallID, arg_iCallsessionNum, arg_iVoiceStatus, arg_iSourceStatus, arg_iMessageStatus, arg_iButtonStatus, arg_iPSAPStatus, arg_iStandbyStatus);
	XCALLLOGD("SDN-Call: %d, %d, %d, %d, %d, %d, %d, %d, %d \n", arg_sRescueStatus, arg_sXCallID, arg_sCallsessionNum, arg_sVoiceStatus, arg_sSourceStatus, arg_sMessageStatus, arg_sButtonStatus, arg_sPSAPStatus, arg_sStandbyStatus);
	service->callManager_->notifyRescueStatusEvent(arg_eCallStatus, arg_bCallStatus, arg_iCallStatus, arg_sdnStatus, arg_backupAudioStatus);

	com_lge_xcallservice_test_interface_complete_set_rescue_status(skeleton,invocation,result);

	return true;
}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequest
 * @brief : Request function for call handling API in CallFeature interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] arg_action <0 = acceptCall, 1 = hangupCall> : accept or decline call
 * @param[in] arg_callId <1-4> : ongoing call id
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::CallHandlingSetRequest (ComLgeXcallserviceCallFeature *object,
											GDBusMethodInvocation *invocation,
											guint arg_action,
											guint arg_callId,
											gpointer user_data) {

	XCallServiceServer *service = (XCallServiceServer *)user_data;

	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->skeleton = (gpointer)object;
	userData->invocation = (gpointer)invocation;
	userData->callbackData = (gpointer)user_data;

	XCALLLOGI("CallHandlingSetRequest() arg_action=%d arg_callId=%d\n", arg_action, arg_callId);

	service->callManager_->CallHandlingSetRequest(arg_action, (guint8) arg_callId,
													nullptr,
													userData);

	return true;
}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequestResponseCb
 * @brief : Response function for call handling API in CallFeature interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Parses the result data
 * - Notifies if any error on call handling API
 * @param[in] err        : DBUS error inforamtion
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceServer::CallHandlingSetRequestResponseCb (GError *err, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	GDBusMethodInvocation *invocation = (GDBusMethodInvocation *)userData->invocation;
	ComLgeXcallserviceCallFeature *skeleton = (ComLgeXcallserviceCallFeature *)userData->skeleton;
	XCallServiceServer *service = (XCallServiceServer *)userData->callbackData;

	XCALLLOGI("CallHandlingSetRequest ResponseCb\n");
	if (err != nullptr ) {
		XCALLLOGE(" error = %s...\n", err->message);

		service->NotifyErrorInformation(err->message);
	}

	com_lge_xcallservice_call_feature_complete_call_handling_set_request(skeleton, invocation);
}

/**
 * ================================================================================
 * @fn : ServiceActivationSetRequest
 * @brief : Request function for service activation API in CallFeature interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] skeleton   : DBUS skeleton object for Test interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] arg_service <2 = InformationCall> : accept or decline call
 * @param[in] arg_action <1=Off 2-On> : service activation on or off
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::ServiceActivationSetRequest (ComLgeXcallserviceCallFeature *object,
												GDBusMethodInvocation *invocation,
												guint arg_service,
												guint arg_action,
												gpointer user_data) {

	XCallServiceServer *service = (XCallServiceServer *)user_data;

	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->skeleton = (gpointer)object;
	userData->invocation = (gpointer)invocation;
	userData->callbackData = (gpointer) user_data;

	XCALLLOGI("ServiceActivationSetRequest() arg_service=%d arg_action=%d\n", arg_service, arg_action);

	service->callManager_->ServiceActivationSetRequest(arg_service, arg_action,
														nullptr,
														userData);

	return true;
}

/**
 * ================================================================================
 * @fn : ServiceActivationSetResponseCb
 * @brief : Response function for Service Activation API in CallFeature interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Parses the result data
 * - Notifies if any error on Service Activation API
 * @param[in] err        : DBUS error inforamtion
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceServer::ServiceActivationSetResponseCb (GError *err, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	GDBusMethodInvocation *invocation = (GDBusMethodInvocation *)userData->invocation;

	ComLgeXcallserviceCallFeature *skeleton = (ComLgeXcallserviceCallFeature *)userData->skeleton;
	XCallServiceServer *service = (XCallServiceServer *)userData->callbackData;

	XCALLLOGI("ServiceActivationSetResponseCb\n");
	if (err != nullptr ) {
		XCALLLOGE("error = %s...\n", err->message);

		service->NotifyErrorInformation(err->message);
	}

	com_lge_xcallservice_call_feature_complete_service_activation_set_request(skeleton, invocation);
}

/**
 * ================================================================================
 * @fn : PremiumAudioRequest
 * @brief : Request function handler in PremiumAudio interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] object     : DBUS skeleton object for PremiumAudio interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::PremiumAudioRequest (ComLgeXcallservicePremiumAudio *object,
										GDBusMethodInvocation *invocation,
										gpointer user_data) {

	GVariant* premiumAudioStatus;
	XCallServiceServer *service = (XCallServiceServer *)user_data;

	guint spkStatus, micStatus;

	service->callManager_->UpdatePremiumAudioDeviceStatus(DBUS_CALL_DIRECTION_TO_MMUS);
	service->callManager_->GetPremiumAudioMICStatus(&micStatus);
	service->callManager_->GetPremiumAudioSPKStatus(&spkStatus);

	XCALLLOGI("PremiumAudioRequest is called with spk:%d, mic:%d \n", spkStatus, micStatus);

	premiumAudioStatus = g_variant_new ("(uu)", micStatus, spkStatus);

	com_lge_xcallservice_premium_audio_complete_request(object, invocation, premiumAudioStatus);

	return true;
}

/**
 * ================================================================================
 * @fn : PremiumAudioSetRequest
 * @brief : SetRequest function handler PremiumAudio interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - starts the unit test function of manager object
 * @param[in] object     : DBUS skeleton object for PremiumAudio interface
 * @param[in] invocation : DBUS invocation object
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::PremiumAudioSetRequest (ComLgeXcallservicePremiumAudio *object,
											GDBusMethodInvocation *invocation,
											gboolean arg_isRequested,
											gpointer user_data) {

	XCALLLOGD("PremiumAudioSetRequest() requested=%d \n", arg_isRequested);

	XCallServiceServer *service = (XCallServiceServer *)user_data;
	GVariant* premiumAudioStatus;
	guint spkStatus, micStatus;

	service->callManager_->UpdatePremiumAudioDeviceStatus(DBUS_CALL_DIRECTION_TO_MMUS);
	service->callManager_->GetPremiumAudioMICStatus(&micStatus);
	service->callManager_->GetPremiumAudioSPKStatus(&spkStatus);
	XCALLLOGI("PremiumAudioSetRequest is called with spk:%d, mic:%d isRequest:%d \n", spkStatus, micStatus, arg_isRequested);

	premiumAudioStatus = g_variant_new ("(uu)", micStatus,spkStatus);

	com_lge_xcallservice_premium_audio_complete_set_request(object, invocation, premiumAudioStatus);

	return true;
}

/**
 * ================================================================================
 * @fn : NotifyPremiumAudioEvent
 * @brief : Handler function for Norification event API in PremiumAudio interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Creates the gvariant object with the premium audio status details
 * - Emits the Notification event of Premium Audio interface
 * @param[in] spkStatus : premium audio speaker status
 * @param[in] micStatus : premium audio mic status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::NotifyPremiumAudioEvent(guint spkStatus, guint micStatus)
{
	GVariant* premiumAudioStatus;

	XCALLLOGI("NotifyPremiumAudioEvent() spkStatus=%d micStatus=%d \n", spkStatus, micStatus);

	premiumAudioStatus = g_variant_new ("(uu)", micStatus,spkStatus);
	com_lge_xcallservice_premium_audio_emit_notification (premiumAudioeSkel_Obj, premiumAudioStatus);

	return true;
}

/**
 * ================================================================================
 * @fn : NotifyCallStatusQueryEvent
 * @brief : Handler function for Rescue status event API in CallFeature interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Emits the Call Status Query event with call details
 * @param[in] callid      : Call id information of on going call
 * @param[in] calltype    : Type of the call
 * @param[in] callStatus  : call status string
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::NotifyCallStatusQueryEvent(guint callid, guint calltype, const gchar *callStatus ) {

	XCALLLOGI("NotifyCallStatusQueryEvent() callid=%d calltype=%d callStatus=%s\n", callid, calltype, callStatus);
	com_lge_xcallservice_call_feature_emit_call_status_query(callFeatureSkel_Obj,callid,calltype,callStatus);

	return true;
}

/**
 * ================================================================================
 * @fn : NotifyErrorInformation
 * @brief : Handler function for convert the DBUS error from TEMIL to temxcallerror
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Parses the dbus error information
 * - converts the DBUS error of TEMIL to temxcall error equivalent
 * @param[in] errMsg  : Call id information of on going call
 * @param[in] errVal  : error information string
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::NotifyErrorInformation(const gchar *errMsg, guint errVal) {

	std::string errStr(errMsg);
	guint errId = 0;

	if (!errVal)
		errId = getXcallErrCode(errStr);
	else
		errId = errVal;

	XCALLLOGI("XCallServiceServer::NotifyCallErrorinformation() errId=%d errStr=%s\n", errId, errStr.c_str());

	com_lge_xcallservice_call_feature_emit_notify_err_info(callFeatureSkel_Obj, errId, errStr.c_str());

	return true;
}

/**
 * ================================================================================
 * @fn : BusNameAcquiredCB
 * @brief : Response call back function for DBUS service registration
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Exports/Registers each interface objects with handler function details on DBUS
 * @param[in] connection : Dbus connection instance
 * @param[in] name       : service name successfully registered on DBUS
 * @param[in] user_data  : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
void XCallServiceServer::BusNameAcquiredCB (GDBusConnection *connection,
									const gchar  *name, gpointer user_data)
{

	XCallServiceServer *service = (XCallServiceServer *)user_data;

	XCALLLOGI("[xCall] Acquired the name %s\n", name);


	//register skeleton for test interface
	testIfaceSkel_obj = com_lge_xcallservice_test_interface_skeleton_new();

	for(auto& handler : service->testIfaceHandler_table)
		g_signal_connect(testIfaceSkel_obj,std::get<0>(handler), std::get<1>(handler), user_data);

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(testIfaceSkel_obj),
									 connection,
									 "/com/lge/xcallservice/testInterface",
									 NULL);



	//register skeleton for call feature
	callFeatureSkel_Obj = com_lge_xcallservice_call_feature_skeleton_new();

	for(auto& handler : service->callFeatureHandler_table)
		g_signal_connect(callFeatureSkel_Obj,std::get<0>(handler), std::get<1>(handler), user_data);

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(callFeatureSkel_Obj),
									connection,
									"/com/lge/xcallservice/callfeature",
									NULL);

	//register skeleton for premium audio
	premiumAudioeSkel_Obj = com_lge_xcallservice_premium_audio_skeleton_new();

	for(auto& handler : service->premiumAudioHandler_table)
		g_signal_connect(premiumAudioeSkel_Obj,std::get<0>(handler), std::get<1>(handler), user_data);

	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(premiumAudioeSkel_Obj),
									connection,
									"/com/lge/xcallservice/premiumAudio",
									NULL);

#ifdef TIME_MEASURE
	XCALLLOGI("[TIME] finish DBUS registration. \n");
	service->callManager_->measureElapseTime();
#endif

}

/**
 * ================================================================================
 * @fn : registerSkeletons
 * @brief : Request function for DBUS service registration
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Service name is requested to DBUS
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceServer::registerSkeletons()
{
	bool ret =true;

	XCALLLOGI("[xCall] registerSkeletons bus(%d,%d) (server)%p | (mgr)%p [com.lge.xcallservice] \n",DBUS_TYPE,G_BUS_TYPE_SYSTEM, this, callManager_);

	mainLoopId = g_bus_own_name(DBUS_TYPE,
						"com.lge.xcallservice",
						G_BUS_NAME_OWNER_FLAGS_NONE,
						NULL,
						BusNameAcquiredCB,
						NULL,
						this,
						NULL);

	XCALLLOGD("[xCall] registerSkeletons : mainLoopId:%d \n",mainLoopId);

	return ret;
}

}// namespace temxcall

