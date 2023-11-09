file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(grassprovider ./ext ${PY_FILES})
