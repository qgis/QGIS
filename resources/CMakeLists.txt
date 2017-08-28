INSTALL(FILES srs.db qgis.db symbology-style.xml spatialite.db customization.xml
        DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY cpt-city-qgis-min DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY themes DESTINATION ${QGIS_DATA_DIR}/resources)
