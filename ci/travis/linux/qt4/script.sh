###########################################################################
#    script.sh
#    ---------------------
#    Date                 : March 2016
#    Copyright            : (C) 2016 by Matthias Kuhn
#    Email                : matthias at opengis dot ch
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

export PYTHONPATH=${HOME}/osgeo4travis/lib/python2.7/site-packages/
export PATH=${HOME}/osgeo4travis/bin:${HOME}/osgeo4travis/sbin:${PATH}
export LD_LIBRARY_PATH=${HOME}/osgeo4travis/lib
export CTEST_PARALLEL_LEVEL=1
export CCACHE_CPP2=yes
export CCACHE_TEMPDIR=/tmp
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  export CCACHE_READONLY=yes
  chmod -R ugo-w ~/.ccache
fi

xvfb-run ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest' -S ./qgis-test-travis.ctest --output-on-failure
