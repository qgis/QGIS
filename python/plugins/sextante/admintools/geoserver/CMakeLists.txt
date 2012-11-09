FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./admintools/geoserver ${PY_FILES})