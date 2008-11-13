#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
__init__.py
installer_data.py
installer_gui.py
installer_plugin.py
plugin_installer.png
qgis-icon.png
qgsplugininstallerbase.py
qgsplugininstallerbase.ui
qgsplugininstallerfetchingbase.py
qgsplugininstallerfetchingbase.ui
qgsplugininstallerinstallingbase.py
qgsplugininstallerinstallingbase.ui
qgsplugininstallerpluginerrorbase.py
qgsplugininstallerpluginerrorbase.ui
qgsplugininstallerrepositorybase.py
qgsplugininstallerrepositorybase.ui
repoConnected.png
repoDisabled.png
repoUnavailable.png
resources.qrc
resources_rc.py
unzip.py
version_compare.py
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
