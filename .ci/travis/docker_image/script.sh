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

# test if ccache dir exists (coming from Travis cache)
[[ -d ${CCACHE_DIR} ]] && echo "cache directory (${CCACHE_DIR}) exists" || mkdir -p "${CCACHE_DIR}"

# copy ccache dir within QGIS source so it can be accessed from docker
cp -r ${CCACHE_DIR}/. ${TRAVIS_BUILD_DIR}/.ccache_image_build

echo "Cache directory size: "$(du -h --max-depth=0 ${TRAVIS_BUILD_DIR}/.ccache_image_build)

# calculate timeouts
CURRENT_TIME=$(date +%s)
TIMEOUT=$((( TRAVIS_AVAILABLE_TIME - TRAVIS_UPLOAD_TIME ) * 60 - CURRENT_TIME + TRAVIS_TIMESTAMP))
#TIMEOUT=$(( TIMEOUT < 300 ? 300 : TIMEOUT ))
echo "Timeout: ${TIMEOUT}s"

# building docker images
pushd "${TRAVIS_BUILD_DIR}/.docker"
echo "${bold}Building QGIS Docker image '${DOCKER_TAG}'...${endbold}"
DOCKER_BUILD_ARGS="--build-arg DOCKER_TAG=${DOCKER_TAG} \
                   --build-arg BUILD_TIMEOUT=${TIMEOUT} \
                   --build-arg CC --build-arg CXX"
docker build ${DOCKER_BUILD_ARGS} \
             --cache-from "qgis/qgis:${DOCKER_TAG}" \
             -t "qgis/qgis:BUILDER" \
             -f qgis.dockerfile ..

docker run --name qgis_container qgis/qgis:BUILDER /bin/true

echo "Copy build cache from Docker container to Travis cache directory"
rm -rf "${CCACHE_DIR:?}/"*
mkdir -p ${CCACHE_DIR}
docker cp qgis_container:/QGIS/.ccache_image_build/. ${CCACHE_DIR}

docker cp qgis_container:/QGIS/build_exit_value ${HOME}/build_exit_value

if [[ $(cat ${HOME}/build_exit_value) == "TIMEOUT" ]]; then
  echo "Build timeout, not pushing image or triggering PyQGIS docs"
  exit 1
else
  echo "${bold}Finalize image…${endbold}"
  # enable experimental features in Docker to squash
  echo '{ "experimental": true}' | sudo tee /etc/docker/daemon.json
  sudo service docker restart
  docker build ${DOCKER_BUILD_ARGS} \
             --cache-from "qgis/qgis:BUILDER" \
             --squash \
             -t "qgis/qgis:${DOCKER_TAG}" \
             -f qgis.dockerfile ..

  echo "${bold}Pushing image to docker hub…${endbold}"
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  docker push "qgis/qgis:${DOCKER_TAG}"

  echo "Trigger build of PyQGIS Documentation…"
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
fi
