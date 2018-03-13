#!/usr/bin/env bash

function create_qgis_image {

if [[ ! $TRAVIS_REPO_SLUG =~ qgis/QGIS ]]; then
  return 0
fi
if [[ $TRAVIS_EVENT_TYPE =~ cron ]] || [[ -n $TRAVIS_TAG ]]; then
  return 1;
fi
return 0;
}
