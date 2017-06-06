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


set(LGECNR_SRCS
    gstlgecnr.cpp
    gstlgecnrprobe.c
   )

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -O0")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -g -O0")

add_library(lgecnr SHARED ${LGECNR_SRCS})

target_link_libraries(lgecnr ${LIBRARIES} ${ECNR_LIBS})

install(TARGETS lgecnr DESTINATION /app/lib/gst-plugins)
