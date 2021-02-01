#!/usr/bin/env bash

set -e

##############
# Setup ccache
##############
export CCACHE_TEMPDIR=/tmp
# Github workflow cache max size is 2.0, but ccache data get compressed (roughly 1/5?)
ccache -M 2.0G

# Temporarily uncomment to debug ccache issues
# export CCACHE_LOGFILE=/tmp/cache.debug
ccache -z

##############################
# Variables for output styling
##############################

bold=$(tput bold)
endbold=$(tput sgr0)

###########
# Configure
###########
pushd /root/QGIS > /dev/null
mkdir -p build
pushd build > /dev/null

echo "${bold}Running cmake...${endbold}"

export CC=/usr/lib/ccache/clang
export CXX=/usr/lib/ccache/clang++

HANA_TESTS_ENABLED=OFF
if [ ${HANA_TESTS} == "true" ] ; then
  HANA_TESTS_ENABLED=ON
fi

cmake \
 -GNinja \
 -DUSE_CCACHE=OFF \
 -DWITH_QUICK=OFF \
 -DWITH_3D=ON \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=OFF \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_TESTS=ON \
 -DENABLE_MODELTEST=ON \
 -DENABLE_PGTEST=ON \
 -DENABLE_SAGA_TESTS=ON \
 -DENABLE_MSSQLTEST=ON \
 -DENABLE_HANATEST=$HANA_TESTS_ENABLED \
 -DENABLE_ORACLETEST=ON \
 -DPUSH_TO_CDASH=${PUSH_TO_CDASH} \
 -DWITH_HANA=ON \
 -DWITH_QSPATIALITE=ON \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_DESKTOP=ON \
 -DWITH_BINDINGS=ON \
 -DWITH_SERVER=ON \
 -DWITH_ORACLE=ON \
 -DORACLE_INCLUDEDIR="/instantclient_19_9/sdk/include/" \
 -DORACLE_LIBDIR="/instantclient_19_9/" \
 -DDISABLE_DEPRECATED=ON \
 -DPYTHON_TEST_WRAPPER="timeout -sSIGSEGV 55s"\
 -DCXX_EXTRA_FLAGS="${CLANG_WARNINGS}" \
 -DWERROR=TRUE \
 -DADD_CLAZY_CHECKS=ON \
 -DQT5_3DEXTRA_LIBRARY="/usr/lib/x86_64-linux-gnu/libQt53DExtras.so" \
 -DQT5_3DEXTRA_INCLUDE_DIR="/root/QGIS/external/qt3dextra-headers" \
 -DCMAKE_PREFIX_PATH="/root/QGIS/external/qt3dextra-headers/cmake" \
 -DQt53DExtras_DIR="/root/QGIS/external/qt3dextra-headers/cmake/Qt53DExtras" \
 ..

#######
# Build
#######
echo "${bold}Building QGIS...${endbold}"
${CTEST_BUILD_COMMAND}

########################
# Show ccache statistics
########################
echo "ccache statistics"
ccache -s

popd > /dev/null # build
popd > /dev/null # /root/QGIS

[ -r /tmp/ctest-important.log ] && cat /tmp/ctest-important.log || true

