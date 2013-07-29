FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES *.txt)

PLUGIN_INSTALL(sextante ./exampleprovider ${PY_FILES})
PLUGIN_INSTALL(sextante ./exampleprovider ${OTHER_FILES})
