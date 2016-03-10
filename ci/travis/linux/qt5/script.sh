export PYTHONPATH=${HOME}/osgeo4travis/lib/python3/dist-packages/

xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
