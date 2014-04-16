FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./gdal/pyogr ${PY_FILES})