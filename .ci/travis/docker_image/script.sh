#!/usr/bin/env bash
###########################################################################
#    before_install.sh
#    ---------------------
#    Date                 : March 2019
#    Copyright            : (C) 2019 by Denis Rouzaud
#    Email                : denis@opengis.ch
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


set -e

mkdir -p "${CCACHE_DIR}"

# copy ccache dir within QGIS source so it can be accessed from docker
cp -r ${CCACHE_DIR} ${TRAVIS_BUILD_DIR}/.ccache_image_build
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
echo "Copy build cache from Docker container to Travis cache directory"
rm -rf "${CCACHE_DIR:?}/"*
docker run --name qgis_container qgis/qgis:${DOCKER_TAG} /bin/true
docker cp qgis_container:/usr/src/.ccache_image_build ${CCACHE_DIR}
docker rm qgis_container
popd
echo "Trigger build of PyQGIS Documentation"
if [[ ${TRIGGER_PYQGIS_DOC} =~ ^TRUE$ ]]; then
  body='{
    "request": {
      "branch":"master",
      "message": "Trigger PyQGIS doc build after release of new Docker image as __DOCKER_TAG__",
      "config": {
        "merge_mode": "deep_merge",
        "matrix": {
          "include": {
            "env": ["QGIS_VERSION_BRANCH=__QGIS_VERSION_BRANCH__"]
          }
        }
      }
    }
  }'
  body=$(sed "s/__QGIS_VERSION_BRANCH__/${TRAVIS_BRANCH}/; s/__DOCKER_TAG__/${DOCKER_TAG}/" <<< $body)
  curl -s -X POST -H "Content-Type: application/json" -H "Accept: application/json" \
    -H "Travis-API-Version: 3" -H "Authorization: token $TRAVIS_TOKEN" -d "$body" \
    https://api.travis-ci.org/repo/qgis%2Fpyqgis/requests
else
  echo "skipped from configuration"
fi
