############################################################
# sources
set(QGIS_QUICK_GUI_MOC_HDRS
  qgsquickcoordinatetransformer.h
  qgsquickelevationprofilecanvas.h
  qgsquickmapcanvasmap.h
  qgsquickmapsettings.h
  qgsquickmaptoscreen.h
  qgsquickmaptransform.h
  qgsquickutils.h
)

set(QGIS_QUICK_GUI_SRC
  qgsquickcoordinatetransformer.cpp
  qgsquickelevationprofilecanvas.cpp
  qgsquickmapcanvasmap.cpp
  qgsquickmapsettings.cpp
  qgsquickmaptoscreen.cpp
  qgsquickmaptransform.cpp
  qgsquickutils.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(SYSTEM
  ${LIBZIP_INCLUDE_DIRS}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

############################################################
# qgis_quick shared library
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

target_link_libraries(qgis_quick
    ${QT_VERSION_BASE}::Quick
    ${QT_VERSION_BASE}::Qml
    ${QT_VERSION_BASE}::Xml
    ${QT_VERSION_BASE}::Concurrent
    ${QT_VERSION_BASE}::Positioning
    qgis_core)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  target_link_libraries(qgis_quick ${QT_VERSION_BASE}::AndroidExtras)
endif()
target_compile_definitions(qgis_quick PRIVATE "-DQT_NO_FOREACH")

GENERATE_EXPORT_HEADER(
  qgis_quick
  BASE_NAME QUICK
  EXPORT_FILE_NAME qgis_quick.h
)

# Installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qgis_quick.h ${QGIS_QUICK_GUI_HDRS} ${QGIS_QUICK_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})

install(TARGETS qgis_quick
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

############################################################
# qgis_quick_plugin module (QML) library
add_subdirectory(plugin)

