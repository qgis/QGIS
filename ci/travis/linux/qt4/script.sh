export PATH=/usr/lib/ccache:$PATH
which clang-3.8
export CCACHE_DIR="${HOME}/.ccache-qt4"
xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
ccache -s
