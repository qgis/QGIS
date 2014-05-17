FILE(GLOB PY_FILES *.py)

SET (WITH_INTERNAL_HTTPLIB TRUE CACHE BOOL "Use internal httplib2")

ADD_SUBDIRECTORY(geoserver)

PLUGIN_INSTALL(processing ./algs/admintools ${PY_FILES})
