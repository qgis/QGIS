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

echo "travis_fold:start:docker"

docker --version

if [[ $DOCKER_QGIS_IMAGE_BUILD_PUSH =~ false ]]; then
  docker-compose --version
  docker-compose -f $DOCKER_COMPOSE config
fi

docker pull "qgis/qgis3-build-deps:${DOCKER_TAG}" || true

if [[ $DOCKER_DEPS_IMAGE_REBUILD =~ true ]]; then
  docker build --no-cache -t "qgis/qgis3-build-deps:${DOCKER_TAG}" .
else
  docker build --cache-from "qgis/qgis3-build-deps:${DOCKER_TAG}" -t "qgis/qgis3-build-deps:${DOCKER_TAG}" -f qgis3-build-deps.dockerfile .
fi

echo "travis_fold:end:docker"

# image should be pushed even if QGIS build fails
# but push is achieved only on branches (not for PRs)
if [[ $DOCKER_DEPS_PUSH =~ true ]]; then
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  #docker tag "qgis/qgis3-build-deps:${DOCKER_TAG}" "qgis/qgis3-build-deps:latest"
  docker push "qgis/qgis3-build-deps:${DOCKER_TAG}"
fi

popd
