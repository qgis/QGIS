FILE(GLOB PY_FILES *.py)
PLUGIN_INSTALL(processing algs/otb ${PY_FILES})
