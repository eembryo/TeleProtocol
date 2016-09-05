set(SRCS 
	IpcmdHost.c
   )

add_library(${IPCOMM_LIB} SHARED ${SRCS})
target_link_libraries(${IPCOMM_LIB} ${LIBRARIES})
install(TARGETS ${IPCOMM_LIB} LIBRARY  DESTINATION /app/lib)
