############################################################
# sources

SET(QGIS_QUICK_PLUGIN_MOC_HDRS
  qgsquickplugin.h
)

SET(QGIS_QUICK_PLUGIN_SRC
  qgsquickplugin.cpp
)

SET(QGIS_QUICK_PLUGIN_RESOURCES
  editor/qgsquickcheckbox.qml
  editor/qgsquickdatetime.qml
  editor/qgsquickexternalresource.qml
  editor/qgsquicktextedit.qml
  editor/qgsquickvaluemap.qml
  qgsquickfeatureform.qml
  qgsquickfeatureformstyling.qml
  qgsquickmapcanvas.qml
  qgsquickmessagelog.qml
  qgsquickphotopanel.qml
  qgsquickpositionmarker.qml
  qgsquickscalebar.qml
  qmldir
)


INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
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
  ${CMAKE_SOURCE_DIR}/src/quickgui

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/quickgui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${LIBZIP_INCLUDE_DIRS}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

ADD_DEFINITIONS(-DCORE_EXPORT=)

############################################################
# qgis_quick_plugin module (QML) library

QT5_WRAP_CPP(QGIS_QUICK_PLUGIN_MOC_SRCS ${QGIS_QUICK_PLUGIN_MOC_HDRS})
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_QUICK_PLUGIN_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_QUICK_PLUGIN_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

SET(QGIS_QUICK_PLUGIN_RUNTIME_DIR ${QGIS_OUTPUT_DIRECTORY}/${QGIS_QML_SUBDIR}/QgsQuick)

ADD_LIBRARY(qgis_quick_plugin MODULE
    ${QGIS_QUICK_PLUGIN_SRC}
    ${QGIS_QUICK_PLUGIN_MOC_HDRS}
    ${QGIS_QUICK_PLUGIN_MOC_SRCS}
    ${QGIS_QUICK_PLUGIN_RESOURCES}
)
TARGET_LINK_LIBRARIES(qgis_quick_plugin qgis_quick)
SET_TARGET_PROPERTIES(qgis_quick_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QGIS_QUICK_PLUGIN_RUNTIME_DIR})
TARGET_COMPILE_DEFINITIONS(qgis_quick_plugin PRIVATE "-DQUICK_EXPORT=" "-DQT_NO_FOREACH")

# Copy qml files to output directory, we need qml files in the same directory as the plugin shared library
FOREACH(qmlfile ${QGIS_QUICK_PLUGIN_RESOURCES})
  ADD_CUSTOM_COMMAND(TARGET qgis_quick_plugin
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${qmlfile} ${QGIS_QUICK_PLUGIN_RUNTIME_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${qmlfile}
  )
ENDFOREACH(qmlfile)

IF(QMLPLUGINDUMP_FOUND)
  # To create typeinfo file, no qml files must be in the directory, otherwise
  # bunch of "QObject: Cannot create children for a parent that is in a different thread." errors
  # appear and typeinfo file is not generated
  SET(QGIS_QUICK_TYPEINFO_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/QgsQuick)

  # Extract QML Types Info from our QML plugin. This is useful for development with Qt Creator as it allows
  # Qt Creator understand also the C++ classes registered in the plugin and thus available in QML code
  # If the qmlplugindumpcommand fails, add -v argument to see detailed output for debugging
  SET(QGIS_QUICK_PLUGIN_TYPEINFO ${QGIS_QUICK_PLUGIN_RUNTIME_DIR}/qgsquick.qmltypes)
  SET(QGIS_QUICK_VERSION 0.1)
  ADD_CUSTOM_COMMAND(
      TARGET qgis_quick_plugin
      COMMAND ${CMAKE_COMMAND} -E make_directory ${QGIS_QUICK_TYPEINFO_GENERATE_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QGIS_QUICK_TYPEINFO_GENERATE_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qgis_quick_plugin> ${QGIS_QUICK_TYPEINFO_GENERATE_DIR}
      COMMAND ${QMLPLUGINDUMP_EXECUTABLE}
      ARGS QgsQuick ${QGIS_QUICK_VERSION} . -noinstantiate --output ${QGIS_QUICK_PLUGIN_TYPEINFO}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generating qgsquick.qmltypes with qmlplugindump"
      POST_BUILD
  )
ENDIF()

# Installation
SET(QUICK_PLUGIN_INSTALL_DIR ${QGIS_QML_DIR}/QgsQuick)
INSTALL(TARGETS qgis_quick_plugin
  RUNTIME DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
  LIBRARY DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
)
INSTALL(FILES ${QGIS_QUICK_PLUGIN_RESOURCES} ${QGIS_QUICK_PLUGIN_TYPEINFO}
  DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
)
