FILE(GLOB IMAGE_FILES *.png *.gif)

PLUGIN_INSTALL(processing ./images ${IMAGE_FILES})
