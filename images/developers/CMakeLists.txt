
FILE (GLOB IMAGES *.jpg)
INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/images/developers)
