set(SRCS
	IpcmdCore.c
	IpcmdBus.c 
	IpcmdHost.c
	IpcmdChannel.c
	IpcmdMessage.c
	IpcmdClient.c
	IpcmdOperation.c
	IpcmdOperationContext.c
	IpcmdServer.c
	IpcmdTransportUdpv4.c
	IpcmdOpStateMachine.c
	IpcmdOpStateMachineCommon.c
	IpcmdOpStateMachineCLRequest.c
	IpcmdOpStateMachineCLSetnor.c
	IpcmdOpStateMachineCLNotreq.c
	IpcmdOpStateMachineCLSetreq.c
	IpcmdOpStateMachineCLNoti.c
	IpcmdConfig.c
	IpcmdService.c
	IpcmdAgent.c
	NetifcMonitor.c
	SocketUtils.c
   )

add_library(${IPCOMM_LIB} SHARED ${SRCS})
target_link_libraries(${IPCOMM_LIB} ${LIBRARIES})
install(TARGETS ${IPCOMM_LIB} LIBRARY  DESTINATION ${IPCOMM_INSTALL_PREFIX}/lib)
