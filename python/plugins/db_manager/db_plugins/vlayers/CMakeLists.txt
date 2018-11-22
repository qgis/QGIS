
FILE(GLOB PY_FILES *.py)

PLUGIN_INSTALL(db_manager db_plugins/vlayers ${PY_FILES} ${PYRC_FILES})

