set (QSCI_OUTPUT_DIR ${QGIS_DATA_DIR}/python/qsci_apis)
file(GLOB API_FILES *.pap *.api)
install(FILES ${API_FILES} DESTINATION ${QSCI_OUTPUT_DIR})
