echo $PATH

export PATH=/usr/bin:${PATH}

ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest|PyQgsServer|ProcessingGdalAlgorithmsTest|qgis_composerhtmltest' -S ./qgis-test-travis.ctest --output-on-failure

