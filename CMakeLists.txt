# @@@LICENSE
#
# Copyright (C) 2015-2016, LG Electronics, All Right Reserved.
#
# No part of this source code may be communicated, distributed, reproduced
# or transmitted in any form or by any means, electronic or mechanical or
# otherwise, for any purpose, without the prior written permission of
# LG Electronics.
#
# LICENSE@@@
# 
# Design/Author : dhilipkumar.raman@lge.com, jinheung.tark@lgepartner.com 
# Date : 02/04/2016 
# 
# <IHU XCallService> Detailed note is as follows. 
# 

cmake_minimum_required(VERSION 2.8.1)

project(TEM-XCALL-SERVICE C CXX)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(DLT REQUIRED automotive-dlt)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

set(TEM-XCALL-SERVICE-BIN xcall-service)
set(TEM-XCALL-EXTERNAL-IF-LIB xcallExternalIFLib)

set(TEM-XCALL-SERVICE-LIBS
	${GLIB2_LDFLAGS}
	${GIO_LDFLAGS}
	${GIO_UNIX_LDFLAGS}
	${GSTREAMER_LDFLAGS}
	${DLT_LDFLAGS}
	${SYSTEMD_LDFLAGS}
	xcallExternalIFLib
	)

add_definitions(-DDEBUG_ENABLED)
#add_definitions(-DRECEIVE_FILESINK_ENABLED)
add_definitions(-DGEELY_HARDWARE)
#add_definitions(-DENABLE_UNITTEST_ASK_MMUS_API)
add_definitions(-DDLT_LOG_ENABLE)
add_definitions(-DSYSTEMD_NOTIFY)
#add_definitions(-DTIME_MEASURE)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GIO_UNIX_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${DLT_INCLUDE_DIRS})
include_directories(${SYSTEMD_INCLUDE_DIRS})
include_directories(src/player)
include_directories(src/pipeline)
include_directories(src/manager)
include_directories(src/interface)
include_directories(src/service)
include_directories(src/interface/external/CommonInterfaces)
include_directories(src/interface/external/TEMILInterfaces)
include_directories(src/interface/external/AudioManager)
include_directories(src/interface/external/MMUS)
include_directories(src/interface/external/VehicleInfoDaemon)
include_directories(src/logger)

add_subdirectory(src/interface/external)
add_subdirectory(xcalltestapp)

set(TEM-XCALL-SERVICE-SRC
	src/xCallMain.cpp
	src/player/unittest_spk_player.cpp
	src/pipeline/unittest_spk_pipeline.cpp
	src/player/unittest_mic_player.cpp
	src/pipeline/unittest_mic_pipeline.cpp
	src/player/unittest_rtpVoice_player.cpp
	src/pipeline/unittest_rtpVoice_pipeline.cpp
	src/service/xcall_service_server.cpp
	src/service/xcall_service_client.cpp
	src/manager/xcall_manager.cpp
	src/manager/network_link_monitor.cpp
	src/player/voicecall_player.cpp
	src/pipeline/voicecall_pipeline.cpp
	src/interface/dbus_xcall_interface.c
	src/logger/xcallservice_logger.cpp
	)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O0 -D__FILENAME__='\"$(notdir $(abspath $<))\"'")

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -Wall -g -O0")


add_executable(${TEM-XCALL-SERVICE-BIN} ${TEM-XCALL-SERVICE-SRC})
target_link_libraries(${TEM-XCALL-SERVICE-BIN}
    ${TEM-XCALL-SERVICE-LIBS})

install(TARGETS ${TEM-XCALL-SERVICE-BIN} DESTINATION /app/bin)

# install a D-Bus configuration
install(FILES resource/dbus-1/system.d/com.lge.xcallservice.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES resource/systemd/system/temxcallservice.service DESTINATION /etc/systemd/system/)

#install(TARGETS ${TEM-XCALL-SERVICE-BIN}
#    DESTINATION ${WEBOS_INSTALL_SBINDIR})
#install(TARGETS temil_test_daemon RUNTIME  DESTINATION /app/bin)
# install(FILES ${PLAYER_ENGINE_SOURCE_DIR}/files/conf/playerengine.conf
#        DESTINATION /app/bin COMPONENT config)
#install(TARGETS ${STARFISH-MEDIA-PIPELINE_BIN}
#    DESTINATION ${WEBOS_INSTALL_SBINDIR})
