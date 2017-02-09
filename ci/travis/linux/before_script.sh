###########################################################################
#    before_script.sh
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

printf "[qgis_test]\nhost=localhost\ndbname=qgis_test\nuser=postgres" > ~/.pg_service.conf

export PGUSER=postgres
$TRAVIS_BUILD_DIR/tests/testdata/provider/testdata_pg.sh
