FILE(GLOB PY_FILES *.py)

ADD_SUBDIRECTORY(templates)

PLUGIN_INSTALL(db_manager db_plugins/postgis/plugins/qgis_topoview ${PY_FILES})
