export PYTHONPATH=${HOME}/osgeo4travis/lib/python3/dist-packages/

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

xvfb-run ctest -V -E "$(cat ${DIR}/blacklist.txt | paste -sd '|' -)" -S ./qgis-test-travis.ctest --output-on-failure
