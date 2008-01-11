#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
icon.png
__init__.py
mapserverexportdialog.py
mapserverexport.py
ms_export.py
resources.py
ui_mapserverexport.py
)

INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qgis/python/plugins/mapserver_export)
