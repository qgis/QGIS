FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./pymorph/scripts ${PY_FILES})
