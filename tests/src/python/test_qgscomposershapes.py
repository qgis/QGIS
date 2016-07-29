# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerShape.

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

from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsComposerShape,
                       QgsComposition,
                       QgsMapSettings
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerShapes(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.mapSettings = QgsMapSettings()

        # create composition
        self.mComposition = QgsComposition(self.mapSettings)
        self.mComposition.setPaperSize(297, 210)

        self.mComposerShape = QgsComposerShape(20, 20, 150, 100, self.mComposition)
        self.mComposerShape.setBackgroundColor(QColor.fromRgb(255, 150, 0))
        self.mComposition.addComposerShape(self.mComposerShape)

    def testRectangle(self):
        """Test rectangle composer shape"""

        self.mComposerShape.setShapeType(QgsComposerShape.Rectangle)

        checker = QgsCompositionChecker('composershapes_rectangle', self.mComposition)
        checker.setControlPathPrefix("composer_shapes")
        myTestResult, myMessage = checker.testComposition()

        assert myTestResult, myMessage

    def testEllipse(self):
        """Test ellipse composer shape"""

        self.mComposerShape.setShapeType(QgsComposerShape.Ellipse)

        checker = QgsCompositionChecker('composershapes_ellipse', self.mComposition)
        checker.setControlPathPrefix("composer_shapes")
        myTestResult, myMessage = checker.testComposition()

        assert myTestResult, myMessage

    def testTriangle(self):
        """Test triangle composer shape"""

        self.mComposerShape.setShapeType(QgsComposerShape.Triangle)

        checker = QgsCompositionChecker('composershapes_triangle', self.mComposition)
        checker.setControlPathPrefix("composer_shapes")
        myTestResult, myMessage = checker.testComposition()

        assert myTestResult, myMessage

    def testRoundedRectangle(self):
        """Test rounded rectangle composer shape"""

        self.mComposerShape.setShapeType(QgsComposerShape.Rectangle)
        self.mComposerShape.setCornerRadius(30)

        checker = QgsCompositionChecker('composershapes_roundedrect', self.mComposition)
        checker.setControlPathPrefix("composer_shapes")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerShape.setCornerRadius(0)
        assert myTestResult, myMessage

if __name__ == '__main__':
    unittest.main()
