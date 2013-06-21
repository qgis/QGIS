INSTALL(FILES srs.db qgis.db symbology-ng-style.db spatialite.db customization.xml
        DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY cpt-city-qgis-min DESTINATION ${QGIS_DATA_DIR}/resources)

ADD_SUBDIRECTORY(context_help)
ADD_SUBDIRECTORY(function_help)
