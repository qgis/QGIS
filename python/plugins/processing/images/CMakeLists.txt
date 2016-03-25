FILE(GLOB IMAGE_FILES *.png *.gif *.svg)

PLUGIN_INSTALL(processing ./images ${IMAGE_FILES})
