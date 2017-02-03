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

#include <cstring>

#include "xcall_manager.h"

//#define USE_RINGTONE  //when ringtone is required.

namespace temxcall {

std::map<int, std::string> CALL_TYPE_MAPPING = {
		{CALL_TYPE_EMERGENCY_CALL,"emergency"},
		{CALL_TYPE_BREAKDOWN_CALL,"breakdown"},
		{CALL_TYPE_INFORMATION_CALL,"information"},
		{CALL_TYPE_SDN_CALL,"sdn"}
};

std::map<int, std::string> CALL_STATUS_MAPPING = {
		{CALL_STATUS_NONE,				"Unknown"},
		{CALL_STATUS_INITIALIZING,		"Initializing"},
		{CALL_STATUS_STANDBY,			"StandBy"},
		{CALL_STATUS_CALLING,			"Connecting"},
		{CALL_STATUS_INCOMING,			"Incoming"},
		{CALL_STATUS_CONNECTED,			"Connected"},
		{CALL_STATUS_ENDCALL,			"Disconnected"},
		{CALL_STATUS_RETRY,				"QuickDisconnect"},
};

/**
 * ================================================================================
 * @fn : XcallManager
 * @brief : This is constructor function for XcallManager class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the member variables
 * - Initialized a main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
XcallManager::XcallManager()
{
	//for initializing
	initValue();
	resetPreviousCallSatus();

	player = NULL;
	svcServer = NULL;
	svcClient = NULL;
	mListener.setXcallManager(this);
	//create G Main Loop
	mainLoop = g_main_loop_new(NULL,false);

	curPlayerType = PLAYER_TYPE_NONE;

	mSosStatus = SOS_STATUS_UNKNOWN;

	//flag ... whether icall button is pressed or not
	isICallBtnPressed=0;
}

/**
 * ================================================================================
 * @fn : ~XcallManager
 * @brief : This is destructor function for XcallManager class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
XcallManager::~XcallManager()
{
	if (mainLoop)
		g_main_loop_unref(mainLoop);

	XCALLLOGI("Destructor");
}

/**
 * ================================================================================
 * @fn : initValue
 * @brief : Initialize function for member variables
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes callId, callType, callStatus, PlayerType, PlayerStatus
 *   Speaker status, Mic status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::initValue()
{
	guint i=XCALL_ID_MAX;

	XCALLLOGD("all call status are cleared and reset");
	for (i = XCALL_ID_SDN; i < XCALL_ID_MAX; i++) {
		curCallInfo[i].callId = -1;
		curCallInfo[i].callType = CALL_TYPE_NONE ;
		curCallInfo[i].callStatus = CALL_STATUS_NONE;
		curCallInfo[i].playerType = PLAYER_TYPE_NONE;
		curCallInfo[i].playerStatus = PLAYER_STATUS_NONE;
		curCallInfo[i].active = NOTACTIVECALL;
	}

	//interface with audio manager
	audioMainConnctionID = 0;

	//as a default, premium audio use OK.
	audioDevStatus.spkStatus = PREMIUM_AUDIO_STATUS_OK;
	audioDevStatus.micStatus = PREMIUM_AUDIO_STATUS_OK;
}

/**
 * ================================================================================
 * @fn : resetPreviousCallSatus
 * @brief : reset function for call status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - resets all the previous call information stored
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::resetPreviousCallSatus()
{
	XCALLLOGD("Previous call status are cleared and reset");
	for (int i=XCALL_ID_SDN; i<XCALL_ID_MAX; i++) {
		mPreviousCallStatus[i].status = StatusNotActive;
		mPreviousCallStatus[i].callId = i;
		mPreviousCallStatus[i].voiceStatus = VoiceStatusNoConnection;
		mPreviousCallStatus[i].voiceSource = SourceStatusUnidentified;
		mPreviousCallStatus[i].messageStatus = MessageStatusNotSent;
		mPreviousCallStatus[i].buttonStatus = ButtonAllReleased;
		mPreviousCallStatus[i].psapConfirmStatus = PSAPStatusConfirmNotRequired;
		mPreviousCallStatus[i].sbStatus = StandbyStatusNotActive;
	}
//	mPreviousCallStatus[3].callId = 0; // i call case
}

/**
 * ================================================================================
 * @fn : getPlayer
 * @brief : Creates the player instance
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Creates the player instance
 * @param[in] playerName : registered player name string for instantiation
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::getPlayer(const char *playerName)
{
	//AbstractPlayer *player = sPlayerBaseFactory::GetInstance().CreateObject("VoiceCallPlayer");
	if (playerName)
		player = sPlayerBaseFactory::GetInstance().CreateObject(playerName);
}

/**
 * ================================================================================
 * @fn : removePlayer
 * @brief : Dummy function
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::removePlayer()
{

}

/**
 * ================================================================================
 * @fn : createPlayer
 * @brief : Function decides the player name by call status information
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - parses the call status information
 * - requests player creation with appropriate name
 * @param[in] callinfo : call status information
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::createPlayer(const char *callinfo)
{
	bool ret=false;

	if ((!strcmp(callinfo,"Incoming")) || (!strcmp(callinfo,"Connecting"))) {

#ifdef USE_RINGTONE
		XCALLLOGD("RingtonePlayer");

		//TODO : Ringtone will be added
		//player = sPlayerBaseFactory::GetInstance().CreateObject("RingtonePlayer");
		getPlayer("RingtonePlayer");
		curPlayerType = PLAYER_TYPE_RINGTONE;

		XCALLLOGD("RingtonePlayer is created");
#else
		XCALLLOGI("VoicecallPlayer");

		//player = sPlayerBaseFactory::GetInstance().CreateObject("VoicecallPlayer");
		getPlayer("VoicecallPlayer");
		curPlayerType = PLAYER_TYPE_VOICECALL;

		XCALLLOGD("VoicecallPlayer is created");
#endif

	} else if (!strcmp(callinfo,"Connected")) {
		if (curPlayerType != PLAYER_TYPE_VOICECALL) {
			//player = sPlayerBaseFactory::GetInstance().CreateObject("VoicecallPlayer");
			getPlayer("VoicecallPlayer");
			curPlayerType = PLAYER_TYPE_VOICECALL;
		}
		XCALLLOGI("VoiceCallPlayer is created");
	} else {
		if (!strcmp(callinfo,"unittest-spk")) {
			XCALLLOGD("UnitTest-SPK is created");
			//player = sPlayerBaseFactory::GetInstance().CreateObject("UnitTest-SPK");
			getPlayer("UnitTest-SPK");
			curPlayerType =PLAYER_TYPE_UNITTEST;
		} else if (!strcmp(callinfo,"unittest-mic")) {
			XCALLLOGD("UnitTest-MIC is created");
			//player = sPlayerBaseFactory::GetInstance().CreateObject("UnitTest-MIC");
			getPlayer("UnitTest-MIC");
			curPlayerType =PLAYER_TYPE_UNITTEST;
		} else if (!strcmp(callinfo,"unittest-rtp")) {
			XCALLLOGD("UnitTest-RTP is created");
			//player = sPlayerBaseFactory::GetInstance().CreateObject("UnitTest-RTP");
			getPlayer("UnitTest-RTP");
			curPlayerType =PLAYER_TYPE_UNITTEST;
		} else {
			XCALLLOGE("ERR -- Not Match [%s]",callinfo);
		}
	}

	return ret;
}

/**
 * ================================================================================
 * @fn : startCall
 * @brief : updates player, call information as per the start call event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - checks any player is already in playing status and removes it
 * - requests new player instance
 * @param[in] callinfo : call status information
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::startCall(const char *callinfo)
{
	bool ret = true;
	guint xcallid = isActiveCallid();
	CallStatusInfo_t *streamingPlayer;

	if(xcallid == XCALL_ID_MAX) return false;

	streamingPlayer = &curCallInfo[xcallid];

	//check player Status
	if (streamingPlayer->playerStatus == PLAYER_STATUS_PLAY) {
		XCALLLOGI(">>> [[NOTICE]] <<< create New Player without Stop of previous Player ");
		player->Unload();
		removePlayer();
		streamingPlayer->playerStatus = PLAYER_STATUS_STOP;
	}

	XCALLLOGI("createPlayer with [%s] ", callinfo);
	createPlayer(callinfo);

#ifdef DEBUG_ENABLED
	XCALLLOGD("[XcallManager] startCall | player->source information is called");
	player->setSourceInformation(this->input);
#endif

	player->Load();
	streamingPlayer->playerStatus = PLAYER_STATUS_PLAY;

	XCALLLOGI("[PlayerStatus:%d] call player->load",streamingPlayer->playerStatus);

	XCALLLOGD("createPlayer");

	return ret;
}

/**
 * ================================================================================
 * @fn : conntectedCall
 * @brief : updates player, call information as per the connected call event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - checks any player is already in playing status and removes it
 * - requests new player instance
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::conntectedCall()
{
	bool ret = true;
	XCALLLOGD("start");

	if (curPlayerType == PLAYER_TYPE_RINGTONE) {
		//unload & remove ringtone player
		player->Unload();
		removePlayer();

		//create voicecall player
		createPlayer("Connected");
		player->Play();

	} else if(curPlayerType == PLAYER_TYPE_VOICECALL) {
		player->Play();
	} else {

		if (curPlayerType == PLAYER_TYPE_UNITTEST) {
			player->Play();
		} else {
			XCALLLOGE("ERR -- NO player is available to play");
		}
	}
	return ret;
}

/**
 * ================================================================================
 * @fn : endCall
 * @brief : updates player, call information as per the endCall event
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - unloads the player instance and removes it
 * - updates the member variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::endCall()
{
	bool ret = true;
	guint xcallid = isActiveCallid();
	guint holdcallid = isHeldCallid();

	CallStatusInfo_t *streamingPlayer;

	if (xcallid == XCALL_ID_MAX) return false;

	if((holdcallid < XCALL_ID_MAX)) {

		//check whether holdplayer is playing or not
		CallStatusInfo_t *holdplayer = 	&curCallInfo[holdcallid];

		if((holdplayer->playerStatus != PLAYER_STATUS_STOP) && (holdplayer->playerStatus != PLAYER_STATUS_PAUSE)) {
			XCALLLOGI("[PlayerStatus]change callid from active to hold ");
			xcallid = holdcallid;
		}
	}

	streamingPlayer = &curCallInfo[xcallid];

#ifdef ENABLE_UNITTEST_ASK_MMUS_API

	g_print("endCall is called \n");
	svcClient->getDeviceStatusFromMMUS();
#else

	if ((streamingPlayer->playerStatus != PLAYER_STATUS_STOP) && (streamingPlayer->playerStatus != PLAYER_STATUS_PAUSE)) {
		XCALLLOGI("[PlayerStatus:%d] call player->unload",streamingPlayer->playerStatus);
		player->Unload();
	} else {
		XCALLLOGI("[PlayerStatus:%d] ignore player->unload",streamingPlayer->playerStatus);
	}

	curPlayerType = PLAYER_TYPE_NONE;
	streamingPlayer->playerStatus = PLAYER_STATUS_STOP;
#endif
	return ret;
}

/**
 * ================================================================================
 * @fn : mainServiceStart
 * @brief : Starts the main loop
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Starts the main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
//g_main_loop
bool XcallManager::mainServiceStart()
{
	bool ret=true;

	XCALLLOGI("start");

#ifdef TIME_MEASURE
	XCALLLOGI("[TIME] before GMainLoop\n");
	measureElapseTime();
#endif

    mLinkMonitor.setListener("tem0", &mListener);

	g_main_loop_run(mainLoop);

	return ret;
}

/**
 * ================================================================================
 * @fn : mainServiceStop
 * @brief : stops the main loop
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - stops the main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::mainServiceStop()
{
	bool ret=true;

	g_main_loop_quit(mainLoop);

	return ret;
}

/**
 * ================================================================================
 * @fn : registerServices
 * @brief : register the server and client handlers
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - stores the server and client handlers in memeber variables
 * @param[in] pServer   : handler of XCallServiceServer object
 * @param[in] pClient   : handler of XCallServiceClient object
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::registerServices(XCallServiceServer *pServer, XCallServiceClient *pClient )
{
	bool ret=true;

	svcServer = pServer;
	svcClient = pClient;

	XCALLLOGI("(svcServer)%p | (svcClient)%p | (mgr)%p",svcServer,svcClient,this);

	// TO DO: Enable/modify the below function when the TEM2 status ready implementation is done
	//svcClient->SOSStatusHandlingRequest(nullptr, nullptr);

	// TO DO: Enable/modify the below function when vehicle daemon ready sync sequence is adapted
	//GetCarConfigInfoRequest(CARCONFIG_ASSTSRV);
	//GetCarConfigInfoRequest(CARCONFIG_TELEMODULE);

	return ret;
}

/**
 * ================================================================================
 * @fn : notifyRescueStatusEvent
 * @brief : Call State handler function
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Parses the information from Rescue status event for each type of call
 * - Identifies the which call type
 * @param[in] arg_eCallStatus       : Emergency call status information in glib object
 * @param[in] arg_bCallStatus       : Breakdown call status information in glib object
 * @param[in] arg_iCallStatus       : Information call status information in glib object
 * @param[in] arg_sdnStatus         : SDN call status information in glib object
 * @param[in] arg_backupAudioStatus : Backup audio device status in glib object
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::notifyRescueStatusEvent(GVariant *arg_eCallStatus,
											GVariant *arg_bCallStatus,
											GVariant *arg_iCallStatus,
											GVariant *arg_sdnStatus,
											guint arg_backupAudioStatus)
{

	bool ret = false;
	CallStatusInfo_t updatedCallInfo = {0,};

	CallStatus_t eCallStatus;
	CallStatus_t bCallStatus;
	CallStatus_t iCallStatus;
	CallStatus_t sdnCallStatus;
	guint backupAudioStatus = 0;

	//1st, check rescuestatus for calltype

	g_variant_get(arg_eCallStatus, "(uuyuuuuuu)", &eCallStatus.status, &eCallStatus.callId, &eCallStatus.callSessioNum, &eCallStatus.voiceStatus, &eCallStatus.voiceSource,
												&eCallStatus.messageStatus, &eCallStatus.buttonStatus, &eCallStatus.psapConfirmStatus, &eCallStatus.sbStatus);

	g_variant_get(arg_bCallStatus, "(uuyuuuuuu)", &bCallStatus.status, &bCallStatus.callId, &bCallStatus.callSessioNum, &bCallStatus.voiceStatus, &bCallStatus.voiceSource,
												&bCallStatus.messageStatus, &bCallStatus.buttonStatus, &bCallStatus.psapConfirmStatus, &bCallStatus.sbStatus);

	g_variant_get(arg_iCallStatus, "(uuyuuuuuu)", &iCallStatus.status, &iCallStatus.callId, &iCallStatus.callSessioNum, &iCallStatus.voiceStatus, &iCallStatus.voiceSource,
												&iCallStatus.messageStatus, &iCallStatus.buttonStatus, &iCallStatus.psapConfirmStatus, &iCallStatus.sbStatus);

	g_variant_get(arg_sdnStatus, "(uuyuuuuuu)", &sdnCallStatus.status, &sdnCallStatus.callId, &sdnCallStatus.callSessioNum, &sdnCallStatus.voiceStatus, &sdnCallStatus.voiceSource,
												&sdnCallStatus.messageStatus, &sdnCallStatus.buttonStatus, &sdnCallStatus.psapConfirmStatus, &sdnCallStatus.sbStatus);
	backupAudioStatus = arg_backupAudioStatus;


	XCALLLOGI("E-Call: %d, %d, %d, %d, %d, %d, %d, %d %d\n", eCallStatus.status, eCallStatus.callId, eCallStatus.callSessioNum, eCallStatus.voiceStatus, eCallStatus.voiceSource,
			eCallStatus.messageStatus, eCallStatus.buttonStatus, eCallStatus.psapConfirmStatus, eCallStatus.sbStatus);

	XCALLLOGI("B-Call: %d, %d, %d, %d, %d, %d, %d, %d %d\n", bCallStatus.status, bCallStatus.callId, bCallStatus.callSessioNum, bCallStatus.voiceStatus, bCallStatus.voiceSource,
			bCallStatus.messageStatus, bCallStatus.buttonStatus, bCallStatus.psapConfirmStatus, bCallStatus.sbStatus);
	XCALLLOGI("I-Call: %d, %d, %d, %d, %d, %d, %d, %d %d\n", iCallStatus.status, iCallStatus.callId, iCallStatus.callSessioNum, iCallStatus.voiceStatus, iCallStatus.voiceSource,
			iCallStatus.messageStatus, iCallStatus.buttonStatus, iCallStatus.psapConfirmStatus, iCallStatus.sbStatus);
	XCALLLOGI("SND-Call: %d, %d, %d, %d, %d, %d, %d, %d %d\n", sdnCallStatus.status, sdnCallStatus.callId, sdnCallStatus.callSessioNum, sdnCallStatus.voiceStatus, sdnCallStatus.voiceSource,
			sdnCallStatus.messageStatus, sdnCallStatus.buttonStatus, sdnCallStatus.psapConfirmStatus, sdnCallStatus.sbStatus);
	XCALLLOGI("Backup audio status = %d \n", backupAudioStatus);


	//XCALLLOGI(" >>> Current : %d %d %d %d ",curCallInfo.callType,curCallInfo.callStatus,curCallInfo.playerType,curCallInfo.playerStatus);

	if (bCallStatus.status == StatusDisabled && eCallStatus.status == StatusDisabled && sdnCallStatus.status == StatusDisabled && iCallStatus.status == StatusDisabled) {
		XCALLLOGI("*********************MODEM IS RESTARTED clear the states***********************\n");
		resetXcallStatus();
		resetPreviousCallSatus();
	} else if(callStatusChanged(&mPreviousCallStatus[XCALL_ID_ECALL] , &eCallStatus)) {

		XCALLLOGI("------------------ E CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[XCALL_ID_ECALL], &eCallStatus, sizeof(CallStatus_t));
		ret = updateVoiceStatus(eCallStatus, updatedCallInfo, XCALL_ID_ECALL);

	} else if (callStatusChanged(&mPreviousCallStatus[XCALL_ID_BCALL] , &bCallStatus)) {

		XCALLLOGI("------------------ B CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[XCALL_ID_BCALL], &bCallStatus, sizeof(CallStatus_t));
		ret = updateVoiceStatus(bCallStatus, updatedCallInfo, XCALL_ID_BCALL);

		// callswitch 조건 무시
	} else if (callStatusChanged(&mPreviousCallStatus[XCALL_ID_ICALL] , &iCallStatus)) {

		XCALLLOGI("------------------ I CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[XCALL_ID_ICALL], &iCallStatus, sizeof(CallStatus_t));

		if (iCallStatus.status == StatusServiceNotAvailable) {
			XCALLLOGI("I-Call Not available\n");
			svcServer->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: Service not available");
		} else {

			ret = updateVoiceStatus(iCallStatus, updatedCallInfo, XCALL_ID_ICALL);
		}

		//call switch 조건 무시
	} else if (callStatusChanged(&mPreviousCallStatus[XCALL_ID_SDN] , &sdnCallStatus)) {

		XCALLLOGI("------------------ SDN CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[XCALL_ID_SDN], &sdnCallStatus, sizeof(CallStatus_t));

		ret = updateVoiceStatus(sdnCallStatus, updatedCallInfo, XCALL_ID_SDN);
	} else {

		//clear button flag.
		isICallBtnPressed=0;

		XCALLLOGE("ERROR - No Change (Rescue Status)");
		return ret;
	}

	if (ret) {
		ret = updateCallInfo(updatedCallInfo);
	}

	//clear button flag.
	isICallBtnPressed=0;

	displayXcallStatus();

//	XCALLLOGI(" >>> Next : %d %d %d %d (iCall btn flag=%d) ",curCallInfo.callType,curCallInfo.callStatus,curCallInfo.playerType,curCallInfo.playerStatus,isICallBtnPressed);

	return ret;


#if 0
 else if (callStatusChanged(&mPreviousCallStatus[1] , &bCallStatus)) {
		XCALLLOGI("------------------ B CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[1], &bCallStatus, sizeof(CallStatus_t));
		if (bCallStatus.status == StatusActive || bCallStatus.sbStatus == StandbyStatusActive || bCallStatus.sbStatus == StandbyStatusCancel || bCallStatus.buttonStatus == ButtonPressed ||
															((curCallInfo.callType  == CALL_TYPE_BREAKDOWN_CALL) && (curCallInfo.callStatus == CALL_STATUS_INITIALIZING) && (bCallStatus.buttonStatus == ButtonReleased))) {

		#if 1	//workaround
			//get wrong ip command during Ecall, make the below for filtering
			if ((curCallInfo.callType  == CALL_TYPE_EMERGENCY_CALL) && (bCallStatus.status == StatusTerminated )) {

				XCALLLOGI("ERROR : wrong IP Command --- Current(E-Call) but got Bcall cmd from TEM2 (ignore this) \n");
				ret = false;
			} else {
				//B-Call
				XCALLLOGI("calltype - B-Call @@@\n");
				updatedCallInfo.callType = CALL_TYPE_BREAKDOWN_CALL;
				updatedCallInfo.callId = bCallStatus.callId;
				ret = updateVoiceStatus(bCallStatus, updatedCallInfo, isSdnCall);
			}
		#else
			//B-Call
			XCALLLOGI("calltype - B-Call\n");
			updatedCallInfo.callType = CALL_TYPE_BREAKDOWN_CALL;
			updatedCallInfo.callId = bCallStatus.callId;
			ret = updateVoiceStatus(bCallStatus, updatedCallInfo, isSdnCall);
		#endif
		}else if ((curCallInfo.callStatus== CALL_STATUS_STANDBY) && (bCallStatus.status==StatusTerminated) && (bCallStatus.voiceStatus==VoiceStatusEndedCall)) {
			// B-Call -- call switch -> Bcall end condition
			XCALLLOGI("calltype - B-Call(call switch : bcall end condition) \n");

			updatedCallInfo.callType = CALL_TYPE_BREAKDOWN_CALL;
			updatedCallInfo.callId = bCallStatus.callId;
		//	ret = updateVoiceStatus(iCallStatus, updatedCallInfo, isSdnCall);
			updatedCallInfo.callStatus = CALL_STATUS_ENDCALL;

			//check premiumAudio flag
			if((curCallInfo.audioStatus.spkStatus == PREMIUM_AUDIO_STATUS_OK) && (curCallInfo.audioStatus.micStatus == PREMIUM_AUDIO_STATUS_OK)){

				XCALLLOGI("(CallSwitch)release Premium Audio");
				endCall();

		//		svcServer->NotifyCallStatusQueryEvent(curCallInfo.callId, curCallInfo.callType, callStatusInformation.c_str());
				//release  mainConnectionID from Audio Manager
				ret = svcClient->releaseAudioMangerResource(audioMainConnctionID);
				if (ret) {
					XCALLLOGI("(CallSwitch)release mainConnectionID(%d) from Audio Manager", audioMainConnctionID);
				} else {
					XCALLLOGE("(CallSwitch)ERROR mainConnectionID(%d) release fail", audioMainConnctionID);
				}
				audioMainConnctionID =0;
			} else {
				XCALLLOGI("(CallSwitch)use Tem sid Backup Audio");
		//		svcServer->NotifyCallStatusQueryEvent(curCallInfo.callId, curCallInfo.callType, callStatusInformation.c_str());
			}
			initValue();

		}
	} else if (callStatusChanged(&mPreviousCallStatus[2] , &iCallStatus)) {
		XCALLLOGI("------------------ I CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[2], &iCallStatus, sizeof(CallStatus_t));
		if (iCallStatus.status == StatusServiceNotAvailable) {
				// I-Call
				XCALLLOGI("calltype - I-Call Not available\n");
				svcServer->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: Service not available");
		} else if (iCallStatus.status == StatusActive) {
			// I-Call
			XCALLLOGI("calltype - I-Call\n");
			updatedCallInfo.callType = CALL_TYPE_INFORMATION_CALL;
			updatedCallInfo.callId = iCallStatus.callId;
			ret = updateVoiceStatus(iCallStatus, updatedCallInfo, isSdnCall);
		} else if ((curCallInfo.callStatus == CALL_STATUS_STANDBY) && (iCallStatus.status==StatusTerminated) && (iCallStatus.voiceStatus==VoiceStatusEndedCall)) {
			// I-Call -- call switch -> icall end condition
			XCALLLOGI("calltype - I-Call(call switch : icall end condition) \n");

			updatedCallInfo.callType = CALL_TYPE_INFORMATION_CALL;
			updatedCallInfo.callId = iCallStatus.callId;
		//	ret = updateVoiceStatus(iCallStatus, updatedCallInfo, isSdnCall);
			updatedCallInfo.callStatus = CALL_STATUS_ENDCALL;

			//check premiumAudio flag
			if((curCallInfo.audioStatus.spkStatus == PREMIUM_AUDIO_STATUS_OK) && (curCallInfo.audioStatus.micStatus == PREMIUM_AUDIO_STATUS_OK)){

				XCALLLOGI("(CallSwitch)release Premium Audio");
				endCall();

		//		svcServer->NotifyCallStatusQueryEvent(curCallInfo.callId, curCallInfo.callType, callStatusInformation.c_str());
				//release  mainConnectionID from Audio Manager
				ret = svcClient->releaseAudioMangerResource(audioMainConnctionID);
				if (ret) {
					XCALLLOGI("(CallSwitch)release mainConnectionID(%d) from Audio Manager", audioMainConnctionID);
				} else {
					XCALLLOGE("(CallSwitch)ERROR mainConnectionID(%d) release fail", audioMainConnctionID);
				}
				audioMainConnctionID =0;
			} else {
				XCALLLOGI("(CallSwitch)use Tem sid Backup Audio");
		//		svcServer->NotifyCallStatusQueryEvent(curCallInfo.callId, curCallInfo.callType, callStatusInformation.c_str());
			}
			initValue();

		}
	} else if (callStatusChanged(&mPreviousCallStatus[3] , &sdnCallStatus)) {
		XCALLLOGI("------------------ SDN CALL STATUS CHANGED ------------------\n");
		memcpy(&mPreviousCallStatus[3], &sdnCallStatus, sizeof(CallStatus_t));
		if (sdnCallStatus.status == StatusActive) {
				// SDN-Call
				// SDN Call is same with e-call
				XCALLLOGI("calltype - SDN-Call \n");
				updatedCallInfo.callType = CALL_TYPE_SDN_CALL;
				updatedCallInfo.callId = sdnCallStatus.callId;
				isSdnCall = true;
				ret = updateVoiceStatus(sdnCallStatus, updatedCallInfo, isSdnCall);
		}
	} else {

		//clear button flag.
		isICallBtnPressed=0;

		XCALLLOGE("ERROR No calls changed");
		return ret;
	}

	if (ret) {
		ret = updateCallInfo(updatedCallInfo);
	}

	//clear button flag.
	isICallBtnPressed=0;

	XCALLLOGI(" >>> Next : %d %d %d %d (iCall btn flag=%d) ",curCallInfo.callType,curCallInfo.callStatus,curCallInfo.playerType,curCallInfo.playerStatus,isICallBtnPressed);

	return ret;
#endif
}

/**
 * ================================================================================
 * @fn : updateVoiceStatus
 * @brief : Function decides the call status from various information of call
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Parses the particular call information and decides the call status
 * - Updates the call status to local call status information variable
 * @param[in] CallStatusInfo  : Call status information
 * @param[in] updatedCallInfo : Locally stored call status information
 * @param[in] isSdnCall       : status variable about sdn call
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::updateVoiceStatus(CallStatus_t &CallStatusInfo, CallStatusInfo_t &updatedCallInfo, guint callid)
{
	guint activecallId = isActiveCallid();
	guint holdCallId = isHeldCallid();
	guint updateCallId = XCALL_ID_MAX;
	bool ret = true; 

	XCALLLOGI("updateVoiceStatus start\n");

	//check current or held for updating
	if (activecallId == callid) {
		updateCallId = activecallId;
	} else if(holdCallId == callid) {
		updateCallId = holdCallId;
	} else {
		XCALLLOGI("updateCallId=XCALL_ID_MAX\n");
	}

	switch (callid)
	{
		case XCALL_ID_ECALL:
			updatedCallInfo.callType = CALL_TYPE_EMERGENCY_CALL;
			updatedCallInfo.callId = CallStatusInfo.callId;
		break;
		case XCALL_ID_BCALL:
			updatedCallInfo.callType = CALL_TYPE_BREAKDOWN_CALL;
			updatedCallInfo.callId = CallStatusInfo.callId;
		break;
		case XCALL_ID_ICALL:
			updatedCallInfo.callType = CALL_TYPE_INFORMATION_CALL;
			updatedCallInfo.callId = CallStatusInfo.callId;
		break;
		case XCALL_ID_SDN:
			updatedCallInfo.callType = CALL_TYPE_SDN_CALL;
			updatedCallInfo.callId = CallStatusInfo.callId;
		break;
		default:
			XCALLLOGI("Error - Wrong CallID [%d]\n",callid);
		break;
	}

	if (CallStatusInfo.buttonStatus == ButtonPressed && CallStatusInfo.sbStatus == StandbyStatusNotActive && CallStatusInfo.voiceStatus == VoiceStatusNoConnection) {

		// Initializing state
		XCALLLOGI("callStatus - Initial\n");
		updatedCallInfo.callStatus = CALL_STATUS_INITIALIZING;
	} else if (CallStatusInfo.buttonStatus == ButtonReleased && CallStatusInfo.sbStatus == StandbyStatusNotActive && CallStatusInfo.voiceStatus == VoiceStatusNoConnection) {

		// Initializing state -> call end
		// If condition is work around for avoiding StandBy to Initial cancel
		if(updateCallId != XCALL_ID_MAX) {
			if (curCallInfo[updateCallId].callStatus == CALL_STATUS_INITIALIZING) {
				XCALLLOGI("callStatus - Initial - Cancel \n");
				updatedCallInfo.callStatus = CALL_STATUS_ENDCALL;
			} else {
				XCALLLOGI("ERROR : get btn release on no initial - ignore this cmd \n");
				return false;
			}
		}else {
			XCALLLOGI("ERROR : VoiceStatus=noConnection --> nothing to update \n");
			return false;
		}
	} else if (CallStatusInfo.sbStatus == StandbyStatusActive && CallStatusInfo.voiceStatus == VoiceStatusNoConnection) {

		// StandBy mode state
		XCALLLOGI("callStatus - Standby \n");
		updatedCallInfo.callStatus = CALL_STATUS_STANDBY;
	} else if (CallStatusInfo.sbStatus == StandbyStatusCancel && CallStatusInfo.voiceStatus == VoiceStatusNoConnection) {

		// StandBy mode cancel
		XCALLLOGI("callStatus - Standby - Cancel \n");
		updatedCallInfo.callStatus = CALL_STATUS_ENDCALL;
	} else {

		switch (CallStatusInfo.voiceStatus) {
			case VoiceStatusConnectingCSC: //connectingCSC(1)
			case VoiceStatusConnectingPSAP: //connectingPSAP(2)
				{
					XCALLLOGI("callStatus - Calling\n");
					updatedCallInfo.callStatus = CALL_STATUS_CALLING;
				}
				break;
			case VoiceStatusConnectedCSC:	//connectedCSC (3)
			case VoiceStatusConnectedPSAP:	//connectedPSAP (4)
			case VoiceStatusConnectedCall:	//connectedCall (6)
			case VoiceStatusConnectedIncoming:	//connectedIncoming (7)
				{
					XCALLLOGI("callStatus - Connected\n");
					updatedCallInfo.callStatus = CALL_STATUS_CONNECTED;
				}
				break;
			case VoiceStatusIncomingCall: // incomingCall (5)
				{
					XCALLLOGI("callStatus - Incoming\n");
					updatedCallInfo.callStatus = CALL_STATUS_INCOMING;
				}
				break;
			case VoiceStatusNoConnection: //noConnection (0)
				{
					XCALLLOGI("callStatus - NoConnection\n");
					updatedCallInfo.callStatus = CALL_STATUS_NONE;
					ret = false;
				}
				break;
			case VoiceStatusEndedCall: //endedcall (8)
				{
					XCALLLOGI("callStatus - VoiceStatusEndedCall\n");
					updatedCallInfo.callStatus = CALL_STATUS_ENDCALL;
				}
				break;
			case VoiceStatusCallRetry: //callretry (9)
				{
					XCALLLOGI("callStatus - VoiceStatusEnded for Retry\n");
					updatedCallInfo.callStatus = CALL_STATUS_RETRY;
				}
				break;
			default :
				{
					XCALLLOGE("ERROR callStatus - No Match\n");
					updatedCallInfo.callStatus = CALL_STATUS_NONE;
					ret = false;
				}
				break;
		}
	}

	XCALLLOGI("updatedCallStatus=%d \n", updatedCallInfo.callStatus);
	return ret;
}

/**
 * ================================================================================
 * @fn : updateCallInfo
 * @brief : Function that maintains the call state sequence and notifications
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Check the call state sequence by previous status and current status
 * - Emit the call status information on valid status changes
 * - Request audio manager for audio resources
 * - Request for voice pipeline for corresponding status
 * @param[in] CallStatusInfo  : Call status information
 * @param[in] updatedCallInfo : Locally stored call status information
 * @param[in] isSdnCall       : status variable about sdn call
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ===================================================================================
 */
bool XcallManager::updateCallInfo(CallStatusInfo_t &callInfo)
{
	bool ret = true;
	std::string callStatusInformation;


	guint curCallId = isActiveCallid();
	guint holdCallId = isHeldCallid();
	guint curStreamId = isRunningStreamingPlayer();
	CallStatusInfo_t *pCurCall;

	if ((curCallId != XCALL_ID_MAX) && (curCallId != callInfo.callId) &&
		(holdCallId == XCALL_ID_MAX)) {
		XCALLLOGI("HOLD AND PAUSE the call [%d]\n", curCallId);
		setActiveStatus(curCallId, HOLDCALL);
		//pauseStreamingPlayer(curCallId);

		//curCallId = isActiveCallid();
		holdCallId = isHeldCallid();
	}

	if ((curCallId == XCALL_ID_MAX) || (curCallId == callInfo.callId)) {

		if (curCallId == XCALL_ID_MAX) {
			XCALLLOGI("New Call | No Info -> newStatus[%d]\n",callInfo.callStatus);

			//set active call
			setActiveStatus(callInfo.callId, ACTIVECALL);
			curCallId = isActiveCallid();
			pCurCall = &curCallInfo[callInfo.callId];

		} else {
			pCurCall = &curCallInfo[curCallId];
			XCALLLOGI("Same Call | curStatus[%d] -> newStatus[%d]\n", pCurCall->callStatus, callInfo.callStatus);
		}

		//update basic information
		pCurCall->callId = callInfo.callId;
		pCurCall->callType = callInfo.callType;

		//check same status 
		if(pCurCall->callStatus == callInfo.callStatus) {
			XCALLLOGI("Same Status | No need to update curStatus[%d] -> newStatus[%d] curActive[%d]\n", pCurCall->callStatus, callInfo.callStatus, pCurCall->active);

			displayXcallStatus();

			//if status=end and handle->active=true then clear the value
			if ((callInfo.callStatus == CALL_STATUS_ENDCALL) && (pCurCall->callStatus == CALL_STATUS_ENDCALL) && (pCurCall->active != NOTACTIVECALL) ) {
				XCALLLOGI("Same Status | clear active status \n");
				setActiveStatus(callInfo.callId, NOTACTIVECALL);
			}
			return ret;
		}

		//update each status and streaming player
		switch (callInfo.callStatus) {
			case CALL_STATUS_CALLING:
			case CALL_STATUS_INCOMING: {
				//001. update app
					//CID 6797220 (#1 of 1): Constant expression result (CONSTANT_EXPRESSION_RESULT)
				if ((pCurCall->callStatus != CALL_STATUS_CALLING) && (pCurCall->callStatus != CALL_STATUS_INCOMING)) {
					//004. update status info
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					XCALLLOGI(" -> INCOMING or CALLING %d\n", pCurCall->callStatus);
					svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
					if (isPremiumAudioWorking()) {
						startStreamingPlayer(pCurCall->callId);
					}
				} else {
					XCALLLOGI("[CHECK]Same Cmd [%d] \n",callInfo.callStatus);
				}
			} break;
			case CALL_STATUS_CONNECTED: {
				if (pCurCall->callStatus != CALL_STATUS_CONNECTED) {
					XCALLLOGI(" -> CONNECTED %d\n", pCurCall->callStatus);
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[CALL_STATUS_CONNECTED];
					svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
				} else {
					XCALLLOGI("[CHECK]Same Connected cmd [%d] \n",callInfo.callStatus);
				}
			} break;
			case CALL_STATUS_ENDCALL: {
				if (pCurCall->callStatus != CALL_STATUS_ENDCALL) {
					switch(pCurCall->callStatus) {
						case CALL_STATUS_INITIALIZING:
						case CALL_STATUS_STANDBY: {

							XCALLLOGI("STANDBY or INITIALIZING --> CALL_STATUS_ENDCALL  %d\n", pCurCall->callStatus);

							if((pCurCall->callStatus == CALL_STATUS_INITIALIZING) && (holdCallId != XCALL_ID_MAX)){
								XCALLLOGI("No send disconnect event to app \n");
							} else {
								callStatusInformation = CALL_STATUS_MAPPING[callInfo.callStatus];
								svcServer->NotifyCallStatusQueryEvent(callInfo.callId, callInfo.callType, callStatusInformation.c_str());
							}
							pCurCall->callStatus = callInfo.callStatus;
						}break;
						default : {
							XCALLLOGI(" --> CALL_STATUS_ENDCALL  %d\n", pCurCall->callStatus);
							callStatusInformation = CALL_STATUS_MAPPING[callInfo.callStatus];
							svcServer->NotifyCallStatusQueryEvent(callInfo.callId, callInfo.callType, callStatusInformation.c_str());

							pCurCall->callStatus = callInfo.callStatus;

							//no need to check PremiumAudio Status on callend
							//if streaming or audioMainID from audio manager is using then we have to clear it on this status

							//if(isPremiumAudioWorking()) {
								//stop streaming player
								stopStreamingPlayer(pCurCall->callId);
							//}
						}break;
					}

					setActiveStatus(callInfo.callId, NOTACTIVECALL);
					guint heldCallId = isHeldCallid();
					if (heldCallId != XCALL_ID_MAX) {
						XCALLLOGI("ACTIVE AND RESUME the call [%d]\n", heldCallId);
						setActiveStatus(heldCallId, ACTIVECALL);

						//no plan to resume player
						//resumeStreamingPlayer(heldCallId);
					}
				}
			} break;
			case CALL_STATUS_INITIALIZING: {
				if (pCurCall->callStatus != CALL_STATUS_INITIALIZING) {
					bool sendStatus = true;
					if((curCallId != XCALL_ID_MAX) && (curCallId != callInfo.callId))
						sendStatus = false;
					XCALLLOGI("CALL_STATUS_INITIALIZING %d sendStatus=%d\n", pCurCall->callStatus, sendStatus);
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					if(sendStatus)
						svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
				}
			} break;
			case CALL_STATUS_STANDBY: {
				if (pCurCall->callStatus != CALL_STATUS_STANDBY) {
					XCALLLOGI("CALL_STATUS_STANDBY %d\n", pCurCall->callStatus);
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());

//					if(holdCallId != XCALL_ID_MAX) {
//						pauseStreamingPlayer(holdCallId);
//						XCALLLOGI("PAUSE the call [%d]\n", holdCallId);
//					}
					}
			} break;
			case CALL_STATUS_RETRY: {
				if (pCurCall->callStatus != CALL_STATUS_RETRY) {
					XCALLLOGI("CALL_STATUS_RETRY %d\n", pCurCall->callStatus);
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
					stopStreamingPlayer(pCurCall->callId);
				}
			} break;
			default : {
				XCALLLOGE("ERROR check prev & current callStatus .... No Match [%d -> %d] \n",pCurCall->callStatus,callInfo.callStatus);
				ret = false;
			}
		}

	} else	{

		XCALLLOGI("[Switch] curCall[%d],Status[%d] -> newCall[%d],Status[%d]\n",curCallInfo[curCallId].callType, curCallInfo[curCallId].callStatus, callInfo.callType, callInfo.callStatus);

		//change current call handle
		pCurCall = &curCallInfo[callInfo.callId];

		pCurCall->callId = callInfo.callId;
		pCurCall->callType = callInfo.callType;

		switch(callInfo.callStatus) {
			case CALL_STATUS_CALLING:{	// Calling  during CALL SWITCH

				// I or B or E call --> SDN Call
				if (pCurCall->callStatus != CALL_STATUS_CALLING) {

					setActiveStatus(callInfo.callId, ACTIVECALL);
					pCurCall->callStatus = callInfo.callStatus;
					callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					XCALLLOGI("[Switch] -> INCOMING or CALLING %d\n", pCurCall->callStatus);
					svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
					if (isPremiumAudioWorking()) {
						startStreamingPlayer(pCurCall->callId);
					}
				} else {
					XCALLLOGI("[Switch][CHECK]Same Cmd [%d] \n",callInfo.callStatus);
				}
			} break;
			case CALL_STATUS_INITIALIZING: { // Initial Call during CALL SWITCH
				// I or B Call --> B or E Call 
				if (pCurCall->callStatus != CALL_STATUS_INITIALIZING) {					

					XCALLLOGI("[Switch] CALL_STATUS_INITIALIZING %d ", pCurCall->callStatus);
					pCurCall->callStatus = callInfo.callStatus;

					setActiveStatus(callInfo.callId, ACTIVECALL);
					//callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
					//svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
				}
			} break;
			case CALL_STATUS_ENDCALL: { // End Call during CALL SWITCH

				if (pCurCall->callStatus != CALL_STATUS_ENDCALL){
					//callStatusInformation = CALL_STATUS_MAPPING[callInfo.callStatus];
					//svcServer->NotifyCallStatusQueryEvent(callInfo.callId, callInfo.callType, callStatusInformation.c_str());
					switch(pCurCall->callStatus) {
						case CALL_STATUS_INITIALIZING:
						case CALL_STATUS_STANDBY: {
							pCurCall->callStatus = callInfo.callStatus;
							XCALLLOGI("[Switch]STANDBY or INITIALIZING --> CALL_STATUS_ENDCALL	%d\n", pCurCall->callStatus);

							//No need to update app
							//callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
							//svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());
						}break;
						default : {
							pCurCall->callStatus = callInfo.callStatus;
							XCALLLOGI("[Switch] --> CALL_STATUS_ENDCALL  %d\n", pCurCall->callStatus);

							callStatusInformation = CALL_STATUS_MAPPING[pCurCall->callStatus];
							//TODO : Need to check whether app is ready for sending disconnect during switching
							svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, callStatusInformation.c_str());

							//no need to check PremiumAudio Status on callend
							//if streaming or audioMainID from audio manager is using then we have to clear it on this status

							//if(isPremiumAudioWorking()) {
								//stop streaming player
								stopStreamingPlayer(pCurCall->callId);
							//}
						}break;
					}

					setActiveStatus(callInfo.callId, NOTACTIVECALL);
					guint heldCallId = isHeldCallid();
					if (heldCallId != XCALL_ID_MAX) {
						XCALLLOGI("[Switch]ACTIVE AND RESUME the call 111 [%d]\n", heldCallId);
						setActiveStatus(heldCallId, ACTIVECALL);

						//no plan to resume player
						//resumeStreamingPlayer(heldCallId);
					}
				}else {
					guint heldCallId = isHeldCallid();
					if (heldCallId != XCALL_ID_MAX) {
						XCALLLOGI("[Switch]ACTIVE AND RESUME the call 222 [%d]\n", heldCallId);
						setActiveStatus(heldCallId, ACTIVECALL);
					}
				}
			}break;
			default : {
				XCALLLOGE("ERROR [Switch] .... No Match [%d -> %d] \n",pCurCall->callStatus,callInfo.callStatus);
				ret = false;
			}
		}
	}

	return ret;
}

