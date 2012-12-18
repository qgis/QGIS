FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./gdal/pyogr ${PY_FILES})