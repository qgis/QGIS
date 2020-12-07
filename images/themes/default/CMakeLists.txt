add_subdirectory(plugins)

file(GLOB IMAGES *.png *.svg)

install (FILES ${IMAGES}
         DESTINATION ${QGIS_DATA_DIR}/themes/default)
