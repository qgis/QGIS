# -*- coding: utf-8 -*-
"""QGIS unit tests for QgsPalLabeling: label rendering to composer

From build dir: ctest -R PyQgsPalLabelingComposer -V
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
__date__ = '2014/02/21'
__copyright__ = 'Copyright 2013, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sys
import os
import tempfile
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from utilities import (
    unittest,
    expectedFailure,
)

from test_qgspallabeling_base import TestQgsPalLabeling, runSuite
from test_qgspallabeling_tests import TestPointBase


# noinspection PyShadowingNames
class TestComposerBase(TestQgsPalLabeling):

    @classmethod
    def setUpClass(cls):
        TestQgsPalLabeling.setUpClass()
        # the blue background (set via layer style) to match renderchecker's
        cls._BkgrdLayer = TestQgsPalLabeling.loadFeatureLayer('background')
        cls._CheckMismatch = 0  # mismatch expected for crosscheck
        cls._CheckGroup = ''

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        TestQgsPalLabeling.tearDownClass()

    def get_composer_image(self, width=600, height=400, dpi=72):
        # set up composition and add map
        comp = QgsComposition(self._MapRenderer)
        """:type: QgsComposition"""
        comp.setPrintResolution(dpi)
        # 600 x 400 px = 211.67 x 141.11 mm @ 72 dpi
        # TODO: figure out why this doesn't work and needs fudging
        #       probably need sets of fudgyness per dpi group (72, 150, 300)?
        paperw = round((width * 25.4 / dpi) + 0.05, 0)
        paperh = round((height * 25.4 / dpi) + 0.05, 1)
        comp.setPaperSize(paperw, paperh)
        compmap = QgsComposerMap(
            comp, 0, 0, comp.paperWidth(), comp.paperHeight())
        """:type: QgsComposerMap"""
        compmap.setFrameEnabled(False)
        comp.addComposerMap(compmap)
        compmap.setNewExtent(self.aoiExtent())

        comp.setPlotStyle(QgsComposition.Print)
        dpi = comp.printResolution()
        dpmm = dpi / 25.4
        img_width = int(dpmm * comp.paperWidth())
        img_height = int(dpmm * comp.paperHeight())

        # create output image and initialize it
        image = QImage(QSize(img_width, img_height), QImage.Format_ARGB32)
        image.setDotsPerMeterX(dpmm * 1000)
        image.setDotsPerMeterY(dpmm * 1000)
        image.fill(QColor(152, 219, 249).rgb())

        # render the composition
        p = QPainter(image)
        p.setRenderHint(QPainter.Antialiasing)
        src = QRectF(0, 0, comp.paperWidth(), comp.paperHeight())
        trgt = QRectF(0, 0, img_width, img_height)
        comp.render(p, trgt, src)
        p.end()

        tmp = tempfile.NamedTemporaryFile(suffix=".png", delete=False)
        filepath = tmp.name
        tmp.close()

        res = image.save(filepath, "png")
        if not res:
            os.unlink(filepath)
            filepath = ''

        return res, filepath


class TestComposerPoint(TestComposerBase, TestPointBase):

    _TestImage = ''

    @classmethod
    def setUpClass(cls):
        TestComposerBase.setUpClass()
        cls.layer = TestQgsPalLabeling.loadFeatureLayer('point')

    def setUp(self):
        """Run before each test."""
        self.configTest('pal_composer', 'sp')
        TestQgsPalLabeling.setDefaultEngineSettings()
        self.lyr = self.defaultSettings()
        self._TestImage = ''

    def tearDown(self):
        """Run after each test."""
        pass

    def checkTest(self, **kwargs):
        self.lyr.writeToLayer(self.layer)
        res_m, self._TestImage = self.get_composer_image()
        self.saveContolImage(self._TestImage)
        self.assertTrue(res_m, 'Failed to retrieve/save image from test server')
        # gp = kwargs['grpprefix'] if 'grpprefix' in kwargs else ''
        self.assertTrue(*self.renderCheck(mismatch=self._CheckMismatch,
                                          imgpath=self._TestImage,
                                          grpprefix=self._CheckGroup))


class TestComposerVsCanvasPoint(TestComposerPoint):

    @classmethod
    def setUpClass(cls):
        TestComposerPoint.setUpClass()
        cls._CheckGroup = 'pal_canvas'
        cls._CheckMismatch = 2700  # rounding errors on composer output?


if __name__ == '__main__':
    # NOTE: unless PAL_SUITE env var is set all test class methods will be run
    # ex: 'TestGroup(Point|Line|Curved|Polygon|Feature).test_method'
    suite = [
        'TestComposerPoint.test_default_label',
        'TestComposerPoint.test_text_size_map_unit',
        'TestComposerPoint.test_text_color',
        'TestComposerPoint.test_partials_labels_enabled',
        'TestComposerPoint.test_partials_labels_disabled',

        'TestComposerVsCanvasPoint.test_default_label',
        'TestComposerVsCanvasPoint.test_text_size_map_unit',
        'TestComposerVsCanvasPoint.test_text_color',
        'TestComposerVsCanvasPoint.test_partials_labels_enabled',
        'TestComposerVsCanvasPoint.test_partials_labels_disabled',
    ]
    res = runSuite(sys.modules[__name__], suite)
    sys.exit(not res.wasSuccessful())
