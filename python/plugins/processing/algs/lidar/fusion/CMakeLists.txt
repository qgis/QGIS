FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./algs/lidar/fusion ${PY_FILES})
