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

docker --version
docker-compose --version
docker-compose -f $DOCKER_COMPOSE config
#docker pull ubuntu:16.04
docker pull "qgis/qgis3-build-deps:${DOCKER_TAG}" || true
docker build --cache-from "qgis/qgis3-build-deps:${DOCKER_TAG}" -t "qgis/qgis3-build-deps:${DOCKER_TAG}" .
# image should be pushed even if QGIS build fails
# but push is achieved only on branches (not for PRs)
if [[ $DOCKER_PUSH =~ true ]]; then
  docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD"
  #docker tag "qgis/qgis3-build-deps:${DOCKER_TAG}" "qgis/qgis3-build-deps:latest"
  docker push "qgis/qgis3-build-deps:${DOCKER_TAG}"
fi

popd
