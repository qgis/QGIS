add_subdirectory(qgis_topoview)
add_subdirectory(versioning)

PLUGIN_INSTALL(db_manager db_plugins/postgis/plugins __init__.py)
