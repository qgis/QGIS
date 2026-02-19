############################################################
# sources

set(QGIS_QUICK_PLUGIN_MOC_HDRS
  qgsquickplugin.h
)

set(QGIS_QUICK_PLUGIN_SRC
  qgsquickplugin.cpp
)

set(QGIS_QUICK_PLUGIN_RESOURCES
  qgsquickmapcanvas.qml
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
  ${LIBZIP_INCLUDE_DIRS}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

############################################################
# qgis_quick_plugin module (QML) library

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
if(QGISPOSTFIX)
  set_target_properties(qgis_quick_plugin PROPERTIES
    RUNTIME_OUTPUT_NAME qgis_quick_plugin${QGISPOSTFIX}
    LIBRARY_OUTPUT_NAME qgis_quick_plugin${QGISPOSTFIX}
  )
endif()

target_link_libraries(qgis_quick_plugin qgis_quick)
set_target_properties(qgis_quick_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QGIS_QUICK_PLUGIN_RUNTIME_DIR})
target_compile_definitions(qgis_quick_plugin PRIVATE "-DQT_NO_FOREACH")

# Copy qml files to output directory, we need qml files in the same directory as the plugin shared library
set(copied_qml_files)
foreach(qmlfile ${QGIS_QUICK_PLUGIN_RESOURCES} ${QGIS_QUICK_PLUGIN_QRC})
  get_filename_component(qmlfile_name ${qmlfile} NAME)
  set(qmlfile_dst ${QGIS_QUICK_PLUGIN_RUNTIME_DIR}/${qmlfile_name})

  add_custom_command(
    OUTPUT ${qmlfile_dst}
    COMMAND ${CMAKE_COMMAND} -E copy ${qmlfile} ${qmlfile_dst}
    DEPENDS ${qmlfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  list(APPEND copied_qml_files ${qmlfile_dst})
endforeach(qmlfile)

add_custom_target(copy_qml_files ALL DEPENDS ${copied_qml_files})
add_dependencies(qgis_quick_plugin copy_qml_files)


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
