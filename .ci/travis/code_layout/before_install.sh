#!/usr/bin/env bash
###########################################################################
#    before_install.sh
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

pip install autopep8 nose2 mock termcolor

export PERL_MM_USE_DEFAULT=1
export PERL_EXTUTILS_AUTOINSTALL="--defaultdeps"
cpanm --notest App::Licensecheck
