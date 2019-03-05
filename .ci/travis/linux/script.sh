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

# running QGIS tests
docker-compose -f ${TRAVIS_BUILD_DIR}/.docker/docker-compose.travis.yml run --rm qgis-deps

# running tests for the python test runner
docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}/tests/src/python:/tests_directory -e DISPLAY=:99 "qgis/qgis:${DOCKER_TAG}"
sleep 10  # Wait for xvfb to finish starting
# Temporary workaround until docker images are built
docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/test_runner/qgis_testrunner.sh qgis-testing-environment:/usr/bin/qgis_testrunner.sh
# Run tests in the docker
# Passing cases:
TEST_SCRIPT_PATH=${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker_test.sh
[[ $(${TEST_SCRIPT_PATH} test_testrunner.run_passing) -eq '0' ]]
[[ $(${TEST_SCRIPT_PATH} test_testrunner.run_skipped_and_passing) -eq '0' ]]
# Failing cases:
[[ $(${TEST_SCRIPT_PATH} test_testrunner) -eq '1' ]]
[[ $(${TEST_SCRIPT_PATH} test_testrunner.run_all) -eq '1' ]]
[[ $(${TEST_SCRIPT_PATH} test_testrunner.run_failing) -eq '1' ]]

