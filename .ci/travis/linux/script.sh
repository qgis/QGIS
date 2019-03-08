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

# build QGIS in docker
docker run -t --name qgis_container \
           -v ${TRAVIS_BUILD_DIR}:/root/QGIS \
           -v ${CCACHE_DIR}:/root/.ccache \
          /root/qgis_test_runner --env-file ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-variables.env \
           qgis/qgis3-build-deps:${DOCKER_TAG} \
           /root/QGIS/.ci/travis/linux/scripts/docker-qgis-build.sh

# commit container
docker commit qgis_container qgis_image

# running QGIS tests in commited image
docker-compose -f ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-compose.travis.yml run qgis-deps /root/QGIS/.ci/travis/linux/scripts/docker-qgis-test.sh

# running tests for the python test runner
docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}/tests/src/python:/tests_directory -e DISPLAY=:99 qgis_image "/usr/bin/supervisord -c /etc/supervisor/supervisord.conf"
sleep 10  # Wait for xvfb to finish starting

declare -A testrunners
# Passing cases:
testrunners["test_testrunner.run_passing"]=0
testrunners["test_testrunner.run_skipped_and_passing"]=0
# Failing cases:
testrunners["test_testrunner"]=1
testrunners["test_testrunner.run_all"]=1
testrunners["test_testrunner.run_failing"]=1
# Run tests in the docker
for i in "${!testrunners[@]}"
do
  echo "test ${i}..."
  [[ $(docker exec -it qgis-testing-environment sh -c "cd /tests_directory && qgis_testrunner.sh ${i}" &>/dev/null) -eq "${testrunners[$i]}" ]] && echo "success" || exit 1
done
