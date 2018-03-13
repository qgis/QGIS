#!/usr/bin/env bash

func create_qgis_image(){

if [[ $TRAVIS_REPO_SLUG !~ qgis/QGIS ]]; then
  return false
fi
if [[ $TRAVIS_EVENT_TYPE =~ cron ]] || [[ -n $TRAVIS_TAG ]]; then
  return true;
fi
return false;
}
