INSTALL(FILES srs.db qgis.db symbology-style.xml spatialite.db customization.xml 2to3migration.txt
        DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(FILES qgis_global_settings.ini
        DESTINATION ${QGIS_DATA_DIR})
INSTALL(DIRECTORY cpt-city-qgis-min DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY themes DESTINATION ${QGIS_DATA_DIR}/resources)
INSTALL(DIRECTORY data DESTINATION ${QGIS_DATA_DIR}/resources)

IF (WITH_SERVER)
        INSTALL(DIRECTORY server DESTINATION ${QGIS_DATA_DIR}/resources)
ENDIF (WITH_SERVER)

IF (APPLE)
  # ASN.1 definition files of PKIX elements
  INSTALL(FILES pkcs8.asn
          DESTINATION ${QGIS_DATA_DIR}/resources)
ENDIF (APPLE)
