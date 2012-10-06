#!/bin/sh
###########################################################################
#    create_new_ts.sh
#    ---------------------
#    Date                 : January 2008
#    Copyright            : (C) 2008 by Tim Sutton
#    Email                : tim dot linfiniti at com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


if [ -z "$1" ] ; then
  echo 
  echo "Usage : $(basename $0) [Locale]"
  echo "Examples :"
  echo "$(basename $0) en_GB"
  echo 
  exit 0
fi

$(dirname $0)/update_ts_files.sh -a $1
