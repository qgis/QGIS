brew tap osgeo/osgeo4mac
brew update
brew install osgeo/osgeo4mac/qgis-28 --only-dependencies
brew install doxygen

brew ln bison --force
brew ln sqlite --force
brew ln openssl --force
brew ln expat --force
brew ln libxml2 --force
brew ln gettext --force
brew ln libffi --force

export HOMEBREW_PREFIX=`brew --prefix`
export PATH=${PATH}:${HOMEBREW_PREFIX}/bin:${HOMEBREW_PREFIX}/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/usr/X11/bin
