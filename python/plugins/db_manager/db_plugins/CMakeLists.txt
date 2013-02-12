ADD_SUBDIRECTORY(postgis)
ADD_SUBDIRECTORY(spatialite)

FILE(GLOB PY_FILES *.py)
PLUGIN_INSTALL(db_manager db_plugins ${PY_FILES})
