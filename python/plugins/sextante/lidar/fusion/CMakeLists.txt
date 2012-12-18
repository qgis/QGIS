FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(sextante ./lidar/fusion ${PY_FILES})
