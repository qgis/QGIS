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
echo "travis_fold:start:docker_build_qgis"
echo "${bold}Docker build QGIS${endbold}"
docker run -t --name qgis_container \
           -v ${TRAVIS_BUILD_DIR}:/root/QGIS \
           -v ${CCACHE_DIR}:/root/.ccache \
           --env-file ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-variables.env \
           qgis/qgis3-build-deps:${DOCKER_TAG} \
           /root/QGIS/.ci/travis/linux/scripts/docker-qgis-build.sh

# commit container
docker commit qgis_container qgis_image
echo "travis_fold:end:docker_build_qgis"

# running QGIS tests in commited image
echo "travis_fold:start:docker_test_qgis"
echo "${bold}Docker run tests${endbold}"
#docker-compose -f ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-compose.travis.yml run qgis-deps /root/QGIS/.ci/travis/linux/scripts/docker-qgis-test.sh
echo "travis_fold:end:docker_test_qgis"

# running tests for the python test runner
echo "travis_fold:start:docker_test_runners"
echo "${bold}Docker test QGIS runners${endbold}"
docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}/tests/src/python:/tests_directory -e DISPLAY=:99 qgis_image /usr/bin/supervisord -c /etc/supervisor/supervisord.conf

echo "Waiting for the docker..."
until [ "`/usr/bin/docker inspect -f {{.State.Running}} qgis-testing-environment`"=="true" ]; do
    printf '🐳'
    sleep 0.5;
done;
echo " done 🥩"
sleep 1  # Wait for xvfb to finish starting

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
  echo "travis_fold:start:docker_test_runner_${i}"
  echo "test ${i}..."
  docker exec -it qgis-testing-environment sh -c "cd /tests_directory && /usr/bin/test_runner/qgis_testrunner.sh ${i}"
  [[ $? -eq "${testrunners[$i]}" ]] && echo "success" || exit 1
  echo "travis_fold:end:docker_test_runner_${i}"
done
docker stop qgis-testing-environment
echo "travis_fold:end:docker_test_runners"
