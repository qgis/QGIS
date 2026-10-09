file(GLOB IMAGE_FILES *.png *.svg)

PLUGIN_INSTALL(processing ./images ${IMAGE_FILES})
