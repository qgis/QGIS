SET(ICON_FILES
iconClearConsole.png
iconOpenConsole.png
iconRunConsole.png
iconTempConsole.png
iconSextanteConsole.png
iconSaveConsole.png 
iconHelpConsole.png
imgHelpDialog.png
iconIfaceConsole.png
iconClassConsole.png
)

FILE(GLOB ICON_FILES *.png)
INSTALL(FILES ${ICON_FILES} DESTINATION ${QGIS_PYTHON_DIR}/iconConsole)
