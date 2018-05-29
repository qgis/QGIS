#!/usr/bin/env bash

# This runs shellcheck on all sh files

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null
result=$(shellcheck $(find . -name '*.sh'))
popd > /dev/null

if [[ $result ]]; then
  echo " *** shellcheck found script errors"
  echo "$result"
  exit 1
fi
