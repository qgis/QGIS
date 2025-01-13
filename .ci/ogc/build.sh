#!/bin/bash

set -e

mkdir /usr/src/qgis/build
cd /usr/src/qgis/build || exit 1

export CCACHE_TEMPDIR=/tmp
# Github workflow cache max size is 2.0, but ccache data get compressed (roughly 1/5?)
ccache -M 2.0G

# Temporarily uncomment to debug ccache issues
# export CCACHE_LOGFILE=/tmp/cache.debug
ccache -z

# To make ccache work properly with precompiled headers
ccache --set-config sloppiness=pch_defines,time_macros,include_file_mtime,include_file_ctime

cmake -GNinja \
 -DUSE_CCACHE=ON \
 -DWITH_QUICK=OFF \
 -DWITH_3D=OFF \
 -DWITH_STAGED_PLUGINS=OFF \
 -DWITH_GRASS=OFF \
 -DENABLE_MODELTEST=OFF \
 -DENABLE_PGTEST=OFF \
 -DENABLE_MSSQLTEST=OFF \
 -DENABLE_TESTS=OFF \
 -DWITH_QSPATIALITE=OFF \
 -DWITH_QWTPOLAR=OFF \
 -DWITH_APIDOC=OFF \
 -DWITH_ASTYLE=OFF \
 -DWITH_ANALYSIS=ON \
 -DWITH_GSL=OFF \
 -DWITH_DESKTOP=OFF \
 -DWITH_GUI=OFF \
 -DWITH_BINDINGS=ON \
 -DWITH_SERVER=ON \
 -DWITH_SERVER_PLUGINS=ON \
 -DWITH_ORACLE=OFF \
 -DWITH_PDAL=OFF \
 -DWITH_QTPRINTER=OFF \
 -DDISABLE_DEPRECATED=ON \
 -DCXX_EXTRA_FLAGS="${CLANG_WARNINGS}" \
 -DCMAKE_C_COMPILER=/bin/clang \
 -DCMAKE_CXX_COMPILER=/bin/clang++ \
 -DADD_CLAZY_CHECKS=OFF \
 ..

ninja

echo "ccache statistics"
ccache -s
