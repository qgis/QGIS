#! /bin/bash
###########################################################################
#    lighttpd_mac.sh
#    ---------------------
#    Date                 : February 2014
#    Copyright            : (C) 2014 by Larry Shaffer
#    Email                : larrys at dakotacarto dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


PROCESS="lighttpd"
LABEL="org.qgis.test-${PROCESS}"
USAGE="${0} {stop|status} or {start|restart lighttpd-path lighttpd_conf qgis_server_temp_dir}"

if [ ! -z $2 ]; then
  LIGHTTPD_BIN=$2
  if [ ! -z $3 ]; then
    LIGHTTPD_CONF_PATH=$3
  fi
  if [ ! -z $4 ]; then
    QGIS_SERVER_TEMP_DIR=$4
  fi
fi

START () {
  launchctl setenv QGIS_SERVER_TEMP_DIR "${QGIS_SERVER_TEMP_DIR}"
  launchctl submit -l $LABEL -- "${LIGHTTPD_BIN}" -D -f "${LIGHTTPD_CONF_PATH}"
  return $?
}

STATUS () {
  launchctl list $LABEL 2>&1 | grep -c 'unknown response'
}

case $1 in
    start)
	echo -n "Starting ${PROCESS}"
	res=$(STATUS)
	if [ $res -gt 0 ]; then
    echo ""
    res=$(START)
    exit $res
  else
    echo ": already running"
    exit 0
  fi
	;;
    stop)
	echo -n "Stopping ${PROCESS}"
	res=$(STATUS)
	if [ $res -eq 0 ]; then
	  echo ""
    launchctl unsetenv QGIS_TEST_TEMP_DIR
    launchctl remove $LABEL
    exit $?
	else
	  echo ": not running"
    exit 0
	fi
	;;
    restart)
  echo -n "Restarting ${PROCESS}"
  # using `launchctl submit` sets the KeepAlive=true for submitted process, 
  # i.e. auto-restarted on stop
  res=$(STATUS)
	if [ $res -eq 0 ]; then
    echo ""
    launchctl stop $LABEL
    exit $?
  else
    echo ": not running, starting now"
    res=$(START)
    exit $res
  fi
	;;
    status)
	echo -n "Service ${LABEL}: "
	# error when not in list: launchctl list returned unknown response
	# NOTE: success does not mean process is properly running, just that its service is loaded
	res=$(STATUS)
	if [ $res -gt 0 ]; then
	  echo "unloaded"
	else
	  echo "loaded"
	fi
	exit $res
	;;
	  *)
	echo -e "Usage:\n  ${USAGE}"
	exit 1
	;;
esac

