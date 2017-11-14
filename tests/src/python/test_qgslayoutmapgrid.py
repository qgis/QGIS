# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemMapGrid.

.. note. This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '20/10/2017'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QPainter, QColor

from qgis.core import (QgsLayoutItemMap,
                       QgsLayoutItemMapGrid,
                       QgsRectangle,
                       QgsLayout,
                       QgsMapSettings,
                       QgsCoordinateReferenceSystem,
                       QgsFontUtils,
                       QgsProject)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerMap(unittest.TestCase):

    def testGrid(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        """Test that we can create a grid for a map."""
        myRectangle = QgsRectangle(781662.375, 3339523.125,
                                   793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setEnabled(True)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setAnnotationFont(QgsFontUtils.getStandardTestFont())
        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(QgsLayoutItemMapGrid.HideAll, QgsLayoutItemMapGrid.Left)
        map.grid().setAnnotationPosition(QgsLayoutItemMapGrid.OutsideMapFrame, QgsLayoutItemMapGrid.Right)
        map.grid().setAnnotationDisplay(QgsLayoutItemMapGrid.HideAll, QgsLayoutItemMapGrid.Top)
        map.grid().setAnnotationPosition(QgsLayoutItemMapGrid.OutsideMapFrame, QgsLayoutItemMapGrid.Bottom)
        map.grid().setAnnotationDirection(QgsLayoutItemMapGrid.Horizontal, QgsLayoutItemMapGrid.Right)
        map.grid().setAnnotationDirection(QgsLayoutItemMapGrid.Horizontal, QgsLayoutItemMapGrid.Bottom)
        map.grid().setAnnotationFontColor(QColor(255, 0, 0, 150))
        map.grid().setBlendMode(QPainter.CompositionMode_Overlay)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_grid', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout()
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        assert myTestResult, myMessage

    def testCrossGrid(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setEnabled(True)
        map.grid().setStyle(QgsLayoutItemMapGrid.Cross)
        map.grid().setCrossLength(2.0)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_crossgrid', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout()

        map.grid().setStyle(QgsLayoutItemMapGrid.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        assert myTestResult, myMessage

    def testMarkerGrid(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setEnabled(True)
        map.grid().setStyle(QgsLayoutItemMapGrid.Markers)
        map.grid().setCrossLength(2.0)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_markergrid', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout()

        map.grid().setStyle(QgsLayoutItemMapGrid.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        assert myTestResult, myMessage

    def testFrameOnly(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setEnabled(True)
        map.grid().setStyle(QgsLayoutItemMapGrid.FrameAnnotationsOnly)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        map.grid().setFramePenSize(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_gridframeonly', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout()

        map.grid().setStyle(QgsLayoutItemMapGrid.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.NoFrame)

        assert myTestResult, myMessage

    def testZebraStyle(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        myRectangle = QgsRectangle(785462.375, 3341423.125,
                                   789262.375, 3343323.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setGridLineColor(QColor(0, 0, 0))
        map.grid().setAnnotationFontColor(QColor(0, 0, 0))
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(255, 100, 0, 200))
        map.grid().setFrameFillColor1(QColor(50, 90, 50, 100))
        map.grid().setFrameFillColor2(QColor(200, 220, 100, 60))
        map.grid().setEnabled(True)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_zebrastyle', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout(0, 100)
        assert myTestResult, myMessage

    def testZebraStyleSides(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setGridLineColor(QColor(0, 0, 0))
        map.grid().setAnnotationFontColor(QColor(0, 0, 0))
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setFrameFillColor1(QColor(0, 0, 0))
        map.grid().setFrameFillColor2(QColor(255, 255, 255))
        map.grid().setEnabled(True)

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameLeft, True)
        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameRight, False)
        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameTop, False)
        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameBottom, False)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_zebrastyle_left', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout(0, 100)
        assert myTestResult, myMessage

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameTop, True)
        map.updateBoundingRect()
        checker = QgsLayoutChecker('composermap_zebrastyle_lefttop', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout(0, 100)
        assert myTestResult, myMessage

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameRight, True)
        map.updateBoundingRect()
        checker = QgsLayoutChecker('composermap_zebrastyle_lefttopright', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout(0, 100)
        assert myTestResult, myMessage

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameBottom, True)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.NoFrame)

    def testInteriorTicks(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationFontColor(QColor(0, 0, 0))
        map.grid().setBlendMode(QPainter.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.InteriorTicks)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setEnabled(True)
        map.grid().setStyle(QgsLayoutItemMapGrid.FrameAnnotationsOnly)
        map.updateBoundingRect()

        checker = QgsLayoutChecker('composermap_interiorticks', layout)
        checker.setControlPathPrefix("composer_mapgrid")
        myTestResult, myMessage = checker.testLayout(0, 100)
        assert myTestResult, myMessage


if __name__ == '__main__':
    unittest.main()
