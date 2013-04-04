FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./admintools/httplib2 ${PY_FILES})