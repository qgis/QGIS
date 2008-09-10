#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
dialog.py
gui.py
icon.xpm
__init__.py
installer_plugin.py
qgis_plugins.py
resources.py
repository_ui.py
repository_dialog.py
qgis-icon.png 
installingbase.ui 
fetchingbase.py 
guibase.py 
repositorybase.py 
plugin_installer.png 
unzip.py 
installer_gui.py 
installingbase.py 
pluginerrorbase.ui 
installer_data.py 
repoConnected.png 
guibase.ui 
pluginerrorbase.py 
repositorybase.ui 
resources_rc.py 
repoDisabled.png
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
