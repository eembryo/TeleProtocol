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


#include <xcall_service_client.h>

namespace temxcall {


#ifdef GEELY_HARDWARE
	#define DBUS_TYPE G_BUS_TYPE_SYSTEM
#else
	#define DBUS_TYPE G_BUS_TYPE_SESSION
#endif

ComLgeTemilCommonPhoneCallHandling * XCallServiceClient::CallHandlingProxy_Obj = nullptr;
ComLgeTemilTelematicsRescueStatus * XCallServiceClient::ResqueStausProxy_Obj = nullptr;
ComLgeTemilTelematicsServiceActivation * XCallServiceClient::ServiceActivationProxy_Obj = nullptr;
ComLgeTemilConnectivitySOSStatus *XCallServiceClient::SOSStatusProxy_Obj = nullptr;

OrgGeniviAudiomanagerCommandinterface *XCallServiceClient::AudioMgrProxy_Obj=nullptr;

ComLgeCarMicomIhu *XCallServiceClient::mmusDeviceStatusCheckProxy_Obj=nullptr;
ComLgeCarMicomCem *XCallServiceClient::mmusCemProxy_Obj=nullptr;
ComLgeGeelyCarConfigProviderInterface *XCallServiceClient::vehicleInfoDaemonProxy_Obj=nullptr;

/**
 * ================================================================================
 * @fn : XCallServiceClient
 * @brief : This is constructor function for XCallServiceClient class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the member variables
 * @param[in] XcallManager : XcallManager instance
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceClient::XCallServiceClient(XcallManager *mgr)
{
	XCALLLOGI("XCallServiceClient creator with mgr param is called \n");
	callManager_ = mgr;
	audioConnectionId = 0;
	bus_type = G_BUS_TYPE_SYSTEM;
}

/**
 * ================================================================================
 * @fn : XCallServiceClient
 * @brief : This is constructor function for XCallServiceClient class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the member variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceClient::XCallServiceClient()
{
	XCALLLOGI("XCallServiceClient creator with no param is called \n");

	callManager_ = NULL;
	audioConnectionId = 0;
	bus_type = G_BUS_TYPE_SYSTEM;
}

/**
 * ================================================================================
 * @fn : ~XCallServiceClient
 * @brief : This is destructor function for XCallServiceServer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
XCallServiceClient::~XCallServiceClient()
{

}

/**
 * ================================================================================
 * @fn : clientCalling
 * @brief : This is dummy function for test stub in Test interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::clientCalling()
{
	XCALLLOGD("[Client ] clientCalling is called \n");

	return true;
}

/**
 * ================================================================================
 * @fn : clientConnected
 * @brief : This is dummy function for test stub in Test interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::clientConnected()
{
	XCALLLOGD("[Client ] clientCalling is called \n");

	return true;
}

/**
 * ================================================================================
 * @fn : clientEnd
 * @brief : This is dummy function for test stub in Test interface
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::clientEnd()
{
	XCALLLOGD("[Client ] clientCalling is called \n");

	return true;
}

/**
 * ================================================================================
 * @fn : registerProxys
 * @brief : This function request all the proxy object handlers from DBUS
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes all proxy objects using auto generated code dbus functions
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::registerProxys()
{
	bool ret = true;
	GError * error = NULL;

	XCALLLOGD("[Client ] registerProxys (client)%p | (mgr)%p \n",this,callManager_);

	CallHandlingProxy_Obj = com_lge_temil_common_phone_call_handling_proxy_new_for_bus_sync(DBUS_TYPE,
																							G_DBUS_PROXY_FLAGS_NONE,
																							"com.lge.temil",
																							"/com/lge/temil/CommonPhone",
																							NULL,
																							&error);
	if (!CallHandlingProxy_Obj) {
		XCALLLOGE("CallHandling Error: %s\n", error->message);
		g_error_free (error);
	}

	ResqueStausProxy_Obj = com_lge_temil_telematics_rescue_status_proxy_new_for_bus_sync(DBUS_TYPE,
																						G_DBUS_PROXY_FLAGS_NONE,
																						"com.lge.temil",
																						"/com/lge/temil/Telematics",
																						NULL,
																						&error);
	if (!ResqueStausProxy_Obj) {
		XCALLLOGE("ResqueStausProxy Error: %s\n", error->message);
		g_error_free (error);
	} else {
		XCALLLOGD("[Client ] signal notification to temil with (client)%p  \n",this);
		g_signal_connect(ResqueStausProxy_Obj,"notification", G_CALLBACK(XCallServiceClient::handleRescueStatusNotification), (gpointer)this);
	}

	ServiceActivationProxy_Obj = com_lge_temil_telematics_service_activation_proxy_new_for_bus_sync (DBUS_TYPE,
																									G_DBUS_PROXY_FLAGS_NONE,
																									"com.lge.temil",
																									"/com/lge/temil/Telematics",
																									NULL,
																									&error);
	if (!ServiceActivationProxy_Obj) {
		XCALLLOGE("ServiceActivation Error: %s\n", error->message);
		g_error_free (error);
	}

	SOSStatusProxy_Obj = com_lge_temil_connectivity_sosstatus_proxy_new_for_bus_sync (DBUS_TYPE,
																					G_DBUS_PROXY_FLAGS_NONE,
																					"com.lge.temil",
																					"/com/lge/temil/Connectivity",
																					NULL,
																					&error);
	if (!SOSStatusProxy_Obj) {
		XCALLLOGE("SOS ConnectionStatus Error: %s\n", error->message);
		g_error_free (error);
	}

	AudioMgrProxy_Obj = org_genivi_audiomanager_commandinterface_proxy_new_for_bus_sync (DBUS_TYPE,
																						G_DBUS_PROXY_FLAGS_NONE,
																						"org.genivi.audiomanager",
																						"/org/genivi/audiomanager/commandinterface",
																						NULL,
																						&error);
	if (!AudioMgrProxy_Obj) {
		XCALLLOGE("audioManagerProxy Error: %s\n", error->message);
		g_error_free (error);
	} else {
		XCALLLOGI("register signal with audio mgr - removedMainConnection \n");
		g_signal_connect(AudioMgrProxy_Obj, "removed-main-connection", G_CALLBACK(XCallServiceClient::handleAudioMangerSignal), (gpointer)this);
	}

	//interface with mmus
	mmusDeviceStatusCheckProxy_Obj = com_lge_car_micom_ihu_proxy_new_for_bus_sync(DBUS_TYPE,
																G_DBUS_PROXY_FLAGS_NONE,
																"com.lge.car.micom",
																"/Ihu",
																NULL,
																&error);

	if (!mmusDeviceStatusCheckProxy_Obj) {
		XCALLLOGE("mmusProxy Error: %s\n", error->message);
		g_error_free (error);
	} else {
		XCALLLOGI("register signal with mmus(Ihu) - notifyCarIfStatus \n");
		g_signal_connect(mmusDeviceStatusCheckProxy_Obj,"notify-car-ifstatus",G_CALLBACK(XCallServiceClient::handleMMUSDeviceStatusSignal),(gpointer)this);
	}

	mmusCemProxy_Obj = com_lge_car_micom_cem_proxy_new_for_bus_sync(DBUS_TYPE,
															G_DBUS_PROXY_FLAGS_NONE,
															"com.lge.car.micom",
															"/Cem",
															NULL,
															&error);
	if(!mmusCemProxy_Obj) {
		XCALLLOGE("mmusProxy Error: %s\n", error->message);
		g_error_free (error);
	}else {
		XCALLLOGI("register signal with mmus(Cem) - notifyEmgyAsscSts, notifyTelmSts \n");
		g_signal_connect(mmusCemProxy_Obj,"notify-emgy-assc-sts",G_CALLBACK(XCallServiceClient::handleCemEmgyAsscStsNotification),(gpointer)this);
		g_signal_connect(mmusCemProxy_Obj,"notify-telm-sts",G_CALLBACK(XCallServiceClient::handleCemTelmStsNotification),(gpointer)this);
	}

	vehicleInfoDaemonProxy_Obj = com_lge_geely_car_config_provider_interface_proxy_new_for_bus_sync(DBUS_TYPE,
															G_DBUS_PROXY_FLAGS_NONE,
															"com.lge.car.geely.carconfigprovider",
															"/carconfigprovider",
															NULL,
															&error);
	if(!vehicleInfoDaemonProxy_Obj) {
		XCALLLOGE("vehicleInfoDaemonProxy_Obj Error: %s\n", error->message);
		g_error_free (error);
	} else {
		XCALLLOGI("register signal with vehicle info daemon - notifyUpdateCarConfigData\n");
		g_signal_connect(vehicleInfoDaemonProxy_Obj,"notify-update-car-config-data",G_CALLBACK(XCallServiceClient::handleCarConfigNotification),(gpointer)this);
	}
	return ret;
}

/**
 * ================================================================================
 * @fn : handleRescueStatusNotification
 * @brief : Call back handler function connected with the Resque status SIGNAL of
 *          Telematics interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the rescue status signal received data to Xcall manager
 * @param[in] object                : DBUS proxy object for Rescue status interface
 * @param[in] arg_eCallStatus       : Emergency call status
 * @param[in] arg_bCallStatus       : Breakdown call status
 * @param[in] arg_iCallStatus       : information call status
 * @param[in] arg_sdnStatus         : SDN call status
 * @param[in] arg_backupAudioStatus : Backup audio devices status
 * @param[in] user_data             : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
void XCallServiceClient::handleRescueStatusNotification (ComLgeTemilTelematicsRescueStatus *object,
															GVariant *arg_eCallStatus,
															GVariant *arg_bCallStatus,
															GVariant *arg_iCallStatus,
															GVariant *arg_sdnStatus,
															guint arg_backupAudioStatus,
															gpointer user_data)
{

	XCallServiceClient *service = (XCallServiceClient *)user_data;


	XCALLLOGI("handleRescueStatusNotification \n");

	service->callManager_->notifyRescueStatusEvent(arg_eCallStatus, arg_bCallStatus, arg_iCallStatus, arg_sdnStatus, arg_backupAudioStatus);

}

/**
 * ================================================================================
 * @fn : ServiceActivationSetRequest
 * @brief : Request function calls the ServiceActivation API of
 *          Telematics interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the service activation to TEMIL service using dbus auto generated code
 * @param[in] arg_service <2 = InformationCall> : accept or decline call
 * @param[in] arg_action <1=Off 2-On>           : service activation on or off
 * @param[in] callback                          : asynchronous call back function
 * @param[in] user_data                         : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::ServiceActivationSetRequest (guint arg_service, guint arg_action,
									GAsyncReadyCallback callback, gpointer user_data)
{
	XCALLLOGI("ServiceActivationSetRequest() arg_service=%d arg_action=%d\n", arg_service, arg_action);

	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->proxy = (gpointer)CallHandlingProxy_Obj;
	userData->object = this;
	userData->callbackData = user_data;
	com_lge_temil_telematics_service_activation_call_set_request(ServiceActivationProxy_Obj,
																arg_service, arg_action,
																NULL,
																&XCallServiceClient::ServiceActivationSetResponseCb,
																userData);

}

/**
 * ================================================================================
 * @fn : ServiceActivationSetResponseCb
 * @brief : Response function called on result of the ServiceActivation API of
 *          Telematics interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the service activation API results received to Xcall manager
 * @param[in] source_object : Dbus proxy object
 * @param[in] res           : asynchronous result information from temil
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::ServiceActivationSetResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	XCallServiceClient *client = (XCallServiceClient *)userData->object;
	ComLgeTemilTelematicsServiceActivation *proxy = (ComLgeTemilTelematicsServiceActivation *)userData->proxy;
	gpointer callbackData = userData->callbackData;
	XCALLLOGI("XCallServiceClient::ServiceActivationSetResponseCb\n");

	 GError *err = nullptr;

	 com_lge_temil_telematics_service_activation_call_set_request_finish(proxy, res, &err);

	if (err != nullptr )
	{
		XCALLLOGE("ServiceActivationSetResponseCb error = %s\n", err->message);
	}
	client->callManager_->ServiceActivationSetResponseCb(err, callbackData);
}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequest
 * @brief : Request function calls the Call handling API of
 *          Common Phone interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the service activation to TEMIL service using dbus auto generated code
 * @param[in] arg_action <0 = acceptCall, 1 = hangupCall> : accept or decline call
 * @param[in] arg_callId <1-4>                            : ongoing call id
 * @param[in] callback                                    : asynchronous call back function
 * @param[in] user_data                                   : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::CallHandlingSetRequest (guint arg_action, guint8 arg_callId,
									GAsyncReadyCallback callback, gpointer user_data)
{

	XCALLLOGI("CallHandlingSetRequest() arg_action=%d arg_callId=%d\n", arg_action, arg_callId);

	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->proxy = (gpointer)CallHandlingProxy_Obj;
	userData->object = this;
	userData->callbackData = user_data;
	com_lge_temil_common_phone_call_handling_call_set_request(CallHandlingProxy_Obj,
																arg_action, arg_callId,
																NULL,
																&XCallServiceClient::CallHandlingSetRequestResponseCb,
																userData);

}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequestResponseCb
 * @brief : Response function called on result of the Call handling API of
 *          Common Phone interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the call handling API results received to Xcall manager
 * @param[in] source_object : Dbus proxy object
 * @param[in] res           : asynchronous result information from temil
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::CallHandlingSetRequestResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	XCallServiceClient *client = (XCallServiceClient *)userData->object;
	ComLgeTemilCommonPhoneCallHandling *proxy = (ComLgeTemilCommonPhoneCallHandling *)userData->proxy;
	gpointer callbackData = userData->callbackData;
	XCALLLOGI("XCallServiceServer::CallHandlingSetRequest\n");

	GError *err = nullptr;
	com_lge_temil_common_phone_call_handling_call_set_request_finish(proxy, res, &err);

	if (err != nullptr )
	{
		XCALLLOGE("XCallServiceClient::CallHandlingSetRequestcb error = %s\n", err->message);
	}
	client->callManager_->CallHandlingSetRequestResponseCb(err, callbackData);
}

/**
 * ================================================================================
 * @fn : handleSOSStatusNotification
 * @brief : Call back handler function connected with the SOS status SIGNAL of
 *          connectivity interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the sos status signal received data to Xcall manager
 * @param[in] object    : DBUS proxy object for Rescue status interface
 * @param[in] SosStatus <0-3>: SOS status
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::handleSOSStatusNotification (ComLgeTemilConnectivitySOSStatus *object,
													guint SosStatus,
													gpointer user_data)
{

	XCallServiceClient *service = (XCallServiceClient *)user_data;


	XCALLLOGI("sosStatus [%d]\n", SosStatus);

	service->callManager_->notifySOSStatusEvent(SosStatus);

}

/**
 * ================================================================================
 * @fn : SOSStatusHandlingRequest
 * @brief : Request function calls the SOS status API of
 *          connectivity interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the SOS status to TEMIL service using dbus auto generated code
 * @param[in] callback                                    : asynchronous call back function
 * @param[in] user_data                                   : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::SOSStatusHandlingRequest (GAsyncReadyCallback callback, gpointer user_data)
{

	XCALLLOGI("SOSStatusHandlingRequest(\n");

	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->proxy = (gpointer)SOSStatusProxy_Obj;
	userData->object = this;
	userData->callbackData = user_data;
	com_lge_temil_connectivity_sosstatus_call_request(SOSStatusProxy_Obj,
														NULL,
														&XCallServiceClient::SOSStatusHandlingResponseCb,
														userData);

}

/**
 * ================================================================================
 * @fn : SOSStatusHandlingResponseCb
 * @brief : Response function called on result of the SOS status API of
 *          connectivity interface of TEMIL service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the SOS status API results received to Xcall manager
 * @param[in] source_object : Dbus proxy object
 * @param[in] res           : asynchronous result information from temil
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::SOSStatusHandlingResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	XCallServiceClient *client = (XCallServiceClient *)userData->object;
	ComLgeTemilConnectivitySOSStatus *proxy = (ComLgeTemilConnectivitySOSStatus *)userData->proxy;
	gpointer callbackData = userData->callbackData;
	XCALLLOGI("XCallServiceClient::SOSStatusHandlingResponseCb\n");

	GError *err = nullptr;
	guint SosStatus = 0;
	com_lge_temil_connectivity_sosstatus_call_request_finish(proxy, &SosStatus, res, &err);
	XCALLLOGI("sos status = %d\n", SosStatus);
	if (err != nullptr )
	{
		XCALLLOGE("XCallServiceClient::SOSStatusHandlingResponseCb error = %s\n", err->message);

	}
	client->callManager_->SOSStatusResponseCb(err, SosStatus, callbackData);

}

/**
 * ================================================================================
 * @fn : handleAudioMangerSignal
 * @brief : Call back handler function connected with the removed-main-connection SIGNAL of
 *          commandinterface of audiomanager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the sos status signal received data to Xcall manager
 * @param[in] proxy     : DBUS proxy object for Command interface of Audio manager service
 * @param[in] value     : audio manager id
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::handleAudioMangerSignal (OrgGeniviAudiomanagerCommandinterface *proxy,
												guint16 value,
												gpointer user_data)
{

	XCallServiceClient *service = (XCallServiceClient *)user_data;

	XCALLLOGI("parameter----- %d \n", value);
	XCALLLOGI("stored------ [%d] \n", service->audioConnectionId);

}

/**
 * ================================================================================
 * @fn : handleMMUSDeviceStatusSignal
 * @brief : Call back handler function connected with the notify-car-ifstatus SIGNAL of
 *          IHU interface of micom manager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the car information status signal received data to Xcall manager
 * @param[in] proxy     : DBUS proxy object for Rescue status interface
 * @param[in] arg_data  : MMUS device status data
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::handleMMUSDeviceStatusSignal (ComLgeCarMicomIhu *proxy,
												GVariant *arg_data,
												gpointer user_data)
{
	XCALLLOGI("handleMMUSDeviceStatusSignal is called (gpointer : %p) \n",user_data);

	// To be done: Parse the response and update the respective client

	XCALLLOGI("Called.\n");
	GVariantIter iter;
	guint8 readBuf;
	gchar* Array;
	int i = 0;
	guint8 devName = 0U,devValue = 0U;

	XCallServiceClient *service = (XCallServiceClient *)user_data;

	g_variant_iter_init (&iter, arg_data);

	while (g_variant_iter_next (&iter, "y", &readBuf))
	{
		  XCALLLOGI("[%d]readBuf = %d\n",i,readBuf);

		if(i==0) {
			//device name : spk=0x1, mic=0x2
			devName = readBuf;
		}else if(i == 1) {
			//device value : NOK=0x0, OK=0x1
			devValue = readBuf;
		}else {
			XCALLLOGI("ERROR : max 2 byte but we get more than 2 byte [%d]readBuf=%d\n",i,readBuf);
//			break;
		}
		i++;
	}

	 XCALLLOGI("devName=%d, devValue=%d\n",devName,devValue);

	if((devName == 1) || (devName == 2)) {
		if(devName == 1){
			//SPK info
			service->callManager_->SetPremiumAudioSPKStatus((bool) devValue);
		}else if(devName == 2) {
			//MIC info
			service->callManager_->SetPremiumAudioMICStatus((bool) devValue);
		}
		service->callManager_->UpdatePremiumAudioDeviceStatus(DBUS_CALL_DIRECTION_TO_TEMIL);
	}else {
		XCALLLOGE("handleCemEmgyAsscStsNotification | wrong value[%d | %d] from MMUS come.. need to check from MMUS  \n",devName,devValue);
	}

}

const guint8 spkMask = 0x02;
const guint8 micMask = 0x04;
const guint8 spkBitShift = 1;
const guint8 micBitShift = 2;

/**
 * ================================================================================
 * @fn : getDeviceStatusFromMMUS
 * @brief : Request function calls the Ask last status API of
 *          IHU interface of micom manager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the Ask last status API to audiomanager service using dbus call sync function
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::getDeviceStatusFromMMUS() {

	gboolean ret = true;
	GVariant *arg_data;
	GError *error = NULL;
	guint8 inputData[10] = {0,};
	guint8 readedData = 0;
	int i = 0;
	GVariantIter *iter;
	guint8 readedMicStatus = 0;
	guint8 readedSpkStatus = 0;

	XCALLLOGI("getDeviceStatusFromMMUS(AskSysLastStatus) is called \n");

	ret = com_lge_car_micom_ihu_call_ask_sys_last_status_sync (
															   mmusDeviceStatusCheckProxy_Obj,
															   &arg_data,
															   NULL,
															   &error);
	if (!error) {
		g_variant_get(arg_data, "ay", &iter);

		while(g_variant_iter_loop(iter,"y",&readedData)) {
			XCALLLOGI("[%d]'th read data : %d \n",i,readedData);
			inputData[i++] = readedData;
		}

		//get data (AskSysLastStatus = 3Byte)
		readedSpkStatus = (inputData[2] & spkMask) >> spkBitShift;
		readedMicStatus = (inputData[2] & micMask) >> micBitShift;

		XCALLLOGI("readedData(spk(%d), mic(%d)) \n",readedSpkStatus,readedMicStatus);
		callManager_->SetPremiumAudioSPKStatus((bool)readedSpkStatus);
		callManager_->SetPremiumAudioMICStatus((bool)readedMicStatus);
	} else {
		XCALLLOGI("getDeviceStatusFromMMUS(AskSysLastStatus) failed with error %s \n", error->message);
		g_error_free(error);
	}

	return ret;
}

/**
 * ================================================================================
 * @fn : handleCemEmgyAsscStsNotification
 * @brief : Call back handler function connected with the notify-emgy-assc-sts SIGNAL of
 *          IHU of micom manager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the sos status signal received data to Xcall manager
 * @param[in] proxy     : DBUS proxy object for Rescue status interface
 * @param[in] arg_data  : MMUS device status data
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
gboolean XCallServiceClient::handleCemEmgyAsscStsNotification(ComLgeCarMicomCem *proxy,
																GVariant *arg_data,
																gpointer user_data)
{
	GVariantIter *iter;
	guint8 inputData[10] = {0,};

	XCallServiceClient *service = (XCallServiceClient *)user_data;

	XCALLLOGI("handleCemEmgyAsscStsNotification is called (gpointer : %p)\n",user_data);

	g_variant_get(arg_data, "ay", &iter);

	guint8 readedData = 0;
	int i = 0;

	while(g_variant_iter_loop(iter,"y",&readedData)) {
		XCALLLOGI("[%d]'th read data : %d \n",i,readedData);

		inputData[i++] = readedData;
	}

	if(inputData[0] == 1){
		//SPK info
		service->callManager_->SetPremiumAudioSPKStatus((bool) inputData[1]);
	}else if(inputData[0] == 2) {
		//MIC info
		service->callManager_->SetPremiumAudioMICStatus((bool) inputData[1]);
	} else {
		XCALLLOGE("handleCemEmgyAsscStsNotification | wrong value[%d | %d] from MMUS come.. need to check from MMUS  \n",inputData[0],inputData[1]);
	}

	service->callManager_->UpdatePremiumAudioDeviceStatus(DBUS_CALL_DIRECTION_TO_TEMIL);

	g_variant_iter_free(iter);

	return true;
}

/**
 * ================================================================================
 * @fn : handleCemTelmStsNotification
 * @brief : Call back handler function connected with the notify-telm-sts SIGNAL of
 *          CEM interface of micom manager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the sos status signal received data to Xcall manager
 * @param[in] proxy     : DBUS proxy object for Rescue status interface
 * @param[in] arg_data  : Telematics device status data
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
gboolean XCallServiceClient::handleCemTelmStsNotification(ComLgeCarMicomCem *proxy,
																GVariant *arg_data,
																gpointer user_data)
{
	XCALLLOGI("handleCemTelmStsNotification is called (gpointer : %p) \n",user_data);

	// To be done: Parse the response and update the respective client
	/*
	GVariantIter *iter;
	guint8 inputData[10] = {0,};

	guint8 readedData = 0;
	int i = 0;

	XCallServiceClient *service = (XCallServiceClient *)user_data;

	g_variant_get(arg_data, "ay", &iter);

	while(g_variant_iter_loop(iter,"y",&readedData)) {
		XCALLLOGI("[%d]'th read data : %d \n",i,readedData);
		inputData[i++] = readedData;
	}
	g_variant_iter_free(iter);
	*/

