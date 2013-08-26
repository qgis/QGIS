FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./admintools/geoserver ${PY_FILES})