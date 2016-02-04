set(SRCS 
   ./IpcomConnection.c
   ./IpcomMessage.c
   ./IpcomTransport.c
   ./IpcomProtocol.c
   ./IpcomService.c
   ./IpcomTransportUDPv4.c
   ./IpcomOperationContext.c)

add_library(${TEMIL_IPCOMM_LIB} SHARED ${SRCS})
target_link_libraries(${TEMIL_IPCOMM_LIB} ${LIBRARIES})
install(TARGETS ${TEMIL_IPCOMM_LIB} LIBRARY  DESTINATION /app/lib)