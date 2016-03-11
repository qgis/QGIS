export CCACHE_PATH="${HOME}/ccacheqt4"
xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
