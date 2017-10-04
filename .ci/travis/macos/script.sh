###########################################################################
#    script.sh
#    ---------------------
#    Date                 : August 2015
#    Copyright            : (C) 2015 by Nyall Dawson
#    Email                : nyall dot dawson at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $PATH

export PATH=/usr/bin:${PATH}

ccache -M 500M
ccache -z

# Calculate the timeout for the tests.
# The tests should be aborted before travis times out, in order to allow uploading
# the ccache and therefore speedup subsequent e builds.
#
# Travis will kill the job after approx 48 minutes, we subtract 8 minutes for
# uploading and initialization (40) and subtract the bootstrapping time from that.
TIMEOUT=$(expr 40 \* 60 - `date +%s` + `cat /tmp/travis_timestamp`)

export CTEST_BUILD_COMMAND=/usr/local/bin/ninja
export CTEST_BUILD_DIR=${TRAVIS_BUILD_DIR}

gtimeout ${TIMEOUT}s ctest -V -E "$(cat ${DIR}/blacklist.txt | gsed -r '/^(#.*?)?$/d' | gpaste -sd '|' -)" -S ${DIR}/../travis.ctest --output-on-failure

rv=$?

if [ $rv -eq 124 ] ; then
    printf '\n\n\033[0;33mBuild and test timeout. Please restart the build for meaningful results.\033[0m\n'
fi

exit $rv
