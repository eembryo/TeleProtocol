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
 # Date : 21/01/2016
 #
 # <IHU TEM-IL> Detailed note is as follows.
 #
 #
 #


set(SRCS
    CommonInterfaces/Common.c
    TEMILInterfaces/CommonAll.c
    TEMILInterfaces/CommonPhone.c
    TEMILInterfaces/Connectivity.c
    TEMILInterfaces/Telematics.c
    TEMILInterfaces/objectManager.c
    AudioManager/dbus_audio_interface.c
    MMUS/dbus_mmus_interface.c
    VehicleInfoDaemon/dbus_carconfigProvider_interface.c
    )

add_library(${TEM-XCALL-EXTERNAL-IF-LIB} SHARED ${SRCS})
target_link_libraries(${TEM-XCALL-EXTERNAL-IF-LIB} ${TEM-XCALL-SERVICE-LIBS})
install(TARGETS ${TEM-XCALL-EXTERNAL-IF-LIB} LIBRARY DESTINATION /app/lib)