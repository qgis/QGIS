###########################################################################
#    runtests.sh
#    ---------------------
#    Date                 : August 2012
#    Copyright            : (C) 2012 by Victor Olaya
#    Email                : volayaf at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

QGISPATH=/usr/local PYTHONPATH=~/Proyectos/qgis/python/plugins/:~/Proyectos/qgis/output/python/ python test.py $@
