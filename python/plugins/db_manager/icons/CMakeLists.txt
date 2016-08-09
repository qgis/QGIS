FILE(GLOB ICON_FILES *.gif *.png *.xpm)
FILE(GLOB TOOLBAR_FILES toolbar/*.png)

PLUGIN_INSTALL(db_manager icons ${ICON_FILES})
PLUGIN_INSTALL(db_manager icons/toolbar ${TOOLBAR_FILES})
