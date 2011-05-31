
#############################################################
# create Info.plist

SET(COMPLETE_VERSION_WITH_RELEASE_NAME \"${COMPLETE_VERSION}-${RELEASE_NAME}\")

IF (NOT EXISTS GIT_MARKER)
  SET (GIT_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file
ENDIF (NOT EXISTS GIT_MARKER)

ADD_CUSTOM_TARGET(Info.plist ALL
  DEPENDS ${GIT_MARKER}
  COMMAND ${CMAKE_COMMAND}
    -D CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -D SOURCE_DIR=${CMAKE_SOURCE_DIR}
    -D VERSION=${COMPLETE_VERSION_WITH_RELEASE_NAME}
    -D VERSION_NUM=${COMPLETE_VERSION}
    -D APP_NAME=${QGIS_APP_NAME}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.cmake)

#############################################################
# process subdirs

ADD_SUBDIRECTORY(Resources)

#############################################################
# install

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
               ${CMAKE_CURRENT_SOURCE_DIR}/PkgInfo
         DESTINATION ${CMAKE_INSTALL_PREFIX}/..)

# Update bundle modification date so Finder scans latest plist
INSTALL (CODE "EXECUTE_PROCESS(COMMAND touch \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../../.\")")
