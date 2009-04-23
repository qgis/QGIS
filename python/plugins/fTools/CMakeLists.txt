#TODO: Need to configure cmake to run pyrcc4 and pyuic4 as required when the resource
#      file or the ui change
SET(INSTALLER_FILES
__init__.py
frmAbout.py
ftools_help.xsl
resources.qrc
frmAbout.ui
fTools.py
doAbout.py
ftools_help.xml
resources.py
)
INSTALL(FILES ${INSTALLER_FILES} DESTINATION ${QGIS_DATA_DIR}/python/plugins/fTools)

SUBDIRS(tools icons)