	return true;
}

/**
 * ================================================================================
 * @fn : acquireAudioMangerResource
 * @brief : Request function calls the Connect API of
 *          commandinterface of audiomanager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the Connect API to audiomanager service using dbus call function
 * @param[in] mainID : pointer to audio manager id
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
//#define USE_DBUS_PROXY
bool XCallServiceClient::acquireAudioMangerResource(guint16 *mainID, guint xcall_type)
{

	GDBusConnection *conn;
	GVariant *busresult = NULL;
	GError *error = NULL;
	gchar *s;

	bool ret=true;

	unsigned short int xcallSourceID=0;
	unsigned short int xcallSinkID=1;

	if((xcall_type == CALL_TYPE_EMERGENCY_CALL) || (xcall_type == CALL_TYPE_SDN_CALL)) {
		xcallSourceID = AUDIO_MGR_TEM_ALERT;
	}else if((xcall_type == CALL_TYPE_BREAKDOWN_CALL) || (xcall_type == CALL_TYPE_INFORMATION_CALL)) {
		xcallSourceID = AUDIO_MGR_TEM_PHONE;
	}else {
		xcallSourceID = AUDIO_MGR_TEM_PHONE;
	}

	XCALLLOGI("[Client]  Request Audio Manager with SrcID[%x], SinkID[%x] on CallType[%d] \n",xcallSourceID,xcallSinkID,xcall_type);

#ifdef USE_DBUS_PROXY

#else
	conn = g_bus_get_sync(DBUS_TYPE, NULL, &error);

	if (error) {
		XCALLLOGE("Error: %s\n", error->message);
		g_error_free(error);
		ret = false;
	} else {
		busresult = g_dbus_connection_call_sync(conn,
										"org.genivi.audiomanager",
										"/org/genivi/audiomanager/commandinterface",
										"org.genivi.audiomanager.commandinterface",
										"Connect",
										g_variant_new("(qq)",(guint16)xcallSourceID,(guint16)xcallSinkID),
										NULL,
										G_DBUS_CALL_FLAGS_NONE,
										3000,
										NULL,
										&error);

		if (busresult == NULL) {
			XCALLLOGD("[Client]  nothing after dbus send \n");
			ret = false;
		} else {
			gint16 err;
			guint16 ConnectionId;

			g_variant_get(busresult,"(nq)",&err,&ConnectionId);

			*mainID = ConnectionId;
			XCALLLOGI("[Client] Acquire from Audio Manager (mainID(%d), errNo(%d) \n",*mainID, err);

			s = g_variant_print(busresult, TRUE);
			XCALLLOGD("[Client]  return value is %s \n",s);
		}
	}
#endif

	return ret;
}

/**
 * ================================================================================
 * @fn : acquireAudioMangerResource
 * @brief : Request function calls the Disconnect API of
 *          commandinterface of audiomanager service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the Disconnect API to audiomanager service using dbus call function
 * @param[in] mainID : audio manager id
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XCallServiceClient::releaseAudioMangerResource(guint16 mainID)
{

	GError *error = NULL;
	GDBusConnection *conn;
	GVariant *busresult = NULL;
	gchar *s;

	bool ret=true;

#ifdef USE_DBUS_PROXY

#else

	conn = g_bus_get_sync(DBUS_TYPE, NULL, &error);

	if (error) {
		XCALLLOGE("[Client ] Error: %s\n", error->message);
		g_error_free(error);

		ret = false;
	} else {
		XCALLLOGI("[Client ] release to Audio Manager (MainID(%d))\n",mainID);

		busresult = g_dbus_connection_call_sync(conn,
										"org.genivi.audiomanager",
										"/org/genivi/audiomanager/commandinterface",
										"org.genivi.audiomanager.commandinterface",
										"Disconnect",
										g_variant_new("(q)",(guint16)mainID),
										NULL,
										G_DBUS_CALL_FLAGS_NONE,
										3000,
										NULL,
										&error);

		if (busresult == NULL) {
			XCALLLOGE("[Client ] No response after dbus-send to audio manager(Disconnect)\n");
			ret = false;
		} else {
			gint16 err;
			s = g_variant_print(busresult, TRUE);
			XCALLLOGD("[Client ] return value is %s \n", s);
			g_variant_get(busresult,"(n)", &err);
			XCALLLOGI("[Client ] after release, return value=%d \n", err);
		}
	}
#endif

	return ret;
}

/**
 * ================================================================================
 * @fn : handleCarConfigNotification
 * @brief : Call back handler function connected with the Car config information SIGNAL of
 *          Car Config provider service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the sos status signal received data to Xcall manager
 * @param[in] object    : DBUS proxy object for Rescue status interface
 * @param[in] arg_index : updated/changed array index number in car configuration data
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
void XCallServiceClient::handleCarConfigNotification (ComLgeGeelyCarConfigProviderInterface *object,
														gint arg_index,
														gpointer user_data)
{
	XCALLLOGI("handleCarConfigNotification is called index changed = %d \n", arg_index);

	XCallServiceClient *service = (XCallServiceClient *)user_data;

	if (arg_index == CARCONFIG_ASSTSRV || arg_index == CARCONFIG_TELEMODULE) {
		service->GetCarConfigInfoRequest(arg_index, nullptr, nullptr);
	}
}

/**
 * ================================================================================
 * @fn : GetCarConfigInfoRequest
 * @brief : Request function calls the Car config information API of
 *          Car Config provider service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Requests the SOS status to TEMIL service using dbus auto generated code
 * @param[in] arg_index : array index to be fected for car config information provider
 * @param[in] callback  : asynchronous call back function
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::GetCarConfigInfoRequest(gint arg_index, GAsyncReadyCallback callback, gpointer user_data)
{
	XCALLLOGI("GetCarConfigInfoRequest is called index =%d\n",arg_index);
	CbUserDataStruct * userData = new CbUserDataStruct;
	userData->proxy = (gpointer)vehicleInfoDaemonProxy_Obj;
	userData->object = this;
	userData->callbackData = user_data;
	com_lge_geely_car_config_provider_interface_call_get_car_config_data(vehicleInfoDaemonProxy_Obj,
																			arg_index,
																			NULL,
																			&XCallServiceClient::GetCarConfigInfoResponseCb,
																			userData);

}

/**
 * ================================================================================
 * @fn : GetCarConfigInfoResponseCb
 * @brief : Response function called on result of the Car config information API of
 *          Car Config provider service
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the SOS status API results received to Xcall manager
 * @param[in] source_object : Dbus proxy object
 * @param[in] res           : asynchronous result information from temil
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XCallServiceClient::GetCarConfigInfoResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	CbUserDataStruct * userData = (CbUserDataStruct *) user_data;
	XCallServiceClient *client = (XCallServiceClient *)userData->object;
	ComLgeGeelyCarConfigProviderInterface *proxy = (ComLgeGeelyCarConfigProviderInterface *)userData->proxy;
	gpointer callbackData = userData->callbackData;
	XCALLLOGI("XCallServiceClient::GetCarConfigInfoResponseCb\n");

	GError *err = nullptr;
	gint carConfigData = 0;
	com_lge_geely_car_config_provider_interface_call_get_car_config_data_finish(proxy, &carConfigData, res, &err);
	XCALLLOGI("carConfigData = %d\n", carConfigData);
	if (err != nullptr )
	{
		XCALLLOGE("XCallServiceClient::GetCarConfigInfoResponseCb error = %s\n", err->message);

	}
}

}//namespace temxcall

