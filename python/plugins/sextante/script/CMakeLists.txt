FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./script ${PY_FILES})
