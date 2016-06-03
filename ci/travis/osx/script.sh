###########################################################################
#    script.sh
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

echo $PATH

export PATH=/usr/bin:${PATH}

ctest -V -E 'qgis_openstreetmaptest|qgis_wcsprovidertest|PyQgsServer|ProcessingGdalAlgorithmsTest|qgis_composerhtmltest' -S ./qgis-test-travis.ctest --output-on-failure

