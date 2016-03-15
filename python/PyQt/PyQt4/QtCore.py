# -*- coding: utf-8 -*-

"""
***************************************************************************
    QtCore.py
    ---------------------
    Date                 : November 2015
    Copyright            : (C) 2015 by Matthias Kuhn
    Email                : matthias at opengis dot ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'November 2015'
__copyright__ = '(C) 2015, Matthias Kuhn'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sip
for api in ["QDate", "QDateTime", "QString", "QTextStream", "QTime", "QUrl", "QVariant"]:
    sip.setapi(api, 2)

from PyQt4.QtCore import *
from PyQt4.QtGui import QItemSelectionModel, QSortFilterProxyModel

# Add a __nonzero__ method onto QPyNullVariant so we can check for null values easier.
#   >>> value = QPyNullVariant("int")
#   >>> if value:
#   >>>       print "Not a null value"
from types import MethodType
from PyQt4.QtCore import QPyNullVariant


def __nonzero__(self):
    return False


def __repr__(self):
    return 'NULL'


def __eq__(self, other):
    return isinstance(other, QPyNullVariant) or other is None


def __ne__(self, other):
    return not isinstance(other, QPyNullVariant) and other is not None


def __hash__(self):
    return 2178309

QPyNullVariant.__nonzero__ = MethodType(__nonzero__, None, QPyNullVariant)
QPyNullVariant.__repr__ = MethodType(__repr__, None, QPyNullVariant)
QPyNullVariant.__eq__ = MethodType(__eq__, None, QPyNullVariant)
QPyNullVariant.__ne__ = MethodType(__ne__, None, QPyNullVariant)
QPyNullVariant.__hash__ = MethodType(__hash__, None, QPyNullVariant)

NULL = QPyNullVariant(int)
