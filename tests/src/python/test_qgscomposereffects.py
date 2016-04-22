# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerEffects.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2012 by Dr. Horst Düster / Dr. Marco Hugentobler'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.PyQt.QtGui import QPainter, QColor

from qgis.core import (QgsComposerShape,
                       QgsComposition,
                       QgsMapRenderer
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerEffects(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        # create composition
        self.mMapRenderer = QgsMapRenderer()
        self.mComposition = QgsComposition(self.mMapRenderer)
        self.mComposition.setPaperSize(297, 210)

        self.mComposerRect1 = QgsComposerShape(20, 20, 150, 100, self.mComposition)
        self.mComposerRect1.setShapeType(QgsComposerShape.Rectangle)
        self.mComposerRect1.setBackgroundColor(QColor.fromRgb(255, 150, 0))
        self.mComposition.addComposerShape(self.mComposerRect1)

        self.mComposerRect2 = QgsComposerShape(50, 50, 150, 100, self.mComposition)
        self.mComposerRect2.setShapeType(QgsComposerShape.Rectangle)
        self.mComposerRect2.setBackgroundColor(QColor.fromRgb(0, 100, 150))
        self.mComposition.addComposerShape(self.mComposerRect2)

    def testBlendModes(self):
        """Test that blend modes work for composer items."""

        self.mComposerRect2.setBlendMode(QPainter.CompositionMode_Multiply)

        checker = QgsCompositionChecker('composereffects_blend', self.mComposition)
        checker.setControlPathPrefix("composer_effects")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerRect2.setBlendMode(QPainter.CompositionMode_SourceOver)

        assert myTestResult, myMessage

    def testTransparency(self):
        """Test that transparency works for composer items."""

        self.mComposerRect2.setTransparency(50)

        checker = QgsCompositionChecker('composereffects_transparency', self.mComposition)
        checker.setControlPathPrefix("composer_effects")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerRect2.setTransparency(100)

        assert myTestResult, myMessage

if __name__ == '__main__':
    unittest.main()
