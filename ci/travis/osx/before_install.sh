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

brew install python3
pip3 install psycopg2 numpy nose2 pyyaml mock future ninja

# Temporary workaround
brew install m-kuhn/qgisdev/qgis3-dev --only-dependencies
brew install spawn-fcgi lighttpd poppler bison expat ccache

ln -s /usr/local/opt/qt5-webkit/Frameworks/QtWebKit.framework /usr/local/Cellar/qt5/5.7.1_1/lib/QtWebKit.framework
