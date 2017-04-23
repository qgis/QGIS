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
    termcolor

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
    gnu-sed

brew link bison --force
brew link flex --force


# Fix qscintilla typo
wget https://gist.githubusercontent.com/m-kuhn/f70e4b160dd7b18eb8d637ed2a75df6d/raw/6eb8a0c8601ec52ad9ad41c01d0fad68fe967aa1/qsci.patch
patch -p1 /usr/local/share/sip/QSci/qscilexer.sip qsci.patch
