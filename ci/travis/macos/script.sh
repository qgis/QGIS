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

ccache -s
ccache -M 1G

# Allow a maximum of 25 minutes for the tests to run in order to timeout before
# travis does and in order to have some time left to upload the ccache to have
# a chance to finish a rebuild in time.
#
# Travis will kill the job after approx 48 minutes, so this leaves us 23 minutes
# for setup and cache uploading
gtimeout 25m ctest -V -E "$(cat ${DIR}/blacklist.txt | gsed -r '/^(#.*?)?$/d' | gpaste -sd '|' -)" -S ${DIR}/travis.ctest --output-on-failure

rv=$?

if [ $? -eq 124 ] ; then
    echo '\033[0;33mBuild and test timeout. Please restart the build for useful results.\033[0m'
fi

exit $rv
