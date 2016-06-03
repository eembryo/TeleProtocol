/* Do not modify this file. Changes will be overwritten.                      */
/* Generated automatically by the ASN.1 to Wireshark dissector compiler       */
/* packet-ipcmd.c                                                             */
/* ../../tools/asn2wrs.py -d t -p ipcmd -c /home/hyotiger/Works/wireshark2/wireshark-2.0.2/plugins/ipcmd/ipcmd.cnf -s /home/hyotiger/Works/wireshark2/wireshark-2.0.2/plugins/ipcmd/packet-ipcmd-template -D /home/hyotiger/Works/wireshark2/wireshark-2.0.2/plugins/ipcmd -O /home/hyotiger/Works/wireshark2/wireshark-2.0.2/plugins/ipcmd ipcmd.asn */

/* Input file: packet-ipcmd-template.c */

#line 1 "../../plugins/ipcmd/packet-ipcmd-template.c"
#include "config.h"

#include <glib.h>
#include <epan/packet.h>
#include <epan/conversation.h>
#include <epan/asn1.h>

#include <stdio.h>
#include <string.h>

#include <epan/dissectors/packet-per.h>
#include "packet-ipcmd.h"

#define PNAME  "IP Command Protocol"
#define PSNAME "IPCMD"
#define PFNAME "ipcmd"

//static dissector_handle_t ipcmd_handle=NULL;

/* Initialize the protocol and registered fields */
static int proto_ipcmd = -1;


/*--- Included file: packet-ipcmd-hf.c ---*/
#line 1 "../../plugins/ipcmd/packet-ipcmd-hf.c"
static int hf_ipcmd_OpTelematicSettings_SetRequest_PDU = -1;  /* OpTelematicSettings_SetRequest */
static int hf_ipcmd_OpTelematicSettings_Response_PDU = -1;  /* OpTelematicSettings_Response */
static int hf_ipcmd_OpPositionData_Response_PDU = -1;  /* OpPositionData_Response */
static int hf_ipcmd_OpSubscriptionActivation_Response_PDU = -1;  /* OpSubscriptionActivation_Response */
static int hf_ipcmd_OpSubscriptionActivation_Notification_PDU = -1;  /* OpSubscriptionActivation_Notification */
static int hf_ipcmd_OpServiceActivation_SetRequest_PDU = -1;  /* OpServiceActivation_SetRequest */
static int hf_ipcmd_OpServiceActivation_Response_PDU = -1;  /* OpServiceActivation_Response */
static int hf_ipcmd_OpRescueStatus_Response_PDU = -1;  /* OpRescueStatus_Response */
static int hf_ipcmd_OpRescueStatus_Notification_PDU = -1;  /* OpRescueStatus_Notification */
static int hf_ipcmd_OpUserPrivacySettings_SetRequest_PDU = -1;  /* OpUserPrivacySettings_SetRequest */
static int hf_ipcmd_OpUserPrivacySettings_Response_PDU = -1;  /* OpUserPrivacySettings_Response */
static int hf_ipcmd_OpSendToCarConfirmation_SetRequest_PDU = -1;  /* OpSendToCarConfirmation_SetRequest */
static int hf_ipcmd_OpSendToCarConfirmation_Response_PDU = -1;  /* OpSendToCarConfirmation_Response */
static int hf_ipcmd_OpTextMessage_SetRequest_PDU = -1;  /* OpTextMessage_SetRequest */
static int hf_ipcmd_OpIHUSystemInfo_Response_PDU = -1;  /* OpIHUSystemInfo_Response */
static int hf_ipcmd_OpSendToCar_SetRequest_PDU = -1;  /* OpSendToCar_SetRequest */
static int hf_ipcmd_OpGenericSettingSynch_Request_PDU = -1;  /* OpGenericSettingSynch_Request */
static int hf_ipcmd_OpGenericSettingSynch_SetRequest_PDU = -1;  /* OpGenericSettingSynch_SetRequest */
static int hf_ipcmd_OpGenericSettingSynch_Response_PDU = -1;  /* OpGenericSettingSynch_Response */
static int hf_ipcmd_OpGenericSettingSynch_Notification_PDU = -1;  /* OpGenericSettingSynch_Notification */
static int hf_ipcmd_OpSoHPackageUploaded_Notification_PDU = -1;  /* OpSoHPackageUploaded_Notification */
static int hf_ipcmd_OpSIMConnect_SetRequest_PDU = -1;  /* OpSIMConnect_SetRequest */
static int hf_ipcmd_OpSIMConnect_Response_PDU = -1;  /* OpSIMConnect_Response */
static int hf_ipcmd_OpSIMConnectionStatus_Response_PDU = -1;  /* OpSIMConnectionStatus_Response */
static int hf_ipcmd_OpSIMConnectionStatus_Notification_PDU = -1;  /* OpSIMConnectionStatus_Notification */
static int hf_ipcmd_OpConnectivityInhibitionStatus_SetRequest_PDU = -1;  /* OpConnectivityInhibitionStatus_SetRequest */
static int hf_ipcmd_OpConnectivityInhibitionStatus_Response_PDU = -1;  /* OpConnectivityInhibitionStatus_Response */
static int hf_ipcmd_OpConnectivityInhibitionStatus_Notification_PDU = -1;  /* OpConnectivityInhibitionStatus_Notification */
static int hf_ipcmd_OpFactoryDefaultRestore_SetRequest_PDU = -1;  /* OpFactoryDefaultRestore_SetRequest */
static int hf_ipcmd_OpFactoryDefaultRestore_Response_PDU = -1;  /* OpFactoryDefaultRestore_Response */
static int hf_ipcmd_OpInternetGateway_SetRequest_PDU = -1;  /* OpInternetGateway_SetRequest */
static int hf_ipcmd_OpInternetGateway_Response_PDU = -1;  /* OpInternetGateway_Response */
static int hf_ipcmd_OpPremiumAudio_SetRequest_PDU = -1;  /* OpPremiumAudio_SetRequest */
static int hf_ipcmd_OpPremiumAudio_Response_PDU = -1;  /* OpPremiumAudio_Response */
static int hf_ipcmd_OpCallHandling_SetRequest_PDU = -1;  /* OpCallHandling_SetRequest */
static int hf_ipcmd_OpTEM2Identification_Response_PDU = -1;  /* OpTEM2Identification_Response */
static int hf_ipcmd_OpDLCConnectedSignal_Response_PDU = -1;  /* OpDLCConnectedSignal_Response */
static int hf_ipcmd_OpDeadReckonedPosition_Response_PDU = -1;  /* OpDeadReckonedPosition_Response */
static int hf_ipcmd_OpGNSSPositionData_Response_PDU = -1;  /* OpGNSSPositionData_Response */
static int hf_ipcmd_OpDeadReckoningRawData_Response_PDU = -1;  /* OpDeadReckoningRawData_Response */
static int hf_ipcmd_OpCurrentJ2534Session_Notification_PDU = -1;  /* OpCurrentJ2534Session_Notification */
static int hf_ipcmd_OpCurrentDoIPState_Notification_PDU = -1;  /* OpCurrentDoIPState_Notification */
static int hf_ipcmd_OpCurrentDoIPConnection_Notification_PDU = -1;  /* OpCurrentDoIPConnection_Notification */
static int hf_ipcmd_serviceId = -1;               /* INTEGER_0_65535 */
static int hf_ipcmd_operationId = -1;             /* INTEGER_0_65535 */
static int hf_ipcmd_msgLength = -1;               /* INTEGER_0_4294967295 */
static int hf_ipcmd_senderHandle = -1;            /* INTEGER_0_4294967295 */
static int hf_ipcmd_protocolVersion = -1;         /* INTEGER_0_255 */
static int hf_ipcmd_operationType = -1;           /* INTEGER_0_255 */
static int hf_ipcmd_dataType = -1;                /* INTEGER_0_255 */
static int hf_ipcmd_reservedII = -1;              /* INTEGER_0_255 */
static int hf_ipcmd_errorCode = -1;               /* IPCommandErrorCode */
static int hf_ipcmd_errorInfo = -1;               /* INTEGER_0_65535 */
static int hf_ipcmd_year = -1;                    /* INTEGER_2000_2127 */
static int hf_ipcmd_month = -1;                   /* INTEGER_1_12 */
static int hf_ipcmd_day = -1;                     /* INTEGER_1_31 */
static int hf_ipcmd_hour = -1;                    /* INTEGER_0_23 */
static int hf_ipcmd_minute = -1;                  /* INTEGER_0_59 */
static int hf_ipcmd_second = -1;                  /* INTEGER_0_59 */
static int hf_ipcmd_micStatus = -1;               /* GenericOkStatus */
static int hf_ipcmd_speakerStatus = -1;           /* GenericOkStatus */
static int hf_ipcmd_uuid = -1;                    /* PrintableString_SIZE_1_36 */
static int hf_ipcmd_partID = -1;                  /* PrintableString_SIZE_1_40 */
static int hf_ipcmd_telemSetting = -1;            /* TelemSettings */
static int hf_ipcmd_keylockEnabled = -1;          /* BOOLEAN */
static int hf_ipcmd_position = -1;                /* SimpleVehiclePosition */
static int hf_ipcmd_noValidData = -1;             /* NULL */
static int hf_ipcmd_wgs84 = -1;                   /* WGS84SimplePositionData */
static int hf_ipcmd_longLat = -1;                 /* CoordinatesLongLat */
static int hf_ipcmd_fixTime = -1;                 /* DateTime */
static int hf_ipcmd_fixType = -1;                 /* GnssFixType */
static int hf_ipcmd_drType = -1;                  /* DeadReckoningType */
static int hf_ipcmd_status = -1;                  /* ActivationStatus */
static int hf_ipcmd_service = -1;                 /* OnCallService */
static int hf_ipcmd_action = -1;                  /* OnOffSetting */
static int hf_ipcmd_responseOk = -1;              /* NULL */
static int hf_ipcmd_eCallStatus = -1;             /* CallStatus */
static int hf_ipcmd_bCallStatus = -1;             /* CallStatus */
static int hf_ipcmd_iCallStatus = -1;             /* CallStatus */
static int hf_ipcmd_sdnStatus = -1;               /* CallStatus */
static int hf_ipcmd_backupAudioStatus = -1;       /* GenericOkStatus */
static int hf_ipcmd_status_01 = -1;               /* RescueStatus */
static int hf_ipcmd_callId = -1;                  /* XCallID */
static int hf_ipcmd_voiceStatus = -1;             /* VoiceStatus */
static int hf_ipcmd_voiceSource = -1;             /* SourceStatus */
static int hf_ipcmd_messageStatus = -1;           /* MessageStatus */
static int hf_ipcmd_buttonStatus = -1;            /* ButtonStatus */
static int hf_ipcmd_psapConfirmStatus = -1;       /* PSAPStatus */
static int hf_ipcmd_sbStatus = -1;                /* StandbyStatus */
static int hf_ipcmd_userPrivacySetting = -1;      /* UserPrivacySettings */
static int hf_ipcmd_carStatUploadEn = -1;         /* BOOLEAN */
static int hf_ipcmd_locationServicesEn = -1;      /* BOOLEAN */
static int hf_ipcmd_carLocatorStatUploadEn = -1;  /* BOOLEAN */
static int hf_ipcmd_journalLogUploadEn = -1;      /* BOOLEAN */
static int hf_ipcmd_simConnectEn = -1;            /* BOOLEAN */
static int hf_ipcmd_remoteStatusUploadEn = -1;    /* BOOLEAN */
static int hf_ipcmd_confirmation = -1;            /* SendToCarConfirmation */
static int hf_ipcmd_confirmedId = -1;             /* SendToCarId */
static int hf_ipcmd_sourceStatus = -1;            /* SourceStatus */
static int hf_ipcmd_source = -1;                  /* PrintableString_SIZE_1_30 */
static int hf_ipcmd_text = -1;                    /* PrintableString_SIZE_1_140 */
static int hf_ipcmd_softwareVersion = -1;         /* OCTET_STRING_SIZE_0_20 */
static int hf_ipcmd_mapBaseVersion = -1;          /* OCTET_STRING_SIZE_0_20 */
static int hf_ipcmd_mapIncrement = -1;            /* INTEGER_0_255 */
static int hf_ipcmd_typeOfPackage = -1;           /* INTEGER_0_255 */
static int hf_ipcmd_failedFetchBooking = -1;      /* INTEGER_0_65535 */
static int hf_ipcmd_failedServiceIP = -1;         /* INTEGER_0_65535 */
static int hf_ipcmd_requestId = -1;               /* SendToCarId */
static int hf_ipcmd_name = -1;                    /* PrintableString_SIZE_0_30 */
static int hf_ipcmd_description = -1;             /* PrintableString_SIZE_0_100 */
static int hf_ipcmd_gpxFile = -1;                 /* OCTET_STRING_SIZE_0_1048575 */
static int hf_ipcmd_time = -1;                    /* DateTime */
static int hf_ipcmd_settingIDs = -1;              /* T_settingIDs */
static int hf_ipcmd_settingIDs_item = -1;         /* INTEGER_0_65535 */
static int hf_ipcmd_settings = -1;                /* SEQUENCE_SIZE_0_50_OF_Setting */
static int hf_ipcmd_settings_item = -1;           /* Setting */
static int hf_ipcmd_id = -1;                      /* INTEGER_0_65535 */
static int hf_ipcmd_sType = -1;                   /* SettingType */
static int hf_ipcmd_length = -1;                  /* INTEGER_0_65535 */
static int hf_ipcmd_value = -1;                   /* OCTET_STRING_SIZE_0_1023 */
static int hf_ipcmd_packetID = -1;                /* PrintableString_SIZE_0_40 */
static int hf_ipcmd_result = -1;                  /* GenericOkStatus */
static int hf_ipcmd_onOff = -1;                   /* OnOffSetting */
static int hf_ipcmd_status_02 = -1;               /* SIMConnectionStatus */
static int hf_ipcmd_technology = -1;              /* WirelessTechnology */
static int hf_ipcmd_sosStatus = -1;               /* SosStatus */
static int hf_ipcmd_connectivityInhibitionResult = -1;  /* BOOLEAN */
static int hf_ipcmd_setRestoration = -1;          /* BOOLEAN */
static int hf_ipcmd_restorationResult = -1;       /* BOOLEAN */
static int hf_ipcmd_internetGateway = -1;         /* Ecu */
static int hf_ipcmd_isRequested = -1;             /* BOOLEAN */
static int hf_ipcmd_premiumAudioStatus = -1;      /* AudioStatus */
static int hf_ipcmd_action_01 = -1;               /* AssistCallAction */
static int hf_ipcmd_imei = -1;                    /* OCTET_STRING_SIZE_15 */
static int hf_ipcmd_wifiMac = -1;                 /* MacAddress */
static int hf_ipcmd_serialNr = -1;                /* OCTET_STRING_SIZE_15 */
static int hf_ipcmd_dlcConnected = -1;            /* BOOLEAN */
static int hf_ipcmd_position_01 = -1;             /* DRVehiclePosition */
static int hf_ipcmd_drPosition = -1;              /* DRPositionData */
static int hf_ipcmd_heading = -1;                 /* INTEGER_0_360 */
static int hf_ipcmd_speedKmph = -1;               /* INTEGER_0_255 */
static int hf_ipcmd_hdopX10 = -1;                 /* INTEGER_0_255 */
static int hf_ipcmd_numSat = -1;                  /* INTEGER_0_127 */
static int hf_ipcmd_drDistance = -1;              /* INTEGER_0_65535 */
static int hf_ipcmd_gnssPositionData = -1;        /* GNSSData */
static int hf_ipcmd_utcTime = -1;                 /* DateTime */
static int hf_ipcmd_gpsTime = -1;                 /* GPSSystemTime */
static int hf_ipcmd_position_02 = -1;             /* GeographicalPosition */
static int hf_ipcmd_movement = -1;                /* Velocity */
static int hf_ipcmd_heading_01 = -1;              /* INTEGER_0_35999 */
static int hf_ipcmd_gnssStatus = -1;              /* GNSSUsage */
static int hf_ipcmd_positioningStatus = -1;       /* GNSSStatus */
static int hf_ipcmd_satelliteInfo = -1;           /* SatelliteUsage */
static int hf_ipcmd_precision = -1;               /* DOPValues */
static int hf_ipcmd_receiverChannels = -1;        /* ReceiverChannelData */
static int hf_ipcmd_weekNumber = -1;              /* INTEGER_0_1023 */
static int hf_ipcmd_timeOfWeek = -1;              /* INTEGER_0_604799999 */
static int hf_ipcmd_altitude = -1;                /* INTEGER_M1000_60000 */
static int hf_ipcmd_longitude = -1;               /* INTEGER_M2147483648_2147483647 */
static int hf_ipcmd_latitude = -1;                /* INTEGER_M1073741824_1073741824 */
static int hf_ipcmd_speed = -1;                   /* INTEGER_0_100000 */
static int hf_ipcmd_horizontalVelocity = -1;      /* INTEGER_0_100000 */
static int hf_ipcmd_verticalVelocity = -1;        /* INTEGER_M100000_100000 */
static int hf_ipcmd_gpsIsUsed = -1;               /* BOOLEAN */
static int hf_ipcmd_glonassIsUsed = -1;           /* BOOLEAN */
static int hf_ipcmd_galileoIsUsed = -1;           /* BOOLEAN */
static int hf_ipcmd_sbasIsUsed = -1;              /* BOOLEAN */
static int hf_ipcmd_qzssL1IsUsed = -1;            /* BOOLEAN */
static int hf_ipcmd_qzssL1SAIFIsUsed = -1;        /* BOOLEAN */
static int hf_ipcmd_dgpsIsUsed = -1;              /* BOOLEAN */
static int hf_ipcmd_selfEphemerisDataUsage = -1;  /* BOOLEAN */
static int hf_ipcmd_nrOfSatellitesVisible = -1;   /* NrOfSatellitesPerSystem */
static int hf_ipcmd_nrOfSatellitesUsed = -1;      /* NrOfSatellitesPerSystem */
static int hf_ipcmd_gps = -1;                     /* INTEGER_0_31 */
static int hf_ipcmd_glonass = -1;                 /* INTEGER_0_31 */
static int hf_ipcmd_galileo = -1;                 /* INTEGER_0_31 */
static int hf_ipcmd_sbas = -1;                    /* INTEGER_0_31 */
static int hf_ipcmd_qzssL1 = -1;                  /* INTEGER_0_31 */
static int hf_ipcmd_qzssL1SAIF = -1;              /* INTEGER_0_31 */
static int hf_ipcmd_hdop = -1;                    /* INTEGER_0_255 */
static int hf_ipcmd_vdop = -1;                    /* INTEGER_0_255 */
static int hf_ipcmd_pdop = -1;                    /* INTEGER_0_255 */
static int hf_ipcmd_tdop = -1;                    /* INTEGER_0_255 */
static int hf_ipcmd_ReceiverChannelData_item = -1;  /* ChannelData */
static int hf_ipcmd_prn = -1;                     /* INTEGER_1_255 */
static int hf_ipcmd_trackingStatus = -1;          /* SatelliteTrackingStatusType */
static int hf_ipcmd_svacc = -1;                   /* INTEGER_0_15 */
static int hf_ipcmd_snr = -1;                     /* INTEGER_0_255 */
static int hf_ipcmd_azimuthAngle = -1;            /* INTEGER_0_255 */
static int hf_ipcmd_elevationAngle = -1;          /* INTEGER_0_255 */
static int hf_ipcmd_extendedData = -1;            /* ExtendedChannelData */
static int hf_ipcmd_notSupported = -1;            /* NULL */
static int hf_ipcmd_data = -1;                    /* ChannelCorrectionData */
static int hf_ipcmd_pseudoRangeMetres = -1;       /* INTEGER_M1000000000_1000000000 */
static int hf_ipcmd_pseudoRangeMillimetres = -1;  /* INTEGER_0_999 */
static int hf_ipcmd_rangeRate = -1;               /* INTEGER_M1000000_1000000 */
static int hf_ipcmd_pseudoRangeCorrectionData = -1;  /* INTEGER_M1000000_1000000 */
static int hf_ipcmd_selfEphemerisPredictionTime = -1;  /* INTEGER_0_255 */
static int hf_ipcmd_rawDeadReckoningData = -1;    /* DeadReckoningRawData */
static int hf_ipcmd_whlRotToothCntrFrntLe = -1;   /* INTEGER_0_255 */
static int hf_ipcmd_whlRotToothCntrFrntRi = -1;   /* INTEGER_0_255 */
static int hf_ipcmd_whlRotToothCntrReLe = -1;     /* INTEGER_0_255 */
static int hf_ipcmd_whlRotToothCntrReRi = -1;     /* INTEGER_0_255 */
static int hf_ipcmd_whlCircum = -1;               /* INTEGER_0_4095 */
static int hf_ipcmd_aLat1 = -1;                   /* INTEGER_M139_139 */
static int hf_ipcmd_aLat1Qf1 = -1;                /* INTEGER_0_3 */
static int hf_ipcmd_aLgt1 = -1;                   /* INTEGER_M139_139 */
static int hf_ipcmd_aLgt1Qf1 = -1;                /* INTEGER_0_3 */
static int hf_ipcmd_gearIndcn = -1;               /* INTEGER_0_15 */
static int hf_ipcmd_sessionStatus = -1;           /* BOOLEAN */
static int hf_ipcmd_doIPState = -1;               /* BOOLEAN */
static int hf_ipcmd_doIPMode = -1;                /* DoIPMode */
static int hf_ipcmd_doIPPhase = -1;               /* DoIPPhase */
static int hf_ipcmd_currentDoIPConn = -1;         /* CurrentDoIPConn */

