
SET (SVG_DIRS
 accommodation
 amenity
 arrows
 backgrounds
 components
 crosses
 emergency
 entertainment
 food
 geometric
 gpsicons
 health
 icon
 landmark
 money
 religion
 services
 shopping
 sport
 symbol
 tourist
 transport
 wind_roses
)
  

FOREACH (SVG_DIR ${SVG_DIRS})

  FILE (GLOB IMAGES ${SVG_DIR}/*.svg)

  INSTALL (FILES ${IMAGES}
           DESTINATION ${QGIS_DATA_DIR}/svg/${SVG_DIR})

ENDFOREACH (SVG_DIR)
