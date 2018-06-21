#!/usr/bin/env bash

# This runs shellcheck on all sh files

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null
result=$(shellcheck -e SC2016,SC2015,SC2086,SC2002,SC1117,SC2154,SC2076,SC2046,SC1090,SC2038,SC2031,SC2030,SC2162,SC2196,SC2034,SC2044,SC2119,SC1001,SC2188,SC2119,SC2188,SC2120,SC2103,SC2059,SC2006,SC2221,SC2222,SC2120,SC2128,SC2005,SC2004,SC2013,SC2027,SC2090,SC2089,SC2124,SC2068,SC2035,SC2001,SC2010,SC1072,SC1073,SC1009,SC2166,SC2062,SC2045,SC2028,SC1091,SC2181,SC1083,SC2116,SC2219 $(find . -name '*.sh'))
popd > /dev/null

if [[ $result ]]; then
  echo " *** shellcheck found script errors"
  echo "$result"
  exit 1
fi
