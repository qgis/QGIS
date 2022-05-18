#!/bin/sh

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

export QGIS_BUILDDIR=build-ci
export QGIS_WORKSPACE=${PWD}
#echo "--=[ PWD is $PWD"
echo "--=[ QGIS_WORKSPACE is $QGIS_WORKSPACE"


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
    -v $(pwd):/root/QGIS \
    --env-file .docker/docker-variables.env \
    --env PUSH_TO_CDASH=false \
    --env WITH_QT5=true \
    --env WITH_QT6=false \
    --env WITH_QUICK=false \
    --env WITH_3D=false \
    --env PATCH_QT_3D=false \
    --env CTEST_BUILD_DIR=/root/QGIS/${QGIS_BUILDDIR} \
    ${IMAGE_BUILD_DEPS} \
    /root/QGIS/.docker/docker-qgis-build.sh ||
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
  docker-compose \
    -f .docker/docker-compose-testing.yml \
    run \
    -e PUSH_TO_CDASH=false \
    -e CTEST_BUILD_DIR=/root/QGIS/${QGIS_BUILDDIR} \
    qgis-deps \
    /root/QGIS/.docker/docker-qgis-test.sh \
    ${TESTS_TO_RUN}
else
  echo "--=[ Starting tests environment via docker-compose"
  docker-compose \
    -f .docker/docker-compose-testing.yml \
    run \
    -e CTEST_BUILD_DIR=/root/QGIS/${QGIS_BUILDDIR} \
    qgis-deps \
    bash
fi


#echo "Not implemented yet" >&2
#exit 1
