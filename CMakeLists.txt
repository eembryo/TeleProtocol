 #
 # @@@ LICENSE
 # Copyright (C) 2016-2017, LG Electronics, All Right Reserved.
 # No part of this source code may be communicated, distributed, reproduced
 # or transmitted in any form or by any means, electronic or mechanical or
 # otherwise, for any purpose, without the prior written permission of
 # LG Electronics.
 # LICENSE @@@
 #
 # Design/Author : hyobeom1.lee@lge.com, dhilipkumar.raman@lge.com
 # Date : 2/17/2017
 # <IHU XCallService Ecnr plugin component>
 #
 #
 #

cmake_minimum_required(VERSION 2.8.1)

project(GST_PLUGINS C CXX)

include(FindPkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GSTAUDIO REQUIRED gstreamer-audio-1.0)
pkg_check_modules(GSTBASE REQUIRED gstreamer-base-1.0)
pkg_check_modules(GSTPLUGINBASE REQUIRED gstreamer-plugins-base-1.0)
pkg_check_modules(TSE REQUIRED tse)

include_directories(include)
include_directories(
                    ${GLIB2_INCLUDE_DIRS}
                    ${GIO_INCLUDE_DIRS}
                    ${GIO_UNIX_INCLUDE_DIRS}
                    ${GST_INCLUDE_DIRS}
                    ${GSTBASE_INCLUDE_DIRS}
                    ${GSTAUDIO_INCLUDE_DIRS}
                    ${TSE_INCLUDE_DIRS}
                    )

add_definitions(-DECNR_ENABLE)

set(LIBRARIES
    ${GLIB2_LDFLAGS}
    ${GIO_LDFLAGS}
    ${GIO_UNIX_LDFLAGS}
    ${GST_LDFLAGS}
    ${GSTBASE_LDFLAGS}
    ${GSTAUDIO_LDFLAGS}
    rt
    pthread
    m
    )

set (ECNR_LIBS ${TSE_LDFLAGS})

add_subdirectory(src)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/geelyconfig/config/ DESTINATION /etc/ecnr/config/)

