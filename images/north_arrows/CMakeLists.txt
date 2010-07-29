
FILE (GLOB PNGIMAGES *.png)

INSTALL (FILES ${PNGIMAGES}
         DESTINATION ${QGIS_DATA_DIR}/images/north_arrows)
