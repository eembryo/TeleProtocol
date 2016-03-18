dprint = function(...)
   print(table.concat({"Lua:", ...}," "))
end

local ipcomm = Proto("ipcomm", "IPCommand Protocol")

local service_code = {
   [0xA1] = "Telematics",
   [0xA3] = "Connectivity",
   [0xA7] = "Common Phone/Telematics",
   [0xA8] = "Common All",
   [0xA9] = "Positioning",
   [0xAA] = "Diagnostic Management",
}

local operationID_code = {
   [0xA1] = {
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
   },
   [0xA3] = {
      [0x0305] = "SIMConnect",
      [0x0306] = "SIMConnectionStatus",
      [0x0308] = "InternetGateway",
      [0x030E] = "FactoryDefaultRestore",
      [0x03FF] = "Connectivity Inhibition",
   },
   [0xA7] = {
      [0x0702] = "PremiumAudio",
      [0x0703] = "CallHandling",
   },
   [0xA8] = {
      [0x0801] = "TEM2Identification",
      [0x0805] = "DLCConnectedSignal",
   },
   [0xA9] = {
      [0x0901] = "DeadReckonedPosition",
      [0x0902] = "GNSSPositionData",
      [0x0903] = "DeadReckoningRawData",
   },
   [0xAA] = {
      [0x0A05] = "CurrentJ2534Session",
      [0x0A06] = "CurrentDoIPState",
      [0x0A07] = "CurrentDoIPConnection",
   },
}

local optype_code = {
   [0x00] = "REQUEST              ",
   [0x01] = "SETREQUEST_NORETURN  ",
   [0x02] = "SETREQUEST           ",
   [0x03] = "NOTIFICATION_REQUEST ",
   [0x04] = "RESPONSE             ",
   [0x05] = "NOTIFICATION         ",
   [0x06] = "NOTIFICATION_CYCLE   ",
   [0x70] = "ACK                  ",
   [0xE0] = "ERROR                "
}

local pf_service_id	= ProtoField.new	("Service ID", "ipcomm.service_id", ftypes.UINT16, nil, base.HEX)
local pf_operation_id	= ProtoField.new	("Operation ID", "ipcomm.op_id", ftypes.UINT16, nil, base.HEX)
local pf_length		= ProtoField.new	("Length", "ipcomm.length", ftypes.UINT32)
local pf_handle_id	= ProtoField.new	("SenderHandleID", "ipcomm.handle_id", ftypes.UINT32, nil, base.HEX)
local pf_handle_service_id	= ProtoField.new	("HandleServiceID", "ipcomm.handle_service_id8", ftypes.UINT8, nil, base.HEX)
local pf_handle_operation_id	= ProtoField.new	("HandleOperationID", "ipcomm.handle_op_id", ftypes.UINT8, nil, base.HEX)
local pf_handle_op_type		= ProtoField.new	("HandleOperationType", "ipcomm.handle_op_type", ftypes.UINT8, nil, base.HEX)
local pf_handle_seqNr		= ProtoField.new	("HandleSeqNr","ipcomm.handle_seqNr", ftypes.UINT8, nil, base.HEX)
local pf_proto_version	= ProtoField.new	("Protocol version", "ipcomm.proto_version", ftypes.UINT8, nil, base.HEX)
local pf_opType		= ProtoField.new	("OpType", "ipcomm.op_type", ftypes.UINT8, nil, base.HEX)
local pf_dataType	= ProtoField.new	("DataType", "ipcomm.data_type", ftypes.UINT8, nil, base.HEX)
local pf_reserved	= ProtoField.new	("Reserved", "ipcomm.reserved", ftypes.UINT8, nil, base.HEX)

ipcomm.fields = { pf_service_id, pf_operation_id,
		  pf_length,
		  pf_handle_id,
		  pf_handle_service_id, pf_handle_operation_id, pf_handle_op_type, pf_handle_seqNr,
		  pf_proto_version, pf_opType, pf_dataType, pf_reserved }

local service_field = Field.new("ipcomm.service_id")
local opid_field = Field.new("ipcomm.op_id")
local optype_field = Field.new("ipcomm.op_type")
local handleid_field = Field.new("ipcomm.handle_id")

function ipcomm.dissector(tvbuf, pktinfo, root)
   pktinfo.cols.protocol:set("IPComm")

   local pktlen = tvbuf:reported_length_remaining()
   local tree = root:add(ipcomm, tvbuf:range(0,pktlen))

   -- service
   local service_item = tree:add(pf_service_id, tvbuf:range(0,2))
   local service_string = service_code[service_field()()];
   if service_string == nil then
      service_string = "unknown service"
   end
   service_item:append_text(" ("..service_string..")")

   -- operation id
   local opid_item = tree:add(pf_operation_id, tvbuf:range(2,2))
   local opid_string = operationID_code[service_field()()][opid_field()()]
   if opid_string == nil then
      opid_string = "unknown operation ID"
   end
   opid_item:append_text(" ("..opid_string..")")

   -- sender handle id
   tree:add(pf_length, tvbuf:range(4,4))
   local handle_tree = tree:add(pf_handle_id, tvbuf:range(8,4))
   handle_tree:add(pf_handle_service_id, tvbuf:range(8,1))
   handle_tree:add(pf_handle_operation_id, tvbuf:range(9,1))
   handle_tree:add(pf_handle_op_type, tvbuf:range(10,1))
   handle_tree:add(pf_handle_seqNr, tvbuf:range(11,1))
   
   tree:add(pf_proto_version, tvbuf:range(12,1))
   local optype_item = tree:add(pf_opType, tvbuf:range(13,1))
   local optype_string = optype_code[optype_field()()]
   if optype_string == nil then
      optype_string = "unknown optype"
   end
   optype_item:append_text(" ("..optype_string..")")
   
   tree:add(pf_dataType, tvbuf:range(14,1))
   tree:add(pf_reserved, tvbuf:range(15,1))

   pktinfo.cols.info:set(opid_string.."."..optype_string.."   (SenderHandleID: "..string.format("0x%.4X", handleid_field()())..")")

   local IPCOM_HDR_LEN = 12
   return IPCOM_HDR_LEN
end

DissectorTable.get("udp.port"):add(50000, ipcomm)
