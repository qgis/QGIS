export PYTHONPATH=${HOME}/osgeo4travis/lib/python3/dist-packages/

xvfb-run ctest -V -R 'qgis_applicationtest' -S ./qgis-test-travis.ctest --output-on-failure
