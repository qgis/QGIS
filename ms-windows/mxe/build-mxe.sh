#!/bin/bash
###########################################################################
#    build-mxe.sh
#    ---------------------
#    Date                 : February 2018
#    Copyright            : (C) 2018 by Alessandro Pasotti
#    Email                : elpaso at itopen dot it
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


set -e

# Usage: you can pass an optional "package" command to skip the build
#        and directly go to the packaging
#        This script needs to be called from the main QGIS directory, the
#        one which contains CMakeLists.txt
#        The artifact will be saved as a zip package in the directory
#        from which this script is launched.


COMMAND=$1

# Location of current script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PYDEPLOY=${DIR}/deploy.py

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Configuration: change this!

# Location of mxe install dir
MXE=${MXE:-/mxe/}

# Directory for build
BUILD_DIR=${PWD}/build-mxe
# Directory where the artifact will be saved
RELEASE_DIR=${PWD}/qgis-mxe-release

# End configuration

# Windows 64 bit with posix threads
TARGET=x86_64-w64-mingw32.shared.posix

# Set base path for all tools
export PATH=${PATH}:/mxe/usr/bin

# Fix CCACHE directory
export CCACHE_DIR=${PWD}/.ccache

if [ ! -e ${CCACHE_DIR} ]; then
  mkdir -p ${CCACHE_DIR}
fi

if [[ "$COMMAND" != *"package"* ]]; then
  [ -d ${BUILD_DIR} ]  && rm -rf ${BUILD_DIR}
  [ -d ${RELEASE_DIR} ] && rm -rf ${RELEASE_DIR}
  # Make sure dirs exist
  [ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}
  [ -d ${RELEASE_DIR} ] || mkdir ${RELEASE_DIR}
fi

pushd .

cd ${BUILD_DIR}

# Build

if [[ "$COMMAND" != *"package"* ]]; then

    ${MXE}/usr/bin/${TARGET}-cmake .. \
        -DCMAKE_BUILD_TYPE=RelWithDebugInfo \
        -DCMAKE_INSTALL_PREFIX=${RELEASE_DIR} \
        -DENABLE_TESTS=OFF \
        -DWITH_QSPATIALITE=ON \
        -DWITH_APIDOC=OFF \
        -DWITH_QWTPOLAR=ON \
        -DWITH_ASTYLE=OFF \
        -DWITH_SERVER=OFF \
        -DWITH_BINDINGS=FALSE \
        -DWITH_QGIS_PROCESS=FALSE \
        -DQT_LRELEASE_EXECUTABLE=${MXE}/usr/${TARGET}/qt5/bin/lrelease \
        -DMXE=ON \
        $ARGS

    make -j16 install

fi

# Collect deps

$PYDEPLOY --build=${RELEASE_DIR} --objdump=${MXE}/usr/bin/${TARGET}-objdump ${RELEASE_DIR}/qgis.exe
for dll in $(ls ${RELEASE_DIR}/*.dll); do \
    $PYDEPLOY --build=${RELEASE_DIR} --objdump=${MXE}/usr/bin/${TARGET}-objdump $dll; \
done

cp -r ${MXE}/usr/${TARGET}/qt5/plugins ${RELEASE_DIR}/qt5plugins

cat <<__TXT__ > ${RELEASE_DIR}/qt.conf
[Paths]
Plugins = qt5plugins
__TXT__

# First cleanup
rm -rf ${BUILD_DIR}
rm -rf ${CCACHE_DIR}

# Make the zip

cd ${RELEASE_DIR}/..
ZIP_NAME=qgis-mxe-release-$(date +%Y-%m-%d-%H-%I-%S).zip
zip -r -m ${ZIP_NAME} $(basename ${RELEASE_DIR})

# Second cleanup
rm -rf ${RELEASE_DIR}

popd

echo "Release in $ZIP_NAME ready."

# vim: et ts=4 :
