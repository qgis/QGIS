#!/bin/bash

################################################################################
# 2013-09-01  Larry Shaffer <larrys@dakotacarto.com>
#
# NOTE: THIS IS ONLY A DEVELOPMENT TESTING SCRIPT. TO BE REMOVED AT LATER DATE.
################################################################################

/usr/local/bin/spawn-fcgi -p 8448 -u $UID -- /qgisserver/qgis_mapserv.fcgi


