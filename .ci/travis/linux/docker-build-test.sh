#!/usr/bin/env bash

set -e

##############
# Setup ccache
##############
export CCACHE_TEMPDIR=/tmp
ccache -M 500M
ccache -z

############################
# Setup the (c)test environment
############################
export LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so
export CTEST_BUILD_COMMAND="/usr/bin/ninja"
export CTEST_PARALLEL_LEVEL=1

###########
# Configure
###########
pushd /root/QGIS
mkdir -p build

pushd build

cmake \
 -GNinja \
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
 -DDISABLE_DEPRECATED=ON \
 -DCXX_EXTRA_FLAGS=${CLANG_WARNINGS} ..

#######
# Build
#######
echo "travis_fold:start:ninja-build.1"
ninja
echo "travis_fold:end:ninja-build.1"

############################
# Restore postgres test data
############################
printf "[qgis_test]\nhost=postgres\nport=5432\ndbname=qgis_test\nuser=docker\npassword=docker" > ~/.pg_service.conf
export PGUSER=docker
export PGHOST=postgres
export PGPASSWORD=docker
export PGDATABASE=qgis_test

pushd /root/QGIS
/root/QGIS/tests/testdata/provider/testdata_pg.sh
popd # /root/QGIS

###########
# Run tests
###########
python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -E "$(cat /root/QGIS/.ci/travis/linux/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure

########################
# Show ccache statistics
########################
ccache -s

popd # build
popd # /root/QGIS

[ -r /tmp/ctest-important.log ] && cat /tmp/ctest-important.log
