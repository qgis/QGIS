
INSTALL(FILES srs.db qgis.db qgis_help.db symbology-ng-style.xml
        DESTINATION ${QGIS_DATA_DIR}/resources)

SUBDIRS(context_help)
