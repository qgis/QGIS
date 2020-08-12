INCLUDE(CopyResources)

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
  FILE(GLOB_RECURSE IMAGES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SVG_DIR}/*.svg)
  SET(SVG_FILES ${SVG_FILES} ${IMAGES})
ENDFOREACH (SVG_DIR)

ADD_QGIS_RESOURCES("${CMAKE_CURRENT_SOURCE_DIR}" svg DEST_SVG_FILES "${SVG_FILES}")

ADD_CUSTOM_TARGET(svg ALL DEPENDS ${DEST_SVG_FILES})

