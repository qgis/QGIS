#! /bin/bash

PROCESS="mapserv"
LABEL="org.qgis.test-${PROCESS}"
USAGE="${0} {stop|status} or {start|restart spawn_bin fcgi_socket fcgi_bin qgis_server_temp_dir}"

if [ ! -z $2 ]; then
  SPAWN_BIN=$2
  if [ ! -z $3 ]; then
    FCGI_SOCKET=$3
  fi
  if [ ! -z $4 ]; then
    FCGI_BIN=$4
  fi
  if [ ! -z $5 ]; then
    QGIS_SERVER_TEMP_DIR=$5
  fi
fi

START () {
  # QGIS_LOG_FILE /test-projects/qgis_wms_server.log
  launchctl setenv QGIS_LOG_FILE "${QGIS_SERVER_TEMP_DIR}/log/qgis_server.log"
  launchctl submit -l $LABEL -- "${SPAWN_BIN}" -n -s "${FCGI_SOCKET}" -- "${FCGI_BIN}"
  
#   QGIS_LOG_FILE="${QGIS_SERVER_TEMP_DIR}/log/qgis_server.log"
#   launchctl submit -l $LABEL -- exec env - QGIS_LOG_FILE=${QGIS_LOG_FILE} "${SPAWN_BIN}" -n -s "${FCGI_SOCKET}" -- "${FCGI_BIN}"
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
	  launchctl unsetenv QGIS_LOG_FILE
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

