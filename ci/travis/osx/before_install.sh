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

# Remove default gdal provided by travis  (we will replace it with gdal 2)
brew remove gdal || true

brew tap osgeo/osgeo4mac
brew update
brew install osgeo/osgeo4mac/qgis-214 --without-postgresql --only-dependencies
brew install spawn-fcgi
brew install lighttpd
brew install poppler

brew ln bison --force
brew ln sqlite --force
brew ln openssl --force
brew ln expat --force
brew ln libxml2 --force
brew ln gettext --force
brew ln libffi --force

mkdir -p ${HOME}/Library/Python/2.7/lib/python/site-packages
echo 'import site; site.addsitedir("/usr/local/lib/python2.7/site-packages")' >> ${HOME}/Library/Python/2.7/lib/python/site-packages/homebrew.pth
echo 'import site; site.addsitedir("/usr/local/opt/gdal-20/lib/python2.7/site-packages")' >> ${HOME}/Library/Python/2.7/lib/python/site-packages/gdal2.pth

# Needed for Processing
pip install psycopg2 numpy nose2 pyyaml mock future
