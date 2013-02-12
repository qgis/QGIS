FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./parameters ${PY_FILES})
