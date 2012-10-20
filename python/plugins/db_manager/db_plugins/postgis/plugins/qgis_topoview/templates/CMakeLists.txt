
FILE(GLOB QML_FILES *.qml)

PLUGIN_INSTALL(db_manager db_plugins/postgis/plugins/qgis_topoview/templates ${QML_FILES})
