
FILE(GLOB PY_FILES *.py)

PYQT_ADD_RESOURCES(PYRC_FILES resources.qrc)

PLUGIN_INSTALL(db_manager db_plugins/vlayers ${PY_FILES} ${PYRC_FILES})

