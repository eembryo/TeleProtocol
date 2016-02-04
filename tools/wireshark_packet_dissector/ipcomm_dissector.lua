dprint = function(...)
   print(table.concat({"Lua:", ...}," "))
end

local ipcomm = Proto("ipcomm", "IPCommand Protocol")

local Telematics_OperationID_code = {
   [0x0104] = "TelematicsSettings",
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

local opid_field = Field.new("ipcomm.op_id")
local optype_field = Field.new("ipcomm.op_type")

function ipcomm.dissector(tvbuf, pktinfo, root)
   pktinfo.cols.protocol:set("IPComm")

   local pktlen = tvbuf:reported_length_remaining()
   local tree = root:add(ipcomm, tvbuf:range(0,pktlen))

   tree:add(pf_service_id, tvbuf:range(0,2))
   local opid_item = tree:add(pf_operation_id, tvbuf:range(2,2))
   local opid_string = Telematics_OperationID_code[opid_field()()]
   if opid_string == nil then
      opid_string = "unknown operation ID"
   end
   opid_item:append_text(" ("..opid_string..")")

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

   pktinfo.cols.info:set(opid_string.."."..optype_string)

   local IPCOM_HDR_LEN = 12
   return IPCOM_HDR_LEN
end

DissectorTable.get("udp.port"):add(50000, ipcomm)
