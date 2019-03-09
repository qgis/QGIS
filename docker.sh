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

export TRAVIS_BUILD_DIR=~/opt/qgis/docker_tree

#docker rm -f qgis_container || true

## build QGIS in docker
#echo "travis_fold:start:docker_build_qgis"
#echo "${bold}Docker build QGIS${endbold}"
#docker run -t --name qgis_container \
#           -v ${TRAVIS_BUILD_DIR}:/root/QGIS \
#           -v ${CCACHE_DIR}:/root/.ccache \
#           --env-file ${TRAVIS_BUILD_DIR}/.ci/travis/linux/docker-variables.env \
#           qgis/qgis3-build-deps:${DOCKER_TAG} \
#           /root/QGIS/.ci/travis/linux/scripts/docker-qgis-build.sh

#docker commit qgis_container qgis_image

docker stop  qgis-testing-environment || true
docker rm  -f qgis-testing-environment || true

echo "hello"

# -v ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor:/etc/supervisor \

docker run -d --name qgis-testing-environment \
           -v ${TRAVIS_BUILD_DIR}:/root/QGIS \
           -v ${TRAVIS_BUILD_DIR}/tests/src/python:/tests_directory \
           -v ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/test_runner:/usr/bin/test_runner \
           -v ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor:/etc/supervisor \
           -e QGIS_BUILD_PATH=/root/QGIS/build/output/bin/qgis \
           -e TEST_RUNNER_PATH=/usr/bin/test_runner/qgis_testrunner.py \
           -e DISPLAY=:99 \
           qgis_image \
           /usr/bin/supervisord -c /etc/supervisor/supervisord.conf


echo "lol"


#docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor/. qgis-testing-environment:/etc/supervisor/
#docker cp ${TRAVIS_BUILD_DIR}/.docker/qgis_resources/supervisor/supervisor.xvfb.conf qgis-testing-environment:/etc/supervisor/supervisor.d/


printf "Waiting for the docker...🐳..."
sleep 10  # Wait for xvfb to finish starting
echo " done 🥩"


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
  if [[ $? -eq "${testrunners[$i]}" ]]; then
    echo "success"
  else
    echo "exiting"
    exit 1
  fi
  echo "travis_fold:end:docker_test_runner_${i}"
done
#docker stop qgis-testing-environment
echo "travis_fold:end:docker_test_runners"
