############################################################
# sources

set(QGIS_QUICK_PLUGIN_MOC_HDRS
  qgsquickplugin.h
)

set(QGIS_QUICK_PLUGIN_SRC
  qgsquickplugin.cpp
)

set(QGIS_QUICK_PLUGIN_RESOURCES
  components/qgsquickicontextitem.qml
  components/qgsquickcheckboxcomponent.qml
  editor/qgsquickeditorwidgetcombobox.qml
  editor/qgsquickcheckbox.qml
  editor/qgsquickdatetime.qml
  editor/qgsquickexternalresource.qml
  editor/qgsquickrange.qml
  editor/qgsquicktextedit.qml
  editor/qgsquickvaluemap.qml
  editor/qgsquickvaluerelation.qml
  qgsquickfeatureform.qml
  qgsquickfeatureformstyling.qml
  qgsquickmapcanvas.qml
  qgsquickmessagelog.qml
  qgsquickphotopanel.qml
  qgsquickpositionmarker.qml
  qgsquickscalebar.qml
)

if (IOS)
  set(QGIS_QUICK_PLUGIN_RESOURCES ${QGIS_QUICK_PLUGIN_RESOURCES} ios/qmldir )
  set(QGIS_QUICK_PLUGIN_QRC ios/qgsquick.qrc)
else()
  set(QGIS_QUICK_PLUGIN_RESOURCES ${QGIS_QUICK_PLUGIN_RESOURCES} qmldir)
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}

  ${CMAKE_SOURCE_DIR}/src/quickgui

  ${CMAKE_BINARY_DIR}/src/quickgui
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
# qgis_quick_plugin module (QML) library

QT5_WRAP_CPP(QGIS_QUICK_PLUGIN_MOC_SRCS ${QGIS_QUICK_PLUGIN_MOC_HDRS})
if(MSVC)
  set_source_files_properties(${QGIS_QUICK_PLUGIN_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
else()
  set_source_files_properties(${QGIS_QUICK_PLUGIN_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
endif()

set(QGIS_QUICK_PLUGIN_RUNTIME_DIR ${QGIS_OUTPUT_DIRECTORY}/${QGIS_QML_SUBDIR}/QgsQuick)

add_library(qgis_quick_plugin ${LIBRARY_TYPE}
    ${QGIS_QUICK_PLUGIN_SRC}
    ${QGIS_QUICK_PLUGIN_MOC_HDRS}
    ${QGIS_QUICK_PLUGIN_MOC_SRCS}
    ${QGIS_QUICK_PLUGIN_RESOURCES}
)

# require c++17
target_compile_features(qgis_quick_plugin PRIVATE cxx_std_17)

target_link_libraries(qgis_quick_plugin qgis_quick)
set_target_properties(qgis_quick_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QGIS_QUICK_PLUGIN_RUNTIME_DIR})
target_compile_definitions(qgis_quick_plugin PRIVATE "-DQT_NO_FOREACH")

# Copy qml files to output directory, we need qml files in the same directory as the plugin shared library
foreach(qmlfile ${QGIS_QUICK_PLUGIN_RESOURCES} ${QGIS_QUICK_PLUGIN_QRC})
  add_custom_command(TARGET qgis_quick_plugin
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${qmlfile} ${QGIS_QUICK_PLUGIN_RUNTIME_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${qmlfile}
  )
endforeach(qmlfile)

if(QMLPLUGINDUMP_FOUND)
  # To create typeinfo file, no qml files must be in the directory, otherwise
  # bunch of "QObject: Cannot create children for a parent that is in a different thread." errors
  # appear and typeinfo file is not generated
  set(QGIS_QUICK_TYPEINFO_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/QgsQuick)

  # Extract QML Types Info from our QML plugin. This is useful for development with Qt Creator as it allows
  # Qt Creator understand also the C++ classes registered in the plugin and thus available in QML code
  # If the qmlplugindumpcommand fails, add -v argument to see detailed output for debugging
  set(QGIS_QUICK_PLUGIN_TYPEINFO ${QGIS_QUICK_PLUGIN_RUNTIME_DIR}/qgsquick.qmltypes)
  set(QGIS_QUICK_VERSION 0.1)
  add_custom_command(
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
endif()

# Installation
set(QUICK_PLUGIN_INSTALL_DIR ${QGIS_QML_DIR}/QgsQuick)
install(TARGETS qgis_quick_plugin
  RUNTIME DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
  LIBRARY DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
  ARCHIVE DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
)
install(FILES ${QGIS_QUICK_PLUGIN_RESOURCES} ${QGIS_QUICK_PLUGIN_TYPEINFO} ${QGIS_QUICK_PLUGIN_QRC}
  DESTINATION ${QUICK_PLUGIN_INSTALL_DIR}
)
