#############################################################
# locate native libs

if (IOS)
  # No native library for IOS
  return()
endif()


set(NATIVE_LINK_LIBS)

if(UNIX AND NOT APPLE AND NOT ANDROID)
  find_package(${QT_VERSION_BASE} COMPONENTS DBus REQUIRED)
endif()

if(APPLE)
  set(APPLE_LIB_LIST ApplicationServices CoreFoundation IOKit AppKit)
  foreach(_lib ${APPLE_LIB_LIST})
    string(TOUPPER ${_lib} _lib_var)
    # prefer /System/Library/Frameworks, in case CMAKE_FIND_FRAMEWORK=LAST, etc.
    find_library(APPLE_${_lib_var}_LIBRARY
      NAMES ${_lib}
      PATHS /System/Library/Frameworks
      NO_DEFAULT_PATH
    )
    # if not found, drop back to standard find paths
    find_library(APPLE_${_lib_var}_LIBRARY ${_lib})

    if(NOT APPLE_${_lib_var}_LIBRARY)
      message(FATAL_ERROR "Couldn't find Apple's '${_lib}' framework or library")
    endif()

    list(APPEND NATIVE_LINK_LIBS "-weak_framework ${_lib}")
  endforeach(_lib ${APPLE_LIB_LIST})
endif()


#############################################################
# sources

set(QGIS_NATIVE_SRCS
  qgsnative.cpp
)

set(QGIS_NATIVE_HDRS
  qgsnative.h
)

if(APPLE)
  set(QGIS_APP_OBJC_SRCS
    mac/cocoainitializer.mm
    mac/qgsmacnative.mm
  )

  set_source_files_properties(${QGIS_APP_OBJC_SRCS} PROPERTIES COMPILE_FLAGS "-x objective-c++")

  set(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    ${QGIS_APP_OBJC_SRCS}
  )
  set (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
    mac/qgsmacnative.h
    mac/cocoainitializer.h
  )
endif()

if(MSVC)
  set(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    ../../external/wintoast/src/wintoastlib.cpp
    win/qgswinnative.cpp
  )
  set (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
     win/qgswinnative.h
  )
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
  set(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    linux/qgslinuxnative.cpp
  )
  set (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
     linux/qgslinuxnative.h
  )
endif()

#############################################################
# qgis_native library

add_library(qgis_native ${LIBRARY_TYPE} ${QGIS_NATIVE_SRCS} ${QGIS_NATIVE_HDRS})
set_property(TARGET qgis_native PROPERTY POSITION_INDEPENDENT_CODE ON)

# require c++17
target_compile_features(qgis_native PRIVATE cxx_std_17)

target_include_directories(qgis_native PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(MSVC)
  target_include_directories(qgis_native SYSTEM PUBLIC
      ../../external/wintoast/src
  )
endif()

GENERATE_EXPORT_HEADER(
   qgis_native
   BASE_NAME NATIVE
   EXPORT_FILE_NAME qgis_native.h
)

set(QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_native.h)

if(NOT APPLE OR NOT QGIS_MACAPP_FRAMEWORK)
  install(FILES ${QGIS_NATIVE_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
else()
  set_target_properties(qgis_native PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_native
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_NATIVE_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
endif()

#generate unversioned libs for android
if(NOT ANDROID)
  set_target_properties(qgis_native PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
  )
endif()

if (BUILD_WITH_QT6)
  target_link_libraries(qgis_native ${Qt6Core_LIBRARIES} ${Qt6Gui_LIBRARIES} "${NATIVE_LINK_LIBS}")
else()
  target_link_libraries(qgis_native ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} "${NATIVE_LINK_LIBS}")
endif()

if (UNIX AND NOT APPLE AND NOT ANDROID)
  target_link_libraries(qgis_native ${QT_VERSION_BASE}::DBus)
endif()

if (MSVC)
  find_package(${QT_VERSION_BASE}WinExtras)

  target_link_libraries(qgis_native shell32)
  target_link_libraries(qgis_native ${QT_VERSION_BASE}::Widgets ${QT_QTMAIN_LIBRARY})
  target_link_libraries(qgis_native ${QT_VERSION_BASE}::WinExtras)

endif()
# install

install(TARGETS qgis_native
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR}
)

# Mac dev frameworks

if (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  install(TARGETS qgis_native FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  install(CODE "execute_process(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_native.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_native\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_native.framework/qgis_native\")")
endif()
