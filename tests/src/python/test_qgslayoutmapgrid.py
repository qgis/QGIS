"""QGIS Unit tests for QgsLayoutItemMapGrid.

.. note. This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "20/10/2017"
__copyright__ = "Copyright 2012, The QGIS Project"

from typing import Optional

from qgis.PyQt.QtCore import QDir, QRectF
from qgis.PyQt.QtGui import QColor, QPainter
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMapGrid,
    QgsLayoutObject,
    QgsProject,
    QgsProperty,
    QgsRectangle,
    QgsTextFormat,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFont, unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMapGrid(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "composer_mapgrid"

    def testGrid(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:32633"))
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        self.assertFalse(map.grids().hasEnabledItems())

        """Test that we can create a grid for a map."""
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setEnabled(True)
        self.assertTrue(map.grids().hasEnabledItems())
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)
        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Left,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Top,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        result = self.render_layout_check("composermap_grid", layout)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        self.assertTrue(result)

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
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Cross)
        map.grid().setCrossLength(2.0)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.updateBoundingRect()

        result = self.render_layout_check("composermap_crossgrid", layout)

        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        self.assertTrue(result)

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
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Markers)
        map.grid().setCrossLength(2.0)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.grid().markerSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        map.updateBoundingRect()

        result = self.render_layout_check("composermap_markergrid", layout)
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)

        self.assertTrue(result)

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
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.FrameAnnotationsOnly)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        map.grid().setFramePenSize(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.updateBoundingRect()

        result = self.render_layout_check("composermap_gridframeonly", layout)

        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Solid)
        map.grid().setEnabled(False)
        map.grid().setAnnotationEnabled(False)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.NoFrame)

        self.assertTrue(result)

    def testZebraStyle(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        myRectangle = QgsRectangle(785462.375, 3341423.125, 789262.375, 3343323.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setGridLineColor(QColor(0, 0, 0))
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(255, 100, 0, 200))
        map.grid().setFrameFillColor1(QColor(50, 90, 50, 100))
        map.grid().setFrameFillColor2(QColor(200, 220, 100, 60))
        map.grid().setEnabled(True)
        map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check(
                "composermap_zebrastyle", layout, allowed_mismatch=100
            )
        )

    def testZebraStyleSides(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setGridLineColor(QColor(0, 0, 0))
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setFrameFillColor1(QColor(0, 0, 0))
        map.grid().setFrameFillColor2(QColor(255, 255, 255))
        map.grid().setEnabled(True)

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameSideFlag.FrameLeft, True)
        map.grid().setFrameSideFlag(
            QgsLayoutItemMapGrid.FrameSideFlag.FrameRight, False
        )
        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameSideFlag.FrameTop, False)
        map.grid().setFrameSideFlag(
            QgsLayoutItemMapGrid.FrameSideFlag.FrameBottom, False
        )
        map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check(
                "composermap_zebrastyle_left", layout, allowed_mismatch=100
            )
        )

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameSideFlag.FrameTop, True)
        map.updateBoundingRect()
        self.assertTrue(
            self.render_layout_check(
                "composermap_zebrastyle_lefttop", layout, allowed_mismatch=100
            )
        )

        map.grid().setFrameSideFlag(QgsLayoutItemMapGrid.FrameSideFlag.FrameRight, True)
        map.updateBoundingRect()
        self.assertTrue(
            self.render_layout_check(
                "composermap_zebrastyle_lefttopright", layout, allowed_mismatch=100
            )
        )

        map.grid().setFrameSideFlag(
            QgsLayoutItemMapGrid.FrameSideFlag.FrameBottom, True
        )
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.NoFrame)

    def testInteriorTicks(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)

        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        myRectangle = QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125)
        map.setExtent(myRectangle)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_SourceOver)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.InteriorTicks)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setEnabled(True)
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.FrameAnnotationsOnly)
        map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check(
                "composermap_interiorticks", layout, allowed_mismatch=100
            )
        )

    def testAnnotationsVariations(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map_configs = [
            (
                10,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                0,
            ),
            (
                10,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                3,
            ),
            (
                90,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                0,
            ),
            (
                90,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                3,
            ),
            (
                170,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                0,
            ),
            (
                170,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                3,
            ),
            (
                250,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                0,
            ),
            (
                250,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                3,
            ),
        ]

        for x, y, pos, style, dist in map_configs:
            map = QgsLayoutItemMap(layout)
            layout.addLayoutItem(map)
            map.attemptSetSceneRect(QRectF(x, y, 50, 50))
            map.setBackgroundColor(QColor(200, 200, 200))
            map.setExtent(QgsRectangle(5, 5, 15, 15))
            map.setFrameEnabled(True)
            map.grid().setFrameStyle(style)
            map.grid().setFrameWidth(7)
            map.grid().setFramePenSize(1)
            map.grid().setFramePenColor(QColor(255, 0, 0))
            map.grid().setEnabled(True)
            map.grid().setIntervalX(10)
            map.grid().setIntervalY(10)
            map.grid().setAnnotationEnabled(True)
            map.grid().setGridLineColor(QColor(0, 255, 0))
            map.grid().setGridLineWidth(0.5)
            map.grid().setAnnotationFont(getTestFont("Bold", 20))
            map.grid().setAnnotationFontColor(QColor(0, 0, 255, 150))
            map.grid().setAnnotationPrecision(0)
            map.grid().setAnnotationFrameDistance(dist)

            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Top)
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Right)
            map.grid().setAnnotationPosition(
                pos, QgsLayoutItemMapGrid.BorderSide.Bottom
            )
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Left)

            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.Vertical,
                QgsLayoutItemMapGrid.BorderSide.Top,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
                QgsLayoutItemMapGrid.BorderSide.Right,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.BoundaryDirection,
                QgsLayoutItemMapGrid.BorderSide.Bottom,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.VerticalDescending,
                QgsLayoutItemMapGrid.BorderSide.Left,
            )

            map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check("composermap_annotations_variations", layout)
        )

    def testAnnotationsVariationsRotated(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map_configs = [
            (
                10,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                0,
            ),
            (
                10,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                3,
            ),
            (
                90,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                0,
            ),
            (
                90,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                3,
            ),
            (
                170,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                0,
            ),
            (
                170,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                3,
            ),
            (
                250,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                0,
            ),
            (
                250,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                3,
            ),
        ]

        for x, y, pos, style, dist in map_configs:
            map = QgsLayoutItemMap(layout)
            layout.addLayoutItem(map)
            map.attemptSetSceneRect(QRectF(x, y, 50, 50))
            map.setBackgroundColor(QColor(200, 200, 200))
            map.setExtent(QgsRectangle(5, 5, 15, 15))
            map.setMapRotation(30)
            map.setFrameEnabled(True)
            map.grid().setFrameStyle(style)
            map.grid().setFrameWidth(7)
            map.grid().setFramePenSize(1)
            map.grid().setFramePenColor(QColor(255, 0, 0))
            map.grid().setEnabled(True)
            map.grid().setIntervalX(10)
            map.grid().setIntervalY(10)
            map.grid().setAnnotationEnabled(True)
            map.grid().setGridLineColor(QColor(0, 255, 0))
            map.grid().setGridLineWidth(0.5)
            map.grid().setAnnotationFont(getTestFont("Bold", 20))
            map.grid().setAnnotationFontColor(QColor(0, 0, 255, 150))
            map.grid().setAnnotationPrecision(0)
            map.grid().setAnnotationFrameDistance(dist)
            map.grid().setRotatedTicksEnabled(True)
            map.grid().setRotatedAnnotationsEnabled(True)

            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Top)
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Right)
            map.grid().setAnnotationPosition(
                pos, QgsLayoutItemMapGrid.BorderSide.Bottom
            )
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Left)

            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.AboveTick,
                QgsLayoutItemMapGrid.BorderSide.Top,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.OnTick,
                QgsLayoutItemMapGrid.BorderSide.Right,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.UnderTick,
                QgsLayoutItemMapGrid.BorderSide.Bottom,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.BoundaryDirection,
                QgsLayoutItemMapGrid.BorderSide.Left,
            )

            map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check(
                "composermap_annotations_variations_rotated", layout
            )
        )

    def testAnnotationsVariationsRotatedThresholds(self):
        """
        Tests various rotated grid threshold settings
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map_configs = [
            (
                10,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                True,
                False,
            ),
            (
                10,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                True,
                False,
            ),
            (
                170,
                30,
                QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.InteriorTicks,
                False,
                True,
            ),
            (
                170,
                120,
                QgsLayoutItemMapGrid.AnnotationPosition.InsideMapFrame,
                QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks,
                False,
                True,
            ),
        ]

        for x, y, pos, style, limit_rot, limit_corners in map_configs:
            map = QgsLayoutItemMap(layout)
            layout.addLayoutItem(map)
            map.attemptSetSceneRect(QRectF(x, y, 100, 50))
            map.setExtent(QgsRectangle(5000000, 800000, 6000000, 1300000))
            map.setBackgroundColor(QColor(200, 200, 200))
            map.setMapRotation(0)
            map.setFrameEnabled(True)
            map.setCrs(QgsCoordinateReferenceSystem.fromEpsgId(2056))
            map.grid().setCrs(QgsCoordinateReferenceSystem.fromEpsgId(4326))
            map.grid().setFrameStyle(style)
            map.grid().setFrameWidth(7)
            map.grid().setFramePenSize(1)
            map.grid().setFramePenColor(QColor(255, 0, 0))
            map.grid().setEnabled(True)
            map.grid().setIntervalX(2)
            map.grid().setIntervalY(2)
            map.grid().setAnnotationEnabled(True)
            map.grid().setGridLineColor(QColor(0, 255, 0))
            map.grid().setGridLineWidth(0.5)
            map.grid().setRotatedTicksLengthMode(
                QgsLayoutItemMapGrid.TickLengthMode.NormalizedTicks
            )
            map.grid().setAnnotationFont(getTestFont("Bold", 15))
            map.grid().setAnnotationFontColor(QColor(0, 0, 255, 150))
            map.grid().setAnnotationPrecision(0)
            map.grid().setAnnotationFrameDistance(2.5)
            map.grid().setRotatedTicksEnabled(True)
            map.grid().setRotatedAnnotationsEnabled(True)

            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Top)
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Right)
            map.grid().setAnnotationPosition(
                pos, QgsLayoutItemMapGrid.BorderSide.Bottom
            )
            map.grid().setAnnotationPosition(pos, QgsLayoutItemMapGrid.BorderSide.Left)

            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.OnTick,
                QgsLayoutItemMapGrid.BorderSide.Top,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.OnTick,
                QgsLayoutItemMapGrid.BorderSide.Right,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.OnTick,
                QgsLayoutItemMapGrid.BorderSide.Bottom,
            )
            map.grid().setAnnotationDirection(
                QgsLayoutItemMapGrid.AnnotationDirection.OnTick,
                QgsLayoutItemMapGrid.BorderSide.Left,
            )

            if limit_rot:
                map.grid().setRotatedAnnotationsMinimumAngle(30)
                map.grid().setRotatedTicksMinimumAngle(30)

            if limit_corners:
                map.grid().setRotatedAnnotationsMarginToCorner(10)
                map.grid().setRotatedTicksMarginToCorner(10)

            map.updateBoundingRect()

        self.assertTrue(
            self.render_layout_check(
                "composermap_annotations_variations_rotated_thresholds", layout
            )
        )

    def testExpressionContext(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        map.setExtent(QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125))
        map.setScale(1000)
        layout.addLayoutItem(map)

        # grid expression context should inherit from map, so variables like @map_scale can be used
        context = map.grid().createExpressionContext()
        self.assertAlmostEqual(context.variable("map_scale"), 1000, 5)
        self.assertEqual(context.variable("grid_number"), 0)
        self.assertEqual(context.variable("grid_axis"), "x")
        self.assertEqual(context.variable("item_uuid"), map.uuid())

    def testDataDefinedEnabled(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)
        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Left,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Top,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridEnabled,
            QgsProperty.fromValue(True),
        )
        map.grid().refresh()

        self.assertTrue(self.render_layout_check("composermap_grid", layout))

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridEnabled,
            QgsProperty.fromValue(False),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check("composermap_datadefined_disabled", layout)
        )

    def testDataDefinedIntervalOffset(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridIntervalX,
            QgsProperty.fromValue(1500),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridIntervalY,
            QgsProperty.fromValue(2500),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridOffsetX,
            QgsProperty.fromValue(500),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridOffsetY,
            QgsProperty.fromValue(250),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check("composermap_datadefined_intervaloffset", layout)
        )

    def testDataDefinedFrameSize(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setFrameFillColor1(QColor(0, 0, 0))
        map.grid().setFrameFillColor2(QColor(255, 255, 255))
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameSize,
            QgsProperty.fromValue(20),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameMargin,
            QgsProperty.fromValue(10),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check("composermap_datadefined_framesizemargin", layout)
        )

    def testDataDefinedCrossSize(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setStyle(QgsLayoutItemMapGrid.GridStyle.Cross)
        map.grid().setCrossLength(2.0)
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridCrossSize,
            QgsProperty.fromValue(4),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check("composermap_datadefined_crosssize", layout)
        )

    def testDataDefinedFrameThickness(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(False)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.Zebra)
        map.grid().setFrameWidth(10)
        map.grid().setFramePenSize(1)
        map.grid().setGridLineWidth(0.5)
        map.grid().setFramePenColor(QColor(0, 0, 0))
        map.grid().setFrameFillColor1(QColor(0, 0, 0))
        map.grid().setFrameFillColor2(QColor(255, 255, 255))
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameLineThickness,
            QgsProperty.fromValue(4),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check("composermap_datadefined_framethickness", layout)
        )

    def testDataDefinedAnnotationDistance(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)

        format = QgsTextFormat.fromQFont(getTestFont("Bold", 20))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)

        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Left,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Top,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridLabelDistance,
            QgsProperty.fromValue(10),
        )
        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check(
                "composermap_datadefined_annotationdistance", layout
            )
        )

    def testDataDefinedTicksAndAnnotationDisplay(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(40, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setBackgroundColor(QColor(150, 100, 100))
        layout.addLayoutItem(map)
        myRectangle = QgsRectangle(0.5, -5.5, 10.5, 0.5)
        map.setExtent(myRectangle)
        map.setMapRotation(45)
        map.grid().setEnabled(True)
        map.grid().setIntervalX(1)
        map.grid().setIntervalY(1)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)
        map.grid().setFrameStyle(QgsLayoutItemMapGrid.FrameStyle.ExteriorTicks)
        map.grid().setFrameWidth(4)
        map.grid().setFramePenSize(1)
        map.grid().setFramePenColor(QColor(0, 0, 255))
        map.grid().setAnnotationFrameDistance(5)

        format = QgsTextFormat.fromQFont(getTestFont("Bold", 20))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)
        map.grid().setAnnotationPrecision(0)

        map.grid().setRotatedTicksEnabled(True)
        map.grid().setRotatedAnnotationsEnabled(True)
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.OnTick
        )

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridAnnotationDisplayLeft,
            QgsProperty.fromValue("x_only"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridAnnotationDisplayRight,
            QgsProperty.fromValue("Y_ONLY"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridAnnotationDisplayTop,
            QgsProperty.fromValue("disabled"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridAnnotationDisplayBottom,
            QgsProperty.fromValue("ALL"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameDivisionsLeft,
            QgsProperty.fromValue("X_ONLY"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameDivisionsRight,
            QgsProperty.fromValue("y_only"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameDivisionsTop,
            QgsProperty.fromValue("DISABLED"),
        )
        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridFrameDivisionsBottom,
            QgsProperty.fromValue("all"),
        )

        map.grid().refresh()

        self.assertTrue(
            self.render_layout_check(
                "composermap_datadefined_ticksandannotationdisplay", layout
            )
        )

    def testDynamicInterval(self):
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
        map.grid().setUnits(QgsLayoutItemMapGrid.GridUnit.DynamicPageSizeBased)
        map.grid().setMinimumIntervalWidth(50)
        map.grid().setMaximumIntervalWidth(100)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)

        format = QgsTextFormat.fromQFont(getTestFont("Bold", 20))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)

        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Left,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Top,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().refresh()

        self.assertTrue(self.render_layout_check("composermap_dynamic_5_10", layout))

        map.setScale(map.scale() * 1.1)

        self.assertTrue(self.render_layout_check("composermap_dynamic_5_10_2", layout))

        map.setScale(map.scale() * 1.8)

        self.assertTrue(self.render_layout_check("composermap_dynamic_5_10_3", layout))

        map.grid().setMinimumIntervalWidth(10)
        map.grid().setMaximumIntervalWidth(40)
        map.grid().refresh()

        self.assertTrue(self.render_layout_check("composermap_dynamic_5_10_4", layout))

    def testCrsChanged(self):
        """
        Test that the CRS changed signal is emitted in the right circumstances
        """
        p = QgsProject()
        layout = QgsLayout(p)
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)

        grid = map.grid()

        spy = QSignalSpy(grid.crsChanged)
        # map grid and map have no explicit crs set, so follows project crs => signal should be emitted
        # when project crs is changed
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 1)
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertEqual(len(spy), 2)
        # set explicit crs on map item
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28355"))
        self.assertEqual(len(spy), 4)
        # should not care about project crs changes anymore..
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 4)
        # set back to project crs
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 5)

        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28355"))
        self.assertEqual(len(spy), 6)
        # data defined crs
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapCrs,
            QgsProperty.fromValue("EPSG:4283"),
        )
        self.assertEqual(len(spy), 6)
        map.refresh()
        self.assertEqual(len(spy), 7)

        # explicit crs for map grid
        grid.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(len(spy), 8)
        grid.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(len(spy), 8)
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapCrs,
            QgsProperty.fromValue("EPSG:3111"),
        )
        map.refresh()
        self.assertEqual(len(spy), 8)
        grid.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 9)

    def testCopyGrid(self):
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
        map.grid().setIntervalX(2000)
        map.grid().setIntervalY(2000)
        map.grid().setAnnotationEnabled(True)
        map.grid().setGridLineColor(QColor(0, 255, 0))
        map.grid().setGridLineWidth(0.5)

        format = QgsTextFormat.fromQFont(getTestFont("Bold", 20))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        map.grid().setAnnotationTextFormat(format)

        map.grid().setAnnotationPrecision(0)
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Left,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDisplay(
            QgsLayoutItemMapGrid.DisplayMode.HideAll,
            QgsLayoutItemMapGrid.BorderSide.Top,
        )
        map.grid().setAnnotationPosition(
            QgsLayoutItemMapGrid.AnnotationPosition.OutsideMapFrame,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Right,
        )
        map.grid().setAnnotationDirection(
            QgsLayoutItemMapGrid.AnnotationDirection.Horizontal,
            QgsLayoutItemMapGrid.BorderSide.Bottom,
        )
        map.grid().setBlendMode(QPainter.CompositionMode.CompositionMode_Overlay)
        map.updateBoundingRect()

        map.grid().dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapGridLabelDistance,
            QgsProperty.fromValue(10),
        )
        map.grid().refresh()

        source_grid = map.grid()
        grid = QgsLayoutItemMapGrid("testGrid", map)
        grid.copyProperties(source_grid)
        map.grids().removeGrid(source_grid.id())
        self.assertEqual(map.grids().size(), 0)
        map.grids().addGrid(grid)
        map.grid().refresh()
        self.assertTrue(
            self.render_layout_check(
                "composermap_datadefined_annotationdistance", layout
            )
        )


if __name__ == "__main__":
    unittest.main()
