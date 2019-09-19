#!/usr/bin/env bash

echo "travis_fold:start:travis_env"
echo "${bold}Travis environment variables${endbold}"
echo "TRAVIS_BRANCH: $TRAVIS_BRANCH"
echo "TRAVIS_EVENT_TYPE: $TRAVIS_EVENT_TYPE"
echo "DOCKER_TAG: $DOCKER_TAG"
echo "TRAVIS_COMMIT_MESSAGE: $TRAVIS_COMMIT_MESSAGE"
echo "DOCKER_BUILD_DEPS_FILE: $DOCKER_BUILD_DEPS_FILE"
echo "TRAVIS_TIMESTAMP: $TRAVIS_TIMESTAMP"
echo "travis_fold:end:travis_env"
