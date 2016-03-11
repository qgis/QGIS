ccache -s
export CCACHE_DIR="${HOME}/.ccache-qt4"
ccache -s
xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
