FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./commander ${PY_FILES})
