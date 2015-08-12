brew tap osgeo/osgeo4mac
brew update
brew install osgeo/osgeo4mac/qgis-28 --only-dependencies
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
