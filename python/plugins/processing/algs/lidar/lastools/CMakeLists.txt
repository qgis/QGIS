FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing ./lidar/lastools ${PY_FILES})
