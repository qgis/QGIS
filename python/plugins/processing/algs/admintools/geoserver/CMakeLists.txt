FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/admintools/geoserver ${PY_FILES})