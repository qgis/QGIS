export PYTHONPATH=${HOME}/osgeo4travis/lib/python3.3/site-packages/
export CTEST_PARALLEL_LEVEL=1

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# ccache -o max_size=150M
# ccache -o run_second_cpp=true

xvfb-run ctest -V -E "$(cat ${DIR}/blacklist.txt | paste -sd '|' -)" -S ./qgis-test-travis.ctest --output-on-failure
# xvfb-run ctest -V -S ./qgis-test-travis.ctest --output-on-failure
