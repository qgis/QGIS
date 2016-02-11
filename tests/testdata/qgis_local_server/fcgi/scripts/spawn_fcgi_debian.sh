#!/bin/sh
###########################################################################
#    spawn_fcgi_debian.sh
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

#culled from lighttpd init script


PATH=/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=$2
NAME=spawn-fcgi
DESC="$NAME"
TEMPDIR=$5
FCGI=qgis_mapserv
PIDFILE=$TEMPDIR/var/$NAME.pid
FCGISOCKET=$3
FCGIBIN=$4
SCRIPTNAME=$NAME


test -x $DAEMON || exit 1

set -e

. /lib/lsb/init-functions

case "$1" in
    start)
        log_daemon_msg "Starting $DESC" $NAME
        if ! start-stop-daemon --start --oknodo --quiet \
            --pidfile $PIDFILE -m -b --exec $DAEMON -- \
            -n -C 0 -s $FCGISOCKET -f $FCGIBIN
        then
            log_end_msg 1
        else
            log_end_msg 0
        fi
        ;;
    stop)
        log_daemon_msg "Stopping $DESC" $NAME
        # this would also kill any other qgis_mapserv.fcgi running; not good
        #if killall --signal 2 $FCGI.fcgi > /dev/null 2> /dev/null
        # we can get around this because there should only be 1 process when testing
        if kill -s 9 $(cat $PIDFILE) > /dev/null 2> /dev/null
        then
            rm -f $PIDFILE $FCGISOCKET
            log_end_msg 0
        else
            log_end_msg 1
        fi
        ;;
    restart)
        $0 stop $2 $3 $4 $5
        $0 start $2 $3 $4 $5
        ;;
    status)
		    status_of_proc -p "$PIDFILE" "$DAEMON" spawn-fcgi && exit 0 || exit $?
        ;;
    *)
        echo "Usage: $SCRIPTNAME {start|stop|restart|status}" >&2
        exit 1
        ;;
esac

exit 0
