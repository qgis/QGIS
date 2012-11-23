FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./outputs ${PY_FILES})
