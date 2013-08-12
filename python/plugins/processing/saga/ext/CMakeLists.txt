FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing saga/ext ${PY_FILES})
