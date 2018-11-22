
FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(db_manager db_plugins/spatialite ${PY_FILES} ${PYRC_FILES})
PLUGIN_INSTALL(db_manager db_plugins/spatialite/icons ${ICON_FILES})

