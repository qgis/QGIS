FILE(GLOB PY_FILES *.py)

ADD_SUBDIRECTORY(geoserver)

PLUGIN_INSTALL(processing ./admintools ${PY_FILES})
