
file (GLOB MODULE_FILES *.qgm *.svg *.png)
install (FILES ${MODULE_FILES}
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)

file (GLOB CONFIG *.qgc)
install (FILES ${CONFIG}
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)
