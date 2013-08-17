# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling: base suite of render check tests

Class is meant to be inherited by classes that test different labeling outputs

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Larry Shaffer'
__date__ = '07/16/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import (
    QgsPalLayerSettings,
)


class TestPointBase(object):

    def __init__(self):
        """Dummy assignments, intended to be overriden in subclasses"""
        self.lyr = QgsPalLayerSettings()
        self._TestFont = QFont()

    def checkTest(self):
        """Intended to be overriden in subclasses"""
        pass

    def test_default_label(self):
        # Default label placement, with text size in points
        self.checkTest()

    def test_text_size_map_unit(self):
        # Label text size in map units
        self.lyr.fontSizeInMapUnits = True
        tmpFont = QFont(self._TestFont)
        tmpFont.setPointSizeF(0.25)
        self.lyr.textFont = tmpFont
        self.checkTest()

    def test_text_color(self):
        # Label color change
        self.lyr.textColor = Qt.blue
        self.checkTest()


if __name__ == '__main__':
    pass
