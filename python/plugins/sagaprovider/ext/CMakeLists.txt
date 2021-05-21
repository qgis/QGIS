file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sagaprovider ./ext ${PY_FILES})
