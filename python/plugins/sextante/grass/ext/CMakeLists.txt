FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante grass/ext ${PY_FILES})
