#############################################################
# locate native libs

SET(NATIVE_LINK_LIBS)

IF(APPLE)
  SET(APPLE_LIB_LIST ApplicationServices CoreFoundation IOKit AppKit)
  FOREACH(_lib ${APPLE_LIB_LIST})
    STRING(TOUPPER ${_lib} _lib_var)
    # prefer /System/Library/Frameworks, in case CMAKE_FIND_FRAMEWORK=LAST, etc.
    FIND_LIBRARY(APPLE_${_lib_var}_LIBRARY
      NAMES ${_lib}
      PATHS /System/Library/Frameworks
      NO_DEFAULT_PATH
    )
    # if not found, drop back to standard find paths
    FIND_LIBRARY(APPLE_${_lib_var}_LIBRARY ${_lib})

    IF(NOT APPLE_${_lib_var}_LIBRARY)
      MESSAGE(FATAL_ERROR "Couldn't find Apple's '${_lib}' framework or library")
    ENDIF(NOT APPLE_${_lib_var}_LIBRARY)

    LIST(APPEND NATIVE_LINK_LIBS "-framework ${_lib}")
  ENDFOREACH(_lib ${APPLE_LIB_LIST})
ENDIF(APPLE)


#############################################################
# sources

SET(QGIS_NATIVE_SRCS
  qgsnative.cpp
)

IF(APPLE)
  SET(QGIS_APP_OBJC_SRCS
    mac/cocoainitializer.mm
    mac/qgsmacnative.mm
  )

  SET_SOURCE_FILES_PROPERTIES(${QGIS_APP_OBJC_SRCS} PROPERTIES COMPILE_FLAGS "-x objective-c++")

  SET(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    ${QGIS_APP_OBJC_SRCS}
  )
ENDIF(APPLE)

IF(WIN32)
  SET(QGIS_APP_WIN32_SRCS
    win/qgswinnative.cpp
  )
  SET(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    ${QGIS_APP_WIN32_SRCS}
  )
ENDIF(WIN32)

IF(UNIX AND NOT APPLE)
  SET(QGIS_APP_LINUX_SRCS
    linux/qgslinuxnative.cpp
  )
  SET(QGIS_NATIVE_SRCS ${QGIS_NATIVE_SRCS}
    ${QGIS_APP_LINUX_SRCS}
  )
ENDIF(UNIX)

SET(QGIS_NATIVE_HDRS
  qgsnative.h
)

# install headers

IF(APPLE)
  SET (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
    mac/qgsmacnative.h
    mac/cocoainitializer.h
  )
ENDIF(APPLE)

IF(WIN32)
  SET (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
     win/qgswinnative.h
  )
ENDIF(WIN32)

IF(UNIX)
  SET (QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS}
     linux/qgslinuxnative.h
  )
ENDIF(UNIX)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################
# qgis_native library

ADD_LIBRARY(qgis_native SHARED ${QGIS_NATIVE_SRCS} ${QGIS_NATIVE_HDRS})
SET_PROPERTY(TARGET qgis_native PROPERTY POSITION_INDEPENDENT_CODE ON)

GENERATE_EXPORT_HEADER(
   qgis_native
   BASE_NAME NATIVE
   EXPORT_FILE_NAME qgis_native.h
)

SET(QGIS_NATIVE_HDRS ${QGIS_NATIVE_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_native.h)

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_NATIVE_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_native PROPERTIES
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
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF(NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_native PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
  )
ENDIF(NOT ANDROID)

TARGET_LINK_LIBRARIES(qgis_native
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    "${NATIVE_LINK_LIBS}"
    )

IF (UNIX AND NOT APPLE)
  TARGET_LINK_LIBRARIES(qgis_native Qt5DBus)
ENDIF (UNIX)

IF (WIN32)
  FIND_PACKAGE(Qt5WinExtras)

  TARGET_LINK_LIBRARIES(qgis_native shell32)
  TARGET_LINK_LIBRARIES(qgis_native ${Qt5Widget_LIBRARIES} ${QT_QTMAIN_LIBRARY})
  TARGET_LINK_LIBRARIES(qgis_native Qt5::WinExtras)
ENDIF (WIN32)
# install

INSTALL(TARGETS qgis_native
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR}
)

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_native FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_native.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_native\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_native.framework/qgis_native\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
