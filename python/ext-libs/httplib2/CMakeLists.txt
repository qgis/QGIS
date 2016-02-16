FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./httplib2 ${PY_FILES})
