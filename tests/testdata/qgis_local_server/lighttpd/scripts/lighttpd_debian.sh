#!/bin/sh
#from init script


PATH=/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=$2
NAME=lighttpd
DESC="web server"
PIDFILE=$3/var/$NAME.pid
SCRIPTNAME=$NAME

export QGIS_SERVER_TEMP_DIR=$3

if [ ! -z $4 ]; then
    DAEMON_OPTS="-f ${4}"
fi

test -x $DAEMON || exit 1

set -e

check_syntax()
{
	$DAEMON -t $DAEMON_OPTS > /dev/null || exit $?
}

. /lib/lsb/init-functions

case "$1" in
    start)
	check_syntax
        log_daemon_msg "Starting $DESC" $NAME
        if ! start-stop-daemon --start --oknodo --quiet \
            --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPTS
        then
            log_end_msg 1
        else
            log_end_msg 0
        fi
        ;;
    stop)
        log_daemon_msg "Stopping $DESC" $NAME
        if start-stop-daemon --stop --retry 30 --oknodo --quiet \
            --pidfile $PIDFILE --exec $DAEMON
        then
            rm -f $PIDFILE
            log_end_msg 0
        else
            log_end_msg 1
        fi
        ;;
    reload|force-reload)
	check_syntax
        log_daemon_msg "Reloading $DESC configuration" $NAME
        if start-stop-daemon --stop --signal INT --quiet \
            --pidfile $PIDFILE --exec $DAEMON
        then
            rm $PIDFILE
            if start-stop-daemon --start --quiet  \
                --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPTS ; then
                log_end_msg 0
            else
                log_end_msg 1
            fi
        else
            log_end_msg 1
        fi
        ;;
    reopen-logs)
        log_daemon_msg "Reopening $DESC logs" $NAME
        if start-stop-daemon --stop --signal HUP --oknodo --quiet \
            --pidfile $PIDFILE --exec $DAEMON
        then
            log_end_msg 0
        else
            log_end_msg 1
        fi
        ;;
    restart)
	check_syntax
        $0 stop $2 $3
        $0 start $2 $3 $4
        ;;
    status)
        status_of_proc -p "$PIDFILE" "$DAEMON" lighttpd && exit 0 || exit $?
        ;;
    *)
        echo "Usage: $SCRIPTNAME {start|stop|restart|reload|force-reload|status}" >&2
        exit 1
        ;;
esac

exit 0
