
SET (SVG_DIRS 
  biology  
  emergency  
  entertainment  
  geometric  
  gpsicons    
  icon  
  landmark  
  north_arrows  
  services  
  sport  
  symbol  
  transport)
  

FOREACH (SVG_DIR ${SVG_DIRS})

  FILE (GLOB IMAGES ${SVG_DIR}/*.svg)

  INSTALL (FILES ${IMAGES}
           DESTINATION ${QGIS_DATA_DIR}/svg/${SVG_DIR})

ENDFOREACH (SVG_DIR)
