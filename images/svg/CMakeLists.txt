
SET (SVG_DIRS biology geometric gpsicons icon north_arrows sport symbol)

FOREACH (SVG_DIR ${SVG_DIRS})

  FILE (GLOB IMAGES ${SVG_DIR}/*.svg)

  INSTALL (FILES ${IMAGES}
           DESTINATION ${QGIS_DATA_DIR}/svg/${SVG_DIR})

ENDFOREACH (SVG_DIR)
