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

add_executable(ServerTest ./ServerTest.c)
target_include_directories(ServerTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(ServerTest
	${LIBRARIES}
	${IPCOMM_LIB})
	