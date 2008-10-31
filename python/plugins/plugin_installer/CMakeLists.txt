#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
fetchingbase.py
fetchingbase.ui
guibase.py
guibase.ui
__init__.py
installer_data.py
installer_gui.py
installer_plugin.py
installingbase.py
installingbase.ui
pluginerrorbase.py
pluginerrorbase.ui
plugin_installer.png
qgis-icon.png
repoConnected.png
repoDisabled.png
repositorybase.py
repositorybase.ui
repoUnavailable.png
resources.qrc
resources_rc.py
unzip.py
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
