#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(VECTOR_GRAPHICS_FILES
gis-0.1.svg
logo.svg
menu_icons.svg
)

PLUGIN_INSTALL(fTools icons ${VECTOR_GRAPHICS_FILES} logo_small.png)

ADD_SUBDIRECTORY(default)
