set(CPACK_GENERATOR)
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake" "--verbose"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS "${QGIS_APP_NAME}")

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


if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND QGIS_MAC_BUNDLE)
  # TODO HINT relative to VCPKG_HOST_DIR
  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "../../macdeployqt-standalone/build/bin/" NO_DEFAULT_PATH)

  configure_file("${CMAKE_SOURCE_DIR}/platform/macos/Info.plist.in" "${CMAKE_BINARY_DIR}/platform//macos/Info.plist" @ONLY)
  install(FILES "${CMAKE_BINARY_DIR}/platform//macos/Info.plist" DESTINATION "${APP_CONTENTS_DIR}")

  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
  set(CPACK_DMG_FORMAT "UDBZ")
  list(APPEND CPACK_GENERATOR "External")
  message(STATUS "   + macdeployqt/DMG                      YES ")
  configure_file(${CMAKE_SOURCE_DIR}/platform/macos/CPackMacDeployQt.cmake.in "${CMAKE_BINARY_DIR}/CPackExternal.cmake" @ONLY)
  set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/CPackExternal.cmake")
  set(CPACK_EXTERNAL_ENABLE_STAGING ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

#  install(CODE "
#    execute_process(
#      COMMAND install_name_tool -add_rpath @executable_path/../Frameworks \"${APP_CONTENTS_DIR}/MacOS/QGIS\"
#      WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\"
#      RESULT_VARIABLE result
#    )
#    if(NOT result EQUAL 0)
#      message(FATAL_ERROR \"install_name_tool failed with error code: ${result}\")
#    endif() 
#  ")
endif()
# 
# 
# 
# 
# if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#   set(CPACK_GENERATOR "Bundle")
#   set(CPACK_PACKAGE_FILE_NAME "${QGIS_APP_NAME}")
#   # DragNDrop
#   set(CPACK_DMG_FORMAT "ULFO")
#   # set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/packaging/osx/background.tiff")
#   # Bundle
#   set(CPACK_BUNDLE_NAME "${QGIS_APP_NAME}")
#   configure_file("${CMAKE_SOURCE_DIR}/macos/app.info.plist.in" "${CMAKE_BINARY_DIR}/mac/Info.plist")
#   set(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/mac/Info.plist")
#   set(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/images/icons/mac/qgis.icns")
#   
#   if(WITH_DESKTOP)
#     install(PROGRAMS $<TARGET_FILE:${QGIS_APP_NAME}> DESTINATION .)
#   endif()
#   install(FILES ${CPACK_BUNDLE_PLIST} DESTINATION .)
#   


# set(VCPKG_BASE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

  # install(CODE "
  # include(BundleUtilities)
  # set(BU_CHMOD_BUNDLE_ITEMS ON)
  # set(QT_PLUGIN_IN_BUNDLE \"\")
  # message(WARNING \"Fixing plugins\")
  # foreach(f ${QT_PLUGIN})
  #   message(WARNING \"Fixing ${f}\")
  #   get_filename_component(QT_PLUGIN_ABSOLUTE \"\${CMAKE_INSTALL_PREFIX}/\${f}\" ABSOLUTE)
  #   list(APPEND QT_PLUGIN_IN_BUNDLE \"\${QT_PLUGIN_ABSOLUTE}\")
  # endforeach(f)
  # get_filename_component(BUNDLE_PATH \"\${CMAKE_INSTALL_PREFIX}\" ABSOLUTE)
  # fixup_bundle(\"\${BUNDLE_PATH}/${APP_BUNDLE_DIR}\" \"\" \"${VCPKG_BASE_DIR}/lib;\${BUNDLE_PATH}/${APP_BUNDLE_DIR}/Contents/Frameworks\")
  # ")
# endif()

# set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/_CPack_Packages")

include(CPack)
