#!/bin/sh
echo Configuring build environment for QGIS
aclocal  \
  && libtoolize --force --copy \
  && automake --add-missing --foreign --copy \
  && autoconf \
  && echo Now run configure to configure QGIS
