#!/usr/bin/env bash
###########################################################################
#    script.sh
#    ---------------------
#    Date                 : February 2017
#    Copyright            : (C) 2017 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
set -e

pushd build

export CTEST_BUILD_DIR=${TRAVIS_BUILD_DIR}
export CTEST_BUILD_COMMAND="/usr/bin/make -j3 -i -k"

python3 "${TRAVIS_BUILD_DIR}/.ci/travis/scripts/ctest2travis.py" xvfb-run ctest -VV --output-on-failure -S "${TRAVIS_BUILD_DIR}/.ci/travis/travis.ctest"

popd
