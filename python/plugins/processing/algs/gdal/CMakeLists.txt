FILE(GLOB PY_FILES *.py)

ADD_SUBDIRECTORY(pyogr)

PLUGIN_INSTALL(processing ./algs/gdal ${PY_FILES})