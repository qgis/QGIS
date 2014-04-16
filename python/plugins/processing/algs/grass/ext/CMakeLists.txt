FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing grass/ext ${PY_FILES})
