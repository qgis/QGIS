# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : January 2007
    Copyright            : (C) 2007 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Martin Dobias'
__date__ = 'January 2007'
__copyright__ = '(C) 2007, Martin Dobias'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import QPyNullVariant
from types import MethodType

# Add a __nonzero__ method onto QPyNullVariant so we can check for null values easier.
#   >>> value = QPyNullVariant("int")
#   >>> if value:
#   >>>	  print "Not a null value"
def __nonzero__(self):
    return False

QPyNullVariant.__nonzero__ = MethodType(__nonzero__, None, QPyNullVariant)
