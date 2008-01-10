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
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qgis/python/plugins/plugin_installer)
