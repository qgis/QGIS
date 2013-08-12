FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES *.txt)

PLUGIN_INSTALL(processing ./exampleprovider ${PY_FILES} ${OTHER_FILES})
