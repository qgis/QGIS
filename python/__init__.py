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

import sip

try:
    apis = ["QDate", "QDateTime", "QString", "QTextStream", "QTime", "QUrl", "QVariant"]
    for api in apis:
        sip.setapi(api, 2)
except ValueError:
    # API has already been set so we can't set it again.
    pass

try:
    # Add a __nonzero__ method onto QPyNullVariant so we can check for null values easier.
    #   >>> value = QPyNullVariant("int")
    #   >>> if value:
    #   >>>	  print "Not a null value"
    from types import MethodType
    from PyQt4.QtCore import QPyNullVariant
    def __nonzero__(self):
        return False

    def __repr__(self):
        return 'NULL'

    def __eq__(self, other):
        return isinstance(other, QPyNullVariant) or other is None

    QPyNullVariant.__nonzero__ = MethodType(__nonzero__, None, QPyNullVariant)
    QPyNullVariant.__repr__ = MethodType(__repr__, None, QPyNullVariant)
    QPyNullVariant.__eq__= MethodType(__eq__, None, QPyNullVariant)

    # define a dummy QPyNullVariant instance NULL in qgis.core
    # this is mainly used to compare against
    # so one can write if feat['attr'] == NULL:
    from qgis import core
    core.NULL = QPyNullVariant( int )
except ImportError:
    pass
