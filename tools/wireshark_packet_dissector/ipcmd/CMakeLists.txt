# CMakeLists.txt
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

set(A2W_OUTPUT_DIR -O ${CMAKE_CURRENT_SOURCE_DIR})

set( PROTOCOL_NAME ipcmd )

set( PROTO_OPT )

set( EXT_ASN_FILE_LIST
)

set( ASN_FILE_LIST
	${PROTOCOL_NAME}.asn
)

set( EXTRA_DIST
	${ASN_FILE_LIST}
	packet-${PROTOCOL_NAME}-template.c
	packet-${PROTOCOL_NAME}-template.h
	${PROTOCOL_NAME}.cnf
)

set( SRC_FILES
	${EXTRA_DIST}
	${EXT_ASN_FILE_LIST}
)

set( A2W_FLAGS -d t )

ASN2WRS()


include(WiresharkPlugin)

# This info is from moduleinfo.nmake
set_module_info(ipcmd 0 1 0 0)

set(DISSECTOR_SRC
        packet-ipcmd.c
)

set(DISSECTOR_SUPPORT_SRC
)

set(PLUGIN_FILES
        plugin.c
        ${DISSECTOR_SRC}
        ${DISSECTOR_SUPPORT_SRC}
)

set(CLEAN_FILES
        ${PLUGIN_FILES}
)

if (WERROR)
        set_source_files_properties(
                ${CLEAN_FILES}
                PROPERTIES
                COMPILE_FLAGS -Werror
        )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

register_dissector_files(plugin.c
        plugin
        ${DISSECTOR_SRC}
        ${DISSECTOR_SUPPORT_SRC}
)

add_plugin_library(ipcmd)

install(TARGETS ipcmd
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION}
)
