FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./preconfigured ${PY_FILES})
