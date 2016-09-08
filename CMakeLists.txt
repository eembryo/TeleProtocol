 #
 # @@@ LICENSE
 # Copyright (C) 2015-2016, LG Electronics, All Right Reserved.
 # No part of this source code may be communicated, distributed, reproduced
 # or transmitted in any form or by any means, electronic or mechanical or
 # otherwise, for any purpose, without the prior written permission of
 # LG Electronics.
 # LICENSE @@@
 #
 # Design/Author : hyobeom1.lee@lge.com
 # Date : 3/10/2016
 #
 # <Ipcommand Protocol> Detailed note is as follows.
 #
 #
 #

cmake_minimum_required(VERSION 2.8.1)

project(IPCOMMANDPROTOCOL C)

add_definitions(-DDEBUG=DLOG_LEVEL_WARN -DENABLE_LOG_COLOR -DTESTAPP_DEBUG -g -Wall)

set(IPCOMM_LIB ipcmd)

include(FindPkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)

include_directories(include)
include_directories(include_internal)
include_directories(
                    ${GLIB2_INCLUDE_DIRS}
                    ${GIO_INCLUDE_DIRS}
                    ${GIO_UNIX_INCLUDE_DIRS}
                    )

set(LIBRARIES
    ${GLIB2_LDFLAGS}
    ${GIO_LDFLAGS}
    ${GIO_UNIX_LDFLAGS}
    rt
    pthread
    m
    )

add_subdirectory(src)
add_subdirectory(test)

install(DIRECTORY include DESTINATION /app/
  FILES_MATCHING PATTERN "*.h")

install(FILES res/libipcmd.pc DESTINATION /app/lib/pkgconfig/)
