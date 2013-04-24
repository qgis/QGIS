SET(INSTALLER_FILES
metadata.txt
__init__.py
installer_data.py
installer_gui.py
installer_plugin.py
plugin_installer.png
qgis-icon.png
repoConnected.png
repoDisabled.png
repoUnavailable.png
pluginExperimental.png
pluginStable.png
unzip.py
version_compare.py
)

PYQT4_WRAP_UI(PYUI_FILES
	qgsplugininstallerbase.ui
	qgsplugininstallerfetchingbase.ui
	qgsplugininstallerinstallingbase.ui
	qgsplugininstallerpluginerrorbase.ui
	qgsplugininstallerrepositorybase.ui
	qgsplugininstalleroldreposbase.ui
)

PYQT4_ADD_RESOURCES(PYRC_FILES resources.qrc)

PLUGIN_INSTALL(plugin_installer . ${PYUI_FILES} ${PYRC_FILES} ${INSTALLER_FILES})
