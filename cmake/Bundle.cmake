set(CPACK_GENERATOR)
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake" "--verbose"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS qgis)

if(WIN32 AND NOT UNIX)
  set (CREATE_NSIS FALSE CACHE BOOL "Create an installer using NSIS")
endif()
set (CREATE_ZIP FALSE CACHE BOOL "Create a ZIP package")
set (STRATA_WINDOWS_CODE_SIGN FALSE CACHE BOOL "Sign Windows packages using Azure Artifact Signing")

# Do not warn about runtime libs when building using VS Express
if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
endif()

if(QGIS_INSTALL_SYS_LIBS)
  include(InstallRequiredSystemLibraries)
endif()

set(CPACK_PACKAGE_NAME "Strata")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Strata - The AI-native GIS")
set(CPACK_PACKAGE_VENDOR "Francesco Mazzi")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Strata ${COMPLETE_VERSION}")
set(CPACK_PACKAGE_EXECUTABLES "${QGIS_APP_NAME}" "Strata")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")

if(CREATE_NSIS)
  list(APPEND CPACK_GENERATOR "NSIS")
  # The win_build/sidebar.bmp referenced by upstream QGIS does not exist in
  # this fork — leave CPACK_PACKAGE_ICON unset so NSIS uses its default header.
  set(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\${QGIS_APP_NAME}.exe")
  set(CPACK_NSIS_DISPLAY_NAME "Strata ${CPACK_PACKAGE_VERSION}")
  set(CPACK_NSIS_PACKAGE_NAME "Strata")
  set(CPACK_NSIS_HELP_LINK "https:\\\\\\\\github.com\\\\francemazzi\\\\strata")
  set(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\github.com\\\\francemazzi\\\\strata")
  set(CPACK_NSIS_CONTACT "francemazzi@gmail.com")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif()

if(CREATE_ZIP)
  list(APPEND CPACK_GENERATOR "ZIP")
endif()

if(WIN32 AND STRATA_WINDOWS_CODE_SIGN)
  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/StrataWindowsCodeSignPreBuild.cmake.in"
    "${CMAKE_BINARY_DIR}/StrataWindowsCodeSignPreBuild.cmake"
    @ONLY
  )
  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/StrataWindowsCodeSignPostBuild.cmake.in"
    "${CMAKE_BINARY_DIR}/StrataWindowsCodeSignPostBuild.cmake"
    @ONLY
  )
  list(APPEND CPACK_PRE_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/StrataWindowsCodeSignPreBuild.cmake")
  list(APPEND CPACK_POST_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/StrataWindowsCodeSignPostBuild.cmake")
  message(STATUS "   + Windows code signing              YES")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND QGIS_MAC_BUNDLE)
  set(CREATE_DMG FALSE CACHE BOOL "Create a dmg bundle")
  set(PYMACDEPLOYQT_EXECUTABLE "${CMAKE_SOURCE_DIR}/platform/macos/pymacdeployqt.py")

  configure_file("${CMAKE_SOURCE_DIR}/platform/macos/Info.plist.in" "${CMAKE_BINARY_DIR}/platform//macos/Info.plist" @ONLY)
  install(FILES "${CMAKE_BINARY_DIR}/platform/macos/Info.plist" DESTINATION "${APP_CONTENTS_DIR}")
  install(FILES "${CMAKE_SOURCE_DIR}/images/icons/mac/strata.icns" DESTINATION "${APP_RESOURCES_DIR}")

  set(CPACK_DMG_VOLUME_NAME "Strata")
  set(CPACK_DMG_FORMAT "UDBZ")
  list(APPEND CPACK_GENERATOR "External")
  message(STATUS "   + macdeployqt/DMG                      YES ")
  configure_file(${CMAKE_SOURCE_DIR}/platform/macos/CPackMacDeployQt.cmake.in "${CMAKE_BINARY_DIR}/CPackExternal.cmake" @ONLY)
  set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/CPackExternal.cmake")
  set(CPACK_EXTERNAL_ENABLE_STAGING ON)
  set(CPACK_PACKAGING_INSTALL_PREFIX "/${QGIS_APP_NAME}.app")
endif()

include(CPack)
