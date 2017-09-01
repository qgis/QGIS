
FILE(GLOB PY_FILES *.py)
FILE(GLOB ICON_FILES icons/*.png)

PYQT_ADD_RESOURCES(PYRC_FILES resources.qrc)

PLUGIN_INSTALL(db_manager db_plugins/gpkg ${PY_FILES} ${PYRC_FILES})
PLUGIN_INSTALL(db_manager db_plugins/gpkg/icons ${ICON_FILES})

