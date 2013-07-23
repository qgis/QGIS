FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante saga/ext ${PY_FILES})
