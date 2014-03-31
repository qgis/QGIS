FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing grass7/ext ${PY_FILES})
