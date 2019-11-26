#!/bin/bash
# Cross-build locally for Windows using mingw
# This scripts needs to be run from QGIS repository top-level directory

# Location of current script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

docker run \
    -v $(pwd):$(pwd) \
    mingw-buildenv \
    ${DIR}/build.sh x86_64 nodebug $(nproc)

pushd .
cd build_mingw64/dist/usr/x86_64-w64-mingw32/sys-root/
zip -r qgis-mingw-`date +%Y-%m-%d`.zip mingw
popd
mv build_mingw64/dist/usr/x86_64-w64-mingw32/sys-root/qgis-mingw-`date +%Y-%m-%d`.zip .
