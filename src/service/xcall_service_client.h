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

//global header
#include <glib.h>
#include <gio/gio.h>
#include <vector>
#include <utility>


 //local header
#include "xcall_manager.h"

#ifdef DEBUG_ENABLED
#include "xcall_debug.h"
#endif

//interface header
#include "dbus_xcall_interface.h"
#include <CommonPhone.h>
#include <Telematics.h>
#include <Connectivity.h>
#include "dbus_audio_interface.h"
#include "dbus_mmus_interface.h"
#include "dbus_carconfigProvider_interface.h"

#ifndef __TEMXCALL_XCALL_SERVICE_CLIENT_H__
#define __TEMXCALL_XCALL_SERVICE_CLIENT_H__

namespace temxcall {

class XcallManager;

class XCallServiceClient {

public:

	XCallServiceClient(XcallManager *mgr);
	XCallServiceClient();
	~XCallServiceClient();

	bool clientCalling();
	bool clientConnected();
	bool clientEnd();

	bool registerProxys();
	bool acquireAudioMangerResource(guint16 *mainID, guint xcall_type);
	bool releaseAudioMangerResource(guint16 mainID);

	static void handleRescueStatusNotification (ComLgeTemilTelematicsRescueStatus *object,
												GVariant *arg_eCallStatus,
												GVariant *arg_bCallStatus,
												GVariant *arg_iCallStatus,
												GVariant *arg_sdnStatus,
												guint arg_backupAudioStatus,
												gpointer user_data);

	static void handleAudioMangerSignal (OrgGeniviAudiomanagerCommandinterface *proxy,
														unsigned short int value,
														gpointer user_data);

	static void handleMMUSDeviceStatusSignal (ComLgeCarMicomIhu *proxy,
												GVariant *arg_data,
												gpointer user_data);

	static void handleCarConfigNotification (ComLgeGeelyCarConfigProviderInterface *object,
												gint arg_index,
												gpointer user_data);

	static gboolean handleCemEmgyAsscStsNotification(ComLgeCarMicomCem *proxy,
																	GVariant *arg_data,
																	gpointer user_data);

	static gboolean handleCemTelmStsNotification(ComLgeCarMicomCem *proxy,
																	GVariant *arg_data,
																	gpointer user_data);

	void ServiceActivationSetRequest (guint arg_service, guint arg_action,
										GAsyncReadyCallback callback, gpointer user_data);
	void CallHandlingSetRequest (guint arg_action, guint8 arg_callId,
										GAsyncReadyCallback callback, gpointer user_data);

	void SOSStatusHandlingRequest (GAsyncReadyCallback callback, gpointer user_data);

	static void handleSOSStatusNotification (ComLgeTemilConnectivitySOSStatus *object,
											guint SosStatus,
											gpointer user_data);

	bool getDeviceStatusFromMMUS();
	void GetCarConfigInfoRequest(gint arg_index, GAsyncReadyCallback callback, gpointer user_data);

	static void CallHandlingSetRequestResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data);
	static void ServiceActivationSetResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data);
	static void SOSStatusHandlingResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data);
	static void GetCarConfigInfoResponseCb (GObject *source_object, GAsyncResult *res, gpointer user_data);

	GBusType bus_type;
	XcallManager *callManager_;
	unsigned short int audioConnectionId;

	static ComLgeTemilCommonPhoneCallHandling *CallHandlingProxy_Obj;
	static ComLgeTemilTelematicsRescueStatus * ResqueStausProxy_Obj;
	static ComLgeTemilTelematicsServiceActivation *ServiceActivationProxy_Obj;
	static ComLgeTemilConnectivitySOSStatus *SOSStatusProxy_Obj;

	static OrgGeniviAudiomanagerCommandinterface *AudioMgrProxy_Obj;

	static ComLgeCarMicomIhu *mmusDeviceStatusCheckProxy_Obj;
	static ComLgeCarMicomCem *mmusCemProxy_Obj;
	static ComLgeGeelyCarConfigProviderInterface *vehicleInfoDaemonProxy_Obj;

};

}//namespace temxcall
#endif //__TEMXCALL_XCALL_SERVICE_CLIENT_H__

