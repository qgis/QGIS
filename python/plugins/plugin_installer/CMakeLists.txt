SET(INSTALLER_FILES
dialog.py
gui.py
icon.xpm
__init__.py
installer_plugin.py
qgis_plugins.py
resources.py
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qgis/python/plugins/plugin_installer)
