add_executable(tem2client
	./IpcomServiceClientTem2.c)

target_include_directories(tem2client PRIVATE ${GLIB_INCLUDE_DIRS})

target_link_libraries(tem2client
	${GLIB_LIBRARIES}
	${GLIB_GIO_LIBRARIES}
	${GLIB_GIO_UNIX_LIBRARIES}
	IpcomProtocol)