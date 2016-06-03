dprint = function(...)
    print(table.concat({"Lua:", ...}," "))
end

local ipcomm = Proto("ipcomm", "IPCommand Protocol")

local service_code = {
    [0x00A1] = "Telematics",
    [0x00A3] = "Connectivity",
    [0x00A7] = "Common Phone/Telematics",
    [0x00A8] = "Common All",
    [0x00A9] = "Positioning",
    [0x00AA] = "Diagnostic Management",
    [0xFFFF] = "IP Activity",
}

local operationID_code = {
    [0x0104] = "TelematicSettings",
    [0x0105] = "PositionData",
    [0x0107] = "SubscriptionActivation",
    [0x010E] = "SendToCarConfirmation",
    [0x0102] = "SVTStatus",
    [0x0103] = "TNStatus",
    [0x010A] = "RescueStatus",
    [0x0106] = "TextMessage",
    [0x010B] = "IHUSystemInfo",
    [0x010C] = "UserPrivacySettings",
    [0x010D] = "SendToCar",
    [0x0111] = "SoHWarning",
    [0x0114] = "GenericSettingSynch",
    [0x0112] = "SoHPacketSend",
    [0x0113] = "SoHPackageUploaded",
    [0x0305] = "SIMConnect",
    [0x0306] = "SIMConnectionStatus",
    [0x0308] = "InternetGateway",
    [0x030E] = "FactoryDefaultRestore",
    [0x03FF] = "Connectivity Inhibition",
    [0x0702] = "PremiumAudio",
    [0x0703] = "CallHandling",
    [0x0801] = "TEM2Identification",
    [0x0805] = "DLCConnectedSignal",
    [0x0901] = "DeadReckonedPosition",
    [0x0902] = "GNSSPositionData",
    [0x0903] = "DeadReckoningRawData",
    [0x0A05] = "CurrentJ2534Session",
    [0x0A06] = "CurrentDoIPState",
    [0x0A07] = "CurrentDoIPConnection",
    [0xff01] = "IP_Activity",
}

local optype_code = {
    [0x00] = "REQUEST",
    [0x01] = "SETREQUEST_NORETURN",
    [0x02] = "SETREQUEST",
    [0x03] = "NOTIFICATION_REQUEST",
    [0x04] = "RESPONSE",
    [0x05] = "NOTIFICATION",
    [0x06] = "NOTIFICATION_CYCLE",
    [0x70] = "ACK",
    [0xE0] = "ERROR"
}

local pf_service_id             = ProtoField.new	("Service ID", "ipcomm.service_id", ftypes.UINT16, service_code, base.HEX)
local pf_operation_id           = ProtoField.new	("Operation ID", "ipcomm.op_id", ftypes.UINT16, operationID_code, base.HEX)
local pf_length                 = ProtoField.new	("Length", "ipcomm.length", ftypes.UINT32)
local pf_handle_id              = ProtoField.new	("SenderHandleID", "ipcomm.handle_id", ftypes.UINT32, nil, base.HEX)
local pf_handle_service_id      = ProtoField.new	("HandleServiceID", "ipcomm.handle_service_id8", ftypes.UINT8, nil, base.HEX)
local pf_handle_operation_id    = ProtoField.new	("HandleOperationID", "ipcomm.handle_op_id", ftypes.UINT8, nil, base.HEX)
local pf_handle_op_type         = ProtoField.new	("HandleOperationType", "ipcomm.handle_op_type", ftypes.UINT8, nil, base.HEX)
local pf_handle_seqNr           = ProtoField.new	("HandleSeqNr","ipcomm.handle_seqNr", ftypes.UINT8, nil, base.HEX)
local pf_proto_version          = ProtoField.new	("Protocol version", "ipcomm.proto_version", ftypes.UINT8, nil, base.HEX)
local pf_opType                 = ProtoField.new	("OpType", "ipcomm.op_type", ftypes.UINT8, optype_code, base.HEX)
local pf_dataType               = ProtoField.new	("DataType", "ipcomm.data_type", ftypes.UINT8, nil, base.HEX)
local pf_reserved               = ProtoField.new	("Reserved", "ipcomm.reserved", ftypes.UINT8, nil, base.HEX)

ipcomm.fields = { pf_service_id, pf_operation_id,
                  pf_length,
                  pf_handle_id,
                  pf_handle_service_id, pf_handle_operation_id, pf_handle_op_type, pf_handle_seqNr,
                  pf_proto_version, pf_opType, pf_dataType, pf_reserved }

local service_field = Field.new("ipcomm.service_id")
local opid_field    = Field.new("ipcomm.op_id")
local optype_field  = Field.new("ipcomm.op_type")
local handleid_field    = Field.new("ipcomm.handle_id")

-- IP command payload dissectors, which is implemented in C code
-- If you want, override payload dissectors
--local ipcmd_pl_dt = DissectorTable.get("ipcmd_payload")
local ipcmd_pl_dt = nil
local payload_dissectors = nil

