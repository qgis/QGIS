file(GLOB PY_FILES *.py)

add_subdirectory(gdal)

PLUGIN_INSTALL(processing algs ${PY_FILES})
