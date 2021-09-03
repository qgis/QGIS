file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES metadata.txt)

add_subdirectory(tests)

PLUGIN_INSTALL(otbprovider . ${PY_FILES} ${OTHER_FILES})
