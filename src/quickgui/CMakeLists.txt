############################################################
# sources
set(QGIS_QUICK_GUI_MOC_HDRS
  qgsquickcoordinatetransformer.h
  qgsquickmapcanvasmap.h
  qgsquickmapsettings.h
  qgsquickmaptransform.h
)

set(QGIS_QUICK_GUI_SRC
  qgsquickcoordinatetransformer.cpp
  qgsquickmapcanvasmap.cpp
  qgsquickmapsettings.cpp
  qgsquickmaptransform.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${LIBZIP_INCLUDE_DIRS}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

############################################################
# qgis_quick shared library
if (WITH_QT6)
  QT6_WRAP_CPP(QGIS_QUICK_GUI_MOC_SRCS ${QGIS_QUICK_GUI_MOC_HDRS})
else()
  QT5_WRAP_CPP(QGIS_QUICK_GUI_MOC_SRCS ${QGIS_QUICK_GUI_MOC_HDRS})
endif()

if(MSVC)
  set_source_files_properties(${QGIS_QUICK_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
else()
  set_source_files_properties(${QGIS_QUICK_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
endif()

add_library(qgis_quick ${LIBRARY_TYPE}
    ${QGIS_QUICK_GUI_SRC}
    ${QGIS_QUICK_GUI_MOC_HDRS}
    ${QGIS_QUICK_GUI_MOC_SRCS}
    ${QGIS_QUICK_GUI_HDRS})

# require c++17
target_compile_features(qgis_quick PRIVATE cxx_std_17)

if (WITH_QT6)
  target_link_libraries(qgis_quick Qt6::Quick Qt6::Qml Qt6::Xml Qt6::Concurrent Qt6::Positioning qgis_core)
else()
  target_link_libraries(qgis_quick Qt5::Quick Qt5::Qml Qt5::Xml Qt5::Concurrent Qt5::Positioning qgis_core)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  if (WITH_QT6)
    target_link_libraries(qgis_quick Qt6::AndroidExtras)
  else()
    target_link_libraries(qgis_quick Qt5::AndroidExtras)
  endif()
endif()
target_compile_definitions(qgis_quick PRIVATE "-DQT_NO_FOREACH")

GENERATE_EXPORT_HEADER(
  qgis_quick
  BASE_NAME QUICK
  EXPORT_FILE_NAME qgis_quick.h
)

# Installation
if(NOT APPLE OR NOT QGIS_MACAPP_FRAMEWORK)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qgis_quick.h ${QGIS_QUICK_GUI_HDRS} ${QGIS_QUICK_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
else()
  set_target_properties(qgis_quick PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_quick
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_QUICK_GUI_HDRS};${QGIS_QUICK_GUI_MOC_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
  # generated export header does not get copied with PUBLIC_HEADER files
  add_custom_command(TARGET qgis_quick
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_quick.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_quick.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_quick.h
  )
endif()

install(TARGETS qgis_quick
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

############################################################
# qgis_quick_plugin module (QML) library
add_subdirectory(plugin)

