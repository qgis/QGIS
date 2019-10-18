############################################################
# sources
SET(QGIS_QUICK_GUI_MOC_HDRS
  attributes/qgsquickattributeformmodel.h
  attributes/qgsquickattributeformmodelbase.h
  attributes/qgsquickattributemodel.h
  attributes/qgsquicksubmodel.h

  qgsquickfeaturelayerpair.h
  qgsquickcoordinatetransformer.h
  qgsquickfeaturehighlight.h
  qgsquickidentifykit.h
  qgsquickmapcanvasmap.h
  qgsquickmapsettings.h
  qgsquickmaptransform.h
  qgsquickmessagelogmodel.h
  qgsquickpositionkit.h
  qgsquickscalebarkit.h
  qgsquicksimulatedpositionsource.h
  qgsquickutils.h
)

SET(QGIS_QUICK_GUI_HDRS
  qgsquickhighlightsgnode.h
)

SET(QGIS_QUICK_GUI_SRC
  attributes/qgsquickattributeformmodel.cpp
  attributes/qgsquickattributeformmodelbase.cpp
  attributes/qgsquickattributemodel.cpp
  attributes/qgsquicksubmodel.cpp

  qgsquickfeaturelayerpair.cpp
  qgsquickcoordinatetransformer.cpp
  qgsquickfeaturehighlight.cpp
  qgsquickhighlightsgnode.cpp
  qgsquickidentifykit.cpp
  qgsquickmapcanvasmap.cpp
  qgsquickmapsettings.cpp
  qgsquickmaptransform.cpp
  qgsquickmessagelogmodel.cpp
  qgsquickpositionkit.cpp
  qgsquickscalebarkit.cpp
  qgsquicksimulatedpositionsource.cpp
  qgsquickutils.cpp
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/attributes
  ${CMAKE_CURRENT_BINARY_DIR}

  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
)

INCLUDE_DIRECTORIES(SYSTEM
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

ADD_DEFINITIONS(-DCORE_EXPORT=)


SET(QGIS_QUICK_GUI_IMAGE_RCCS ./images/images.qrc)
QT5_ADD_RESOURCES(QGIS_QUICK_GUI_IMAGE_RCC_SRCS ${QGIS_QUICK_GUI_IMAGE_RCCS})

############################################################
# qgis_quick shared library
QT5_WRAP_CPP(QGIS_QUICK_GUI_MOC_SRCS ${QGIS_QUICK_GUI_MOC_HDRS})
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_QUICK_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_QUICK_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

ADD_LIBRARY(qgis_quick ${LIBRARY_TYPE}
    ${QGIS_QUICK_GUI_IMAGE_RCC_SRCS}
    ${QGIS_QUICK_GUI_SRC}
    ${QGIS_QUICK_GUI_MOC_HDRS}
    ${QGIS_QUICK_GUI_MOC_SRCS}
    ${QGIS_QUICK_GUI_HDRS})
TARGET_LINK_LIBRARIES(qgis_quick Qt5::Quick Qt5::Qml Qt5::Xml Qt5::Concurrent Qt5::Positioning qgis_core)
IF(CMAKE_SYSTEM_NAME STREQUAL "Android")
  TARGET_LINK_LIBRARIES(qgis_quick Qt5::AndroidExtras)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Android")
TARGET_COMPILE_DEFINITIONS(qgis_quick PRIVATE "-DQT_NO_FOREACH")

GENERATE_EXPORT_HEADER(
  qgis_quick
  BASE_NAME QUICK
  EXPORT_FILE_NAME qgis_quick.h
)

# Installation
INSTALL(TARGETS qgis_quick
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

IF(NOT APPLE)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/qgis_quick.h ${QGIS_QUICK_GUI_HDRS} ${QGIS_QUICK_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_quick PROPERTIES
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
  ADD_CUSTOM_COMMAND(TARGET qgis_quick
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_quick.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_core.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_quick.h
  )
ENDIF(NOT APPLE)

############################################################
# qgis_quick_plugin module (QML) library
ADD_SUBDIRECTORY(plugin)

