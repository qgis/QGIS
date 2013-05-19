FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./core ${PY_FILES})
