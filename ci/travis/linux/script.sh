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

export PYTHONPATH=${HOME}/osgeo4travis/lib/python3.3/site-packages/
export PATH=${HOME}/osgeo4travis/bin:${HOME}/osgeo4travis/sbin:${HOME}/OTB-5.6.0-Linux64/bin:${PATH}
export LD_LIBRARY_PATH=${HOME}/osgeo4travis/lib
export CTEST_PARALLEL_LEVEL=1
export CCACHE_TEMPDIR=/tmp

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Set OTB application path (installed in before_install.sh script)
export OTB_APPLICATION_PATH=${HOME}/OTB-5.6.0-Linux64/lib/otb/applications

echo "ls -al /home/travis/virtualenv/python3.3.6/bin/python3"
ls -al /home/travis/virtualenv/python3.3.6/bin/python3

/home/travis/virtualenv/python3.3.6/bin/python3 -m PyQt5.pyrcc_main ../python/plugins/GdalTools/resources.qrc

xvfb-run ctest -V -E "qgis_filedownloader|qgis_openstreetmaptest|qgis_wcsprovidertest|PyQgsWFSProviderGUI|qgis_ziplayertest|$(cat ${DIR}/blacklist.txt | paste -sd '|' -)" -S ./qgis-test-travis.ctest --output-on-failure
# xvfb-run ctest -V -E "qgis_openstreetmaptest|qgis_wcsprovidertest" -S ./qgis-test-travis.ctest --output-on-failure
