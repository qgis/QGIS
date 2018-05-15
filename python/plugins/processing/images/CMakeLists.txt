FILE(GLOB IMAGE_FILES *.png *.svg)

ADD_SUBDIRECTORY(gdaltools)

PLUGIN_INSTALL(processing ./images ${IMAGE_FILES})