/**
 * ================================================================================
 * @fn : ServiceActivationSetRequest
 * @brief : Request function for service activation API
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forward the service activation request to Client component
 * @param[in] arg_service <2 = InformationCall> : accept or decline call
 * @param[in] arg_action <1=Off 2-On>           : service activation on or off
 * @param[in] callback                          : asynchronous call back function
 * @param[in] user_data                         : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::ServiceActivationSetRequest (guint arg_service, guint arg_action,
									GAsyncReadyCallback callback, gpointer user_data)
{
	XCALLLOGI("arg_service=%d arg_action=%d", arg_service, arg_action);

	svcClient->ServiceActivationSetRequest(arg_service, arg_action,
											callback,
											user_data);

	//set flag to 1 .... for checking icall button press
	isICallBtnPressed = 1;
}

/**
 * ================================================================================
 * @fn : ServiceActivationSetResponseCb
 * @brief : Response function called on result of the ServiceActivation
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the service activation API results received to Server component
 * @param[in] err           : error information object
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XcallManager::ServiceActivationSetResponseCb (GError * err, gpointer user_data)
{

	XCALLLOGI("ServiceActivationSetResponse \n");
	svcServer->ServiceActivationSetResponseCb(err, user_data);

}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequest
 * @brief : Request function for Call handling API
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forward the service activation request to Client component
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
void XcallManager::CallHandlingSetRequest (guint arg_action, guint8 arg_callId,
									GAsyncReadyCallback callback, gpointer user_data)
{

	XCALLLOGI("arg_action=%d arg_callId=%d", arg_action, arg_callId);
	svcClient->CallHandlingSetRequest(arg_action, arg_callId,
											callback,
											user_data);

}

/**
 * ================================================================================
 * @fn : CallHandlingSetRequestResponseCb
 * @brief : Response function called on result of the Call Handling
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forwards the service activation API results received to Server component
 * @param[in] err           : error information object
 * @param[in] user_data     : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XcallManager::CallHandlingSetRequestResponseCb (GError * err, gpointer user_data)
{

	XCALLLOGI("CallHandlingSetRequestResponse \n");
	svcServer->CallHandlingSetRequestResponseCb (err, user_data);

}

/**
 * ================================================================================
 * @fn : SOSStatusResponseCb
 * @brief : Response function called on result of the SOS status information
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Check the SOS status information
 * - Notifes the error information on specific SOS status
 * @param[in] err       : error information object
 * @param[in] SosStatus : SOS status information object
 * @param[in] user_data : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void XcallManager::SOSStatusResponseCb (GError * err, guint SosStatus, gpointer user_data)
{

	XCALLLOGI("SOSStatusResponseCb %d \n", SosStatus);
	SOS_STATUS_T upatedStatus = static_cast<SOS_STATUS_T>(SosStatus);
	if (mSosStatus != upatedStatus) {
		mSosStatus = upatedStatus;
		switch (mSosStatus) {
			case SOS_STATUS_LIMITED:
			case SOS_STATUS_NOT_AVAILABLE:
				XCALLLOGI("SOS service NOT AVAILABLE\n");
				svcServer->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: No response");
				break;
			case SOS_STATUS_AVAILABLE:
				XCALLLOGI("SOS service AVAILABLE\n");
				break;
			case SOS_STATUS_UNKNOWN:
			default:
				XCALLLOGI("SOS service Invalid\n");
				break;
		}
	}

}

/**
 * ================================================================================
 * @fn : notifySOSStatusEvent
 * @brief : Handler function called on SOS status information event received
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Check the SOS status information
 * - Notifes the error information on specific SOS status
 * @param[in] SosStatus : SOS status information object
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::notifySOSStatusEvent(guint SosStatus)
{
	XCALLLOGI("SOSStatusResponseCb %d \n", SosStatus);
	SOS_STATUS_T upatedStatus = static_cast<SOS_STATUS_T>(SosStatus);
	if (mSosStatus != upatedStatus) {
		mSosStatus = upatedStatus;
		switch (mSosStatus) {
		case SOS_STATUS_LIMITED:
		case SOS_STATUS_NOT_AVAILABLE:
			XCALLLOGI("SOS service NOT AVAILABLE\n");
			svcServer->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: No response");
			break;
		case SOS_STATUS_AVAILABLE:
			XCALLLOGI("SOS service AVAILABLE\n");
			break;
		case SOS_STATUS_UNKNOWN:
		default:
			XCALLLOGI("SOS service Invalid\n");
			break;
		}
	}
	return true;
}

bool XcallManager::UpdatePremiumAudioDeviceStatus(CALL_DIRECTION_T direction)
{
	bool ret = true;
	XCALLLOGI("UpdatePremiumAudioDeviceStatus() is called \n");


	if (direction == DBUS_CALL_DIRECTION_TO_MMUS) {
		svcClient->getDeviceStatusFromMMUS();
	} else if(direction == DBUS_CALL_DIRECTION_TO_TEMIL) {

		guint spkStatus=0;
		guint micStatus=0;

		GetPremiumAudioSPKStatus(&spkStatus);
		GetPremiumAudioMICStatus(&micStatus);

		svcServer->NotifyPremiumAudioEvent(spkStatus,micStatus);
	}

	return ret;
}

/**
 * ================================================================================
 * @fn : SetPremiumAudioSPKStatus
 * @brief : Utility function for updating the premium audio speaker status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Updates the member variable related to speaker status
 * @param[in] spkStatus : Premium audio speaker status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::SetPremiumAudioSPKStatus(bool spkStatus)
{
	bool ret = true;

	//TODO : check how we will get hw status information from micom manager
	//            now jsut use bool temporarilly

	if (spkStatus) {
		audioDevStatus.spkStatus = PREMIUM_AUDIO_STATUS_OK;
	} else {
		audioDevStatus.spkStatus = PREMIUM_AUDIO_STATUS_NOT_OK;
	}
	XCALLLOGI("Set(SPK)PremiumAudio[spk(%d), mic(%d)]", audioDevStatus.spkStatus ,audioDevStatus.micStatus);
	return ret;
}

/**
 * ================================================================================
 * @fn : SetPremiumAudioMICStatus
 * @brief : Utility function for updating the premium audio mic status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Updates the member variable related to mic status
 * @param[in] micStatus : Premium audio mic status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::SetPremiumAudioMICStatus(bool micStatus)
{
	bool ret = true;

	if (micStatus) {
		audioDevStatus.micStatus = PREMIUM_AUDIO_STATUS_OK;
	} else {
		audioDevStatus.micStatus = PREMIUM_AUDIO_STATUS_NOT_OK;
	}

	XCALLLOGI("Set(MIC)PremiumAudio[spk(%d), mic(%d)]", audioDevStatus.spkStatus ,audioDevStatus.micStatus);
	return ret;
}

/**
 * ================================================================================
 * @fn : GetPremiumAudioSPKStatus
 * @brief : Utility function for getting the premium audio speaker status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Get speaker status from member variables
 * @param[in] spkStatus : Premium audio speaker status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::GetPremiumAudioSPKStatus(guint *spkStatus)
{
	bool ret = true;

	if (audioDevStatus.spkStatus == PREMIUM_AUDIO_STATUS_OK) {
		*spkStatus = PREMIUM_AUDIO_STATUS_OK;
	} else {
		*spkStatus = PREMIUM_AUDIO_STATUS_NOT_OK;
	}

	XCALLLOGI("Get(SPK)PremiumAudio[spk(%d)]",*spkStatus);
	return ret;
}

/**
 * ================================================================================
 * @fn : GetPremiumAudioMICStatus
 * @brief : Utility function for getting the premium audio mic status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Get mic status from member variables
 * @param[in] micStatus : Premium audio mic status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::GetPremiumAudioMICStatus(guint *micStatus)
{
	bool ret = true;

	if (audioDevStatus.micStatus == PREMIUM_AUDIO_STATUS_OK) {
		*micStatus = PREMIUM_AUDIO_STATUS_OK;
	} else {
		*micStatus = PREMIUM_AUDIO_STATUS_NOT_OK;
	}

	XCALLLOGI("Get(MIC)PremiumAudio[mic(%d)]",*micStatus);
	return ret;
}

bool XcallManager::isPremiumAudioWorking(void)
{
	bool ret=false;

	if ((audioDevStatus.spkStatus == PREMIUM_AUDIO_STATUS_OK) && (audioDevStatus.micStatus == PREMIUM_AUDIO_STATUS_OK)) {
		XCALLLOGI("use PremiumAudio [spk=%d mic=%d]",audioDevStatus.spkStatus, audioDevStatus.micStatus);
		ret = true;
	} else {
		ret = false;
		XCALLLOGI("use TEM Backup [spk=%d mic=%d]",audioDevStatus.spkStatus, audioDevStatus.micStatus);
	}

	return ret;
}

/**
 * ================================================================================
 * @fn : unitTest
 * @brief : unit test case function
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - unit test function as per the test name with parameters
 * @param[in] testName : unit test case name
 * @param[in] param1   : data to be used for unit test case
 * @param[in] param2   : data to be used for unit test case
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
bool XcallManager::unitTest(char *testName, char *param1, char *param2)
{
	bool ret =true;

#if 0
	if (!strcmp(testName,"PremiumAudio")) {
		guint spkStatus=atoi(param1);
		guint micStatus=atoi(param2);
		guint prevSpkstatus =0;
		guint prevMicstatus =0;

		GetPremiumAudioMICStatus(&prevMicstatus);
		GetPremiumAudioSPKStatus(&prevSpkstatus);

		XCALLLOGI("unitTest(PremiumAudio)| spk[%d] mic[%d] ",spkStatus,micStatus);

		if ((spkStatus==0 || spkStatus==1) && (micStatus==0 || micStatus==1)) {

			XCALLLOGI("unitTest | change status value spk[%d -> %d] mic[%d -> %d]",prevSpkstatus,spkStatus,prevMicstatus,micStatus);
			SetPremiumAudioSPKStatus(spkStatus);
			SetPremiumAudioMICStatus(micStatus);

			//현재 상태가 calling와 connected 인경우만 event 발생
			if ((curCallInfo.callStatus == CALL_STATUS_CALLING) ||
				(curCallInfo.callStatus == CALL_STATUS_INCOMING) ||
				(curCallInfo.callStatus == CALL_STATUS_CONNECTED)){

				XCALLLOGI("unitTest | call NotifyPremiumAudioEvent");
				svcServer->NotifyPremiumAudioEvent(spkStatus,micStatus);
			}

		} else if((spkStatus==2) && (micStatus==2)) {
			XCALLLOGI("unitTest | call getDeviceStatusFromMMUS[Ask System Last Status] to MMUS");
			svcClient->getDeviceStatusFromMMUS();
		}

	} else {
		XCALLLOGI("unitTest | wrong testName = %s",testName);
		ret = false;
	}
#endif

	return ret;
}

/**
 * ================================================================================
 * @fn : GetCarConfigInfoRequest
 * @brief : Request function for car config information with particular index value
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Forward the service activation request to Client component
 * @param[in] dataIndex   : index value to be read from car config info data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
bool XcallManager::GetCarConfigInfoRequest(guint dataIndex)
{
	XCALLLOGI("GetCarConfigInfoRequest(%d)\n",dataIndex);
	svcClient->GetCarConfigInfoRequest(dataIndex, nullptr, nullptr);
	return true;
}

/**
 * ================================================================================
 * @fn : setSourceInformation
 * @brief : Fucntion to provide the IP, port, encoding scheme details to Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - forwards the data to player instance
 * @param[in] xcall_debug_input   : Structure contains IP, Port, encoding scheme details
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
#ifdef DEBUG_ENABLED
void XcallManager::setSourceInformation(xcall_debug_input input) {
	XCALLLOGD("start");
	this->input = input;
}
#endif

/**
 * ================================================================================
 * @fn : callStatusChanged
 * @brief : Utility function for checking the call status changed or not
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Compare the previous call status and current call status
 * - return the true if call status changed else false
 * @param[in] prevStatus : previous call status
 * @param[in] newStatus  : new call status
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool XcallManager::callStatusChanged(CallStatus_t *prevStatus, CallStatus_t* newStatus) {
	XCALLLOGI("prevStatus: [%d %d %d %d]\n",prevStatus->status, prevStatus->voiceStatus, prevStatus->buttonStatus, prevStatus->sbStatus);
	XCALLLOGI("newStatus: [%d %d %d %d]\n",newStatus->status, newStatus->voiceStatus, newStatus->buttonStatus, newStatus->sbStatus);
	if (	(prevStatus->status != newStatus->status) || (prevStatus->voiceStatus != newStatus->voiceStatus)
			|| (prevStatus->buttonStatus != newStatus->buttonStatus) || (prevStatus->sbStatus != newStatus->sbStatus)) {
		return true;
	} else {

		//check whether same call is valid or not
		if ((newStatus->callId == XCALL_ID_ICALL) && (newStatus->status == StatusServiceNotAvailable) && (isICallBtnPressed==1)) {
			XCALLLOGI("I-Call ServiceNotavailable ---> need to display popup \n");
			return true;
		} else {
			return false;
		}

	}

}

bool XcallManager::setActiveStatus(guint callid, ACTIVE_STATUS_T status)
{
	XCALLLOGI("setActiveStatus|callid[%d], status[%d]",callid,status);

	if (callid==XCALL_ID_MAX) return false;

	curCallInfo[callid].active = status;
	return true;
}

guint XcallManager::isActiveCallid(void)
{
	guint i=XCALL_ID_SDN;

	//check current active call id
	while (i<XCALL_ID_MAX) {
		if (curCallInfo[i].active == ACTIVECALL) {
			break;
		}
		i++;
	}

	if (i==XCALL_ID_MAX) XCALLLOGI("No Active Call\n");

	return i;
}

guint XcallManager::isHeldCallid(void)
{
	guint i=XCALL_ID_SDN;

	//check current active call id
	while (i<XCALL_ID_MAX) {
		if (curCallInfo[i].active == HOLDCALL) {
			break;
		}
		i++;
	}

	if (i==XCALL_ID_MAX) XCALLLOGI("No Held Call\n");

	return i;
}

guint XcallManager::isRunningStreamingPlayer(void)
{
	guint i=XCALL_ID_SDN;

	//check currrent running player
	while (i<XCALL_ID_MAX) {
		if ((curCallInfo[i].playerStatus== PLAYER_STATUS_PLAY) ||(curCallInfo[i].playerStatus== PLAYER_STATUS_PAUSE)) {
			break;
		}
		i++;
	}

	if (i==XCALL_ID_MAX) XCALLLOGI("No running player\n");

	return i;
}

void XcallManager::startStreamingPlayer(guint callid)
{

	bool ret = false;
	CallStatusInfo_t *streamingPlayer;
	std::string callStatusInformation;

	if (callid == XCALL_ID_MAX) return;

	streamingPlayer = &curCallInfo[callid];
	XCALLLOGI("startStreamingPlayer callid =%d, callStatus=%d", curCallInfo[callid].callId, curCallInfo[callid].callStatus);
	callStatusInformation = CALL_STATUS_MAPPING[streamingPlayer->callStatus];
	XCALLLOGI("startStreamingPlayer callid =%d, callStatus=%s", callid, callStatusInformation.c_str());

	//acquire mainConnectionID from Audio Manager
	if (audioMainConnctionID <=0) {
		ret = svcClient->acquireAudioMangerResource(&audioMainConnctionID, streamingPlayer->callType);

		if (ret) {
			XCALLLOGI("get mainConnectionID(%d) from Audio Manager", audioMainConnctionID);
			startCall(callStatusInformation.c_str());
	//		svcServer->NotifyCallStatusQueryEvent(streamingPlayer->callId, streamingPlayer->callType, callStatusInformation.c_str());
			conntectedCall();
		} else {
			XCALLLOGE("ERROR mainConnectionID(%d) is invalid", audioMainConnctionID);
		}
	} else {
		XCALLLOGE(" audio resource mainConnectionID(%d) is already exists.Not requesting new", audioMainConnctionID);
		guint heldCallId = isHeldCallid();
		if (heldCallId != XCALL_ID_MAX) {
			XCALLLOGI("RELEASE THE HOLD CALL id %d for new call\n", heldCallId);
			streamingPlayer = &curCallInfo[heldCallId];

			if ((streamingPlayer->playerStatus != PLAYER_STATUS_STOP) && (streamingPlayer->playerStatus != PLAYER_STATUS_NONE)) {
				XCALLLOGI("[PlayerStatus:%d] call player->unload",streamingPlayer->playerStatus);
				player->Unload();
			} else {
				XCALLLOGI("[PlayerStatus:%d] ignore player->unload",streamingPlayer->playerStatus);
			}
			curPlayerType = PLAYER_TYPE_NONE;
			streamingPlayer->playerStatus = PLAYER_STATUS_STOP;
			setActiveStatus(heldCallId, NOTACTIVECALL);
		}
		/*ret = svcClient->acquireAudioMangerResource(&audioMainConnctionID, streamingPlayer->callType);

		if (ret) {
			XCALLLOGI("get mainConnectionID(%d) from Audio Manager", audioMainConnctionID);
			startCall(callStatusInformation.c_str());
			svcServer->NotifyCallStatusQueryEvent(streamingPlayer->callId, streamingPlayer->callType, callStatusInformation.c_str());
			conntectedCall();
		} else {
			XCALLLOGE("ERROR mainConnectionID(%d) is invalid", audioMainConnctionID);
		}*/
		startCall(callStatusInformation.c_str());
		//svcServer->NotifyCallStatusQueryEvent(streamingPlayer->callId, streamingPlayer->callType, callStatusInformation.c_str());
		conntectedCall();

	}
}

