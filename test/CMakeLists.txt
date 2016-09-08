add_executable(CoreTest	./CoreTest.c)
target_include_directories(CoreTest PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(CoreTest
	${LIBRARIES}
	${IPCOMM_LIB})
