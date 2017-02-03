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

#ifndef __TEMXCALL_XCALL_SERVICE_SERVER_H__
#define __TEMXCALL_XCALL_SERVICE_SERVER_H__

#include <glib.h>
#include <gio/gio.h>

#include <vector>
#include <utility>

//local header
#include "xcall_manager.h"

//interface header
#include "dbus_xcall_interface.h"
#include <CommonPhone.h>
#include <Telematics.h>


#ifdef DEBUG_ENABLED
#include "xcall_debug.h"
#endif

namespace temxcall {

class XcallManager;

class XCallServiceServer {

public:
	XCallServiceServer();
	XCallServiceServer(XcallManager *mgr);
	~XCallServiceServer();

	bool registerSkeletons();
	bool NotifyCallStatusQueryEvent(guint callid, guint calltype, const gchar *callStatus);
	bool NotifyErrorInformation (const gchar *errMsg, guint errId = 0);

	static void BusNameAcquiredCB (GDBusConnection *connection,
									const gchar* name,
									gpointer user_data);
	//interface function
	////test interface
	static bool Calling(ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation,char *callinfo, char *callname,
								 gpointer user_data);
	static bool Connected(ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation,
								 gpointer user_data);
	static bool End(ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation,
								 gpointer user_data);

	static bool UnitTest(ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation, char *testName, char *param1, char *param2,
								 gpointer user_data);
	static bool sendErrorInfoNotification (ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation,
								 guint errId, char *errMsg,
								 gpointer user_data);
	static bool HandleSetRescueStatus (ComLgeXcallserviceTestInterface *skeleton,
								 GDBusMethodInvocation *invocation,
								 guint arg_eRescueStatus, guint arg_eXCallID, guint arg_eVoiceStatus, guint arg_eSourceStatus, guint arg_eMessageStatus, guint arg_eButtonStatus, guint arg_ePSAPStatus, guint arg_eStandbyStatus,
								 guint arg_bRescueStatus, guint arg_bXCallID, guint arg_bVoiceStatus, guint arg_bSourceStatus, guint arg_bMessageStatus, guint arg_bButtonStatus, guint arg_bPSAPStatus, guint arg_bStandbyStatus,
								 guint arg_iRescueStatus, guint arg_iXCallID, guint arg_iVoiceStatus, guint arg_iSourceStatus, guint arg_iMessageStatus, guint arg_iButtonStatus, guint arg_iPSAPStatus, guint arg_iStandbyStatus,
								 guint arg_sRescueStatus, guint arg_sXCallID, guint arg_sVoiceStatus, guint arg_sSourceStatus, guint arg_sMessageStatus, guint arg_sButtonStatus, guint arg_sPSAPStatus, guint arg_sStandbyStatus,
								 guint arg_backupAudioStatus,
								 gpointer user_data);
	///call feature interface
	static bool CallHandlingSetRequest (ComLgeXcallserviceCallFeature *object,
										GDBusMethodInvocation *invocation,
										guint arg_action,
										guint arg_callId,
										gpointer user_data);
	static void CallHandlingSetRequestResponseCb (GError *err, gpointer user_data);

	static bool ServiceActivationSetRequest (ComLgeXcallserviceCallFeature *object,
											GDBusMethodInvocation *invocation,
											guint arg_service,
											guint arg_action,
											gpointer user_data);
	static void ServiceActivationSetResponseCb (GError *err, gpointer user_data);

	////premium audio interface
	static bool PremiumAudioRequest (ComLgeXcallservicePremiumAudio *object,
									GDBusMethodInvocation *invocation,
									gpointer user_data);
	static bool PremiumAudioSetRequest (ComLgeXcallservicePremiumAudio *object,
										GDBusMethodInvocation *invocation,
										gboolean arg_isRequested,
										gpointer user_data);
	bool NotifyPremiumAudioEvent(guint spkStatus, guint micStatus);

private:

	XcallManager *callManager_;
	guint mainLoopId;

	static int mCallType;
	static int mCallId;
	//skeletons
	static ComLgeXcallserviceTestInterface *testIfaceSkel_obj;
	static ComLgeXcallserviceCallFeature *callFeatureSkel_Obj;
	static ComLgeXcallservicePremiumAudio *premiumAudioeSkel_Obj;

	//interface table
	static std::vector<std::pair<const char*, GCallback>> testIfaceHandler_table;
	static std::vector<std::pair<const char*, GCallback>> callFeatureHandler_table;
	static std::vector<std::pair<const char*, GCallback>> premiumAudioHandler_table;

};

}//namespace temxcall
#endif //__TEMXCALL_XCALL_SERVICE_SERVER_H__
