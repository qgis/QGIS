file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES metadata.txt)

PLUGIN_INSTALL(gdalprovider . ${PY_FILES} ${OTHER_FILES})

add_subdirectory(images)
