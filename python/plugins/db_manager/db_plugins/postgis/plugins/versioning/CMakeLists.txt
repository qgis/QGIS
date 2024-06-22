file(GLOB PY_FILES *.py)
file(GLOB UI_FILES *.ui)
PLUGIN_INSTALL(db_manager db_plugins/postgis/plugins/versioning ${PY_FILES} ${UI_FILES})
