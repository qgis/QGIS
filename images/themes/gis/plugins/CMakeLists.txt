ADD_SUBDIRECTORY(coordinate_capture)

FILE (GLOB IMAGES *.png)

INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/themes/gis/plugins)
