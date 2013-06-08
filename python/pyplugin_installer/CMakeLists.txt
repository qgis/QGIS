SET (QGIS_PLUGININSTALLER_DIR ${QGIS_DATA_DIR}/python/pyplugin_installer)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

SET(PY_PLUGININSTALLER_FILES
    __init__.py
    installer.py
    installer_data.py
    qgsplugininstallerinstallingdialog.py
    qgsplugininstallerpluginerrordialog.py
    qgsplugininstallerfetchingdialog.py
    qgsplugininstallerrepositorydialog.py
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
)

ADD_CUSTOM_TARGET(pyplugin-installer ALL DEPENDS ${PYUI_FILES})

ADD_CUSTOM_COMMAND(TARGET pyplugin-installer
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/pyplugin_installer
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

FOREACH(pyfile ${PY_PLUGININSTALLER_FILES} ${PYUI_FILES})
  ADD_CUSTOM_COMMAND(TARGET pyplugin-installer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} ${PYTHON_OUTPUT_DIRECTORY}/pyplugin_installer
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
ENDFOREACH(pyfile)

INSTALL(FILES ${PY_PLUGININSTALLER_FILES} ${PYUI_FILES} DESTINATION "${QGIS_PLUGININSTALLER_DIR}")
