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

.ci/travis/scripts/echo_travis_var.sh

pushd .docker



echo "travis_fold:start:docker_build"
echo "${bold}Docker build deps${endbold}"
docker --version

docker pull "qgis/qgis3-build-deps:${DOCKER_TAG}" || true
docker build --cache-from "qgis/qgis3-build-deps:${DOCKER_TAG}" -t "qgis/qgis3-build-deps:${DOCKER_TAG}" -f ${DOCKER_BUILD_DEPS_FILE} .

echo "travis_fold:end:docker_build"


popd
