
#############################################################
# create Info.plist

ADD_CUSTOM_TARGET(Info.plist ALL
                  VERSION=`grep VERSION ${CMAKE_BINARY_DIR}/qgsconfig.h | head -1 |
                           sed -e "s/#define VERSION //"
                               -e "s/\\\"//g"` &&
                  SVNVERSION=`grep QGSSVNVERSION ${CMAKE_BINARY_DIR}/qgssvnversion.h |
                              sed -e "s/#define QGSSVNVERSION //"
                                  -e "s/\\\"//g"` &&
                  sed -e "s/@VERSION/$$VERSION/"
                      -e "s/@SHORTVERSION/$$VERSION/"
                      -e "s/@SVNVERSION/$$SVNVERSION/"
                      < ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in > Info.plist
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
                          ${CMAKE_BINARY_DIR}/qgsconfig.h)

ADD_DEPENDENCIES(Info.plist svnversion)

#############################################################
# process subdirs

SUBDIRS(Resources)

#############################################################
# install

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
               ${CMAKE_CURRENT_SOURCE_DIR}/PkgInfo
         DESTINATION ${CMAKE_INSTALL_PREFIX}/..)

# Update bundle modification date so Finder scans latest plist
INSTALL (CODE "EXECUTE_PROCESS(COMMAND touch ${CMAKE_INSTALL_PREFIX}/../../.)")
