add_library(IpcomProtocol SHARED
   ./IpcomConnection.c
   ./IpcomMessage.c
   ./IpcomTransport.c
   ./IpcomProtocol.c
   ./IpcomService.c
   ./IpcomTransportUDPv4.c)

target_include_directories(IpcomProtocol PRIVATE ${GLIB_INCLUDE_DIRS})

target_link_libraries(IpcomProtocol
	${GLIB_LIBRARIES}
	${GLIB_GIO_LIBRARIES}
	${GLIB_GIO_UNIX_LIBRARIES})
