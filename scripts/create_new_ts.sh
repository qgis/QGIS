#!/bin/sh

if [ -z "$1" ] ; then
  echo 
  echo "Usage : $(basename $0) [Locale]"
  echo "Examples :"
  echo "$(basename $0) en_GB"
  echo 
  exit 0
fi

$(dirname $0)/update_ts_files.sh -a $1
