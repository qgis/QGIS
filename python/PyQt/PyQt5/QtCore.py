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

from PyQt5.QtCore import *

from types import MethodType


_QVariant__repr__ = QVariant.__repr__
_QVariant__eq__ = QVariant.__eq__
_QVariant__ne__ = QVariant.__ne__
_QVariant__hash__ = QVariant.__hash__


def __bool__(self):
    return not self.isNull()


def __repr__(self):
    if self.isNull():
        return 'NULL'
    else:
        return _QVariant__repr__(self)


def __eq__(self, other):
    if self.isNull():
        return (isinstance(other, QVariant) and other.isNull())or other is None
    else:
        return _QVariant__eq__(self, other)


def __ne__(self, other):
    if self.isNull():
        return not (isinstance(other, QVariant) and other.isNull()) and other is not None
    else:
        return _QVariant__ne__(self, other)


def __hash__(self):
    if self.isNull():
        return 2178309
    else:
        return _QVariant__hash__(self)


QVariant.__bool__ = __bool__
QVariant.__repr__ = __repr__
QVariant.__eq__ = __eq__
QVariant.__ne__ = __ne__
QVariant.__hash__ = __hash__

NULL = QVariant(QVariant.Int)
