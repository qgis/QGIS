file(GLOB PY_FILES *.py)

add_subdirectory(help)
add_subdirectory(gdal)
add_subdirectory(grass7)
add_subdirectory(otb)
add_subdirectory(saga)
add_subdirectory(qgis)

PLUGIN_INSTALL(processing algs ${PY_FILES})
