#!/bin/sh
echo Configuring build environment for QGIS
aclocal  \
  && libtoolize --force --copy \
  && automake --add-missing --foreign --copy \
  && autoconf --force \
  && echo Now running configure to configure QGIS \
	&& ./configure
