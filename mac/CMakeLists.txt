# mac bundling must happen at end, so all binaries installed
# and install_names can be adjusted
if (APPLE AND QGIS_MACAPP_BUNDLE GREATER -1)
  # for included scripts that set policies (ie OS X bundling)
  install (CODE "cmake_policy(SET CMP0011 NEW)")
  configure_file (cmake/0vars.cmake.in 0vars.cmake @ONLY)
  configure_file (cmake/0qgis.cmake.in 0qgis.cmake @ONLY)
  install (SCRIPT ${CMAKE_BINARY_DIR}/mac/0qgis.cmake)
  if (QGIS_MACAPP_BUNDLE GREATER 0)
    # start with Qt
    configure_file (cmake/1qt.cmake.in 1qt.cmake @ONLY)
    install (SCRIPT ${CMAKE_BINARY_DIR}/mac/1qt.cmake)
    if (QGIS_MACAPP_BUNDLE GREATER 1)
      # next - libs
      configure_file (cmake/2lib.cmake.in 2lib.cmake @ONLY)
      install (SCRIPT ${CMAKE_BINARY_DIR}/mac/2lib.cmake)
      if (QGIS_MACAPP_BUNDLE GREATER 2)
        # last - frameworks
        configure_file (cmake/3fw.cmake.in 3fw.cmake @ONLY)
        install (SCRIPT ${CMAKE_BINARY_DIR}/mac/3fw.cmake)
      endif()
    endif()
  endif()
  # user bundling
  if (QGIS_MACAPP_BUNDLE_USER)
    configure_file (${QGIS_MACAPP_BUNDLE_USER} 4user.cmake @ONLY)
    install (SCRIPT ${CMAKE_BINARY_DIR}/mac/4user.cmake)
  endif()
  # tickle app bundle
  install (CODE "execute_process(COMMAND touch \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../../.\")")
endif()
