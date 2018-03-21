#!/usr/bin/env bash

create_qgis_image () {

if [[ ! $TRAVIS_REPO_SLUG =~ qgis/QGIS ]]; then
  echo false
  return
fi
if [[ $TRAVIS_EVENT_TYPE =~ cron ]] || [[ -n $TRAVIS_TAG ]]; then
  echo true
  return
fi
echo false
}
