export CCACHE_DIR="${HOME}/ccacheqt4"
export CCACHE_CPP2=YES
xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
ccache -s
