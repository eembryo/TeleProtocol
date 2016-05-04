add_executable(ServiceClient	./ServiceClient.c)
target_include_directories(ServiceClient PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServiceClient
	${LIBRARIES}
	${IPCOMM_LIB})

add_executable(ServiceServer	./ServiceServer.c)
target_include_directories(ServiceServer PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServiceServer
	${LIBRARIES}
	${IPCOMM_LIB})
	
add_executable(ServerAgent	./ServerAgent.c)
target_include_directories(ServerAgent PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServerAgent
	${LIBRARIES}
	${IPCOMM_LIB})

	add_executable(ClientAgent	./ClientAgent.c)
target_include_directories(ClientAgent PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ClientAgent
	${LIBRARIES}
	${IPCOMM_LIB})