
#############################################################
# create Info.plist

EXECUTE_PROCESS(COMMAND grep VERSION ${CMAKE_SOURCE_DIR}/qgsconfig.h
                OUTPUT_VARIABLE VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
STRING(REGEX REPLACE "#define VERSION \"(.*)\"" "\\1" VERSION ${VERSION})

EXECUTE_PROCESS(COMMAND grep SVNVERSION ${CMAKE_SOURCE_DIR}/qgssvnversion.h
                OUTPUT_VARIABLE SVNVERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
STRING(REGEX REPLACE "#define QGSSVNVERSION \"(.*)\"" "\\1" SVNVERSION ${SVNVERSION})

SET(SHORTVERSION ${VERSION})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
               ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

#############################################################
# process subdirs

SUBDIRS(Resources)

#############################################################
# install

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
         DESTINATION ${CMAKE_INSTALL_PREFIX}/..)

# Update bundle modification date so Finder scans latest plist
INSTALL (CODE "EXECUTE_PROCESS(COMMAND touch ${CMAKE_INSTALL_PREFIX}/../../.)")
