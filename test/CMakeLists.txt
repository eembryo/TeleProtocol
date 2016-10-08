add_executable(CoreTest	./CoreTest.c)
target_include_directories(CoreTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(CoreTest
	${LIBRARIES}
	${IPCOMM_LIB})

add_executable(ClientTest ./ClientTest.c)
target_include_directories(ClientTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ClientTest
	${LIBRARIES}
	${IPCOMM_LIB})

add_executable(ClientTestNoti ./ClientTestNoti.c)
target_include_directories(ClientTestNoti PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ClientTestNoti
	${LIBRARIES}
	${IPCOMM_LIB})
	
add_executable(ServerTest ./ServerTest.c)
target_include_directories(ServerTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServerTest
	${LIBRARIES}
	${IPCOMM_LIB})
	
add_executable(ServerTestNoti ./ServerTestNoti.c)
target_include_directories(ServerTestNoti PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServerTestNoti
	${LIBRARIES}
	${IPCOMM_LIB})
	
add_executable(ServerAgentTest ./ServerAgentTest.c)
target_include_directories(ServerAgentTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServerAgentTest
	${LIBRARIES}
	${IPCOMM_LIB})
	