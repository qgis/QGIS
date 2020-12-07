#!/usr/bin/env bash
###########################################################################
#    before_script.sh
#    ---------------------
#    Date                 : August 2015
#    Copyright            : (C) 2015 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

DOCKER_DEPS_PUSH=$( [[ $TRAVIS_REPO_SLUG =~ qgis/QGIS ]] && [[ "${TRAVIS_EVENT_TYPE}" != "pull_request" ]] && echo "true" || echo "false" )

.ci/travis/scripts/echo_travis_var.sh

pushd .docker

echo "travis_fold:start:docker_build"
echo "${bold}Docker build deps${endbold}"
docker --version
docker-compose --version
docker-compose -f ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-compose.travis.yml config
docker pull "qgis/qgis3-build-deps:${DOCKER_TAG}" || true
docker build --cache-from "qgis/qgis3-build-deps:${DOCKER_TAG}" -t "qgis/qgis3-build-deps:${DOCKER_TAG}" -f ${DOCKER_BUILD_DEPS_FILE} .
echo "travis_fold:end:docker_build"

echo "travis_fold:start:docker_push"
echo "${bold}Docker push deps${endbold}"
# image should be pushed even if QGIS build fails
# but push is achieved only on branches (not for PRs)
if [[ $DOCKER_DEPS_PUSH =~ true ]]; then
  echo "push to qgis/qgis3-build-deps:${DOCKER_TAG}"
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  docker push "qgis/qgis3-build-deps:${DOCKER_TAG}"
fi
echo "travis_fold:end:docker_push"

popd
