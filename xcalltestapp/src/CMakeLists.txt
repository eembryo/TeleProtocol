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

set(SRCS
    main.cpp
    xcalltest.cpp
    )

add_executable(${XCALL_TESTAPP_BIN} ${SRCS})
target_link_libraries(${XCALL_TESTAPP_BIN} ${XCALL_TESTAPP_LIBS})
install(TARGETS ${XCALL_TESTAPP_BIN} RUNTIME DESTINATION /app/bin)