void XcallManager::stopStreamingPlayer(guint callid)
{
	bool ret = false;
	CallStatusInfo_t *streamingPlayer = &curCallInfo[callid];

	//release  mainConnectionID from Audio Manager
	if (audioMainConnctionID >0) {
		ret = svcClient->releaseAudioMangerResource(audioMainConnctionID);
		if (ret) {
			XCALLLOGI("release mainConnectionID(%d) from Audio Manager", audioMainConnctionID);
		} else {
			XCALLLOGE("ERROR mainConnectionID(%d) release fail", audioMainConnctionID);
		}
		audioMainConnctionID =0;
	}

	if((streamingPlayer->playerStatus == PLAYER_STATUS_PLAY) || (streamingPlayer->playerStatus == PLAYER_STATUS_PAUSE)) {
		endCall();
	}

	//update player status
	streamingPlayer->playerStatus = PLAYER_STATUS_STOP;
}

void XcallManager::pauseStreamingPlayer(guint callid)
{
	bool ret = false;
	CallStatusInfo_t *streamingPlayer = &curCallInfo[callid];

	player->Pause();
	//update player status
	streamingPlayer->playerStatus = PLAYER_STATUS_PAUSE;
}

void XcallManager::resumeStreamingPlayer(guint callid)
{
	bool ret = false;
	CallStatusInfo_t *streamingPlayer = &curCallInfo[callid];

	player->Resume();
	//update player status
	streamingPlayer->playerStatus = PLAYER_STATUS_PLAY;
}

