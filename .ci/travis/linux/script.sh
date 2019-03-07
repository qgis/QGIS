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

docker run -t --name qgis_container -v ${TRAVIS_BUILD_DIR}:/root/QGIS -v ${CCACHE_DIR}:/root/.ccache qgis/qgis3-build-deps:${DOCKER_TAG} /root/QGIS/.ci/travis/linux/scripts/docker-qgis-build.sh
docker commit qgis_container qgis_image

docker-compose -f ${TRAVIS_BUILD_DIR}/.docker/docker-compose.travis.yml run qgis-deps

 docker run -it qgis-deps

# running QGIS tests
docker-compose -f ${TRAVIS_BUILD_DIR}/.docker/docker-compose.travis.yml run qgis-deps
