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
brew install python3
pip3 install psycopg2 numpy nose2 pyyaml mock future

brew install qgis/qgisdev/qgis3-dev --only-dependencies
brew install spawn-fcgi
brew install lighttpd
brew install poppler
brew install bison
brew install expat
brew install gdal2 --with-python3
