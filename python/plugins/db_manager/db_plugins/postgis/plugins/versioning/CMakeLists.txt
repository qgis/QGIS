FILE(GLOB PY_FILES *.py)
FILE(GLOB UI_FILES *.ui)
PYQT_WRAP_UI(PYUI_FILES ${UI_FILES})
PLUGIN_INSTALL(db_manager db_plugins/postgis/plugins/versioning ${PY_FILES} ${PYUI_FILES})
