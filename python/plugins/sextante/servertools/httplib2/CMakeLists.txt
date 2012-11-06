FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante servertools/httplib2 ${PY_FILES})