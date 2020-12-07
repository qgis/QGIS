include(CopyResources)

# OS X app/doc icons
if (APPLE)
  add_subdirectory(mac)
endif()

file(GLOB_RECURSE ICON_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.png)

ADD_QGIS_RESOURCES("${CMAKE_CURRENT_SOURCE_DIR}" "images/icons" DEST_ICON_FILES "${ICON_FILES}")

add_custom_target(icons ALL DEPENDS ${DEST_ICON_FILES})
