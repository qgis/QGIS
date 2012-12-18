FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./gdal/scripts ${PY_FILES})
