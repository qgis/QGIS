file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES metadata.txt)

PLUGIN_INSTALL(qgisprovider . ${PY_FILES} ${OTHER_FILES})

add_subdirectory(tests)
