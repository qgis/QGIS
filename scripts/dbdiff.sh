#!/bin/bash

if [ "$1" = "-h" ]; then
	echo "usage: $0 [-h] [database] [sha1] [sha2]"
	echo "  -h         show this help"
	echo "  database   databases to compare (defaults to resources/srs.db"
	echo "  sha1	   sha of previous state (defaults to HEAD^)"
	echo "  sha2	   sha of next state (defaults to HEAD)"
	exit 1
fi

set -e

db=${1:-resources/srs.db}
prev=${2:-HEAD^}
next=${3:-HEAD}

echo "db:$db prev:$prev next:$next"

git cat-file blob $prev:resources/srs.db >/tmp/prev.db
git cat-file blob $next:resources/srs.db >/tmp/next.db

sqlite3 /tmp/prev.db .dump >/tmp/prev.sql
sqlite3 /tmp/next.db .dump >/tmp/curr.sql

diff -u /tmp/prev.sql /tmp/curr.sql

rm /tmp/prev.db /tmp/next.db /tmp/prev.sql /tmp/curr.sql
