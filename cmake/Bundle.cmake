set(CPACK_GENERATOR)
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS qgis)

if(WIN32 AND NOT UNIX)
  set (CREATE_NSIS FALSE CACHE BOOL "Create an installer using NSIS")
endif()
set (CREATE_ZIP FALSE CACHE BOOL "Create a ZIP package")

# Do not warn about runtime libs when building using VS Express
if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
endif()

if(QGIS_INSTALL_SYS_LIBS)
  include(InstallRequiredSystemLibraries)
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QGIS")
set(CPACK_PACKAGE_VENDOR "Open Source Geospatial Foundation")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "QGIS ${COMPLETE_VERSION}")
set(CPACK_PACKAGE_EXECUTABLES "qgis" "QGIS")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")

if(CREATE_NSIS)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/win_build\\\\sidebar.bmp")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\qgis.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} QGIS")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\qgis.org")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\qgis.org")
  set(CPACK_NSIS_CONTACT "info@qgis.org")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif()

if(CREATE_ZIP)
  list(APPEND CPACK_GENERATOR "ZIP")
endif()

include(CPack)
