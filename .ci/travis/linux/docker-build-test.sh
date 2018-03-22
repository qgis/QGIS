#!/usr/bin/env bash

set -e

##############
# Setup ccache
##############
export CCACHE_TEMPDIR=/tmp
ccache -M 1G

# Temporarily uncomment to debug ccache issues
# export CCACHE_LOGFILE=/tmp/cache.debug
ccache -z

############################
# Setup the (c)test environment
############################
export LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so
export SEGFAULT_SIGNALS="abrt segv"
export CTEST_BUILD_COMMAND="/usr/bin/ninja"
export CTEST_PARALLEL_LEVEL=1

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
cmake \
 -GNinja \
 -DWITH_3D=ON \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=OFF \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_MODELTEST=ON \
 -DENABLE_PGTEST=ON \
 -DWITH_QSPATIALITE=ON \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_DESKTOP=ON \
 -DWITH_BINDINGS=ON \
 -DWITH_SERVER=ON \
 -DDISABLE_DEPRECATED=ON \
 -DCXX_EXTRA_FLAGS=${CLANG_WARNINGS} ..
echo "travis_fold:end:cmake"

#######
# Build
#######
# Calculate the timeout for building.
# The tests should be aborted before travis times out, in order to allow uploading
# the ccache and therefore speedup subsequent e builds.
#
# Travis will kill the job after approx 90 minutes, we subtract 8 minutes for
# uploading and subtract the bootstrapping time from that.
# Hopefully clocks are in sync :)
TRAVIS_TIME=90
UPLOAD_TIME=8
CURRENT_TIME=`date +%s`
TIMEOUT=$(expr \( ${TRAVIS_TIME} - ${UPLOAD_TIME} \) \* 60 - ${CURRENT_TIME} + ${TRAVIS_TIMESTAMP})
TIMEOUT=$(( ${TIMEOUT} < 300 ? 300 : ${TIMEOUT} ))
echo "Timeout: ${TIMEOUT}s (started at ${TRAVIS_TIMESTAMP}, current: ${CURRENT_TIME})"

# echo "travis_fold:start:ninja-build.1"
echo "${bold}Building QGIS...${endbold}"
timeout ${TIMEOUT}s ${CTEST_BUILD_COMMAND}
# echo "travis_fold:end:ninja-build.1"

rv=$?
if [ $rv -eq 124 ] ; then
    printf '\n\n${bold}Build and test timeout. Please restart the build for meaningful results.${endbold}\n'
    exit #$rv
fi

# Temporarily uncomment to debug ccache issues
# echo "travis_fold:start:ccache-debug"
# cat /tmp/cache.debug
# echo "travis_fold:end:ccache-debug"

############################
# Restore postgres test data
############################
printf "[qgis_test]\nhost=postgres\nport=5432\ndbname=qgis_test\nuser=docker\npassword=docker" > ~/.pg_service.conf
export PGUSER=docker
export PGHOST=postgres
export PGPASSWORD=docker
export PGDATABASE=qgis_test

pushd /root/QGIS > /dev/null
/root/QGIS/tests/testdata/provider/testdata_pg.sh
popd > /dev/null # /root/QGIS

##########################################################
# Set Postgres server location inside the docker running
# the test. This is for that tests that require
# setting up a custom server
#########################################################
export QGIS_POSTGRES_EXECUTABLE_PATH=/usr/lib/postgresql/10/bin
export QGIS_POSTGRES_SERVER_PORT=55432

###########
# Run tests
###########
python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -E "$(cat /root/QGIS/.ci/travis/linux/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure

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
