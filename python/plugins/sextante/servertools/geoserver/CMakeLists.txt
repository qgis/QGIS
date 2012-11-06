FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante servertools/geoserver ${PY_FILES})