# -*- coding: utf-8 -*-

"""
***************************************************************************
    ranges.py
    ---------------------
    Date                 : Mar 2021
    Copyright            : (C) 2021 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from qgis.PyQt.QtCore import Qt


# add some __repr__ methods to QGIS range classes. We can't do this via sip because they are template based classes


def datetime_range_repr(self):
    return f"<QgsDateTimeRange:{'[' if self.includeBeginning() else '('}{self.begin().toString(Qt.ISODate)}, {self.end().toString(Qt.ISODate)}{']' if self.includeEnd() else ')'}>"


def date_range_repr(self):
    return f"<QgsDateTimeRange:{'[' if self.includeBeginning() else '('}{self.begin().toString(Qt.ISODate)}, {self.end().toString(Qt.ISODate)}{']' if self.includeEnd() else ')'}>"
