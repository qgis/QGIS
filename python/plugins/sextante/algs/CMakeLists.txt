FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./algs ${PY_FILES})
