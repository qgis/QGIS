#!/bin/bash

# A simple script to create a release tarball
#
#  Tim Sutton, October 2011

TAG_OR_BRANCH="release-1_7_2"
VERSION="1.7.2"

git archive --format=tar --prefix=qgis-${VERSION}/ ${TAG_OR_BRANCH=} | \
   bzip2 > /tmp/qgis-${VERSION}.tar.bz2
md5sum /tmp/qgis-${VERSION}.tar.bz2 > \
    /tmp/qgis-${VERSION}.tar.bz2.md5
