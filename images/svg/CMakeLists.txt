include(CopyResources)

set (SVG_DIRS
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
  
foreach (SVG_DIR ${SVG_DIRS})
  file(GLOB_RECURSE IMAGES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SVG_DIR}/*.svg)
  set(SVG_FILES ${SVG_FILES} ${IMAGES})
endforeach (SVG_DIR)

ADD_QGIS_RESOURCES("${CMAKE_CURRENT_SOURCE_DIR}" svg DEST_SVG_FILES "${SVG_FILES}")

add_custom_target(svg ALL DEPENDS ${DEST_SVG_FILES})