/*--- End of included file: packet-ipcmd-hf.c ---*/
#line 24 "../../plugins/ipcmd/packet-ipcmd-template.c"

/* Initialize the subtree pointers */
static int ett_ipcmd = -1;


/*--- Included file: packet-ipcmd-ett.c ---*/
#line 1 "../../plugins/ipcmd/packet-ipcmd-ett.c"
static gint ett_ipcmd_VccPduHeader = -1;
static gint ett_ipcmd_OpGeneric_Error = -1;
static gint ett_ipcmd_DateTime = -1;
static gint ett_ipcmd_AudioStatus = -1;
static gint ett_ipcmd_UUID = -1;
static gint ett_ipcmd_PartIdentifier = -1;
static gint ett_ipcmd_OpTelematicSettings_SetRequest = -1;
static gint ett_ipcmd_OpTelematicSettings_Response = -1;
static gint ett_ipcmd_TelemSettings = -1;
static gint ett_ipcmd_OpPositionData_Response = -1;
static gint ett_ipcmd_SimpleVehiclePosition = -1;
static gint ett_ipcmd_WGS84SimplePositionData = -1;
static gint ett_ipcmd_OpSubscriptionActivation_Response = -1;
static gint ett_ipcmd_OpSubscriptionActivation_Notification = -1;
static gint ett_ipcmd_OpServiceActivation_SetRequest = -1;
static gint ett_ipcmd_OpServiceActivation_Response = -1;
static gint ett_ipcmd_OpRescueStatus_Response = -1;
static gint ett_ipcmd_OpRescueStatus_Notification = -1;
static gint ett_ipcmd_CallStatus = -1;
static gint ett_ipcmd_OpUserPrivacySettings_SetRequest = -1;
static gint ett_ipcmd_OpUserPrivacySettings_Response = -1;
static gint ett_ipcmd_UserPrivacySettings = -1;
static gint ett_ipcmd_OpSendToCarConfirmation_SetRequest = -1;
static gint ett_ipcmd_OpSendToCarConfirmation_Response = -1;
static gint ett_ipcmd_OpTextMessage_SetRequest = -1;
static gint ett_ipcmd_OpIHUSystemInfo_Response = -1;
static gint ett_ipcmd_OpSendToCar_SetRequest = -1;
static gint ett_ipcmd_OpGenericSettingSynch_Request = -1;
static gint ett_ipcmd_T_settingIDs = -1;
static gint ett_ipcmd_OpGenericSettingSynch_SetRequest = -1;
static gint ett_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting = -1;
static gint ett_ipcmd_OpGenericSettingSynch_Response = -1;
static gint ett_ipcmd_OpGenericSettingSynch_Notification = -1;
static gint ett_ipcmd_Setting = -1;
static gint ett_ipcmd_OpSoHPackageUploaded_Notification = -1;
static gint ett_ipcmd_OpSIMConnect_SetRequest = -1;
static gint ett_ipcmd_OpSIMConnect_Response = -1;
static gint ett_ipcmd_OpSIMConnectionStatus_Response = -1;
static gint ett_ipcmd_OpSIMConnectionStatus_Notification = -1;
static gint ett_ipcmd_OpConnectivityInhibitionStatus_SetRequest = -1;
static gint ett_ipcmd_OpConnectivityInhibitionStatus_Response = -1;
static gint ett_ipcmd_OpConnectivityInhibitionStatus_Notification = -1;
static gint ett_ipcmd_OpFactoryDefaultRestore_SetRequest = -1;
static gint ett_ipcmd_OpFactoryDefaultRestore_Response = -1;
static gint ett_ipcmd_OpInternetGateway_SetRequest = -1;
static gint ett_ipcmd_OpInternetGateway_Response = -1;
static gint ett_ipcmd_OpPremiumAudio_SetRequest = -1;
static gint ett_ipcmd_OpPremiumAudio_Response = -1;
static gint ett_ipcmd_OpCallHandling_SetRequest = -1;
static gint ett_ipcmd_OpTEM2Identification_Response = -1;
static gint ett_ipcmd_OpDLCConnectedSignal_Response = -1;
static gint ett_ipcmd_OpDeadReckonedPosition_Response = -1;
static gint ett_ipcmd_DRVehiclePosition = -1;
static gint ett_ipcmd_DRPositionData = -1;
static gint ett_ipcmd_OpGNSSPositionData_Response = -1;
static gint ett_ipcmd_GNSSData = -1;
static gint ett_ipcmd_GPSSystemTime = -1;
static gint ett_ipcmd_GeographicalPosition = -1;
static gint ett_ipcmd_CoordinatesLongLat = -1;
static gint ett_ipcmd_Velocity = -1;
static gint ett_ipcmd_GNSSUsage = -1;
static gint ett_ipcmd_GNSSStatus = -1;
static gint ett_ipcmd_SatelliteUsage = -1;
static gint ett_ipcmd_NrOfSatellitesPerSystem = -1;
static gint ett_ipcmd_DOPValues = -1;
static gint ett_ipcmd_ReceiverChannelData = -1;
static gint ett_ipcmd_ChannelData = -1;
static gint ett_ipcmd_ExtendedChannelData = -1;
static gint ett_ipcmd_ChannelCorrectionData = -1;
static gint ett_ipcmd_OpDeadReckoningRawData_Response = -1;
static gint ett_ipcmd_DeadReckoningRawData = -1;
static gint ett_ipcmd_OpCurrentJ2534Session_Notification = -1;
static gint ett_ipcmd_OpCurrentDoIPState_Notification = -1;
static gint ett_ipcmd_OpCurrentDoIPConnection_Notification = -1;

