

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

#ifndef __TEMXCALL_XCALL_EXPORT_H__
#define __TEMXCALL_XCALL_EXPORT_H__

#include <cstdint>
namespace temxcall {

#define UNITTEST_SPK_URL "file:///app/share/BluetoothHMI/wav/bell.wav"

#ifdef GEELY_HARDWARE
	#define CARCONFIG_ASSTSRV         87
	#define CARCONFIG_TELEMODULE     218
#endif

typedef enum CALL_STATUS
{
	CALL_STATUS_NONE,
	CALL_STATUS_INITIALIZING,
	CALL_STATUS_STANDBY,
	CALL_STATUS_CALLING,
	CALL_STATUS_INCOMING,
	CALL_STATUS_CONNECTED,
	CALL_STATUS_ENDCALL,
	CALL_STATUS_RETRY,
} CALL_STATUS_T;

typedef enum PLAYER_STATUS
{
	PLAYER_STATUS_NONE,
	PLAYER_STATUS_PLAY,
	PLAYER_STATUS_PAUSE,
	PLAYER_STATUS_STOP
} PLAYER_STATUS_T;

typedef enum PLAYER_TYPE
{
	PLAYER_TYPE_NONE,
	PLAYER_TYPE_RINGTONE,
	PLAYER_TYPE_VOICECALL,
	PLAYER_TYPE_UNITTEST
} PLAYER_TYPE_T;

typedef enum CALL_TYPE
{
	CALL_TYPE_NONE,
	CALL_TYPE_EMERGENCY_CALL,
	CALL_TYPE_BREAKDOWN_CALL,
	CALL_TYPE_INFORMATION_CALL,
	CALL_TYPE_SDN_CALL
} CALL_TYPE_T;

typedef enum PREMIUM_AUDIO_STATUS_TYPE
{
	PREMIUM_AUDIO_STATUS_NONE,
	PREMIUM_AUDIO_STATUS_OK,
	PREMIUM_AUDIO_STATUS_NOT_OK
}PREMIUM_AUDIO_STATUS_TYPE_T;

typedef struct PremiumAudioStatusInfo {

	PREMIUM_AUDIO_STATUS_TYPE_T spkStatus;
	PREMIUM_AUDIO_STATUS_TYPE_T micStatus;
}PremiumAudioStatusInfo_t;

typedef enum XCALL_ID
{
	XCALL_ID_SDN =0,
	XCALL_ID_ECALL =1,
	XCALL_ID_BCALL =2,
	XCALL_ID_ICALL =3,
	XCALL_ID_MAX=4
}XCALL_ID_T;

typedef enum ACTIVE_STATUS
{
	ACTIVECALL,
	HOLDCALL,
	NOTACTIVECALL
} ACTIVE_STATUS_T;

typedef struct CallStatusInfo {
	uint32_t callId;
	CALL_TYPE_T callType;
	CALL_STATUS_T callStatus;
	PLAYER_TYPE_T playerType;
	PLAYER_STATUS_T playerStatus;
	ACTIVE_STATUS_T active;
} CallStatusInfo_t;

typedef enum RescueStatus {
	StatusDisabled 				= 0,
	StatusNotActive 			= 1,
	StatusServiceNotAvailable 	= 2,
	StatusActive 				= 3,
	StatusTerminated 			= 4,
} RescueStatus_t;

typedef enum ButtonStatus {
	ButtonDisabled			= 0,
	ButtonPressed			= 1,
	ButtonReleased			= 2,
	ButtonAllReleased		= 3,
} ButtonStatus_t;

typedef enum VoiceStatus {
	VoiceStatusNoConnection		 = 0,
	VoiceStatusConnectingCSC	 = 1,
	VoiceStatusConnectingPSAP	 = 2,
	VoiceStatusConnectedCSC		 = 3,
	VoiceStatusConnectedPSAP	 = 4,
	VoiceStatusIncomingCall		 = 5,
	VoiceStatusConnectedCall	 = 6,
	VoiceStatusConnectedIncoming = 7,
	VoiceStatusEndedCall 		 = 8,
	VoiceStatusCallRetry		 = 9,
} VoiceStatus_t;

typedef enum SourceStatus {
	SourceStatusUnidentified	= 0,
	SourceStatusTrusted			= 1,
	SourceStatusCallCenter		= 2,
	SourceStatusPsap			= 3,
	SourceStatusOutgoing		= 4,
} SourceStatus_t;

typedef enum MessageStatus {
	MessageStatusNotSent		= 0,
	MessageStatusSending		= 1,
	MessageStatusssent			= 2,
	MessageStatusAckUndefined	= 3,
	MessageStatusAckDefined		= 4,
} MessageStatus_t;

typedef enum PSAPStatus {
	PSAPStatusConfirmRequired		= 0,
	PSAPStatusConfirmNotRequired	= 1,
} PSAPStatus_t;

typedef enum StandbyStatus {
	StandbyStatusNotActive	= 0,
	StandbyStatusActive		= 1,
	StandbyStatusCancel		= 2,
} StandbyStatus_t;


typedef struct CallStatus {
	RescueStatus_t status;
	int callId ;
	uint8_t callSessioNum;
	VoiceStatus_t voiceStatus;
	SourceStatus_t voiceSource;
	MessageStatus_t messageStatus;
	ButtonStatus_t buttonStatus;
	PSAPStatus_t psapConfirmStatus;
	StandbyStatus_t sbStatus;
} CallStatus_t;

typedef enum CALL_DIRECTION {
	DBUS_CALL_DIRECTION_NONE,
	DBUS_CALL_DIRECTION_TO_MMUS,
	DBUS_CALL_DIRECTION_TO_TEMIL
}CALL_DIRECTION_T;

typedef enum SOS_STATUS {
	SOS_STATUS_UNKNOWN,
	SOS_STATUS_LIMITED,
	SOS_STATUS_NOT_AVAILABLE,
	SOS_STATUS_AVAILABLE
}SOS_STATUS_T;


typedef enum AUDIO_CHANNEL_REQUEST {
	AUDIO_MGR_TEM_PHONE = 0x120,
	AUDIO_MGR_TEM_BELL = 0x121,
	AUDIO_MGR_TEM_ALERT = 0x123,
} AUDIO_CHANNEL_REQUEST_T;

} //temxcall

#endif //__TEMXCALL_XCALL_EXPORT_H__

