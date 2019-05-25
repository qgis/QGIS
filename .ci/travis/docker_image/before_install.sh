#!/usr/bin/env bash
###########################################################################
#    before_install.sh
#    ---------------------
#    Date                 : March 2019
#    Copyright            : (C) 2019 by Denis Rouzaud
#    Email                : denis@opengis.ch
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

if [[ ${INSTALL_LIBSECCOMP} =~ ^TRUE$ ]]; then
  # When building QGIS with Qt 5.10+ in a Docker container, statx calls are required on the host
  # see https://bugs.launchpad.net/ubuntu/+source/docker.io/+bug/1755250
  # this is required on Xenial (currently most recent distribution on Travis), but sould not on Bionic+
  wget https://launchpad.net/ubuntu/+archive/primary/+files/libseccomp2_2.4.1-0ubuntu0.16.04.2_amd64.deb
  sudo dpkg -i libseccomp2_2.4.1-0ubuntu0.16.04.2_amd64.deb
  sudo apt-get install -f
fi
