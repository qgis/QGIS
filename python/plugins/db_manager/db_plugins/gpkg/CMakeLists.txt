
FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(db_manager db_plugins/gpkg ${PY_FILES} ${PYRC_FILES})
PLUGIN_INSTALL(db_manager db_plugins/gpkg/icons ${ICON_FILES})

