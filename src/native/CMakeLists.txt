#############################################################
# locate native libs

if (IOS)
  # No native library for IOS
  return()
endif()


if(UNIX AND NOT APPLE AND NOT ANDROID)
  find_package(${QT_VERSION_BASE} COMPONENTS DBus REQUIRED)
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

install(FILES ${QGIS_NATIVE_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})

#generate unversioned libs for android
if(NOT ANDROID)
  set_target_properties(qgis_native PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
  )
endif()

target_link_libraries(qgis_native ${QT_VERSION_BASE}::Core ${QT_VERSION_BASE}::Gui)

if (UNIX AND NOT APPLE AND NOT ANDROID)
  target_link_libraries(qgis_native ${QT_VERSION_BASE}::DBus)
endif()

if (MSVC AND NOT BUILD_WITH_QT6)
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
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR}
)
