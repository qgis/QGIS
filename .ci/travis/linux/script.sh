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

export PYTHONPATH=${HOME}/osgeo4travis/lib/python3.3/site-packages/
export PATH=${HOME}/osgeo4travis/bin:${HOME}/osgeo4travis/sbin:${HOME}/OTB-5.6.0-Linux64/bin:${PATH}
export LD_LIBRARY_PATH=${HOME}/osgeo4travis/lib
export CTEST_PARALLEL_LEVEL=1
export CCACHE_TEMPDIR=/tmp
ccache -M 500M
ccache -z

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Set OTB application path (installed in before_install.sh script)
export OTB_APPLICATION_PATH=${HOME}/OTB-5.6.0-Linux64/lib/otb/applications
export LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so

export CTEST_BUILD_COMMAND="/usr/bin/make -j3 -i -k"

# This works around an issue where travis would timeout on master because
# when make is run inside ctest no output is generated. At the current time
# nobody know why, but at least this workaround gets travis results for master
# back. Better approaches VERY welcome.
if [[ ${TRAVIS_PULL_REQUEST} == "false" ]];
then
    pushd build
    $CTEST_BUILD_COMMAND
    popd
fi

python ${TRAVIS_BUILD_DIR}/.ci/travis/scripts/ctest2travis.py \
  xvfb-run ctest -V -E "$(cat ${DIR}/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" -S ${DIR}/../travis.ctest --output-on-failure
