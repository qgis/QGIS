
SET (SVG_DIRS 
 accommodation
 amenity
 biology
 biology  
 emergency
 entertainment
 food
 geometric
 gpsicons
 health
 icon
 landmark
 money
 north_arrows
 religion
 services
 shopping
 sport
 symbol
 tourist
 transport
)
  

FOREACH (SVG_DIR ${SVG_DIRS})

  FILE (GLOB IMAGES ${SVG_DIR}/*.svg)

  INSTALL (FILES ${IMAGES}
           DESTINATION ${QGIS_DATA_DIR}/svg/${SVG_DIR})

ENDFOREACH (SVG_DIR)
