FILE(GLOB PY_FILES *.py)

ADD_SUBDIRECTORY(fusion)
ADD_SUBDIRECTORY(lastools)

PLUGIN_INSTALL(processing ./algs/lidar ${PY_FILES})
