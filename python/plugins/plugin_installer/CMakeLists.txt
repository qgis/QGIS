SET(INSTALLER_FILES
__init__.py
installer_data.py
installer_gui.py
installer_plugin.py
plugin_installer.png
qgis-icon.png
repoConnected.png
repoDisabled.png
repoUnavailable.png
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

ADD_CUSTOM_TARGET(pluginstaller ALL DEPENDS ${PYUI_FILES} ${PYRC_FILES})

INSTALL(FILES ${PYUI_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
INSTALL(FILES ${PYRC_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/plugin_installer)