function ipcomm.dissector(tvbuf, pktinfo, root)
    pktinfo.cols.protocol:set("IPComm")

    local tree = root:add(ipcomm, tvbuf:range(0,-1))

    tree:add(pf_service_id, tvbuf:range(0,2))
    tree:add(pf_operation_id, tvbuf:range(2,2))

    -- sender handle id
    tree:add(pf_length, tvbuf:range(4,4))
    local handle_tree = tree:add(pf_handle_id, tvbuf:range(8,4))
    handle_tree:add(pf_handle_service_id, tvbuf:range(8,1))
    handle_tree:add(pf_handle_operation_id, tvbuf:range(9,1))
    handle_tree:add(pf_handle_op_type, tvbuf:range(10,1))
    handle_tree:add(pf_handle_seqNr, tvbuf:range(11,1))
    
    tree:add(pf_proto_version, tvbuf:range(12,1))
    tree:add(pf_opType, tvbuf:range(13,1))
    tree:add(pf_dataType, tvbuf:range(14,1))
    tree:add(pf_reserved, tvbuf:range(15,1))

    pktinfo.cols.info:set("(SHID: "..string.format("0x%.4X", handleid_field()())..") "..operationID_code[opid_field()()].."."..optype_code[optype_field()()])

    if payload_dissectors ~= nil and
        payload_dissectors[service_field()()] ~= nil and
        payload_dissectors[service_field()()][opid_field()()] ~= nil and
        payload_dissectors[service_field()()][opid_field()()][optype_field()()] ~= nil
    then
        payload_dissectors[service_field()()][opid_field()()][optype_field()()](tvbuf:range(16,-1):tvb(), pktinfo, tree)
    end
    
    return tvbuf:reported_length_remaining()
end

do
    --If you loaded IPCMD dissector plugin, comment out the following line
    --ipcmd_pl_dt = DissectorTable.get("ipcmd_payload")

    if ipcmd_pl_dt ~= nil then
        payload_dissectors = {
            [0x00A1] = {
                [0x0104] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpTelematicSettings-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpTelematicSettings-Response"),
                },
                [0x0105] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpPositionData-Response"),
                },
                [0x0107] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpSubscriptionActivation-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpSubscriptionActivation-Notification"),
                },
                [0x0108] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpServiceActivation-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpServiceActivation-Response"),
                },
                [0x010A] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpRescueStatus-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpRescueStatus-Notification"),
                },
                [0x010C] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpUserPrivacySettings-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpUserPrivacySettings-Response"),
                },
                [0x010C] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpUserPrivacySettings-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpUserPrivacySettings-Response"),
                },
                [0x010E] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpSendToCarConfirmation-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpSendToCarConfirmation-Response"),
                },
                [0x0106] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpTextMessage-SetRequest"),
                },
                [0x010B] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpIHUSystemInfo-Response"),
                },
                [0x010D] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpSendToCar-SetRequest"),
                },
                [0x0113] = {
                    [0x05] = ipcmd_pl_dt:get_dissector("OpSoHPackageUploaded-Notification"),
                },
                [0x0114] = {
                    [0x00] = ipcmd_pl_dt:get_dissector("OpGenericSettingSynch-Request"),
                    [0x02] = ipcmd_pl_dt:get_dissector("OpGenericSettingSynch-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpGenericSettingSynch-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpGenericSettingSynch-Notification"),
                },
            },
            [0x00A3] = {
                [0x0305] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpSIMConnect-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpSIMConnect-Response"),
                },
                [0x0306] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpSIMConnectionStatus-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpSIMConnectionStatus-Notification"),
                },
                [0x03FF] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpConnectivityInhibitionStatus-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpConnectivityInhibitionStatus-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpConnectivityInhibitionStatus-Notification"),
                },
                [0x030E] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpFactoryDefaultRestore-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpFactoryDefaultRestore-Response"),
                },
                [0x0308] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpInternetGateway-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpInternetGateway-Response"),
                },
            },
            [0x00A7] = {
                [0x0702] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpPremiumAudio-SetRequest"),
                    [0x04] = ipcmd_pl_dt:get_dissector("OpPremiumAudio-Response"),
                },
                [0x0703] = {
                    [0x02] = ipcmd_pl_dt:get_dissector("OpCallHandling-SetRequest"),
                },
            },
            [0x00A8] = {
                [0x0801] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpTEM2Identification-Response"),
                },
                [0x0805] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpDLCConnectedSignal-Response"),
                },
            },
            [0x00A9] = {
                [0x0901] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpDeadReckonedPosition-Response"),
                    [0x05] = ipcmd_pl_dt:get_dissector("OpDeadReckonedPosition-Response"),
                },
                [0x0902] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpGNSSPositionData-Response"),
                },
                [0x0903] = {
                    [0x04] = ipcmd_pl_dt:get_dissector("OpDeadReckoningRawData-Response"),
                },
            },
            [0x00AA] = {
                [0x0A05] = {
                    [0x05] = ipcmd_pl_dt:get_dissector("OpCurrentJ2534Session-Notification"),
                },
                [0x0A06] = {
                    [0x05] = ipcmd_pl_dt:get_dissector("OpCurrentDoIPState-Notification"),
                },
                [0x0A07] = {
                    [0x05] = ipcmd_pl_dt:get_dissector("OpCurrentDoIPConnection-Notification"),
                },
            },
        }
    end
    
    DissectorTable.get("udp.port"):add(50001, ipcomm)
    DissectorTable.get("udp.port"):add(50000, ipcomm)
end