/*--- End of included file: packet-ipcmd-ett.c ---*/
#line 29 "../../plugins/ipcmd/packet-ipcmd-template.c"


/*--- Included file: packet-ipcmd-fn.c ---*/
#line 1 "../../plugins/ipcmd/packet-ipcmd-fn.c"


static int
dissect_ipcmd_INTEGER_0_65535(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 65535U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_4294967295(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 4294967295U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_255(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 255U, NULL, FALSE);

  return offset;
}


static const per_sequence_t VccPduHeader_sequence[] = {
  { &hf_ipcmd_serviceId     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { &hf_ipcmd_operationId   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { &hf_ipcmd_msgLength     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_4294967295 },
  { &hf_ipcmd_senderHandle  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_4294967295 },
  { &hf_ipcmd_protocolVersion, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_operationType , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_dataType      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_reservedII    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_VccPduHeader(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_VccPduHeader, VccPduHeader_sequence);

  return offset;
}


static const value_string ipcmd_IPCommandErrorCode_vals[] = {
  {   0, "notOk" },
  {   1, "serviceIdNotAvailable" },
  {   2, "operationIdNotAvailable" },
  {   3, "opTypeNotAvailable" },
  {   4, "invalidProtocolVersion" },
  {   5, "segmentationError" },
  {   6, "invalidLength" },
  {   7, "applicationError" },
  {   8, "timeout" },
  {   9, "busy" },
  {  32, "incorrectState" },
  {  35, "asn1decodeError" },
  {  36, "parameterInvalid" },
  { 0, NULL }
};

static guint32 IPCommandErrorCode_value_map[13+0] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 32, 35, 36};

static int
dissect_ipcmd_IPCommandErrorCode(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     13, NULL, FALSE, 0, IPCommandErrorCode_value_map);

  return offset;
}


static const per_sequence_t OpGeneric_Error_sequence[] = {
  { &hf_ipcmd_errorCode     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_IPCommandErrorCode },
  { &hf_ipcmd_errorInfo     , ASN1_NO_EXTENSIONS     , ASN1_OPTIONAL    , dissect_ipcmd_INTEGER_0_65535 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGeneric_Error(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGeneric_Error, OpGeneric_Error_sequence);

  return offset;
}



static int
dissect_ipcmd_OpGeneric_Request(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_null(tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_ipcmd_OpGeneric_SetRequestNoReturn(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_null(tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_ipcmd_OpGeneric_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_null(tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_ipcmd_INTEGER_2000_2127(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            2000U, 2127U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_1_12(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            1U, 12U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_1_31(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            1U, 31U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_23(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 23U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_59(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 59U, NULL, FALSE);

  return offset;
}


static const per_sequence_t DateTime_sequence[] = {
  { &hf_ipcmd_year          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_2000_2127 },
  { &hf_ipcmd_month         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_1_12 },
  { &hf_ipcmd_day           , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_1_31 },
  { &hf_ipcmd_hour          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_23 },
  { &hf_ipcmd_minute        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_59 },
  { &hf_ipcmd_second        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_59 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_DateTime(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_DateTime, DateTime_sequence);

  return offset;
}


static const value_string ipcmd_SourceStatus_vals[] = {
  {   0, "unidentified" },
  {   1, "trusted" },
  {   2, "callCenter" },
  {   3, "psap" },
  {   4, "outgoing" },
  { 0, NULL }
};


static int
dissect_ipcmd_SourceStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_OnOffStatus_vals[] = {
  {   0, "off" },
  {   1, "on" },
  {   2, "unknown" },
  { 0, NULL }
};


static int
dissect_ipcmd_OnOffStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_SecurityStatus_vals[] = {
  {   0, "idle" },
  {   1, "standby" },
  {   2, "active" },
  {   3, "activeStandalone" },
  { 0, NULL }
};


static int
dissect_ipcmd_SecurityStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_GenericOkStatus_vals[] = {
  {   0, "unknown" },
  {   1, "ok" },
  {   2, "notOk" },
  { 0, NULL }
};


static int
dissect_ipcmd_GenericOkStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t AudioStatus_sequence[] = {
  { &hf_ipcmd_micStatus     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GenericOkStatus },
  { &hf_ipcmd_speakerStatus , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GenericOkStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_AudioStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_AudioStatus, AudioStatus_sequence);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_1_36(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 36, FALSE);

  return offset;
}


static const per_sequence_t UUID_sequence[] = {
  { &hf_ipcmd_uuid          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_1_36 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_UUID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_UUID, UUID_sequence);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_1_40(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 40, FALSE);

  return offset;
}


static const per_sequence_t PartIdentifier_sequence[] = {
  { &hf_ipcmd_partID        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_1_40 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_PartIdentifier(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_PartIdentifier, PartIdentifier_sequence);

  return offset;
}


static const value_string ipcmd_XCallID_vals[] = {
  {   0, "sdnCall" },
  {   1, "eCall" },
  {   2, "bCall" },
  {   3, "iCall" },
  { 0, NULL }
};


static int
dissect_ipcmd_XCallID(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_ipcmd_BOOLEAN(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_boolean(tvb, offset, actx, tree, hf_index, NULL);

  return offset;
}


static const per_sequence_t TelemSettings_sequence[] = {
  { &hf_ipcmd_keylockEnabled, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_TelemSettings(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_TelemSettings, TelemSettings_sequence);

  return offset;
}


static const per_sequence_t OpTelematicSettings_SetRequest_sequence[] = {
  { &hf_ipcmd_telemSetting  , ASN1_NO_EXTENSIONS     , ASN1_OPTIONAL    , dissect_ipcmd_TelemSettings },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpTelematicSettings_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpTelematicSettings_SetRequest, OpTelematicSettings_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpTelematicSettings_Response_sequence[] = {
  { &hf_ipcmd_telemSetting  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_TelemSettings },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpTelematicSettings_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpTelematicSettings_Response, OpTelematicSettings_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_NULL(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_null(tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M2147483648_2147483647(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            G_MININT32, 2147483647U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M1073741824_1073741824(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -1073741824, 1073741824U, NULL, FALSE);

  return offset;
}


static const per_sequence_t CoordinatesLongLat_sequence[] = {
  { &hf_ipcmd_longitude     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M2147483648_2147483647 },
  { &hf_ipcmd_latitude      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M1073741824_1073741824 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_CoordinatesLongLat(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_CoordinatesLongLat, CoordinatesLongLat_sequence);

  return offset;
}


static const value_string ipcmd_GnssFixType_vals[] = {
  {   0, "notAvailable" },
  {   1, "noFix" },
  {   2, "fix2D" },
  {   3, "fix3D" },
  {   4, "startupMode" },
  { 0, NULL }
};


static int
dissect_ipcmd_GnssFixType(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_DeadReckoningType_vals[] = {
  {   0, "noDr" },
  {   1, "drNoMapMatch" },
  {   2, "drMapMatched" },
  { 0, NULL }
};


static int
dissect_ipcmd_DeadReckoningType(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t WGS84SimplePositionData_sequence[] = {
  { &hf_ipcmd_longLat       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CoordinatesLongLat },
  { &hf_ipcmd_fixTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_fixType       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GnssFixType },
  { &hf_ipcmd_drType        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DeadReckoningType },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_WGS84SimplePositionData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_WGS84SimplePositionData, WGS84SimplePositionData_sequence);

  return offset;
}


static const value_string ipcmd_SimpleVehiclePosition_vals[] = {
  {   0, "noValidData" },
  {   1, "wgs84" },
  { 0, NULL }
};

static const per_choice_t SimpleVehiclePosition_choice[] = {
  {   0, &hf_ipcmd_noValidData   , ASN1_NO_EXTENSIONS     , dissect_ipcmd_NULL },
  {   1, &hf_ipcmd_wgs84         , ASN1_NO_EXTENSIONS     , dissect_ipcmd_WGS84SimplePositionData },
  { 0, NULL, 0, NULL }
};

static int
dissect_ipcmd_SimpleVehiclePosition(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_ipcmd_SimpleVehiclePosition, SimpleVehiclePosition_choice,
                                 NULL);

  return offset;
}


static const per_sequence_t OpPositionData_Response_sequence[] = {
  { &hf_ipcmd_position      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SimpleVehiclePosition },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpPositionData_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpPositionData_Response, OpPositionData_Response_sequence);

  return offset;
}


static const value_string ipcmd_ActivationStatus_vals[] = {
  {   0, "deactivated" },
  {   1, "activated-provisioned" },
  {   3, "activated-unprovisioned" },
  {   4, "remote-provisioning-ongoing" },
  {   5, "provisioning-ongoing" },
  { 0, NULL }
};

static guint32 ActivationStatus_value_map[5+0] = {0, 1, 3, 4, 5};

static int
dissect_ipcmd_ActivationStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, ActivationStatus_value_map);

  return offset;
}


static const per_sequence_t OpSubscriptionActivation_Response_sequence[] = {
  { &hf_ipcmd_status        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ActivationStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSubscriptionActivation_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSubscriptionActivation_Response, OpSubscriptionActivation_Response_sequence);

  return offset;
}


static const per_sequence_t OpSubscriptionActivation_Notification_sequence[] = {
  { &hf_ipcmd_status        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ActivationStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSubscriptionActivation_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSubscriptionActivation_Notification, OpSubscriptionActivation_Notification_sequence);

  return offset;
}


static const value_string ipcmd_OnCallService_vals[] = {
  {   2, "informationCall" },
  { 0, NULL }
};

static guint32 OnCallService_value_map[1+0] = {2};

static int
dissect_ipcmd_OnCallService(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     1, NULL, FALSE, 0, OnCallService_value_map);

  return offset;
}


static const value_string ipcmd_OnOffSetting_vals[] = {
  {   0, "off" },
  {   1, "on" },
  { 0, NULL }
};


static int
dissect_ipcmd_OnOffSetting(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpServiceActivation_SetRequest_sequence[] = {
  { &hf_ipcmd_service       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OnCallService },
  { &hf_ipcmd_action        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OnOffSetting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpServiceActivation_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpServiceActivation_SetRequest, OpServiceActivation_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpServiceActivation_Response_sequence[] = {
  { &hf_ipcmd_responseOk    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_NULL },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpServiceActivation_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpServiceActivation_Response, OpServiceActivation_Response_sequence);

  return offset;
}


static const value_string ipcmd_RescueStatus_vals[] = {
  {   0, "disabled" },
  {   1, "notActive" },
  {   2, "serviceNotAvailable" },
  {   3, "active" },
  {   4, "terminated" },
  { 0, NULL }
};


static int
dissect_ipcmd_RescueStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_VoiceStatus_vals[] = {
  {   0, "noConnection" },
  {   1, "connectingCSC" },
  {   2, "connectingPSAP" },
  {   3, "connectedCSC" },
  {   4, "connectedPSAP" },
  {   5, "incomingCall" },
  {   6, "connectedCall" },
  {   7, "connectedIncoming" },
  { 0, NULL }
};


static int
dissect_ipcmd_VoiceStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     8, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_MessageStatus_vals[] = {
  {   0, "notSent" },
  {   1, "sending" },
  {   2, "sent" },
  {   3, "ackUndefined" },
  {   4, "ackDefined" },
  { 0, NULL }
};


static int
dissect_ipcmd_MessageStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_ButtonStatus_vals[] = {
  {   0, "disabled" },
  {   1, "pressed" },
  {   2, "released" },
  {   3, "allReleased" },
  { 0, NULL }
};


static int
dissect_ipcmd_ButtonStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_PSAPStatus_vals[] = {
  {   0, "confirmRequired" },
  {   1, "confirmNotRequired" },
  { 0, NULL }
};


static int
dissect_ipcmd_PSAPStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_StandbyStatus_vals[] = {
  {   0, "notActive" },
  {   1, "active" },
  {   2, "cancel" },
  { 0, NULL }
};


static int
dissect_ipcmd_StandbyStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t CallStatus_sequence[] = {
  { &hf_ipcmd_status_01     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_RescueStatus },
  { &hf_ipcmd_callId        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_XCallID },
  { &hf_ipcmd_voiceStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_VoiceStatus },
  { &hf_ipcmd_voiceSource   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SourceStatus },
  { &hf_ipcmd_messageStatus , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_MessageStatus },
  { &hf_ipcmd_buttonStatus  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ButtonStatus },
  { &hf_ipcmd_psapConfirmStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PSAPStatus },
  { &hf_ipcmd_sbStatus      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_StandbyStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_CallStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_CallStatus, CallStatus_sequence);

  return offset;
}


static const per_sequence_t OpRescueStatus_Response_sequence[] = {
  { &hf_ipcmd_eCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_bCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_iCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_sdnStatus     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_backupAudioStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GenericOkStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpRescueStatus_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpRescueStatus_Response, OpRescueStatus_Response_sequence);

  return offset;
}


static const per_sequence_t OpRescueStatus_Notification_sequence[] = {
  { &hf_ipcmd_eCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_bCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_iCallStatus   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_sdnStatus     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CallStatus },
  { &hf_ipcmd_backupAudioStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GenericOkStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpRescueStatus_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpRescueStatus_Notification, OpRescueStatus_Notification_sequence);

  return offset;
}


static const per_sequence_t UserPrivacySettings_sequence[] = {
  { &hf_ipcmd_carStatUploadEn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_locationServicesEn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_carLocatorStatUploadEn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_journalLogUploadEn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_simConnectEn  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_remoteStatusUploadEn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_UserPrivacySettings(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_UserPrivacySettings, UserPrivacySettings_sequence);

  return offset;
}


static const per_sequence_t OpUserPrivacySettings_SetRequest_sequence[] = {
  { &hf_ipcmd_userPrivacySetting, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_UserPrivacySettings },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpUserPrivacySettings_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpUserPrivacySettings_SetRequest, OpUserPrivacySettings_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpUserPrivacySettings_Response_sequence[] = {
  { &hf_ipcmd_userPrivacySetting, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_UserPrivacySettings },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpUserPrivacySettings_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpUserPrivacySettings_Response, OpUserPrivacySettings_Response_sequence);

  return offset;
}


static const value_string ipcmd_SendToCarConfirmation_vals[] = {
  {   0, "addedAsDestination" },
  {   1, "addedAsWaypoint" },
  {   2, "rejected" },
  {   3, "desinationOutsideOfMapData" },
  {   4, "errorWhenAdding" },
  { 0, NULL }
};


static int
dissect_ipcmd_SendToCarConfirmation(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_ipcmd_SendToCarId(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 4294967295U, NULL, FALSE);

  return offset;
}


static const per_sequence_t OpSendToCarConfirmation_SetRequest_sequence[] = {
  { &hf_ipcmd_confirmation  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SendToCarConfirmation },
  { &hf_ipcmd_confirmedId   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SendToCarId },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSendToCarConfirmation_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSendToCarConfirmation_SetRequest, OpSendToCarConfirmation_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpSendToCarConfirmation_Response_sequence[] = {
  { &hf_ipcmd_confirmation  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SendToCarConfirmation },
  { &hf_ipcmd_confirmedId   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SendToCarId },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSendToCarConfirmation_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSendToCarConfirmation_Response, OpSendToCarConfirmation_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_1_30(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 30, FALSE);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_1_140(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          1, 140, FALSE);

  return offset;
}


static const per_sequence_t OpTextMessage_SetRequest_sequence[] = {
  { &hf_ipcmd_sourceStatus  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SourceStatus },
  { &hf_ipcmd_source        , ASN1_NO_EXTENSIONS     , ASN1_OPTIONAL    , dissect_ipcmd_PrintableString_SIZE_1_30 },
  { &hf_ipcmd_text          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_1_140 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpTextMessage_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpTextMessage_SetRequest, OpTextMessage_SetRequest_sequence);

  return offset;
}



static int
dissect_ipcmd_OCTET_STRING_SIZE_0_20(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       0, 20, FALSE, NULL);

  return offset;
}


static const per_sequence_t OpIHUSystemInfo_Response_sequence[] = {
  { &hf_ipcmd_softwareVersion, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_0_20 },
  { &hf_ipcmd_mapBaseVersion, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_0_20 },
  { &hf_ipcmd_mapIncrement  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_typeOfPackage , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_failedFetchBooking, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { &hf_ipcmd_failedServiceIP, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpIHUSystemInfo_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpIHUSystemInfo_Response, OpIHUSystemInfo_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_0_30(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          0, 30, FALSE);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_0_100(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          0, 100, FALSE);

  return offset;
}



static int
dissect_ipcmd_OCTET_STRING_SIZE_0_1048575(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       0, 1048575, FALSE, NULL);

  return offset;
}


static const per_sequence_t OpSendToCar_SetRequest_sequence[] = {
  { &hf_ipcmd_requestId     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SendToCarId },
  { &hf_ipcmd_longLat       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CoordinatesLongLat },
  { &hf_ipcmd_name          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_0_30 },
  { &hf_ipcmd_description   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_0_100 },
  { &hf_ipcmd_gpxFile       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_0_1048575 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSendToCar_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSendToCar_SetRequest, OpSendToCar_SetRequest_sequence);

  return offset;
}


static const per_sequence_t T_settingIDs_sequence_of[1] = {
  { &hf_ipcmd_settingIDs_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
};

static int
dissect_ipcmd_T_settingIDs(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_ipcmd_T_settingIDs, T_settingIDs_sequence_of,
                                                  0, 50, FALSE);

  return offset;
}


static const per_sequence_t OpGenericSettingSynch_Request_sequence[] = {
  { &hf_ipcmd_time          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_settingIDs    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_T_settingIDs },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGenericSettingSynch_Request(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGenericSettingSynch_Request, OpGenericSettingSynch_Request_sequence);

  return offset;
}


static const value_string ipcmd_SettingType_vals[] = {
  {   0, "setting" },
  {   1, "error" },
  { 0, NULL }
};


static int
dissect_ipcmd_SettingType(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_ipcmd_OCTET_STRING_SIZE_0_1023(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       0, 1023, FALSE, NULL);

  return offset;
}


static const per_sequence_t Setting_sequence[] = {
  { &hf_ipcmd_id            , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { &hf_ipcmd_sType         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SettingType },
  { &hf_ipcmd_length        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { &hf_ipcmd_value         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_0_1023 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_Setting(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_Setting, Setting_sequence);

  return offset;
}


static const per_sequence_t SEQUENCE_SIZE_0_50_OF_Setting_sequence_of[1] = {
  { &hf_ipcmd_settings_item , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_Setting },
};

static int
dissect_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting, SEQUENCE_SIZE_0_50_OF_Setting_sequence_of,
                                                  0, 50, FALSE);

  return offset;
}


static const per_sequence_t OpGenericSettingSynch_SetRequest_sequence[] = {
  { &hf_ipcmd_time          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_settings      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGenericSettingSynch_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGenericSettingSynch_SetRequest, OpGenericSettingSynch_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpGenericSettingSynch_Response_sequence[] = {
  { &hf_ipcmd_time          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_settings      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGenericSettingSynch_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGenericSettingSynch_Response, OpGenericSettingSynch_Response_sequence);

  return offset;
}


static const per_sequence_t OpGenericSettingSynch_Notification_sequence[] = {
  { &hf_ipcmd_time          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_settings      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGenericSettingSynch_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGenericSettingSynch_Notification, OpGenericSettingSynch_Notification_sequence);

  return offset;
}



static int
dissect_ipcmd_PrintableString_SIZE_0_40(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_PrintableString(tvb, offset, actx, tree, hf_index,
                                          0, 40, FALSE);

  return offset;
}


static const per_sequence_t OpSoHPackageUploaded_Notification_sequence[] = {
  { &hf_ipcmd_packetID      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_PrintableString_SIZE_0_40 },
  { &hf_ipcmd_result        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GenericOkStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSoHPackageUploaded_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSoHPackageUploaded_Notification, OpSoHPackageUploaded_Notification_sequence);

  return offset;
}


static const per_sequence_t OpSIMConnect_SetRequest_sequence[] = {
  { &hf_ipcmd_onOff         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OnOffSetting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSIMConnect_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSIMConnect_SetRequest, OpSIMConnect_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpSIMConnect_Response_sequence[] = {
  { &hf_ipcmd_onOff         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OnOffSetting },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSIMConnect_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSIMConnect_Response, OpSIMConnect_Response_sequence);

  return offset;
}


static const value_string ipcmd_SIMConnectionStatus_vals[] = {
  {   0, "connectedHome" },
  {   1, "connectedRoaming" },
  {   2, "connecting" },
  {   3, "disconnected" },
  {   4, "prohibited" },
  { 0, NULL }
};


static int
dissect_ipcmd_SIMConnectionStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_WirelessTechnology_vals[] = {
  {   0, "unknown" },
  {   1, "gprs" },
  {   2, "edge" },
  {  10, "umts" },
  {  11, "hsdpa" },
  {  12, "hsupa" },
  {  30, "lte" },
  { 0, NULL }
};

static guint32 WirelessTechnology_value_map[7+0] = {0, 1, 2, 10, 11, 12, 30};

static int
dissect_ipcmd_WirelessTechnology(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     7, NULL, FALSE, 0, WirelessTechnology_value_map);

  return offset;
}


static const value_string ipcmd_SosStatus_vals[] = {
  {   0, "unknown" },
  {   1, "limited" },
  {   2, "nonAvailable" },
  {   3, "available" },
  { 0, NULL }
};


static int
dissect_ipcmd_SosStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpSIMConnectionStatus_Response_sequence[] = {
  { &hf_ipcmd_status_02     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SIMConnectionStatus },
  { &hf_ipcmd_technology    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_WirelessTechnology },
  { &hf_ipcmd_sosStatus     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SosStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSIMConnectionStatus_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSIMConnectionStatus_Response, OpSIMConnectionStatus_Response_sequence);

  return offset;
}


static const per_sequence_t OpSIMConnectionStatus_Notification_sequence[] = {
  { &hf_ipcmd_status_02     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SIMConnectionStatus },
  { &hf_ipcmd_technology    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_WirelessTechnology },
  { &hf_ipcmd_sosStatus     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SosStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpSIMConnectionStatus_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpSIMConnectionStatus_Notification, OpSIMConnectionStatus_Notification_sequence);

  return offset;
}


static const per_sequence_t OpConnectivityInhibitionStatus_SetRequest_sequence[] = {
  { &hf_ipcmd_connectivityInhibitionResult, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpConnectivityInhibitionStatus_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpConnectivityInhibitionStatus_SetRequest, OpConnectivityInhibitionStatus_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpConnectivityInhibitionStatus_Response_sequence[] = {
  { &hf_ipcmd_connectivityInhibitionResult, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpConnectivityInhibitionStatus_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpConnectivityInhibitionStatus_Response, OpConnectivityInhibitionStatus_Response_sequence);

  return offset;
}


static const per_sequence_t OpConnectivityInhibitionStatus_Notification_sequence[] = {
  { &hf_ipcmd_connectivityInhibitionResult, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpConnectivityInhibitionStatus_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpConnectivityInhibitionStatus_Notification, OpConnectivityInhibitionStatus_Notification_sequence);

  return offset;
}


static const per_sequence_t OpFactoryDefaultRestore_SetRequest_sequence[] = {
  { &hf_ipcmd_setRestoration, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpFactoryDefaultRestore_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpFactoryDefaultRestore_SetRequest, OpFactoryDefaultRestore_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpFactoryDefaultRestore_Response_sequence[] = {
  { &hf_ipcmd_restorationResult, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpFactoryDefaultRestore_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpFactoryDefaultRestore_Response, OpFactoryDefaultRestore_Response_sequence);

  return offset;
}


static const value_string ipcmd_Ecu_vals[] = {
  {   0, "none" },
  {   1, "ihu" },
  {   2, "tem2" },
  { 0, NULL }
};


static int
dissect_ipcmd_Ecu(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpInternetGateway_SetRequest_sequence[] = {
  { &hf_ipcmd_internetGateway, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_Ecu },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpInternetGateway_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpInternetGateway_SetRequest, OpInternetGateway_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpInternetGateway_Response_sequence[] = {
  { &hf_ipcmd_internetGateway, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_Ecu },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpInternetGateway_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpInternetGateway_Response, OpInternetGateway_Response_sequence);

  return offset;
}


static const per_sequence_t OpPremiumAudio_SetRequest_sequence[] = {
  { &hf_ipcmd_isRequested   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpPremiumAudio_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpPremiumAudio_SetRequest, OpPremiumAudio_SetRequest_sequence);

  return offset;
}


static const per_sequence_t OpPremiumAudio_Response_sequence[] = {
  { &hf_ipcmd_premiumAudioStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_AudioStatus },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpPremiumAudio_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpPremiumAudio_Response, OpPremiumAudio_Response_sequence);

  return offset;
}


static const value_string ipcmd_AssistCallAction_vals[] = {
  {   0, "acceptCall" },
  {   1, "hangupCall" },
  { 0, NULL }
};


static int
dissect_ipcmd_AssistCallAction(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     2, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpCallHandling_SetRequest_sequence[] = {
  { &hf_ipcmd_action_01     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_AssistCallAction },
  { &hf_ipcmd_callId        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_XCallID },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpCallHandling_SetRequest(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpCallHandling_SetRequest, OpCallHandling_SetRequest_sequence);

  return offset;
}



static int
dissect_ipcmd_OCTET_STRING_SIZE_15(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       15, 15, FALSE, NULL);

  return offset;
}



static int
dissect_ipcmd_MacAddress(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_octet_string(tvb, offset, actx, tree, hf_index,
                                       11, 23, FALSE, NULL);

  return offset;
}


static const per_sequence_t OpTEM2Identification_Response_sequence[] = {
  { &hf_ipcmd_imei          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_15 },
  { &hf_ipcmd_wifiMac       , ASN1_NO_EXTENSIONS     , ASN1_OPTIONAL    , dissect_ipcmd_MacAddress },
  { &hf_ipcmd_serialNr      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_OCTET_STRING_SIZE_15 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpTEM2Identification_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpTEM2Identification_Response, OpTEM2Identification_Response_sequence);

  return offset;
}


static const per_sequence_t OpDLCConnectedSignal_Response_sequence[] = {
  { &hf_ipcmd_dlcConnected  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpDLCConnectedSignal_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpDLCConnectedSignal_Response, OpDLCConnectedSignal_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_360(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 360U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_127(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 127U, NULL, FALSE);

  return offset;
}


static const per_sequence_t DRPositionData_sequence[] = {
  { &hf_ipcmd_longLat       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CoordinatesLongLat },
  { &hf_ipcmd_heading       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_360 },
  { &hf_ipcmd_speedKmph     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_hdopX10       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_numSat        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_127 },
  { &hf_ipcmd_fixTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_fixType       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GnssFixType },
  { &hf_ipcmd_drType        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DeadReckoningType },
  { &hf_ipcmd_drDistance    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_65535 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_DRPositionData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_DRPositionData, DRPositionData_sequence);

  return offset;
}


static const value_string ipcmd_DRVehiclePosition_vals[] = {
  {   0, "noValidData" },
  {   1, "drPosition" },
  { 0, NULL }
};

static const per_choice_t DRVehiclePosition_choice[] = {
  {   0, &hf_ipcmd_noValidData   , ASN1_NO_EXTENSIONS     , dissect_ipcmd_NULL },
  {   1, &hf_ipcmd_drPosition    , ASN1_NO_EXTENSIONS     , dissect_ipcmd_DRPositionData },
  { 0, NULL, 0, NULL }
};

static int
dissect_ipcmd_DRVehiclePosition(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_ipcmd_DRVehiclePosition, DRVehiclePosition_choice,
                                 NULL);

  return offset;
}


static const per_sequence_t OpDeadReckonedPosition_Response_sequence[] = {
  { &hf_ipcmd_position_01   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DRVehiclePosition },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpDeadReckonedPosition_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpDeadReckonedPosition_Response, OpDeadReckonedPosition_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_1023(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 1023U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_604799999(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 604799999U, NULL, FALSE);

  return offset;
}


static const per_sequence_t GPSSystemTime_sequence[] = {
  { &hf_ipcmd_weekNumber    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_1023 },
  { &hf_ipcmd_timeOfWeek    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_604799999 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_GPSSystemTime(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_GPSSystemTime, GPSSystemTime_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M1000_60000(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -1000, 60000U, NULL, FALSE);

  return offset;
}


static const per_sequence_t GeographicalPosition_sequence[] = {
  { &hf_ipcmd_longLat       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CoordinatesLongLat },
  { &hf_ipcmd_altitude      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M1000_60000 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_GeographicalPosition(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_GeographicalPosition, GeographicalPosition_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_100000(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 100000U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M100000_100000(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -100000, 100000U, NULL, FALSE);

  return offset;
}


static const per_sequence_t Velocity_sequence[] = {
  { &hf_ipcmd_speed         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_100000 },
  { &hf_ipcmd_horizontalVelocity, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_100000 },
  { &hf_ipcmd_verticalVelocity, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M100000_100000 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_Velocity(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_Velocity, Velocity_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_35999(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 35999U, NULL, FALSE);

  return offset;
}


static const per_sequence_t GNSSUsage_sequence[] = {
  { &hf_ipcmd_gpsIsUsed     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_glonassIsUsed , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_galileoIsUsed , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_sbasIsUsed    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_qzssL1IsUsed  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_qzssL1SAIFIsUsed, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_GNSSUsage(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_GNSSUsage, GNSSUsage_sequence);

  return offset;
}


static const per_sequence_t GNSSStatus_sequence[] = {
  { &hf_ipcmd_fixType       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GnssFixType },
  { &hf_ipcmd_dgpsIsUsed    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_selfEphemerisDataUsage, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_GNSSStatus(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_GNSSStatus, GNSSStatus_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_31(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 31U, NULL, FALSE);

  return offset;
}


static const per_sequence_t NrOfSatellitesPerSystem_sequence[] = {
  { &hf_ipcmd_gps           , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { &hf_ipcmd_glonass       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { &hf_ipcmd_galileo       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { &hf_ipcmd_sbas          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { &hf_ipcmd_qzssL1        , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { &hf_ipcmd_qzssL1SAIF    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_31 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_NrOfSatellitesPerSystem(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_NrOfSatellitesPerSystem, NrOfSatellitesPerSystem_sequence);

  return offset;
}


static const per_sequence_t SatelliteUsage_sequence[] = {
  { &hf_ipcmd_nrOfSatellitesVisible, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_NrOfSatellitesPerSystem },
  { &hf_ipcmd_nrOfSatellitesUsed, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_NrOfSatellitesPerSystem },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_SatelliteUsage(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_SatelliteUsage, SatelliteUsage_sequence);

  return offset;
}


static const per_sequence_t DOPValues_sequence[] = {
  { &hf_ipcmd_hdop          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_vdop          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_pdop          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_tdop          , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_DOPValues(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_DOPValues, DOPValues_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_1_255(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            1U, 255U, NULL, FALSE);

  return offset;
}


static const value_string ipcmd_SatelliteTrackingStatusType_vals[] = {
  {   0, "searching" },
  {   1, "tracking" },
  {   2, "collectedAndNotUsed" },
  {   3, "collectedAndUsed" },
  { 0, NULL }
};


static int
dissect_ipcmd_SatelliteTrackingStatusType(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_15(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 15U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M1000000000_1000000000(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -1000000000, 1000000000U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_999(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 999U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M1000000_1000000(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -1000000, 1000000U, NULL, FALSE);

  return offset;
}


static const per_sequence_t ChannelCorrectionData_sequence[] = {
  { &hf_ipcmd_pseudoRangeMetres, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M1000000000_1000000000 },
  { &hf_ipcmd_pseudoRangeMillimetres, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_999 },
  { &hf_ipcmd_rangeRate     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M1000000_1000000 },
  { &hf_ipcmd_pseudoRangeCorrectionData, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M1000000_1000000 },
  { &hf_ipcmd_selfEphemerisPredictionTime, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_ChannelCorrectionData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_ChannelCorrectionData, ChannelCorrectionData_sequence);

  return offset;
}


static const value_string ipcmd_ExtendedChannelData_vals[] = {
  {   0, "notSupported" },
  {   1, "data" },
  { 0, NULL }
};

static const per_choice_t ExtendedChannelData_choice[] = {
  {   0, &hf_ipcmd_notSupported  , ASN1_NO_EXTENSIONS     , dissect_ipcmd_NULL },
  {   1, &hf_ipcmd_data          , ASN1_NO_EXTENSIONS     , dissect_ipcmd_ChannelCorrectionData },
  { 0, NULL, 0, NULL }
};

static int
dissect_ipcmd_ExtendedChannelData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_choice(tvb, offset, actx, tree, hf_index,
                                 ett_ipcmd_ExtendedChannelData, ExtendedChannelData_choice,
                                 NULL);

  return offset;
}


static const per_sequence_t ChannelData_sequence[] = {
  { &hf_ipcmd_prn           , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_1_255 },
  { &hf_ipcmd_trackingStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SatelliteTrackingStatusType },
  { &hf_ipcmd_svacc         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_15 },
  { &hf_ipcmd_snr           , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_azimuthAngle  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_elevationAngle, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_extendedData  , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ExtendedChannelData },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_ChannelData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_ChannelData, ChannelData_sequence);

  return offset;
}


static const per_sequence_t ReceiverChannelData_sequence_of[1] = {
  { &hf_ipcmd_ReceiverChannelData_item, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ChannelData },
};

static int
dissect_ipcmd_ReceiverChannelData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_sequence_of(tvb, offset, actx, tree, hf_index,
                                                  ett_ipcmd_ReceiverChannelData, ReceiverChannelData_sequence_of,
                                                  0, 127, FALSE);

  return offset;
}


static const per_sequence_t GNSSData_sequence[] = {
  { &hf_ipcmd_utcTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_gpsTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GPSSystemTime },
  { &hf_ipcmd_position_02   , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GeographicalPosition },
  { &hf_ipcmd_movement      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_Velocity },
  { &hf_ipcmd_heading_01    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_35999 },
  { &hf_ipcmd_gnssStatus    , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GNSSUsage },
  { &hf_ipcmd_positioningStatus, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GNSSStatus },
  { &hf_ipcmd_satelliteInfo , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_SatelliteUsage },
  { &hf_ipcmd_precision     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DOPValues },
  { &hf_ipcmd_receiverChannels, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_ReceiverChannelData },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_GNSSData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_GNSSData, GNSSData_sequence);

  return offset;
}


static const per_sequence_t OpGNSSPositionData_Response_sequence[] = {
  { &hf_ipcmd_gnssPositionData, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GNSSData },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpGNSSPositionData_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpGNSSPositionData_Response, OpGNSSPositionData_Response_sequence);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_4095(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 4095U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_M139_139(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            -139, 139U, NULL, FALSE);

  return offset;
}



static int
dissect_ipcmd_INTEGER_0_3(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_constrained_integer(tvb, offset, actx, tree, hf_index,
                                                            0U, 3U, NULL, FALSE);

  return offset;
}


static const per_sequence_t DeadReckoningRawData_sequence[] = {
  { &hf_ipcmd_utcTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DateTime },
  { &hf_ipcmd_gpsTime       , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_GPSSystemTime },
  { &hf_ipcmd_whlRotToothCntrFrntLe, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_whlRotToothCntrFrntRi, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_whlRotToothCntrReLe, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_whlRotToothCntrReRi, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_255 },
  { &hf_ipcmd_whlCircum     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_4095 },
  { &hf_ipcmd_aLat1         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M139_139 },
  { &hf_ipcmd_aLat1Qf1      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_3 },
  { &hf_ipcmd_aLgt1         , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_M139_139 },
  { &hf_ipcmd_aLgt1Qf1      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_3 },
  { &hf_ipcmd_gearIndcn     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_INTEGER_0_15 },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_DeadReckoningRawData(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_DeadReckoningRawData, DeadReckoningRawData_sequence);

  return offset;
}


static const per_sequence_t OpDeadReckoningRawData_Response_sequence[] = {
  { &hf_ipcmd_rawDeadReckoningData, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DeadReckoningRawData },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpDeadReckoningRawData_Response(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpDeadReckoningRawData_Response, OpDeadReckoningRawData_Response_sequence);

  return offset;
}


static const per_sequence_t OpCurrentJ2534Session_Notification_sequence[] = {
  { &hf_ipcmd_sessionStatus , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpCurrentJ2534Session_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpCurrentJ2534Session_Notification, OpCurrentJ2534Session_Notification_sequence);

  return offset;
}


static const value_string ipcmd_DoIPMode_vals[] = {
  {   0, "none" },
  {   1, "local" },
  {   2, "remote" },
  { 0, NULL }
};


static int
dissect_ipcmd_DoIPMode(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     3, NULL, FALSE, 0, NULL);

  return offset;
}


static const value_string ipcmd_DoIPPhase_vals[] = {
  {   0, "none" },
  {   1, "announcement" },
  {   2, "activation" },
  {   3, "session" },
  { 0, NULL }
};


static int
dissect_ipcmd_DoIPPhase(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     4, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpCurrentDoIPState_Notification_sequence[] = {
  { &hf_ipcmd_doIPState     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_BOOLEAN },
  { &hf_ipcmd_doIPMode      , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DoIPMode },
  { &hf_ipcmd_doIPPhase     , ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_DoIPPhase },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpCurrentDoIPState_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpCurrentDoIPState_Notification, OpCurrentDoIPState_Notification_sequence);

  return offset;
}


static const value_string ipcmd_CurrentDoIPConn_vals[] = {
  {   0, "none" },
  {   1, "ethernetp2p" },
  {   2, "ethernetlan" },
  {   3, "wlan" },
  {   4, "phone" },
  { 0, NULL }
};


static int
dissect_ipcmd_CurrentDoIPConn(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_enumerated(tvb, offset, actx, tree, hf_index,
                                     5, NULL, FALSE, 0, NULL);

  return offset;
}


static const per_sequence_t OpCurrentDoIPConnection_Notification_sequence[] = {
  { &hf_ipcmd_currentDoIPConn, ASN1_NO_EXTENSIONS     , ASN1_NOT_OPTIONAL, dissect_ipcmd_CurrentDoIPConn },
  { NULL, 0, 0, NULL }
};

static int
dissect_ipcmd_OpCurrentDoIPConnection_Notification(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_ipcmd_OpCurrentDoIPConnection_Notification, OpCurrentDoIPConnection_Notification_sequence);

  return offset;
}

/*--- PDUs ---*/

static void dissect_OpTelematicSettings_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpTelematicSettings_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpTelematicSettings_SetRequest_PDU);
}
static void dissect_OpTelematicSettings_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpTelematicSettings_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpTelematicSettings_Response_PDU);
}
static void dissect_OpPositionData_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpPositionData_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpPositionData_Response_PDU);
}
static void dissect_OpSubscriptionActivation_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSubscriptionActivation_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSubscriptionActivation_Response_PDU);
}
static void dissect_OpSubscriptionActivation_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSubscriptionActivation_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSubscriptionActivation_Notification_PDU);
}
static void dissect_OpServiceActivation_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpServiceActivation_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpServiceActivation_SetRequest_PDU);
}
static void dissect_OpServiceActivation_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpServiceActivation_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpServiceActivation_Response_PDU);
}
static void dissect_OpRescueStatus_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpRescueStatus_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpRescueStatus_Response_PDU);
}
static void dissect_OpRescueStatus_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpRescueStatus_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpRescueStatus_Notification_PDU);
}
static void dissect_OpUserPrivacySettings_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpUserPrivacySettings_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpUserPrivacySettings_SetRequest_PDU);
}
static void dissect_OpUserPrivacySettings_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpUserPrivacySettings_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpUserPrivacySettings_Response_PDU);
}
static void dissect_OpSendToCarConfirmation_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSendToCarConfirmation_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSendToCarConfirmation_SetRequest_PDU);
}
static void dissect_OpSendToCarConfirmation_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSendToCarConfirmation_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSendToCarConfirmation_Response_PDU);
}
static void dissect_OpTextMessage_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpTextMessage_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpTextMessage_SetRequest_PDU);
}
static void dissect_OpIHUSystemInfo_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpIHUSystemInfo_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpIHUSystemInfo_Response_PDU);
}
static void dissect_OpSendToCar_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSendToCar_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSendToCar_SetRequest_PDU);
}
static void dissect_OpGenericSettingSynch_Request_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpGenericSettingSynch_Request(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpGenericSettingSynch_Request_PDU);
}
static void dissect_OpGenericSettingSynch_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpGenericSettingSynch_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpGenericSettingSynch_SetRequest_PDU);
}
static void dissect_OpGenericSettingSynch_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpGenericSettingSynch_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpGenericSettingSynch_Response_PDU);
}
static void dissect_OpGenericSettingSynch_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpGenericSettingSynch_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpGenericSettingSynch_Notification_PDU);
}
static void dissect_OpSoHPackageUploaded_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSoHPackageUploaded_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSoHPackageUploaded_Notification_PDU);
}
static void dissect_OpSIMConnect_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSIMConnect_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSIMConnect_SetRequest_PDU);
}
static void dissect_OpSIMConnect_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSIMConnect_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSIMConnect_Response_PDU);
}
static void dissect_OpSIMConnectionStatus_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSIMConnectionStatus_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSIMConnectionStatus_Response_PDU);
}
static void dissect_OpSIMConnectionStatus_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpSIMConnectionStatus_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpSIMConnectionStatus_Notification_PDU);
}
static void dissect_OpConnectivityInhibitionStatus_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpConnectivityInhibitionStatus_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpConnectivityInhibitionStatus_SetRequest_PDU);
}
static void dissect_OpConnectivityInhibitionStatus_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpConnectivityInhibitionStatus_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpConnectivityInhibitionStatus_Response_PDU);
}
static void dissect_OpConnectivityInhibitionStatus_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpConnectivityInhibitionStatus_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpConnectivityInhibitionStatus_Notification_PDU);
}
static void dissect_OpFactoryDefaultRestore_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpFactoryDefaultRestore_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpFactoryDefaultRestore_SetRequest_PDU);
}
static void dissect_OpFactoryDefaultRestore_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpFactoryDefaultRestore_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpFactoryDefaultRestore_Response_PDU);
}
static void dissect_OpInternetGateway_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpInternetGateway_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpInternetGateway_SetRequest_PDU);
}
static void dissect_OpInternetGateway_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpInternetGateway_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpInternetGateway_Response_PDU);
}
static void dissect_OpPremiumAudio_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpPremiumAudio_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpPremiumAudio_SetRequest_PDU);
}
static void dissect_OpPremiumAudio_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpPremiumAudio_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpPremiumAudio_Response_PDU);
}
static void dissect_OpCallHandling_SetRequest_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpCallHandling_SetRequest(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpCallHandling_SetRequest_PDU);
}
static void dissect_OpTEM2Identification_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpTEM2Identification_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpTEM2Identification_Response_PDU);
}
static void dissect_OpDLCConnectedSignal_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpDLCConnectedSignal_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpDLCConnectedSignal_Response_PDU);
}
static void dissect_OpDeadReckonedPosition_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpDeadReckonedPosition_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpDeadReckonedPosition_Response_PDU);
}
static void dissect_OpGNSSPositionData_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpGNSSPositionData_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpGNSSPositionData_Response_PDU);
}
static void dissect_OpDeadReckoningRawData_Response_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpDeadReckoningRawData_Response(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpDeadReckoningRawData_Response_PDU);
}
static void dissect_OpCurrentJ2534Session_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpCurrentJ2534Session_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpCurrentJ2534Session_Notification_PDU);
}
static void dissect_OpCurrentDoIPState_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpCurrentDoIPState_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpCurrentDoIPState_Notification_PDU);
}
static void dissect_OpCurrentDoIPConnection_Notification_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_) {
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_PER, TRUE, pinfo);
  dissect_ipcmd_OpCurrentDoIPConnection_Notification(tvb, 0, &asn1_ctx, tree, hf_ipcmd_OpCurrentDoIPConnection_Notification_PDU);
}


