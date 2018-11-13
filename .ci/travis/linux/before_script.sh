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

pushd .docker

source $(git rev-parse --show-toplevel)/.ci/travis/scripts/travis_envvar_helper.sh


DOCKER_DEPS_PUSH=$( [[ $TRAVIS_REPO_SLUG =~ qgis/QGIS ]] && [[ $TRAVIS_EVENT_TYPE =~ push ]] && echo "true" || echo "false" )
DOCKER_DEPS_IMAGE_REBUILD=$( [[ $TRAVIS_COMMIT_MESSAGE =~ '[docker] update dependencies' ]] && echo "true" || echo "false" )
# on cron job, QGIS image is built and push without testing
DOCKER_QGIS_IMAGE_BUILD_PUSH=$(create_qgis_image)
QGIS_LAST_BUILD_SUCCESS=true # TODO use API to know if last build succeed https://developer.travis-ci.com/resource/builds


echo "travis_fold:start:travis_env"
echo "${bold}Travis environment variables${endbold}"
echo "TRAVIS_BRANCH: $TRAVIS_BRANCH"
echo "TRAVIS_EVENT_TYPE: $TRAVIS_EVENT_TYPE"
echo "DOCKER_TAG: $DOCKER_TAG"
echo "TRAVIS_COMMIT_MESSAGE: $TRAVIS_COMMIT_MESSAGE"
echo "DOCKER_DEPS_PUSH: $DOCKER_DEPS_PUSH"
echo "DOCKER_DEPS_IMAGE_REBUILD: $DOCKER_DEPS_IMAGE_REBUILD"
echo "DOCKER_QGIS_IMAGE_BUILD_PUSH: $DOCKER_QGIS_IMAGE_BUILD_PUSH"
echo "QGIS_LAST_BUILD_SUCCESS: $QGIS_LAST_BUILD_SUCCESS"
echo "TRAVIS_TIMESTAMP: $TRAVIS_TIMESTAMP"
echo "travis_fold:end:travis_env"


echo "travis_fold:start:docker_build"
echo "${bold}Docker build deps${endbold}"
docker --version
if [[ $DOCKER_QGIS_IMAGE_BUILD_PUSH =~ false ]]; then
  docker-compose --version
  docker-compose -f "${DOCKER_COMPOSE}" config
fi

if [[ $DOCKER_DEPS_IMAGE_REBUILD =~ true ]]; then
  docker build --no-cache -t "qgis/qgis3-build-deps:${DOCKER_TAG}" -f qgis3-build-deps.dockerfile .
else
  docker pull "qgis/qgis3-build-deps:${DOCKER_TAG}" || true
  docker build --cache-from "qgis/qgis3-build-deps:${DOCKER_TAG}" -t "qgis/qgis3-build-deps:${DOCKER_TAG}" -f qgis3-build-deps.dockerfile .
fi
echo "travis_fold:end:docker_build"

echo "travis_fold:start:docker_push"
echo "${bold}Docker push deps${endbold}"
# image should be pushed even if QGIS build fails
# but push is achieved only on branches (not for PRs)
if [[ $DOCKER_DEPS_PUSH =~ true ]]; then
  echo "push to qgis/qgis3-build-deps:${DOCKER_TAG}"
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  #docker tag "qgis/qgis3-build-deps:${DOCKER_TAG}" "qgis/qgis3-build-deps:latest"
  docker push "qgis/qgis3-build-deps:${DOCKER_TAG}"
fi
echo "travis_fold:end:docker_push"

popd
