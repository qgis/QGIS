FILE(GLOB IMAGE_FILES *.png)

PLUGIN_INSTALL(processing ./images/ftools ${IMAGE_FILES})
