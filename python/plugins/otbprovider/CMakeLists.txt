file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES metadata.txt)

PLUGIN_INSTALL(otbprovider . ${PY_FILES} ${OTHER_FILES})
