# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering to screen canvas

From build dir: ctest -R PyQgsPalLabelingCanvas -V
Set the following env variables when manually running tests:
  PAL_SUITE to run specific tests (define in __main__)
  PAL_VERBOSE to output individual test summary
  PAL_CONTROL_IMAGE to trigger building of new control images
  PAL_REPORT to open any failed image check reports in web browser

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Larry Shaffer'
__date__ = '07/09/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from utilities import (
    unittest,
    expectedFailure,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite


class TestQgsPalLabelingPoint(TestQgsPalLabeling):
    """Most layer-level labeling properties shared across feature types are
    tested in this class"""

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_canvas', 'sp')
        self.lyr = self.defaultSettings()

    def tearDown(self):
        """Run after each test."""
        pass

    def test_default_label(self):
        # Verify basic default label placement and text size in points
        self.lyr.writeToLayer(self.layer)
        self.saveContolImage()
        self.assertTrue(*self.renderCheck())

    def test_text_size_map_unit(self):
        # Verify label text size in map units
        self.lyr.fontSizeInMapUnits = True
        tmpFont = QFont(self._TestFont)
        tmpFont.setPointSizeF(0.25)
        self.lyr.textFont = tmpFont
        self.lyr.writeToLayer(self.layer)
        self.saveContolImage()
        self.assertTrue(*self.renderCheck())

    def test_text_color(self):
        # Verify label color change
        self.lyr.textColor = Qt.blue
        self.lyr.writeToLayer(self.layer)
        self.saveContolImage()
        self.assertTrue(*self.renderCheck())


# class TestQgsPalLabelingLine(TestQgsPalLabeling):
#     """Layer-level property tests for line features"""
#
#     @classmethod
#     def setUpClass(cls):
#         TestQgsPalLabeling.setUpClass()
#         cls.layer = TestQgsPalLabeling.loadFeatureLayer('line')
#
#     def setUp(self):
#         """Run before each test."""
#         self.configTest('pal_canvas', 'sl')
#         self.lyr = self.defaultSettings()
#
#     def tearDown(self):
#         """Run after each test."""
#         pass


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set
    # all test class methods will be run
    sp = 'TestQgsPalLabelingPoint.'
    sl = 'TestQgsPalLabelingLine.'
    sc = 'TestQgsPalLabelingCurved.'
    sg = 'TestQgsPalLabelingPolygon.'
    mf = 'TestQgsPalLabelingMultiFeature.'

    tests = [sp + 'test_default_label']
    res = runSuite(sys.modules[__name__], tests)
    sys.exit(not res.wasSuccessful())
