#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
FILE(GLOB ICON_FILES *.png)

PLUGIN_INSTALL(fTools icons/default ${ICON_FILES})
