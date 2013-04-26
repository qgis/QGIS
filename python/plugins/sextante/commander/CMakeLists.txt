FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./commander ${PY_FILES})
