
FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(db_manager db_plugins/oracle ${PY_FILES} ${PYRC_FILES})
PLUGIN_INSTALL(db_manager db_plugins/oracle/icons ${ICON_FILES})

