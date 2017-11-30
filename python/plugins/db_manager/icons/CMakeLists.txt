FILE(GLOB ICON_FILES *.gif *.png *svg *.xpm toolbar/*.png)

PLUGIN_INSTALL(db_manager icons ${ICON_FILES})
