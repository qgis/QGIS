file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing algs/qgis ${PY_FILES})
