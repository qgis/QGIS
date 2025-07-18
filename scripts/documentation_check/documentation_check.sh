#!/usr/bin/env bash
# shellcheck disable=SC2164
PROG_NAME=$(basename $0)

set -E
set -o functrace
function handle_error {
    local retval=$?
    local line=${last_lineno:-$1}
    echo "Failed at $PROG_NAME:$line $BASH_COMMAND"
    exit $retval
}
trap 'handle_error $LINENO ${BASH_LINENO[@]}' ERR

QGIS_DIR=${1:-/qgis}
DOXY_DIR=${2:-/usr/local/doxygen}

PATH="$PATH:$DOXY_DIR/bin"

echo "===== ENV var:"
echo "  - QGIS_DIR: $QGIS_DIR"
echo "  - DOXY_DIR: $DOXY_DIR"
echo "  - PATH: $PATH"

git config --global --add safe.directory $QGIS_DIR

echo "===== Running cmake"
mkdir -p "$QGIS_DIR/build_doc"
cd "$QGIS_DIR/build_doc"
cmake -DUSE_CCACHE=OFF -DWITH_CORE=OFF -DWITH_APIDOC=ON -DWITH_ASTYLE=ON -DENABLE_TESTS=ON \
    -DWITH_DOT=NO -DWERROR=OFF -DDOXYGEN_EXECUTABLE="$DOXY_DIR/bin/doxygen" .. >/dev/null

echo "===== Running apidoc"
make -j8 apidoc 2>&1 | grep " warning: " && false

echo "===== Running PyQgsDocCoverage"
cd "$QGIS_DIR/build_doc"
export QGIS_TEST_ACCEPT_GITSTATUS_CHECK_FAILURE=1

RET=0
ctest -V -R PyQgsDocCoverage > $QGIS_DIR/PyQgsDocCoverage.out 2>&1 || RET=1

if grep -q "CTEST_FULL_OUTPUT" $QGIS_DIR/PyQgsDocCoverage.out
then
  cat $QGIS_DIR/PyQgsDocCoverage.out | \
    sed -n '/15: CTEST_FULL_OUTPUT/,/15: \(F\|OK\)$/{/15: CTEST_FULL_OUTPUT/b;/15: \(F\|OK\)$/b;p}'
else
  cat $QGIS_DIR/PyQgsDocCoverage.out
fi

if [ "$RET" == "0" ]
then
  echo "===== Success!"
else
  echo "===== Failure!"
  exit $RET
fi
