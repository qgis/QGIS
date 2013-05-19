ADD_SUBDIRECTORY(coordinate_capture)
ADD_SUBDIRECTORY(gps_importer)

FILE(GLOB IMAGES *.png *.svg)

INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/themes/default/plugins)
