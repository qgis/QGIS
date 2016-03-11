export PATH=/usr/local/opt/ccache/libexec:$PATH
export CCACHE_DIR="${HOME}/.ccache-osx"
ccache -s
ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest|PyQgsServer' -S ./qgis-test-travis.ctest --output-on-failure

ccache -s