/*--- End of included file: packet-ipcmd-fn.c ---*/
#line 31 "../../plugins/ipcmd/packet-ipcmd-template.c"

/*--- proto_register_ipcmd -------------------------------------------*/
void proto_register_ipcmd(void) {
    /* List of fields */
    static hf_register_info hf[] = {


/*--- Included file: packet-ipcmd-hfarr.c ---*/
#line 1 "../../plugins/ipcmd/packet-ipcmd-hfarr.c"
    { &hf_ipcmd_OpTelematicSettings_SetRequest_PDU,
      { "OpTelematicSettings-SetRequest", "ipcmd.OpTelematicSettings_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpTelematicSettings_Response_PDU,
      { "OpTelematicSettings-Response", "ipcmd.OpTelematicSettings_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpPositionData_Response_PDU,
      { "OpPositionData-Response", "ipcmd.OpPositionData_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSubscriptionActivation_Response_PDU,
      { "OpSubscriptionActivation-Response", "ipcmd.OpSubscriptionActivation_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSubscriptionActivation_Notification_PDU,
      { "OpSubscriptionActivation-Notification", "ipcmd.OpSubscriptionActivation_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpServiceActivation_SetRequest_PDU,
      { "OpServiceActivation-SetRequest", "ipcmd.OpServiceActivation_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpServiceActivation_Response_PDU,
      { "OpServiceActivation-Response", "ipcmd.OpServiceActivation_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpRescueStatus_Response_PDU,
      { "OpRescueStatus-Response", "ipcmd.OpRescueStatus_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpRescueStatus_Notification_PDU,
      { "OpRescueStatus-Notification", "ipcmd.OpRescueStatus_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpUserPrivacySettings_SetRequest_PDU,
      { "OpUserPrivacySettings-SetRequest", "ipcmd.OpUserPrivacySettings_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpUserPrivacySettings_Response_PDU,
      { "OpUserPrivacySettings-Response", "ipcmd.OpUserPrivacySettings_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSendToCarConfirmation_SetRequest_PDU,
      { "OpSendToCarConfirmation-SetRequest", "ipcmd.OpSendToCarConfirmation_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSendToCarConfirmation_Response_PDU,
      { "OpSendToCarConfirmation-Response", "ipcmd.OpSendToCarConfirmation_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpTextMessage_SetRequest_PDU,
      { "OpTextMessage-SetRequest", "ipcmd.OpTextMessage_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpIHUSystemInfo_Response_PDU,
      { "OpIHUSystemInfo-Response", "ipcmd.OpIHUSystemInfo_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSendToCar_SetRequest_PDU,
      { "OpSendToCar-SetRequest", "ipcmd.OpSendToCar_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpGenericSettingSynch_Request_PDU,
      { "OpGenericSettingSynch-Request", "ipcmd.OpGenericSettingSynch_Request_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpGenericSettingSynch_SetRequest_PDU,
      { "OpGenericSettingSynch-SetRequest", "ipcmd.OpGenericSettingSynch_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpGenericSettingSynch_Response_PDU,
      { "OpGenericSettingSynch-Response", "ipcmd.OpGenericSettingSynch_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpGenericSettingSynch_Notification_PDU,
      { "OpGenericSettingSynch-Notification", "ipcmd.OpGenericSettingSynch_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSoHPackageUploaded_Notification_PDU,
      { "OpSoHPackageUploaded-Notification", "ipcmd.OpSoHPackageUploaded_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSIMConnect_SetRequest_PDU,
      { "OpSIMConnect-SetRequest", "ipcmd.OpSIMConnect_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSIMConnect_Response_PDU,
      { "OpSIMConnect-Response", "ipcmd.OpSIMConnect_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSIMConnectionStatus_Response_PDU,
      { "OpSIMConnectionStatus-Response", "ipcmd.OpSIMConnectionStatus_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpSIMConnectionStatus_Notification_PDU,
      { "OpSIMConnectionStatus-Notification", "ipcmd.OpSIMConnectionStatus_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpConnectivityInhibitionStatus_SetRequest_PDU,
      { "OpConnectivityInhibitionStatus-SetRequest", "ipcmd.OpConnectivityInhibitionStatus_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpConnectivityInhibitionStatus_Response_PDU,
      { "OpConnectivityInhibitionStatus-Response", "ipcmd.OpConnectivityInhibitionStatus_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpConnectivityInhibitionStatus_Notification_PDU,
      { "OpConnectivityInhibitionStatus-Notification", "ipcmd.OpConnectivityInhibitionStatus_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpFactoryDefaultRestore_SetRequest_PDU,
      { "OpFactoryDefaultRestore-SetRequest", "ipcmd.OpFactoryDefaultRestore_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpFactoryDefaultRestore_Response_PDU,
      { "OpFactoryDefaultRestore-Response", "ipcmd.OpFactoryDefaultRestore_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpInternetGateway_SetRequest_PDU,
      { "OpInternetGateway-SetRequest", "ipcmd.OpInternetGateway_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpInternetGateway_Response_PDU,
      { "OpInternetGateway-Response", "ipcmd.OpInternetGateway_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpPremiumAudio_SetRequest_PDU,
      { "OpPremiumAudio-SetRequest", "ipcmd.OpPremiumAudio_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpPremiumAudio_Response_PDU,
      { "OpPremiumAudio-Response", "ipcmd.OpPremiumAudio_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpCallHandling_SetRequest_PDU,
      { "OpCallHandling-SetRequest", "ipcmd.OpCallHandling_SetRequest_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpTEM2Identification_Response_PDU,
      { "OpTEM2Identification-Response", "ipcmd.OpTEM2Identification_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpDLCConnectedSignal_Response_PDU,
      { "OpDLCConnectedSignal-Response", "ipcmd.OpDLCConnectedSignal_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpDeadReckonedPosition_Response_PDU,
      { "OpDeadReckonedPosition-Response", "ipcmd.OpDeadReckonedPosition_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpGNSSPositionData_Response_PDU,
      { "OpGNSSPositionData-Response", "ipcmd.OpGNSSPositionData_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpDeadReckoningRawData_Response_PDU,
      { "OpDeadReckoningRawData-Response", "ipcmd.OpDeadReckoningRawData_Response_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpCurrentJ2534Session_Notification_PDU,
      { "OpCurrentJ2534Session-Notification", "ipcmd.OpCurrentJ2534Session_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpCurrentDoIPState_Notification_PDU,
      { "OpCurrentDoIPState-Notification", "ipcmd.OpCurrentDoIPState_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_OpCurrentDoIPConnection_Notification_PDU,
      { "OpCurrentDoIPConnection-Notification", "ipcmd.OpCurrentDoIPConnection_Notification_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_serviceId,
      { "serviceId", "ipcmd.serviceId",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_operationId,
      { "operationId", "ipcmd.operationId",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_msgLength,
      { "msgLength", "ipcmd.msgLength",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4294967295", HFILL }},
    { &hf_ipcmd_senderHandle,
      { "senderHandle", "ipcmd.senderHandle",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4294967295", HFILL }},
    { &hf_ipcmd_protocolVersion,
      { "protocolVersion", "ipcmd.protocolVersion",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_operationType,
      { "operationType", "ipcmd.operationType",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_dataType,
      { "dataType", "ipcmd.dataType",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_reservedII,
      { "reservedII", "ipcmd.reservedII",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_errorCode,
      { "errorCode", "ipcmd.errorCode",
        FT_UINT32, BASE_DEC, VALS(ipcmd_IPCommandErrorCode_vals), 0,
        "IPCommandErrorCode", HFILL }},
    { &hf_ipcmd_errorInfo,
      { "errorInfo", "ipcmd.errorInfo",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_year,
      { "year", "ipcmd.year",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_2000_2127", HFILL }},
    { &hf_ipcmd_month,
      { "month", "ipcmd.month",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_12", HFILL }},
    { &hf_ipcmd_day,
      { "day", "ipcmd.day",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_31", HFILL }},
    { &hf_ipcmd_hour,
      { "hour", "ipcmd.hour",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_23", HFILL }},
    { &hf_ipcmd_minute,
      { "minute", "ipcmd.minute",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_59", HFILL }},
    { &hf_ipcmd_second,
      { "second", "ipcmd.second",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_59", HFILL }},
    { &hf_ipcmd_micStatus,
      { "micStatus", "ipcmd.micStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_GenericOkStatus_vals), 0,
        "GenericOkStatus", HFILL }},
    { &hf_ipcmd_speakerStatus,
      { "speakerStatus", "ipcmd.speakerStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_GenericOkStatus_vals), 0,
        "GenericOkStatus", HFILL }},
    { &hf_ipcmd_uuid,
      { "uuid", "ipcmd.uuid",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_1_36", HFILL }},
    { &hf_ipcmd_partID,
      { "partID", "ipcmd.partID",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_1_40", HFILL }},
    { &hf_ipcmd_telemSetting,
      { "telemSetting", "ipcmd.telemSetting_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "TelemSettings", HFILL }},
    { &hf_ipcmd_keylockEnabled,
      { "keylockEnabled", "ipcmd.keylockEnabled",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_position,
      { "position", "ipcmd.position",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SimpleVehiclePosition_vals), 0,
        "SimpleVehiclePosition", HFILL }},
    { &hf_ipcmd_noValidData,
      { "noValidData", "ipcmd.noValidData_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_wgs84,
      { "wgs84", "ipcmd.wgs84_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "WGS84SimplePositionData", HFILL }},
    { &hf_ipcmd_longLat,
      { "longLat", "ipcmd.longLat_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CoordinatesLongLat", HFILL }},
    { &hf_ipcmd_fixTime,
      { "fixTime", "ipcmd.fixTime_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DateTime", HFILL }},
    { &hf_ipcmd_fixType,
      { "fixType", "ipcmd.fixType",
        FT_UINT32, BASE_DEC, VALS(ipcmd_GnssFixType_vals), 0,
        "GnssFixType", HFILL }},
    { &hf_ipcmd_drType,
      { "drType", "ipcmd.drType",
        FT_UINT32, BASE_DEC, VALS(ipcmd_DeadReckoningType_vals), 0,
        "DeadReckoningType", HFILL }},
    { &hf_ipcmd_status,
      { "status", "ipcmd.status",
        FT_UINT32, BASE_DEC, VALS(ipcmd_ActivationStatus_vals), 0,
        "ActivationStatus", HFILL }},
    { &hf_ipcmd_service,
      { "service", "ipcmd.service",
        FT_UINT32, BASE_DEC, VALS(ipcmd_OnCallService_vals), 0,
        "OnCallService", HFILL }},
    { &hf_ipcmd_action,
      { "action", "ipcmd.action",
        FT_UINT32, BASE_DEC, VALS(ipcmd_OnOffSetting_vals), 0,
        "OnOffSetting", HFILL }},
    { &hf_ipcmd_responseOk,
      { "responseOk", "ipcmd.responseOk_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_eCallStatus,
      { "eCallStatus", "ipcmd.eCallStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CallStatus", HFILL }},
    { &hf_ipcmd_bCallStatus,
      { "bCallStatus", "ipcmd.bCallStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CallStatus", HFILL }},
    { &hf_ipcmd_iCallStatus,
      { "iCallStatus", "ipcmd.iCallStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CallStatus", HFILL }},
    { &hf_ipcmd_sdnStatus,
      { "sdnStatus", "ipcmd.sdnStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CallStatus", HFILL }},
    { &hf_ipcmd_backupAudioStatus,
      { "backupAudioStatus", "ipcmd.backupAudioStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_GenericOkStatus_vals), 0,
        "GenericOkStatus", HFILL }},
    { &hf_ipcmd_status_01,
      { "status", "ipcmd.status",
        FT_UINT32, BASE_DEC, VALS(ipcmd_RescueStatus_vals), 0,
        "RescueStatus", HFILL }},
    { &hf_ipcmd_callId,
      { "callId", "ipcmd.callId",
        FT_UINT32, BASE_DEC, VALS(ipcmd_XCallID_vals), 0,
        "XCallID", HFILL }},
    { &hf_ipcmd_voiceStatus,
      { "voiceStatus", "ipcmd.voiceStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_VoiceStatus_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_voiceSource,
      { "voiceSource", "ipcmd.voiceSource",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SourceStatus_vals), 0,
        "SourceStatus", HFILL }},
    { &hf_ipcmd_messageStatus,
      { "messageStatus", "ipcmd.messageStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_MessageStatus_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_buttonStatus,
      { "buttonStatus", "ipcmd.buttonStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_ButtonStatus_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_psapConfirmStatus,
      { "psapConfirmStatus", "ipcmd.psapConfirmStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_PSAPStatus_vals), 0,
        "PSAPStatus", HFILL }},
    { &hf_ipcmd_sbStatus,
      { "sbStatus", "ipcmd.sbStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_StandbyStatus_vals), 0,
        "StandbyStatus", HFILL }},
    { &hf_ipcmd_userPrivacySetting,
      { "userPrivacySetting", "ipcmd.userPrivacySetting_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "UserPrivacySettings", HFILL }},
    { &hf_ipcmd_carStatUploadEn,
      { "carStatUploadEn", "ipcmd.carStatUploadEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_locationServicesEn,
      { "locationServicesEn", "ipcmd.locationServicesEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_carLocatorStatUploadEn,
      { "carLocatorStatUploadEn", "ipcmd.carLocatorStatUploadEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_journalLogUploadEn,
      { "journalLogUploadEn", "ipcmd.journalLogUploadEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_simConnectEn,
      { "simConnectEn", "ipcmd.simConnectEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_remoteStatusUploadEn,
      { "remoteStatusUploadEn", "ipcmd.remoteStatusUploadEn",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_confirmation,
      { "confirmation", "ipcmd.confirmation",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SendToCarConfirmation_vals), 0,
        "SendToCarConfirmation", HFILL }},
    { &hf_ipcmd_confirmedId,
      { "confirmedId", "ipcmd.confirmedId",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SendToCarId", HFILL }},
    { &hf_ipcmd_sourceStatus,
      { "sourceStatus", "ipcmd.sourceStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SourceStatus_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_source,
      { "source", "ipcmd.source",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_1_30", HFILL }},
    { &hf_ipcmd_text,
      { "text", "ipcmd.text",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_1_140", HFILL }},
    { &hf_ipcmd_softwareVersion,
      { "softwareVersion", "ipcmd.softwareVersion",
        FT_STRING, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_0_20", HFILL }},
    { &hf_ipcmd_mapBaseVersion,
      { "mapBaseVersion", "ipcmd.mapBaseVersion",
        FT_STRING, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_0_20", HFILL }},
    { &hf_ipcmd_mapIncrement,
      { "mapIncrement", "ipcmd.mapIncrement",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_typeOfPackage,
      { "typeOfPackage", "ipcmd.typeOfPackage",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_failedFetchBooking,
      { "failedFetchBooking", "ipcmd.failedFetchBooking",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_failedServiceIP,
      { "failedServiceIP", "ipcmd.failedServiceIP",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_requestId,
      { "requestId", "ipcmd.requestId",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SendToCarId", HFILL }},
    { &hf_ipcmd_name,
      { "name", "ipcmd.name",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_0_30", HFILL }},
    { &hf_ipcmd_description,
      { "description", "ipcmd.description",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_0_100", HFILL }},
    { &hf_ipcmd_gpxFile,
      { "gpxFile", "ipcmd.gpxFile",
        FT_STRING, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_0_1048575", HFILL }},
    { &hf_ipcmd_time,
      { "time", "ipcmd.time_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DateTime", HFILL }},
    { &hf_ipcmd_settingIDs,
      { "settingIDs", "ipcmd.settingIDs",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_settingIDs_item,
      { "settingIDs item", "ipcmd.settingIDs_item",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_settings,
      { "settings", "ipcmd.settings",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_0_50_OF_Setting", HFILL }},
    { &hf_ipcmd_settings_item,
      { "Setting", "ipcmd.Setting_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_id,
      { "id", "ipcmd.id",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_sType,
      { "sType", "ipcmd.sType",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SettingType_vals), 0,
        "SettingType", HFILL }},
    { &hf_ipcmd_length,
      { "length", "ipcmd.length",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_value,
      { "value", "ipcmd.value",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_0_1023", HFILL }},
    { &hf_ipcmd_packetID,
      { "packetID", "ipcmd.packetID",
        FT_STRING, BASE_NONE, NULL, 0,
        "PrintableString_SIZE_0_40", HFILL }},
    { &hf_ipcmd_result,
      { "result", "ipcmd.result",
        FT_UINT32, BASE_DEC, VALS(ipcmd_GenericOkStatus_vals), 0,
        "GenericOkStatus", HFILL }},
    { &hf_ipcmd_onOff,
      { "onOff", "ipcmd.onOff",
        FT_UINT32, BASE_DEC, VALS(ipcmd_OnOffSetting_vals), 0,
        "OnOffSetting", HFILL }},
    { &hf_ipcmd_status_02,
      { "status", "ipcmd.status",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SIMConnectionStatus_vals), 0,
        "SIMConnectionStatus", HFILL }},
    { &hf_ipcmd_technology,
      { "technology", "ipcmd.technology",
        FT_UINT32, BASE_DEC, VALS(ipcmd_WirelessTechnology_vals), 0,
        "WirelessTechnology", HFILL }},
    { &hf_ipcmd_sosStatus,
      { "sosStatus", "ipcmd.sosStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SosStatus_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_connectivityInhibitionResult,
      { "connectivityInhibitionResult", "ipcmd.connectivityInhibitionResult",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_setRestoration,
      { "setRestoration", "ipcmd.setRestoration",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_restorationResult,
      { "restorationResult", "ipcmd.restorationResult",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_internetGateway,
      { "internetGateway", "ipcmd.internetGateway",
        FT_UINT32, BASE_DEC, VALS(ipcmd_Ecu_vals), 0,
        "Ecu", HFILL }},
    { &hf_ipcmd_isRequested,
      { "isRequested", "ipcmd.isRequested",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_premiumAudioStatus,
      { "premiumAudioStatus", "ipcmd.premiumAudioStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "AudioStatus", HFILL }},
    { &hf_ipcmd_action_01,
      { "action", "ipcmd.action",
        FT_UINT32, BASE_DEC, VALS(ipcmd_AssistCallAction_vals), 0,
        "AssistCallAction", HFILL }},
    { &hf_ipcmd_imei,
      { "imei", "ipcmd.imei",
        FT_STRING, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_15", HFILL }},
    { &hf_ipcmd_wifiMac,
      { "wifiMac", "ipcmd.wifiMac",
        FT_BYTES, BASE_NONE, NULL, 0,
        "MacAddress", HFILL }},
    { &hf_ipcmd_serialNr,
      { "serialNr", "ipcmd.serialNr",
        FT_STRING, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_15", HFILL }},
    { &hf_ipcmd_dlcConnected,
      { "dlcConnected", "ipcmd.dlcConnected",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_position_01,
      { "position", "ipcmd.position",
        FT_UINT32, BASE_DEC, VALS(ipcmd_DRVehiclePosition_vals), 0,
        "DRVehiclePosition", HFILL }},
    { &hf_ipcmd_drPosition,
      { "drPosition", "ipcmd.drPosition_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DRPositionData", HFILL }},
    { &hf_ipcmd_heading,
      { "heading", "ipcmd.heading",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_360", HFILL }},
    { &hf_ipcmd_speedKmph,
      { "speedKmph", "ipcmd.speedKmph",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_hdopX10,
      { "hdopX10", "ipcmd.hdopX10",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_numSat,
      { "numSat", "ipcmd.numSat",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_127", HFILL }},
    { &hf_ipcmd_drDistance,
      { "drDistance", "ipcmd.drDistance",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_65535", HFILL }},
    { &hf_ipcmd_gnssPositionData,
      { "gnssPositionData", "ipcmd.gnssPositionData_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GNSSData", HFILL }},
    { &hf_ipcmd_utcTime,
      { "utcTime", "ipcmd.utcTime_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DateTime", HFILL }},
    { &hf_ipcmd_gpsTime,
      { "gpsTime", "ipcmd.gpsTime_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GPSSystemTime", HFILL }},
    { &hf_ipcmd_position_02,
      { "position", "ipcmd.position_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GeographicalPosition", HFILL }},
    { &hf_ipcmd_movement,
      { "movement", "ipcmd.movement_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "Velocity", HFILL }},
    { &hf_ipcmd_heading_01,
      { "heading", "ipcmd.heading",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_35999", HFILL }},
    { &hf_ipcmd_gnssStatus,
      { "gnssStatus", "ipcmd.gnssStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GNSSUsage", HFILL }},
    { &hf_ipcmd_positioningStatus,
      { "positioningStatus", "ipcmd.positioningStatus_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GNSSStatus", HFILL }},
    { &hf_ipcmd_satelliteInfo,
      { "satelliteInfo", "ipcmd.satelliteInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "SatelliteUsage", HFILL }},
    { &hf_ipcmd_precision,
      { "precision", "ipcmd.precision_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DOPValues", HFILL }},
    { &hf_ipcmd_receiverChannels,
      { "receiverChannels", "ipcmd.receiverChannels",
        FT_UINT32, BASE_DEC, NULL, 0,
        "ReceiverChannelData", HFILL }},
    { &hf_ipcmd_weekNumber,
      { "weekNumber", "ipcmd.weekNumber",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_1023", HFILL }},
    { &hf_ipcmd_timeOfWeek,
      { "timeOfWeek", "ipcmd.timeOfWeek",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_604799999", HFILL }},
    { &hf_ipcmd_altitude,
      { "altitude", "ipcmd.altitude",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M1000_60000", HFILL }},
    { &hf_ipcmd_longitude,
      { "longitude", "ipcmd.longitude",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M2147483648_2147483647", HFILL }},
    { &hf_ipcmd_latitude,
      { "latitude", "ipcmd.latitude",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M1073741824_1073741824", HFILL }},
    { &hf_ipcmd_speed,
      { "speed", "ipcmd.speed",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_100000", HFILL }},
    { &hf_ipcmd_horizontalVelocity,
      { "horizontalVelocity", "ipcmd.horizontalVelocity",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_100000", HFILL }},
    { &hf_ipcmd_verticalVelocity,
      { "verticalVelocity", "ipcmd.verticalVelocity",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M100000_100000", HFILL }},
    { &hf_ipcmd_gpsIsUsed,
      { "gpsIsUsed", "ipcmd.gpsIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_glonassIsUsed,
      { "glonassIsUsed", "ipcmd.glonassIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_galileoIsUsed,
      { "galileoIsUsed", "ipcmd.galileoIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_sbasIsUsed,
      { "sbasIsUsed", "ipcmd.sbasIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_qzssL1IsUsed,
      { "qzssL1IsUsed", "ipcmd.qzssL1IsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_qzssL1SAIFIsUsed,
      { "qzssL1SAIFIsUsed", "ipcmd.qzssL1SAIFIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_dgpsIsUsed,
      { "dgpsIsUsed", "ipcmd.dgpsIsUsed",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_selfEphemerisDataUsage,
      { "selfEphemerisDataUsage", "ipcmd.selfEphemerisDataUsage",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_nrOfSatellitesVisible,
      { "nrOfSatellitesVisible", "ipcmd.nrOfSatellitesVisible_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "NrOfSatellitesPerSystem", HFILL }},
    { &hf_ipcmd_nrOfSatellitesUsed,
      { "nrOfSatellitesUsed", "ipcmd.nrOfSatellitesUsed_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "NrOfSatellitesPerSystem", HFILL }},
    { &hf_ipcmd_gps,
      { "gps", "ipcmd.gps",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_glonass,
      { "glonass", "ipcmd.glonass",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_galileo,
      { "galileo", "ipcmd.galileo",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_sbas,
      { "sbas", "ipcmd.sbas",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_qzssL1,
      { "qzssL1", "ipcmd.qzssL1",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_qzssL1SAIF,
      { "qzssL1SAIF", "ipcmd.qzssL1SAIF",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_31", HFILL }},
    { &hf_ipcmd_hdop,
      { "hdop", "ipcmd.hdop",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_vdop,
      { "vdop", "ipcmd.vdop",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_pdop,
      { "pdop", "ipcmd.pdop",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_tdop,
      { "tdop", "ipcmd.tdop",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_ReceiverChannelData_item,
      { "ChannelData", "ipcmd.ChannelData_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_prn,
      { "prn", "ipcmd.prn",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_255", HFILL }},
    { &hf_ipcmd_trackingStatus,
      { "trackingStatus", "ipcmd.trackingStatus",
        FT_UINT32, BASE_DEC, VALS(ipcmd_SatelliteTrackingStatusType_vals), 0,
        "SatelliteTrackingStatusType", HFILL }},
    { &hf_ipcmd_svacc,
      { "svacc", "ipcmd.svacc",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_15", HFILL }},
    { &hf_ipcmd_snr,
      { "snr", "ipcmd.snr",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_azimuthAngle,
      { "azimuthAngle", "ipcmd.azimuthAngle",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_elevationAngle,
      { "elevationAngle", "ipcmd.elevationAngle",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_extendedData,
      { "extendedData", "ipcmd.extendedData",
        FT_UINT32, BASE_DEC, VALS(ipcmd_ExtendedChannelData_vals), 0,
        "ExtendedChannelData", HFILL }},
    { &hf_ipcmd_notSupported,
      { "notSupported", "ipcmd.notSupported_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_ipcmd_data,
      { "data", "ipcmd.data_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "ChannelCorrectionData", HFILL }},
    { &hf_ipcmd_pseudoRangeMetres,
      { "pseudoRangeMetres", "ipcmd.pseudoRangeMetres",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M1000000000_1000000000", HFILL }},
    { &hf_ipcmd_pseudoRangeMillimetres,
      { "pseudoRangeMillimetres", "ipcmd.pseudoRangeMillimetres",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_999", HFILL }},
    { &hf_ipcmd_rangeRate,
      { "rangeRate", "ipcmd.rangeRate",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M1000000_1000000", HFILL }},
    { &hf_ipcmd_pseudoRangeCorrectionData,
      { "pseudoRangeCorrectionData", "ipcmd.pseudoRangeCorrectionData",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M1000000_1000000", HFILL }},
    { &hf_ipcmd_selfEphemerisPredictionTime,
      { "selfEphemerisPredictionTime", "ipcmd.selfEphemerisPredictionTime",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_rawDeadReckoningData,
      { "rawDeadReckoningData", "ipcmd.rawDeadReckoningData_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "DeadReckoningRawData", HFILL }},
    { &hf_ipcmd_whlRotToothCntrFrntLe,
      { "whlRotToothCntrFrntLe", "ipcmd.whlRotToothCntrFrntLe",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_whlRotToothCntrFrntRi,
      { "whlRotToothCntrFrntRi", "ipcmd.whlRotToothCntrFrntRi",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_whlRotToothCntrReLe,
      { "whlRotToothCntrReLe", "ipcmd.whlRotToothCntrReLe",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_whlRotToothCntrReRi,
      { "whlRotToothCntrReRi", "ipcmd.whlRotToothCntrReRi",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_ipcmd_whlCircum,
      { "whlCircum", "ipcmd.whlCircum",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4095", HFILL }},
    { &hf_ipcmd_aLat1,
      { "aLat1", "ipcmd.aLat1",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M139_139", HFILL }},
    { &hf_ipcmd_aLat1Qf1,
      { "aLat1Qf1", "ipcmd.aLat1Qf1",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_3", HFILL }},
    { &hf_ipcmd_aLgt1,
      { "aLgt1", "ipcmd.aLgt1",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER_M139_139", HFILL }},
    { &hf_ipcmd_aLgt1Qf1,
      { "aLgt1Qf1", "ipcmd.aLgt1Qf1",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_3", HFILL }},
    { &hf_ipcmd_gearIndcn,
      { "gearIndcn", "ipcmd.gearIndcn",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_15", HFILL }},
    { &hf_ipcmd_sessionStatus,
      { "sessionStatus", "ipcmd.sessionStatus",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_doIPState,
      { "doIPState", "ipcmd.doIPState",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_ipcmd_doIPMode,
      { "doIPMode", "ipcmd.doIPMode",
        FT_UINT32, BASE_DEC, VALS(ipcmd_DoIPMode_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_doIPPhase,
      { "doIPPhase", "ipcmd.doIPPhase",
        FT_UINT32, BASE_DEC, VALS(ipcmd_DoIPPhase_vals), 0,
        NULL, HFILL }},
    { &hf_ipcmd_currentDoIPConn,
      { "currentDoIPConn", "ipcmd.currentDoIPConn",
        FT_UINT32, BASE_DEC, VALS(ipcmd_CurrentDoIPConn_vals), 0,
        NULL, HFILL }},

/*--- End of included file: packet-ipcmd-hfarr.c ---*/
#line 38 "../../plugins/ipcmd/packet-ipcmd-template.c"
    };
    
    /* List of subtrees */    
    static gint *ett[] = {
        &ett_ipcmd,

/*--- Included file: packet-ipcmd-ettarr.c ---*/
#line 1 "../../plugins/ipcmd/packet-ipcmd-ettarr.c"
    &ett_ipcmd_VccPduHeader,
    &ett_ipcmd_OpGeneric_Error,
    &ett_ipcmd_DateTime,
    &ett_ipcmd_AudioStatus,
    &ett_ipcmd_UUID,
    &ett_ipcmd_PartIdentifier,
    &ett_ipcmd_OpTelematicSettings_SetRequest,
    &ett_ipcmd_OpTelematicSettings_Response,
    &ett_ipcmd_TelemSettings,
    &ett_ipcmd_OpPositionData_Response,
    &ett_ipcmd_SimpleVehiclePosition,
    &ett_ipcmd_WGS84SimplePositionData,
    &ett_ipcmd_OpSubscriptionActivation_Response,
    &ett_ipcmd_OpSubscriptionActivation_Notification,
    &ett_ipcmd_OpServiceActivation_SetRequest,
    &ett_ipcmd_OpServiceActivation_Response,
    &ett_ipcmd_OpRescueStatus_Response,
    &ett_ipcmd_OpRescueStatus_Notification,
    &ett_ipcmd_CallStatus,
    &ett_ipcmd_OpUserPrivacySettings_SetRequest,
    &ett_ipcmd_OpUserPrivacySettings_Response,
    &ett_ipcmd_UserPrivacySettings,
    &ett_ipcmd_OpSendToCarConfirmation_SetRequest,
    &ett_ipcmd_OpSendToCarConfirmation_Response,
    &ett_ipcmd_OpTextMessage_SetRequest,
    &ett_ipcmd_OpIHUSystemInfo_Response,
    &ett_ipcmd_OpSendToCar_SetRequest,
    &ett_ipcmd_OpGenericSettingSynch_Request,
    &ett_ipcmd_T_settingIDs,
    &ett_ipcmd_OpGenericSettingSynch_SetRequest,
    &ett_ipcmd_SEQUENCE_SIZE_0_50_OF_Setting,
    &ett_ipcmd_OpGenericSettingSynch_Response,
    &ett_ipcmd_OpGenericSettingSynch_Notification,
    &ett_ipcmd_Setting,
    &ett_ipcmd_OpSoHPackageUploaded_Notification,
    &ett_ipcmd_OpSIMConnect_SetRequest,
    &ett_ipcmd_OpSIMConnect_Response,
    &ett_ipcmd_OpSIMConnectionStatus_Response,
    &ett_ipcmd_OpSIMConnectionStatus_Notification,
    &ett_ipcmd_OpConnectivityInhibitionStatus_SetRequest,
    &ett_ipcmd_OpConnectivityInhibitionStatus_Response,
    &ett_ipcmd_OpConnectivityInhibitionStatus_Notification,
    &ett_ipcmd_OpFactoryDefaultRestore_SetRequest,
    &ett_ipcmd_OpFactoryDefaultRestore_Response,
    &ett_ipcmd_OpInternetGateway_SetRequest,
    &ett_ipcmd_OpInternetGateway_Response,
    &ett_ipcmd_OpPremiumAudio_SetRequest,
    &ett_ipcmd_OpPremiumAudio_Response,
    &ett_ipcmd_OpCallHandling_SetRequest,
    &ett_ipcmd_OpTEM2Identification_Response,
    &ett_ipcmd_OpDLCConnectedSignal_Response,
    &ett_ipcmd_OpDeadReckonedPosition_Response,
    &ett_ipcmd_DRVehiclePosition,
    &ett_ipcmd_DRPositionData,
    &ett_ipcmd_OpGNSSPositionData_Response,
    &ett_ipcmd_GNSSData,
    &ett_ipcmd_GPSSystemTime,
    &ett_ipcmd_GeographicalPosition,
    &ett_ipcmd_CoordinatesLongLat,
    &ett_ipcmd_Velocity,
    &ett_ipcmd_GNSSUsage,
    &ett_ipcmd_GNSSStatus,
    &ett_ipcmd_SatelliteUsage,
    &ett_ipcmd_NrOfSatellitesPerSystem,
    &ett_ipcmd_DOPValues,
    &ett_ipcmd_ReceiverChannelData,
    &ett_ipcmd_ChannelData,
    &ett_ipcmd_ExtendedChannelData,
    &ett_ipcmd_ChannelCorrectionData,
    &ett_ipcmd_OpDeadReckoningRawData_Response,
    &ett_ipcmd_DeadReckoningRawData,
    &ett_ipcmd_OpCurrentJ2534Session_Notification,
    &ett_ipcmd_OpCurrentDoIPState_Notification,
    &ett_ipcmd_OpCurrentDoIPConnection_Notification,

/*--- End of included file: packet-ipcmd-ettarr.c ---*/
#line 44 "../../plugins/ipcmd/packet-ipcmd-template.c"
    };

    /* Register protocol */
    proto_ipcmd = proto_register_protocol(PNAME, PSNAME, PFNAME);
    /* Register fields and subtrees */
    proto_register_field_array(proto_ipcmd, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));

    /* Register new dissector table */
    register_dissector_table("ipcmd_payload", "IP Command Bus Payload Message", FT_STRING, BASE_NONE);
}

/*--- proto_reg_handoff_ipcmd ---------------------------------------*/
#define IPCMD_ADD_PAYLOAD_DISSECTOR(HANDLE, OPNAME, OPTYPE)             \
    do {                                                                \
        HANDLE = create_dissector_handle(dissect_##OPNAME##_##OPTYPE##_PDU, proto_ipcmd); \
        dissector_add_string("ipcmd_payload", #OPNAME"-"#OPTYPE, HANDLE); \
    } while(0)

void
proto_reg_handoff_ipcmd(void)
{
    static gboolean inited = FALSE;
    
    if( !inited ) {
        dissector_handle_t handle=NULL;
        
        /* Add dissector handle to the table */
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpTelematicSettings, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpTelematicSettings, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpPositionData, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSubscriptionActivation, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSubscriptionActivation, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpServiceActivation, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpServiceActivation, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpRescueStatus, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpRescueStatus, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpUserPrivacySettings, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpUserPrivacySettings, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSendToCarConfirmation, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSendToCarConfirmation, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpTextMessage, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpIHUSystemInfo, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSendToCar, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpGenericSettingSynch, Request);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpGenericSettingSynch, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpGenericSettingSynch, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpGenericSettingSynch, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSoHPackageUploaded, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSIMConnect, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSIMConnect, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSIMConnectionStatus, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpSIMConnectionStatus, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpConnectivityInhibitionStatus, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpConnectivityInhibitionStatus, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpConnectivityInhibitionStatus, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpFactoryDefaultRestore, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpFactoryDefaultRestore, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpInternetGateway, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpInternetGateway, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpPremiumAudio, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpPremiumAudio, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpCallHandling, SetRequest);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpTEM2Identification, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpDLCConnectedSignal, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpDeadReckonedPosition, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpGNSSPositionData, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpDeadReckoningRawData, Response);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpCurrentJ2534Session, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpCurrentDoIPState, Notification);
        IPCMD_ADD_PAYLOAD_DISSECTOR(handle, OpCurrentDoIPConnection, Notification);

        inited = TRUE;
    }
}
