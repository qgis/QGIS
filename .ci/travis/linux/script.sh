#!/usr/bin/env bash
###########################################################################
#    script.sh
#    ---------------------
#    Date                 : March 2016
#    Copyright            : (C) 2016 by Matthias Kuhn
#    Email                : matthias at opengis dot ch
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

mkdir -p "$CCACHE_DIR"

if [[ ${DOCKER_BUILD_QGIS_IMAGE} =~ true ]]; then
  # copy ccache dir within QGIS source so it can be accessed from docker
  cp -r ${CCACHE_DIR} ${TRAVIS_BUILD_DIR}/.ccache
  # building docker images
  DIR=$(git rev-parse --show-toplevel)/.docker
  pushd "${DIR}"
  echo "${bold}Building QGIS Docker image '${DOCKER_TAG}'...${endbold}"
  docker build --build-arg DOCKER_TAG="${DOCKER_TAG}" \
               --cache-from "qgis/qgis:${DOCKER_TAG}" \
               -t "qgis/qgis:${DOCKER_TAG}" \
               -f qgis.dockerfile ..
  echo "${bold}Pushing image to docker hub...${endbold}"
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  docker push "qgis/qgis:${DOCKER_TAG}"
  popd
  echo "Trigger build of PyQGIS Documentation"
  body='{
    "request": {
      "branch":"master",
      "message": "Trigger PyQGIS doc build after release of new Docker image",
      "config": {
        "merge_mode": "deep_merge",
        "env": {
          "global": ["QGIS_VERSION_BRANCH=release-3_4"]
        }
      }
    }
  }'
  curl -s -X POST -H "Content-Type: application/json" -H "Accept: application/json" \
    -H "Travis-API-Version: 3" -H "Authorization: token $TRAVIS_TOKEN" -d "$body" \
    https://api.travis-ci.org/repo/qgis%2Fpyqgis/requests
else
  # running tests
  docker-compose -f ${TRAVIS_BUILD_DIR}/.docker/docker-compose.travis.yml run --rm qgis-deps
fi
