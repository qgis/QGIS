FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./modeler ${PY_FILES})
