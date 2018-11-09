#!/usr/bin/env bash
###########################################################################
#    install.sh
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

export CORES=2

mkdir build
pushd build || exit

cmake -DWITH_SERVER=ON -DUSE_CCACHE=OFF -DWITH_CORE=OFF -DWITH_APIDOC=ON -DWITH_ASTYLE=ON -DENABLE_TESTS=ON ..

popd || exit
