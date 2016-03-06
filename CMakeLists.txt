#
 # @@@ LICENSE
 # Copyright (C) 2015, LG Electronics, All Right Reserved.
 # No part of this source code may be communicated, distributed, reproduced
 # or transmitted in any form or by any means, electronic or mechanical or
 # otherwise, for any purpose, without the prior written permission of
 # LG Electronics.
 # LICENSE @@@
 #
 # Design/Author : jaewoon.kim@lge.com, sandeep.ankolekar@lge.com
 # Date : 12/17/2015
 #
 # <IHU TEM-IL> Detailed note is as follows.
 #
 #
 #

cmake_minimum_required(VERSION 2.8.1)

project(TEMIL_SERVICE C CXX)

add_definitions(-DDEBUG -DENABLE_LOG_COLOR -g)
#add_definitions(-DTARGET_GEELY)

set(TEMIL_IF_LIB temilinterface)
set(TEMIL_ASN1_LIB temilasn1)
set(TEMIL_ASN1_LIB_PATH ${PROJECT_SOURCE_DIR}/ASN1CLib/lib)
set(TEMIL_IPCOMM_LIB temilipcomm)

set(TARGET_GEELY ON)

include(FindPkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)

include_directories(ASN1CLib)
include_directories(utils/logger)
include_directories(IpCommandProtocol/include)
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

add_subdirectory(IpCommandProtocol)
#add_subdirectory(ServiceCore)
#if(TARGET_GEELY)
#    add_subdirectory(ASN1CLib)
#endif(TARGET_GEELY)

# install a systemd configuration
install(DIRECTORY Resource/systemd/system/ DESTINATION /etc/systemd/system
        FILE_PERMISSIONS OWNER_READ
                         GROUP_READ
                         WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                            GROUP_EXECUTE GROUP_READ
                                            WORLD_EXECUTE WORLD_READ)

# TBD
# make a symlink of the systemd configuration
#install(DIRECTORY DESTINATION /etc/systemd/system/basic.target.wants)
#install(CODE "EXECUTE_PROCESS(COMMAND ln -sf FL Fl /etc/systemd/system /etc/systemd/system/basic.target.wants)" )
#add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ln -sf /etc/systemd/system/teminterfacelayer.service /etc/systemd/system/basic.target.wants
#)


# install a D-Bus configuration
install(FILES Resource/dbus-1/system.d/com.lge.temil.conf DESTINATION /etc/dbus-1/system.d/)
