FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing algs/grass/ext ${PY_FILES})
