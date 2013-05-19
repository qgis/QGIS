SET (QGIS_PLUGININSTALLER_DIR ${QGIS_DATA_DIR}/python/pyplugin_installer)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

SET(PY_PLUGININSTALLER_FILES
    __init__.py
    installer.py
    installer_data.py
    installer_gui.py
    unzip.py
    version_compare.py
)

# FILE(GLOB UI_FILES *.ui)
# PYQT4_WRAP_UI(PYUI_FILES ${UI_FILES})

PYQT4_WRAP_UI(PYUI_FILES
    qgsplugininstallerfetchingbase.ui
    qgsplugininstallerinstallingbase.ui
    qgsplugininstallerpluginerrorbase.ui
    qgsplugininstallerrepositorybase.ui
    qgsplugininstalleroldreposbase.ui
)

ADD_CUSTOM_TARGET(pyplugin_installer ALL DEPENDS ${PYUI_FILES})

INSTALL(FILES ${PY_PLUGININSTALLER_FILES} ${PYUI_FILES} DESTINATION "${QGIS_PLUGININSTALLER_DIR}")