void XcallManager::displayXcallStatus(void)
{
	for(int i=0; i< XCALL_ID_MAX; i++)
	{
		XCALLLOGI("ID[%d] Call: voice[%d] player[%d], active[%d] ",curCallInfo[i].callId, curCallInfo[i].callStatus, curCallInfo[i].playerStatus, curCallInfo[i].active);
	}
}

#ifdef TIME_MEASURE
void XcallManager::setStartTime(clock_t time)
{
	baseTime = time;
	XCALLLOGI("[TIME]setStartTime(tick) = [%f][%f] \n",(double)time, (double)baseTime);
}

void XcallManager::measureElapseTime(void)
{
	clock_t toc = clock();
	double elapsedTime = (double)(toc - baseTime);

	XCALLLOGI("[TIME]elapseTime(tick|s) = [%f][%f] \n",elapsedTime, (elapsedTime/CLOCKS_PER_SEC));

}
#endif

/**
 * ================================================================================
 * @fn : resetXcallStatus
 * @brief : reset function for call status
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - resets all the previous call information stored
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ===================================================================================
 */
void XcallManager::resetXcallStatus()
{
	bool ret = false;
	svcServer->NotifyErrorInformation("GDBus.Error:org.freedesktop.DBus.Error.Failed: TEM Connection Lost");
	
	guint streamingCallId = isRunningStreamingPlayer();
	guint activecallId = isActiveCallid();
	guint heldcallId = isHeldCallid();
	CallStatusInfo_t *pCurCall;

	if (activecallId != XCALL_ID_MAX) {
		pCurCall = &curCallInfo[activecallId];
		svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, "Disconnected");
	}

	if (heldcallId != XCALL_ID_MAX) {
		pCurCall = &curCallInfo[heldcallId];
		svcServer->NotifyCallStatusQueryEvent(pCurCall->callId, pCurCall->callType, "Disconnected");
	}

	if (streamingCallId != XCALL_ID_MAX) {
		//reset streaming player
		stopStreamingPlayer(streamingCallId);
	}

	//reset values
	initValue();
	XCALLLOGI("*********************RESET XCALL STATUS***********************\n");

}

} //temxcall

