SUBDIRS (plugins)

FILE (GLOB IMAGES *.png)

INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/themes/newgis)
