#!/usr/bin/env bash

set -e

##############
# Setup ccache
##############
export CCACHE_TEMPDIR=/tmp
ccache -M 1.2G

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

echo "travis_fold:start:cmake"
echo "${bold}Running cmake...${endbold}"

export CC=/usr/lib/ccache/clang
export CXX=/usr/lib/ccache/clang++

cmake \
 -GNinja \
 -DUSE_CCACHE=OFF \
 -DWITH_QUICK=ON \
 -DWITH_3D=ON \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=OFF \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_MODELTEST=ON \
 -DENABLE_PGTEST=ON \
 -DENABLE_SAGA_TESTS=ON \
 -DENABLE_MSSQLTEST=ON \
 -DWITH_QSPATIALITE=ON \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_DESKTOP=ON \
 -DWITH_BINDINGS=ON \
 -DWITH_SERVER=ON \
 -DDISABLE_DEPRECATED=ON \
 -DPYTHON_TEST_WRAPPER="timeout -sSIGSEGV 55s"\
 -DCXX_EXTRA_FLAGS="${CLANG_WARNINGS}" \
 -DWERROR=TRUE \
 -DADD_CLAZY_CHECKS=ON \
 -DQT5_3DEXTRA_LIBRARY="/usr/lib/x86_64-linux-gnu/libQt53DExtras.so" \
 -DQT5_3DEXTRA_INCLUDE_DIR="/root/QGIS/external/qt3dextra-headers" \
 -DCMAKE_PREFIX_PATH="/root/QGIS/external/qt3dextra-headers/cmake" \
 ..
echo "travis_fold:end:cmake"

#######
# Build
#######
# Calculate the timeout for building.
# The tests should be aborted before travis times out, in order to allow uploading
# the ccache and therefore speedup subsequent e builds.
#
# Travis will kill the job after approx 150 minutes, we subtract 5 minutes for
# uploading and subtract the bootstrapping time from that.
# Hopefully clocks are in sync :)

CURRENT_TIME=$(date +%s)
TIMEOUT=$((( TRAVIS_AVAILABLE_TIME - TRAVIS_UPLOAD_TIME ) * 60 - CURRENT_TIME + TRAVIS_TIMESTAMP))
TIMEOUT=$(( TIMEOUT < 300 ? 300 : TIMEOUT ))
echo "Timeout: ${TIMEOUT}s (started at ${TRAVIS_TIMESTAMP}, current: ${CURRENT_TIME})"

# echo "travis_fold:start:ninja-build.1"
echo "${bold}Building QGIS...${endbold}"
timeout ${TIMEOUT}s ${CTEST_BUILD_COMMAND}
# echo "travis_fold:end:ninja-build.1"
rv=$?

########################
# Show ccache statistics
########################
echo "travis_fold:start:ccache.stats"
echo "ccache statistics"
ccache -s
echo "travis_fold:end:ccache.stats"

popd > /dev/null # build
popd > /dev/null # /root/QGIS

[ -r /tmp/ctest-important.log ] && cat /tmp/ctest-important.log || true

############################
# Exit with error if timeout
############################
if [ $rv -eq 124 ] ; then
    printf '\n\n${bold}Build and test timeout. Please restart the build for meaningful results.${endbold}\n'
    exit #$rv
fi
