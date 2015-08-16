#!/usr/bin/env sh

if [ $# -eq 2 ]; then
  curl -s http://l.md/api/post -F "url=${1}" -F "custom=${2}" | python -c 'import sys, json; print json.load(sys.stdin)["data"]["url"]'
elif [ ! -z $1 ]; then
  curl -s http://l.md/api/post -F "url=${1}" | python -c 'import sys, json; print json.load(sys.stdin)["data"]["url"]'
else
  echo 'Usage: lmd http://example.com/link [custom-name]'
  return 1
fi
