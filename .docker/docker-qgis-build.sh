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
echo "::group::cmake"

if [[ -f "/usr/lib64/ccache/clang" ]]; then
  export CC=/usr/lib64/ccache/clang
  export CXX=/usr/lib64/ccache/clang++
else
  export CC=/usr/lib/ccache/clang
  export CXX=/usr/lib/ccache/clang++
fi

if [[ ${WITH_QT6} = "ON" ]]; then
  CLANG_WARNINGS="-Wrange-loop-construct"
fi

CMAKE_EXTRA_ARGS=()
if [[ ${PATCH_QT_3D} == "true" ]]; then
  CMAKE_EXTRA_ARGS+=(
    "-DQT5_3DEXTRA_LIBRARY=/usr/lib/x86_64-linux-gnu/libQt53DExtras.so"
    "-DQT5_3DEXTRA_INCLUDE_DIR=/root/QGIS/external/qt3dextra-headers"
    "-DCMAKE_PREFIX_PATH=/root/QGIS/external/qt3dextra-headers/cmake"
    "-DQt53DExtras_DIR=/root/QGIS/external/qt3dextra-headers/cmake/Qt53DExtras"
  )
fi

cmake \
 -GNinja \
 -DUSE_CCACHE=OFF \
 -DWITH_QT6=${WITH_QT6} \
 -DWITH_DESKTOP=${WITH_QT5} \
 -DWITH_ANALYSIS=ON \
 -DWITH_GUI=${WITH_QT5} \
 -DWITH_QUICK=${WITH_QUICK} \
 -DWITH_3D=${WITH_3D} \
 -DWITH_STAGED_PLUGINS=ON \
 -DWITH_GRASS=OFF \
 -DSUPPRESS_QT_WARNINGS=ON \
 -DENABLE_TESTS=ON \
 -DENABLE_MODELTEST=${WITH_QT5} \
 -DENABLE_PGTEST=${WITH_QT5} \
 -DENABLE_SAGA_TESTS=${WITH_QT5} \
 -DENABLE_MSSQLTEST=${WITH_QT5} \
 -DENABLE_HANATEST=${HANA_TESTS_ENABLED} \
 -DENABLE_ORACLETEST=${WITH_QT5} \
 -DPUSH_TO_CDASH=${PUSH_TO_CDASH} \
 -DWITH_HANA=${WITH_QT5} \
 -DWITH_QGIS_PROCESS=ON \
 -DWITH_QSPATIALITE=${WITH_QT5} \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_BINDINGS=${WITH_QT5} \
 -DWITH_SERVER=${WITH_QT5} \
 -DWITH_SERVER_LANDINGPAGE_WEBAPP=${WITH_QT5} \
 -DWITH_ORACLE=${WITH_QT5} \
 -DWITH_PDAL=${WITH_QT5} \
 -DWITH_QT5SERIALPORT=${WITH_QT5} \
 -DWITH_QTWEBKIT=${WITH_QT5} \
 -DWITH_OAUTH2_PLUGIN=${WITH_QT5} \
 -DORACLE_INCLUDEDIR=/instantclient_19_9/sdk/include/ \
 -DORACLE_LIBDIR=/instantclient_19_9/ \
 -DDISABLE_DEPRECATED=ON \
 -DPYTHON_TEST_WRAPPER="timeout -sSIGSEGV 55s" \
 -DCXX_EXTRA_FLAGS="${CLANG_WARNINGS}" \
 -DWERROR=TRUE \
 -DADD_CLAZY_CHECKS=ON \
 ${CMAKE_EXTRA_ARGS[*]} ..
echo "::endgroup::"

#######
# Build
#######
echo "${bold}Building QGIS...${endbold}"
echo "::group::build"
ctest -V -S /root/QGIS/.ci/config_build.ctest
echo "::endgroup::"

########################
# Show ccache statistics
########################
echo "ccache statistics"
ccache -s

popd > /dev/null # build
popd > /dev/null # /root/QGIS

[ -r /tmp/ctest-important.log ] && cat /tmp/ctest-important.log || true
