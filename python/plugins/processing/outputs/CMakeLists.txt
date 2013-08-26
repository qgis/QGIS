FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./outputs ${PY_FILES})
