# -#- coding: utf-8 -#-

###########################################################################
#    __init__.py
#    ---------------------
#    Date                 : November 2018
#    Copyright            : (C) 2018 by Nathan Woodrow
#    Email                : woodrow dot nathan at gmail dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

"""
QGIS Processing Python additions.

This module contains stable API adding additional Python specific functionality
to the core QGIS c++ Processing classes.
"""

__author__ = 'Nathan Woodrow'
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nathan Woodrow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from .algfactory import ProcessingAlgFactory

alg = ProcessingAlgFactory()
