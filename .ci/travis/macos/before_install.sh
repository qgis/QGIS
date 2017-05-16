#!/usr/bin/env bash
###########################################################################
#    before_install.sh
#    ---------------------
#    Date                 : August 2015
#    Copyright            : (C) 2015 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

date +%s > /tmp/travis_timestamp

brew tap osgeo/osgeo4mac
brew update
brew install python3

pip3 install \
    numpy \
    psycopg2 \
    numpy \
    nose2 \
    pyyaml \
    mock \
    future \
    termcolor \
    oauthlib

brew install \
    qscintilla2 \
    qt \
    qt5-webkit \
    qca \
    qwtpolar \
    gsl \
    sqlite \
    expat \
    proj \
    gdal2-python --with-python3 \
    spawn-fcgi \
    lighttpd \
    poppler \
    bison \
    expat \
    bison \
    flex \
    ninja \
    ccache \
    spatialindex \
    fastcgi \
    qtkeychain \
    gnu-sed \
    libzip


mkdir -p "${HOME}/Library/Python/3.6/lib/python/site-packages"
echo 'import site; site.addsitedir("/usr/local/opt/gdal2-python/lib/python3.6/site-packages")'  >> "${HOME}/Library/Python/3.6/lib/python/site-packages/gdal2.pth"
