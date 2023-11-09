set (QGIS_PLUGININSTALLER_DIR ${QGIS_DATA_DIR}/python/pyplugin_installer)

set(PY_PLUGININSTALLER_FILES
    __init__.py
    installer.py
    installer_data.py
    qgsplugininstallerinstallingdialog.py
    qgsplugininstallerpluginerrordialog.py
    qgsplugininstallerfetchingdialog.py
    qgsplugininstallerrepositorydialog.py
    qgsplugindependenciesdialog.py
    plugindependencies.py
    unzip.py
    version_compare.py
)

# file(GLOB UI_FILES *.ui)
# PYQT_WRAP_UI(PYUI_FILES ${UI_FILES})

PYQT_WRAP_UI(PYUI_FILES
    qgsplugininstallerfetchingbase.ui
    qgsplugininstallerinstallingbase.ui
    qgsplugininstallerpluginerrorbase.ui
    qgsplugininstallerrepositorybase.ui
    qgsplugindependenciesdialogbase.ui
)

add_custom_target(pyplugin-installer ALL DEPENDS ${PYUI_FILES})

add_custom_command(TARGET pyplugin-installer
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/pyplugin_installer
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(pyfile ${PY_PLUGININSTALLER_FILES} ${PYUI_FILES})
  add_custom_command(TARGET pyplugin-installer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} ${PYTHON_OUTPUT_DIRECTORY}/pyplugin_installer
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
endforeach(pyfile)
PY_COMPILE(pyplugin-installer "${PYTHON_OUTPUT_DIRECTORY}/pyplugin_installer")

install(FILES ${PY_PLUGININSTALLER_FILES} ${PYUI_FILES} DESTINATION "${QGIS_PLUGININSTALLER_DIR}")
