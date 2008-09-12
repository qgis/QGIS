#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
dialog.py
fetchingbase.py 
fetchingbase.ui
guibase.py 
guibase.ui 
gui.py
icon.xpm
__init__.py
installer_data.py 
installer_gui.py 
installer_plugin.py
installingbase.py 
installingbase.ui 
pluginerrorbase.py 
pluginerrorbase.ui 
plugin_installer.png 
PluginInstaller.png
qgis-icon.png 
qgis_plugins.py
repoConnected.png 
repoDisabled.png
repositorybase.py 
repositorybase.ui 
repository_dialog.py
repository_ui.py
repoUnavailable.png
resources.py
resources_rc.py 
unzip.py 
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
