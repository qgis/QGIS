#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
mapserver_export.png
__init__.py
mapserverexportdialog.py
mapserverexport.py
ms_export.py
)
PYQT4_WRAP_UI(PYUI_FILES qgsmapserverexportbase.ui) 
	 	PYQT4_ADD_RESOURCES(PYRC_FILES resources.qrc) 
ADD_CUSTOM_TARGET(mapserverexport ALL DEPENDS ${PYUI_FILES} ${PYRC_FILES}) 
	 	 
SET(INSTALLER_FILES ${INSTALLER_FILES} ${PYUI_FILES} ${PYRC_FILES}) 
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/mapserver_export)
