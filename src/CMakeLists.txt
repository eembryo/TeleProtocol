set(SRCS 
   ./IpcomConnection.c
   ./IpcomMessage.c
   ./IpcomTransport.c
   ./IpcomProtocol.c
   ./IpcomService.c
   ./IpcomTransportUDPv4.c
   ./IpcomOperationContext.c
   ./IpcomOpStateMachine.c
   ./IpcomEnums.c)

add_library(${IPCOMM_LIB} SHARED ${SRCS})
target_link_libraries(${IPCOMM_LIB} ${LIBRARIES})
install(TARGETS ${IPCOMM_LIB} LIBRARY  DESTINATION /app/lib)
