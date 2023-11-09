file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES algnames.txt)

PLUGIN_INSTALL(processing gui ${PY_FILES} ${OTHER_FILES})
