# -*- coding: utf-8 -*-

"""
***************************************************************************
    QtGui.py
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

from PyQt5.QtGui import *


def __qcolor_repr__(self: QColor):
    if not self.isValid():
        return '<QColor: invalid>'
    elif self.spec() == QColor.Rgb:
        return f'<QColor: RGBA {self.red()}, {self.green()}, {self.blue()}, {self.alpha()}>'
    elif self.spec() == QColor.Hsv:
        return f'<QColor: HSVA {self.hsvHue()}, {self.hsvSaturation()}, {self.value()}, {self.alpha()}>'
    elif self.spec() == QColor.Cmyk:
        return f'<QColor: CMYKA {self.cyan()}, {self.magenta()}, {self.yellow()}, {self.black()}, {self.alpha()}>'
    elif self.spec() == QColor.Hsl:
        return f'<QColor: HSLA {self.hslHue()}, {self.hslSaturation()}, {self.lightness()}, {self.alpha()}>'
    elif self.spec() == QColor.ExtendedRgb:
        return f'<QColor: Extended RGBA {self.redF()}, {self.greenF()}, {self.blueF()}, {self.alphaF()}>'


# PyQt doesn't provide __repr__ for QColor, but it's highly desirable!
QColor.__repr__ = __qcolor_repr__
