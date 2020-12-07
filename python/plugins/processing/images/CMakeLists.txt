file(GLOB IMAGE_FILES *.png *.svg)

add_subdirectory(gdaltools)

PLUGIN_INSTALL(processing ./images ${IMAGE_FILES})
