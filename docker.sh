#!/usr/bin/env bash


export TRAVIS_AVAILABLE_TIME=1000000000000000000
export TRAVIS_TIMESTAMP=100000
export TRAVIS_UPLOAD_TIME=0
export TRAVIS_BRANCH=master
export TRAVIS_PULL_REQUEST=
export TRAVIS_OS_NAME=linux
export TRAVIS_CONFIG=linux
export TRAVIS

export DOCKER_TAG=latest
export CCACHE_DIR=/tmp/ccache

export TRAVIS_BUILD_DIR=~/opt/qgis/QGIS


docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}/tests/src/python:/tests_directory -e DISPLAY=:99 qgis/qgis /usr/bin/supervisord -c /etc/supervisor/supervisord.conf

docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/test_runner/. qgis-testing-environment:/usr/bin/
docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor/supervisord.conf qgis-testing-environment:/etc/supervisor/
docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor/supervisor.xvfb.conf qgis-testing-environment:/etc/supervisor/supervisor.d/


echo "Waiting for the docker..."
until [ "`docker inspect -f {{.State.Running}} qgis-testing-environment`"=="true" ]; do
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
  docker exec -it qgis-testing-environment sh -c "cd /tests_directory && qgis_testrunner.sh ${i}"
  [[ $? -eq "${testrunners[$i]}" ]] && echo "success" || exit 1
  echo "travis_fold:end:docker_test_runner_${i}"
done
docker stop qgis-testing-environment
echo "travis_fold:end:docker_test_runners"
