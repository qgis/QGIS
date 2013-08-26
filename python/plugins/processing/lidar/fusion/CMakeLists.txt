FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./lidar/fusion ${PY_FILES})
