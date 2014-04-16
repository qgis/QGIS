FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./admintools/httplib2 ${PY_FILES})