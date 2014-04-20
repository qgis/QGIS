FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/gdal/pyogr ${PY_FILES})