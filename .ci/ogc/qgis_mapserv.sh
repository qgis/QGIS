#!/bin/bash

export QGIS_SERVER_LOG_FILE=/var/log/qgisserver.log
export QGIS_SERVER_LOG_LEVEL=0
export QGIS_PREFIX_PATH=/usr/src/qgis/build/output

exec /usr/bin/spawn-fcgi -n -p 5555 /usr/src/qgis/build/output/bin/qgis_mapserv.fcgi
