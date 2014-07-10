FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/gdal ${PY_FILES})
