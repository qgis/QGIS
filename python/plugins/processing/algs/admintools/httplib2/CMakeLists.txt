FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/admintools/httplib2 ${PY_FILES})