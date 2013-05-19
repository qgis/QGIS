SET(INSTALLER_FILES
        metadata.txt
        mapserver_export.png
        __init__.py
        mapserverexportdialog.py
        mapserverexport.py
        ms_export.py
)

PYQT4_WRAP_UI(PYUI_FILES qgsmapserverexportbase.ui)
PYQT4_ADD_RESOURCES(PYRC_FILES resources.qrc)

SET(INSTALLER_FILES ${INSTALLER_FILES} ${PYUI_FILES} ${PYRC_FILES})

PLUGIN_INSTALL(mapserver_export . ${INSTALLER_FILES})
