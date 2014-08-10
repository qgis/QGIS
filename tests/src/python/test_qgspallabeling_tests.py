# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPalLabeling: base suite of render check tests

Class is meant to be inherited by classes that test different labeling outputs

See <qgis-src-dir>/tests/testdata/labeling/README.rst for description.

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
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from utilities import (
    svgSymbolsPath
)


# noinspection PyPep8Naming
class TestPointBase(object):

    def __init__(self):
        """Dummy assignments, intended to be overridden in subclasses"""
        self.lyr = QgsPalLayerSettings()
        """:type: QgsPalLayerSettings"""
        # noinspection PyArgumentList
        self._TestFont = QFont()  # will become a standard test font
        self._Pal = None
        """:type: QgsPalLabeling"""
        self._Canvas = None
        """:type: QgsMapCanvas"""
        # custom mismatches per group/test (should not mask any needed anomaly)
        # e.g. self._Mismatches['TestClassName'] = 300
        # check base output class's checkTest() or sublcasses for any defaults
        self._Mismatches = dict()
        # custom color tolerances per group/test: 1 - 20 (0 default, 20 max)
        # (should not mask any needed anomaly)
        # e.g. self._ColorTols['TestClassName'] = 10
        # check base output class's checkTest() or sublcasses for any defaults
        self._ColorTols = dict()

    # noinspection PyMethodMayBeStatic
    def checkTest(self, **kwargs):
        """Intended to be overridden in subclasses"""
        pass

    def test_default_label(self):
        # Default label placement, with text size in points
        self.checkTest()

    def test_text_size_map_unit(self):
        # Label text size in map units
        self.lyr.fontSizeInMapUnits = True
        font = QFont(self._TestFont)
        font.setPointSizeF(460)
        self.lyr.textFont = font
        self.checkTest()

    def test_text_color(self):
        # Label color change
        self.lyr.textColor = Qt.blue
        self.checkTest()

    def test_background_rect(self):
        self.lyr.shapeDraw = True
        self.checkTest()

    def test_background_rect_w_offset(self):
        # Label rectangular background
        # verify fix for issues
        #   http://hub.qgis.org/issues/9057
        #   http://gis.stackexchange.com/questions/86900
        self.lyr.fontSizeInMapUnits = True
        font = QFont(self._TestFont)
        font.setPointSizeF(460)
        self.lyr.textFont = font

        self.lyr.shapeDraw = True
        self.lyr.shapeOffsetUnits = QgsPalLayerSettings.MapUnits
        self.lyr.shapeOffset = QPointF(-2900.0, -450.0)
        self.checkTest()

    def test_background_svg(self):
        # Label SVG background
        self.lyr.fontSizeInMapUnits = True
        font = QFont(self._TestFont)
        font.setPointSizeF(460)
        self.lyr.textFont = font

        self.lyr.shapeDraw = True
        self.lyr.shapeType = QgsPalLayerSettings.ShapeSVG
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        self.lyr.shapeSVGFile = svg
        self.lyr.shapeSizeUnits = QgsPalLayerSettings.MapUnits
        self.lyr.shapeSizeType = QgsPalLayerSettings.SizeBuffer
        self.lyr.shapeSize = QPointF(100.0, 0.0)
        self._Mismatches['TestComposerPdfVsComposerPoint'] = 580
        self.checkTest()

    def test_background_svg_w_offset(self):
        # Label SVG background
        self.lyr.fontSizeInMapUnits = True
        font = QFont(self._TestFont)
        font.setPointSizeF(460)
        self.lyr.textFont = font

        self.lyr.shapeDraw = True
        self.lyr.shapeType = QgsPalLayerSettings.ShapeSVG
        svg = os.path.join(
            svgSymbolsPath(), 'backgrounds', 'background_square.svg')
        self.lyr.shapeSVGFile = svg
        self.lyr.shapeSizeUnits = QgsPalLayerSettings.MapUnits
        self.lyr.shapeSizeType = QgsPalLayerSettings.SizeBuffer
        self.lyr.shapeSize = QPointF(100.0, 0.0)

        self.lyr.shapeOffsetUnits = QgsPalLayerSettings.MapUnits
        self.lyr.shapeOffset = QPointF(-2850.0, 500.0)
        self._Mismatches['TestComposerPdfVsComposerPoint'] = 760
        self.checkTest()

    def test_partials_labels_enabled(self):
        # Set Big font size
        font = QFont(self._TestFont)
        font.setPointSizeF(84)
        self.lyr.textFont = font
        # Enable partials labels
        self._Pal.setShowingPartialsLabels(True)
        self._Pal.saveEngineSettings()
        self.checkTest()

    def test_partials_labels_disabled(self):
        # Set Big font size
        font = QFont(self._TestFont)
        font.setPointSizeF(84)
        self.lyr.textFont = font
        # Disable partials labels
        self._Pal.setShowingPartialsLabels(False)
        self._Pal.saveEngineSettings()
        self.checkTest()


# noinspection PyPep8Naming
def suiteTests():
    """
    Use to define which tests are run when PAL_SUITE is set.
    Use sp_vs_suite for comparison of server and composer outputs to canvas
    """
    sp_suite = [
        # 'test_default_label',
        # 'test_text_size_map_unit',
        # 'test_text_color',
        # 'test_background_rect',
        # 'test_background_rect_w_offset',
        # 'test_background_svg',
        # 'test_background_svg_w_offset',
        # 'test_partials_labels_enabled',
        # 'test_partials_labels_disabled',
    ]
    sp_vs_suite = [
        #'test_something_specific',
    ]
    # extended separately for finer control of PAL_SUITE (comment-out undesired)
    sp_vs_suite.extend(sp_suite)

    return {
        'sp_suite': sp_suite,
        'sp_vs_suite': sp_vs_suite
    }


if __name__ == '__main__':
    pass
