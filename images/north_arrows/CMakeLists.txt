
FILE (GLOB IMAGES *.png)

INSTALL (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/images/north_arrows)
