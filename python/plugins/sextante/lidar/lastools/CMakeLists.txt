FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./lidar/lastools ${PY_FILES})
