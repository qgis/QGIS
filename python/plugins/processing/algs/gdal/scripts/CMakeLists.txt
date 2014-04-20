FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./gdal/algs/scripts ${PY_FILES})
