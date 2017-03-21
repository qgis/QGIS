###########################################################################
#    before_install.sh
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

##################################################
#
# Get precompiled dependencies
#
##################################################

pushd ${HOME}

# fetching data from github should be just as fast as S3
curl -s -S -L https://github.com/opengisch/osgeo4travis/archive/qt55bin.tar.gz | tar --strip-components=1 -xz -C /home/travis &
SETUP_OSGEO4W_PID=$!

mkdir /home/travis/osgeo4travis

# other dependencies live in a cached folder
pushd depcache
# Download newer version of cmake than in the repository
[[ -f cmake-3.5.0-Linux-x86_64.tar.gz ]] || curl -s -S -O https://cmake.org/files/v3.5/cmake-3.5.0-Linux-x86_64.tar.gz
tar --strip-components=1 -zx -f cmake-3.5.0-Linux-x86_64.tar.gz -C /home/travis/osgeo4travis 

# Download OTB package for Processing tests
[[ -f OTB-5.6.0-Linux64.run ]] || curl -s -S -O https://www.orfeo-toolbox.org/packages/archives/OTB/OTB-5.6.0-Linux64.run
sh ./OTB-5.6.0-Linux64.run

wait $SETUP_OSGEO4W_PID

popd
popd

pip install psycopg2 numpy nose2 pyyaml mock future termcolor
