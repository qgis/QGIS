#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change

SET (OSM_PLUGIN_DIR ${QGIS_DATA_DIR}/python/plugins/osm)

FILE (GLOB PYTHON_FILES *.py)
FILE (GLOB MAPTOOLS_PYTHON_FILES map_tools/*.py)
FILE (GLOB STYLE_FILES styles/*.style)

INSTALL (FILES ${PYTHON_FILES} DESTINATION ${OSM_PLUGIN_DIR})
INSTALL (FILES ${MAPTOOLS_PYTHON_FILES} DESTINATION ${OSM_PLUGIN_DIR}/map_tools)
INSTALL (FILES ${STYLE_FILES} DESTINATION ${OSM_PLUGIN_DIR}/styles)
