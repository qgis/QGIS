file(GLOB IMAGE_FILES *.png)

PLUGIN_INSTALL(gdalprovider ./images ${IMAGE_FILES})
