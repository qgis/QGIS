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

import os
import qgis
from PyQt4.QtCore import QFileInfo
from PyQt4.QtXml import QDomDocument
from PyQt4.QtGui import (QPainter, QColor)

from qgis.core import (QgsComposerShape,
                       QgsRectangle,
                       QgsComposition,
                       QgsMapRenderer
                     )
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       expectedFailure
                      )
from qgscompositionchecker import QgsCompositionChecker

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerEffects(TestCase):

    def __init__(self, methodName):
        """Run once on class initialisation."""
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

        checker = QgsCompositionChecker()
        myPath = os.path.join(TEST_DATA_DIR,
                              'control_images',
                              'expected_composereffects',
                              'composereffect_blend.png')
        myTestResult, myMessage = checker.testComposition('Composer effects blending',
                                             self.mComposition, myPath)
        self.mComposerRect2.setBlendMode(QPainter.CompositionMode_SourceOver)

        assert myTestResult == True, myMessage

    def testTransparency(self):
        """Test that transparency works for composer items."""

        self.mComposerRect2.setTransparency( 50 )

        checker = QgsCompositionChecker()
        myPath = os.path.join(TEST_DATA_DIR,
                              'control_images',
                              'expected_composereffects',
                              'composereffect_transparency.png')
        myTestResult, myMessage = checker.testComposition('Composer effects transparency',
                                             self.mComposition, myPath)
        self.mComposerRect2.setTransparency( 100 )

        assert myTestResult == True, myMessage

if __name__ == '__main__':
    unittest.main()

