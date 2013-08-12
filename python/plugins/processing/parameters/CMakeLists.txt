FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./parameters ${PY_FILES})
