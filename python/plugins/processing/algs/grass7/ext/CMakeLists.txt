FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing algs/grass7/ext ${PY_FILES})
