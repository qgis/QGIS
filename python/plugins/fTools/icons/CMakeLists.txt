#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(VECTOR_GRAPHICS_FILES
gis-0.1.svg
logo.svg
menu_icons.svg
)
INSTALL(FILES ${VECTOR_GRAPHICS_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/fTools/icons)

SUBDIRS(default gis)
