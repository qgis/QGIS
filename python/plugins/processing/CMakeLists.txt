set (PROCESSING_PLUGIN_DIR ${QGIS_DATA_DIR}/python/plugins/processing)

add_subdirectory(algs)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(images)
add_subdirectory(modeler)
add_subdirectory(script)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(ui)

file(GLOB OTHER_FILES metadata.txt)
file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing . ${PY_FILES} ${OTHER_FILES})
