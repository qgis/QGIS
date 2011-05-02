
FILE (GLOB MODULE_FILES *.qgm *.svg *.png)
INSTALL (FILES ${MODULE_FILES}
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)
