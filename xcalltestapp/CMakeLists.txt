#
 # @@@ LICENSE
 # Copyright (C) 2016-2017, LG Electronics, All Right Reserved.
 # No part of this source code may be communicated, distributed, reproduced
 # or transmitted in any form or by any means, electronic or mechanical or
 # otherwise, for any purpose, without the prior written permission of
 # LG Electronics.
 # LICENSE @@@
 #
 # Design/Author : manisha.manisha@lge.com, dhilipkumar.raman@lge.com
 # Date : Oct. 24, 2016
 #
 # <IHU Xcall Test Application> Detailed note is as follows.
 #
 #

cmake_minimum_required(VERSION 2.8.1)

project(XCALL_TESTAPP CXX C)

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)

include_directories(
                    ${GLIB2_INCLUDE_DIRS}
                    ${GIO_INCLUDE_DIRS}
                    ${GIO_UNIX_INCLUDE_DIRS}
                    )


set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O0")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -g -O0")

add_definitions(-DCONNECT_DBUS_SYSTEM_BUS)

set(XCALL_TESTAPP_BIN xcalltestapp)

set(XCALL_TESTAPP_LIBS
    ${GLIB2_LDFLAGS}
    ${GIO_LDFLAGS}
    ${GIO_UNIX_LDFLAGS}
    )

add_subdirectory(src)

