FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./gdal/scripts ${PY_FILES})
