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

#include "packet-ipcmd-hf.c"

/* Initialize the subtree pointers */
static int ett_ipcmd = -1;

#include "packet-ipcmd-ett.c"

#include "packet-ipcmd-fn.c"

/*--- proto_register_ipcmd -------------------------------------------*/
void proto_register_ipcmd(void) {
    /* List of fields */
    static hf_register_info hf[] = {

#include "packet-ipcmd-hfarr.c"
    };
    
    /* List of subtrees */    
    static gint *ett[] = {
        &ett_ipcmd,
#include "packet-ipcmd-ettarr.c"
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
