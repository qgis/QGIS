#!/bin/bash
###########################################################################
#    build.sh
#    ---------------------
#    Date                 : November 2010
#    Copyright            : (C) 2010 by Tim Sutton
#    Email                : tim at kartoza dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

txt2tags -o ../INSTALL -t txt INSTALL.t2t 
txt2tags -o INSTALL.html -t html INSTALL.t2t
txt2tags -o INSTALL.tex -t tex INSTALL.t2t
pdflatex INSTALL.tex
mv INSTALL.pdf ..
txt2tags -o ../CODING -t txt CODING.t2t
txt2tags -o CODING.html -t html CODING.t2t
txt2tags -o CODING.tex -t tex CODING.t2t
pdflatex CODING.tex
mv CODING.pdf ..
