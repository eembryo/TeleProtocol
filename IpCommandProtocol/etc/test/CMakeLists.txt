add_executable(ServiceClient	./ServiceClient.c)
target_include_directories(ServiceClient PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServiceClient
	${LIBRARIES}
	${TEMIL_IPCOMM_LIB})

add_executable(ServiceServer	./ServiceServer.c)
target_include_directories(ServiceServer PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServiceServer
	${LIBRARIES}
	${TEMIL_IPCOMM_LIB})
