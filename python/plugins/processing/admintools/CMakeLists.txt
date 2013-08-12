FILE(GLOB PY_FILES *.py)

ADD_SUBDIRECTORY(geoserver)
ADD_SUBDIRECTORY(httplib2)

PLUGIN_INSTALL(processing ./admintools ${PY_FILES})