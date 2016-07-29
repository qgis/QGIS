# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerMap.

.. note. This program is free software; you can redistribute it and/or modify
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

from qgis.core import (QgsComposerMap,
                       QgsComposerMapGrid,
                       QgsRectangle,
                       QgsComposition,
                       QgsMapSettings,
                       QgsCoordinateReferenceSystem,
                       QgsFontUtils
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerMap(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        # create composition with composer map
        self.mMapSettings = QgsMapSettings()
        crs = QgsCoordinateReferenceSystem(32633)
        self.mMapSettings.setDestinationCrs(crs)
        self.mMapSettings.setCrsTransformEnabled(False)
        self.mComposition = QgsComposition(self.mMapSettings)
        self.mComposition.setPaperSize(297, 210)
        self.mComposerMap = QgsComposerMap(self.mComposition, 20, 20, 200, 100)
        self.mComposerMap.setFrameEnabled(True)
        self.mComposerMap.setBackgroundColor(QColor(150, 100, 100))
        self.mComposition.addComposerMap(self.mComposerMap)

    def testGrid(self):
        """Test that we can create a grid for a map."""
        myRectangle = QgsRectangle(781662.375, 3339523.125,
                                   793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setAnnotationEnabled(True)
        self.mComposerMap.grid().setGridLineColor(QColor(0, 255, 0))
        self.mComposerMap.grid().setGridLineWidth(0.5)
        self.mComposerMap.grid().setAnnotationFont(QgsFontUtils.getStandardTestFont())
        self.mComposerMap.grid().setAnnotationPrecision(0)
        self.mComposerMap.grid().setAnnotationPosition(QgsComposerMapGrid.Disabled, QgsComposerMapGrid.Left)
        self.mComposerMap.grid().setAnnotationPosition(QgsComposerMapGrid.OutsideMapFrame, QgsComposerMapGrid.Right)
        self.mComposerMap.grid().setAnnotationPosition(QgsComposerMapGrid.Disabled, QgsComposerMapGrid.Top)
        self.mComposerMap.grid().setAnnotationPosition(QgsComposerMapGrid.OutsideMapFrame, QgsComposerMapGrid.Bottom)
        self.mComposerMap.grid().setAnnotationDirection(QgsComposerMapGrid.Horizontal, QgsComposerMapGrid.Right)
        self.mComposerMap.grid().setAnnotationDirection(QgsComposerMapGrid.Horizontal, QgsComposerMapGrid.Bottom)
        self.mComposerMap.grid().setAnnotationFontColor(QColor(255, 0, 0, 150))
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_Overlay)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_grid', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition()
        self.mComposerMap.setGridEnabled(False)
        self.mComposerMap.setShowGridAnnotation(False)

        assert myTestResult, myMessage

    def testCrossGrid(self):
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.Cross)
        self.mComposerMap.grid().setCrossLength(2.0)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setAnnotationEnabled(False)
        self.mComposerMap.grid().setGridLineColor(QColor(0, 255, 0))
        self.mComposerMap.grid().setGridLineWidth(0.5)
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_crossgrid', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.Solid)
        self.mComposerMap.grid().setEnabled(False)
        self.mComposerMap.grid().setAnnotationEnabled(False)

        assert myTestResult, myMessage

    def testMarkerGrid(self):
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.Markers)
        self.mComposerMap.grid().setCrossLength(2.0)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setAnnotationEnabled(False)
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_markergrid', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.Solid)
        self.mComposerMap.grid().setEnabled(False)
        self.mComposerMap.grid().setAnnotationEnabled(False)

        assert myTestResult, myMessage

    def testFrameOnly(self):
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.FrameAnnotationsOnly)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setAnnotationEnabled(False)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.Zebra)
        self.mComposerMap.grid().setFramePenSize(0.5)
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_gridframeonly', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition()

        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.Solid)
        self.mComposerMap.grid().setEnabled(False)
        self.mComposerMap.grid().setAnnotationEnabled(False)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.NoFrame)

        assert myTestResult, myMessage

    def testZebraStyle(self):
        self.mComposerMap.setGridFrameStyle(QgsComposerMap.Zebra)
        myRectangle = QgsRectangle(785462.375, 3341423.125,
                                   789262.375, 3343323.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setGridLineColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setAnnotationFontColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.Zebra)
        self.mComposerMap.grid().setFrameWidth(10)
        self.mComposerMap.grid().setFramePenSize(1)
        self.mComposerMap.grid().setGridLineWidth(0.5)
        self.mComposerMap.grid().setFramePenColor(QColor(255, 100, 0, 200))
        self.mComposerMap.grid().setFrameFillColor1(QColor(50, 90, 50, 100))
        self.mComposerMap.grid().setFrameFillColor2(QColor(200, 220, 100, 60))
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_zebrastyle', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition(0, 100)
        assert myTestResult, myMessage

    def testZebraStyleSides(self):
        self.mComposerMap.setGridFrameStyle(QgsComposerMap.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setGridLineColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setAnnotationFontColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.Zebra)
        self.mComposerMap.grid().setFrameWidth(10)
        self.mComposerMap.grid().setFramePenSize(1)
        self.mComposerMap.grid().setGridLineWidth(0.5)
        self.mComposerMap.grid().setFramePenColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setFrameFillColor1(QColor(0, 0, 0))
        self.mComposerMap.grid().setFrameFillColor2(QColor(255, 255, 255))
        self.mComposerMap.grid().setEnabled(True)

        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameLeft, True)
        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameRight, False)
        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameTop, False)
        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameBottom, False)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_zebrastyle_left', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition(0, 100)
        assert myTestResult, myMessage

        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameTop, True)
        self.mComposerMap.updateBoundingRect()
        checker = QgsCompositionChecker('composermap_zebrastyle_lefttop', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition(0, 100)
        assert myTestResult, myMessage

        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameRight, True)
        self.mComposerMap.updateBoundingRect()
        checker = QgsCompositionChecker('composermap_zebrastyle_lefttopright', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition(0, 100)
        assert myTestResult, myMessage

        self.mComposerMap.grid().setFrameSideFlag(QgsComposerMapGrid.FrameBottom, True)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.NoFrame)

    def testInteriorTicks(self):
        self.mComposerMap.setGridFrameStyle(QgsComposerMap.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        self.mComposerMap.setNewExtent(myRectangle)
        self.mComposerMap.grid().setIntervalX(2000)
        self.mComposerMap.grid().setIntervalY(2000)
        self.mComposerMap.grid().setAnnotationFontColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        self.mComposerMap.grid().setFrameStyle(QgsComposerMapGrid.InteriorTicks)
        self.mComposerMap.grid().setFrameWidth(10)
        self.mComposerMap.grid().setFramePenSize(1)
        self.mComposerMap.grid().setFramePenColor(QColor(0, 0, 0))
        self.mComposerMap.grid().setEnabled(True)
        self.mComposerMap.grid().setStyle(QgsComposerMapGrid.FrameAnnotationsOnly)
        self.mComposerMap.updateBoundingRect()

        checker = QgsCompositionChecker('composermap_interiorticks', self.mComposition)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testComposition(0, 100)
        assert myTestResult, myMessage


if __name__ == '__main__':
    unittest.main()
