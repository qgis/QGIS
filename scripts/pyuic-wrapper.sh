#!/bin/sh
###########################################################################
#    pyuic-wrapper.sh
#    ---------------------
#    Date                 : March 2016
#    Copyright            : (C) 2016 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


PYUIC4=$1
LD_LIBRARY_PATH=$2:$LD_LIBRARY_PATH
PYTHONPATH=$3:$PYTHONPATH
PYTHON=$4
shift 4

export LD_LIBRARY_PATH PYTHONPATH

exec $PYTHON $(dirname $0)/pyuic-wrapper.py $@
