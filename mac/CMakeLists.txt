# mac bundling must happen at end, so all binaries installed
# and install_names can be adjusted
IF (APPLE)
  # for included scripts that set policies (ie OS X bundling)
  INSTALL (CODE "cmake_policy(SET CMP0011 NEW)")
  CONFIGURE_FILE (cmake/0vars.cmake.in 0vars.cmake @ONLY)
  CONFIGURE_FILE (cmake/0qgis.cmake.in 0qgis.cmake @ONLY)
  INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/0qgis.cmake)
  IF (QGIS_MACAPP_BUNDLE GREATER 0)
    # start with Qt
    CONFIGURE_FILE (cmake/1osg.cmake.in 1osg.cmake @ONLY)
    INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/1osg.cmake)
    CONFIGURE_FILE (cmake/1qt.cmake.in 1qt.cmake @ONLY)
    INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/1qt.cmake)
    IF (QGIS_MACAPP_BUNDLE GREATER 1)
      # next - libs
      CONFIGURE_FILE (cmake/2lib.cmake.in 2lib.cmake @ONLY)
      INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/2lib.cmake)
      IF (QGIS_MACAPP_BUNDLE GREATER 2)
        # last - frameworks
        CONFIGURE_FILE (cmake/3fw.cmake.in 3fw.cmake @ONLY)
        INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/3fw.cmake)
      ENDIF (QGIS_MACAPP_BUNDLE GREATER 2)
    ENDIF (QGIS_MACAPP_BUNDLE GREATER 1)
  ENDIF (QGIS_MACAPP_BUNDLE GREATER 0)
  # user bundling
  IF (QGIS_MACAPP_BUNDLE_USER)
    CONFIGURE_FILE (${QGIS_MACAPP_BUNDLE_USER} 4user.cmake @ONLY)
    INSTALL (SCRIPT ${CMAKE_BINARY_DIR}/mac/4user.cmake)
  ENDIF (QGIS_MACAPP_BUNDLE_USER)
  # tickle app bundle
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND touch \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../../.\")")
ENDIF (APPLE)
