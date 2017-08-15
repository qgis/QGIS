#!/usr/bin/env bash

#set -e

# locale-gen en_US.UTF-8
# export LANG=en_US.UTF-8
# export LANGUAGE=en_US:en
# export LC_ALL=en_US.UTF-8

export CTEST_PARALLEL_LEVEL=1
export CCACHE_TEMPDIR=/tmp
ccache -M 500M
ccache -z

cd /root/QGIS

#sleep 20

printf "[qgis_test]\nhost=postgres\nport=5432\ndbname=qgis_test\nuser=docker\npassword=docker" > ~/.pg_service.conf
export PGUSER=docker
export PGHOST=postgres
export PGPASSWORD=docker
export PGDATABASE=qgis_test

# export PYTHONIOENCODING="utf-8"

/root/QGIS/tests/testdata/provider/testdata_pg.sh

mkdir -p build-docker &&

pushd build-docker

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

export LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so
export CTEST_BUILD_COMMAND="/usr/bin/ninja"

ls -la --full-time python/plugins/processing/tests/testdata/expected/polys_centroid.*

ninja

python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -R ProcessingQgisAlgorithmsTest -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure

find / -iname "*.shp"

#python3 /root/QGIS/.ci/travis/scripts/ctest2travis.py xvfb-run ctest -V -E "$(cat /root/QGIS/.ci/travis/linux/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" -S /root/QGIS/.ci/travis/travis.ctest --output-on-failure

ccache -s

popd


[ -r /tmp/ctest-important.log ] && cat /tmp/ctest-important.log
