#!/bin/sh

set -e

# check for docker-compose and docker availability
command -v docker > /dev/null || {
	echo "Please install docker" >&2
	exit 1
}
command -v docker-compose > /dev/null || {
	echo "Please install docker-compose" >&2
	exit 1
}

IMAGE_BUILD_DEPS=qgis/qgis3-build-deps:latest
UPDATE_IMAGES=yes
INTERACTIVE=no
FORCE_REBUILD=no
export QT_VERSION=5 # TODO: ask user for this one
export DISTRO_VERSION=21.10 # TODO: ask user for this one
# can be: ALL, ALL_BUT_PROVIDERS, POSTGRES, HANA, ORACLE, SQLSERVER
TESTS_TO_RUN=ALL_BUT_PROVIDERS # TODO: ask user for this one

usage() {
  echo "Usage: $(basename $0) [--skip-update-images] [--force-rebuild] [--interactive]"
}

while test -n "$1"; do
  if test "$1" = '--help' || test "$1" = '-h'; then
    usage
    exit 0
  elif test "$1" = '--skip-update-images'; then
    UPDATE_IMAGES=no
    shift
  elif test "$1" = '--force-rebuild'; then
    FORCE_REBUILD=yes
    shift
  elif test "$1" = '--interactive'; then
    INTERACTIVE=yes
    shift
  else
    echo "Unrecognized option $1" >&2
    usage >&2
    exit 1
  fi
done

cd $(dirname $0)/.. || exit 1
#echo "--=[ PWD is $PWD"

export QGIS_BUILDDIR=build-ci

QGIS_WORKSPACE="$(pwd -P)"
export QGIS_WORKSPACE
echo "--=[ QGIS_WORKSPACE is $QGIS_WORKSPACE"

QGIS_WORKSPACE_MOUNTPOINT=${QGIS_WORKSPACE} # was /root/QGIS
export QGIS_WORKSPACE_MOUNTPOINT
echo "--=[ QGIS_WORKSPACE_MOUNTPOINT is $QGIS_WORKSPACE_MOUNTPOINT"

QGIS_GIT_DIR="$(git rev-parse --git-dir)"
if test -f ${QGIS_GIT_DIR}/commondir; then
  QGIS_COMMON_GIT_DIR="$(cat ${QGIS_GIT_DIR}/commondir)"
else
  QGIS_COMMON_GIT_DIR=${QGIS_WORKSPACE}
fi
QGIS_COMMON_GIT_DIR="$(cd ${QGIS_COMMON_GIT_DIR} && pwd -P)"
export QGIS_COMMON_GIT_DIR
echo "--=[ QGIS_COMMON_GIT_DIR is $QGIS_COMMON_GIT_DIR"



#
# Make qgis3-build-deps-binary-image available, building it if needed
#

if test "$(docker images -q ${IMAGE_BUILD_DEPS})" = ""; then
  echo "--=[ Fetching qgis build dependencies image"
  docker pull ${IMAGE_BUILD_DEPS}
elif test "${UPDATE_IMAGES}" = "yes"; then
  echo "--=[ Updating qgis build dependencies image"
  docker pull ${IMAGE_BUILD_DEPS}
fi

if test -d ${QGIS_BUILDDIR} -a "${FORCE_REBUILD}" = "no"; then
  echo "--=[ Testing against pre-existing build directory ${QGIS_BUILDDIR}. To rebuild use --force-rebuild or move it away"
else
  echo "--=[ Building qgis inside the dependencies container"
  docker run -t --name qgis_container \
    --rm \
    -v ${QGIS_WORKSPACE}:${QGIS_WORKSPACE} \
    -v ${QGIS_COMMON_GIT_DIR}:${QGIS_COMMON_GIT_DIR} \
    --env-file .docker/docker-variables.env \
    --env PUSH_TO_CDASH=false \
    --env WITH_QT5=true \
    --env BUILD_WITH_QT6=false \
    --env WITH_QUICK=false \
    --env WITH_3D=false \
    --env PATCH_QT_3D=false \
    --env CTEST_SOURCE_DIR=${QGIS_WORKSPACE} \
    --env CTEST_BUILD_DIR=${QGIS_WORKSPACE}/${QGIS_BUILDDIR} \
    ${IMAGE_BUILD_DEPS} \
    ${QGIS_WORKSPACE_MOUNTPOINT}/.docker/docker-qgis-build.sh ||
    exit 1

  test -d ${QGIS_BUILDDIR} || {
    echo "Building failed" >&2
    exit 1
  }
fi

if test "$(docker images -q qgis3-build-deps-binary-image)" = ""; then
  echo "--=[ Tagging qgis build dependencies image as required by .docker/docker-compose-testing.yml"
  docker tag ${IMAGE_BUILD_DEPS} qgis3-build-deps-binary-image
fi

if test "${INTERACTIVE}" = "no"; then
  echo "--=[ Running tests via docker-compose"
  COMMAND=${QGIS_WORKSPACE_MOUNTPOINT}/.docker/docker-qgis-test.sh
  COMMAND_ARGS="${TESTS_TO_RUN}"
else
  echo "--=[ Starting interactive shell into test environment"
  COMMAND=bash
fi

# Create an empty webdav folder with appropriate permissions so www user can write inside it
mkdir -p /tmp/webdav_tests && chmod 777 /tmp/webdav_tests

docker-compose \
  -f .docker/docker-compose-testing.yml \
  run \
  -w "${QGIS_WORKSPACE_MOUNTPOINT}" \
  -e PUSH_TO_CDASH=false \
  -e CTEST_SOURCE_DIR="${QGIS_WORKSPACE}" \
  -e CTEST_BUILD_DIR="${QGIS_WORKSPACE}/${QGIS_BUILDDIR}" \
  qgis-deps \
  ${COMMAND} ${COMMAND_ARGS}
