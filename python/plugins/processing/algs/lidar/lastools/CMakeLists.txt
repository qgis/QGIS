FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/lidar/lastools ${PY_FILES})
