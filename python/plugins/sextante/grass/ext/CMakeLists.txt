FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante grass/postproc ${PY_FILES})
