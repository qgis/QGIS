# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDistanceVsElevationPlotCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '28/3/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (
    QDir,
    QEvent,
    Qt,
    QPoint,
    QPointF
)
from qgis.PyQt.QtGui import (
    QKeyEvent,
    QMouseEvent,
    QWheelEvent
)

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsPoint,
    QgsPointXY
)
from qgis.gui import (
    QgsDistanceVsElevationPlotCanvas,
    QgsPlotTool,
    QgsPlotMouseEvent
)

from qgis.testing import start_app, unittest

app = start_app()


class TestTool(QgsPlotTool):

    def __init__(self, canvas):
        super().__init__(canvas)
        self.events = []

    def plotMoveEvent(self, event):
        self.events.append(event)

    def plotDoubleClickEvent(self, event):
        self.events.append(event)

    def plotPressEvent(self, event):
        self.events.append(event)

    def plotReleaseEvent(self, event):
        self.events.append(event)

    def wheelEvent(self, event):
        self.events.append(event)

    def keyPressEvent(self, event):
        self.events.append(event)

    def keyReleaseEvent(self, event):
        self.events.append(event)


class TestQgsDistanceVsElevationPlotCanvas(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsDistanceVsElevationPlotCanvas Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testGettersSetters(self):
        canvas = QgsDistanceVsElevationPlotCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(canvas.crs().authid(), 'EPSG:3111')

        ls = QgsLineString()
        ls.fromWkt('LineString(1 2, 3 4)')
        canvas.setProfileCurve(ls)
        self.assertEqual(canvas.profileCurve().asWkt(), 'LineString (1 2, 3 4)')

    def testZToCanvasY(self):
        canvas = QgsDistanceVsElevationPlotCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.show()
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        self.assertEqual(canvas.zToCanvasY(0), 400)
        self.assertEqual(canvas.zToCanvasY(100), 0)

    def testToFromMapCoordinates(self):
        """
        Test converting canvas coordinates to map coordinates
        """
        canvas = QgsDistanceVsElevationPlotCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.show()
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        # no profile curve set
        self.assertTrue(canvas.toMapCoordinates(QgsPointXY(300, 200)).isEmpty())
        self.assertTrue(canvas.toCanvasCoordinates(QgsPoint(6, 3)).isEmpty())

        ls = QgsLineString()
        ls.fromWkt('LineString(0 2, 10 2, 10 4)')
        canvas.setProfileCurve(ls)

        res = canvas.toMapCoordinates(QgsPointXY(300, 200))
        self.assertAlmostEqual(res.x(), 6, 4)
        self.assertAlmostEqual(res.y(), 2, 4)
        self.assertAlmostEqual(res.z(), 50, 4)

        res = canvas.toCanvasCoordinates(QgsPoint(6, 2, 50))
        self.assertAlmostEqual(res.x(), 300, 4)
        self.assertAlmostEqual(res.y(), 200, 4)

        res = canvas.toMapCoordinates(QgsPointXY(0, 0))
        self.assertAlmostEqual(res.x(), 0, 4)
        self.assertAlmostEqual(res.y(), 2, 4)
        self.assertAlmostEqual(res.z(), 0, 4)

        res = canvas.toCanvasCoordinates(QgsPoint(0, 2, 0))
        self.assertAlmostEqual(res.x(), 0, 4)
        self.assertAlmostEqual(res.y(), 0, 4)

        res = canvas.toMapCoordinates(QgsPointXY(600, 400))
        self.assertAlmostEqual(res.x(), 10, 4)
        self.assertAlmostEqual(res.y(), 4, 4)
        self.assertAlmostEqual(res.z(), 100, 4)

        res = canvas.toCanvasCoordinates(QgsPoint(10, 4, 100))
        self.assertAlmostEqual(res.x(), 600, 4)
        self.assertAlmostEqual(res.y(), 400, 4)

    def test_tool(self):
        """
        Test some plot tool logic
        """
        canvas = QgsDistanceVsElevationPlotCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.show()
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)
        ls = QgsLineString()
        ls.fromWkt('LineString(0 2, 10 2, 10 4)')
        canvas.setProfileCurve(ls)

        self.assertFalse(canvas.tool())
        tool = TestTool(canvas)
        self.assertFalse(tool.isActive())
        canvas.setTool(tool)
        self.assertEqual(canvas.tool(), tool)

        key_press_event = QKeyEvent(QEvent.KeyPress, 54, Qt.ShiftModifier)
        canvas.keyPressEvent(key_press_event)
        self.assertEqual(tool.events[-1].type(), QEvent.KeyPress)

        key_release_event = QKeyEvent(QEvent.KeyRelease, 54, Qt.ShiftModifier)
        canvas.keyReleaseEvent(key_release_event)
        self.assertEqual(tool.events[-1].type(), QEvent.KeyRelease)

        mouse_dbl_click_event = QMouseEvent(QEvent.MouseButtonDblClick, QPointF(300, 200), Qt.LeftButton, Qt.MouseButtons(), Qt.ShiftModifier)
        canvas.mouseDoubleClickEvent(mouse_dbl_click_event)
        self.assertEqual(tool.events[-1].type(), QEvent.MouseButtonDblClick)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 6, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 50, 4)

        mouse_move_event = QMouseEvent(QEvent.MouseMove, QPointF(300, 200), Qt.LeftButton, Qt.MouseButtons(), Qt.ShiftModifier)
        canvas.mouseMoveEvent(mouse_move_event)
        self.assertEqual(tool.events[-1].type(), QEvent.MouseMove)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 6, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 50, 4)

        mouse_press_event = QMouseEvent(QEvent.MouseButtonPress, QPointF(300, 200), Qt.LeftButton, Qt.MouseButtons(), Qt.ShiftModifier)
        canvas.mousePressEvent(mouse_press_event)
        self.assertEqual(tool.events[-1].type(), QEvent.MouseButtonPress)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 6, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 50, 4)

        mouse_release_event = QMouseEvent(QEvent.MouseButtonRelease, QPointF(300, 200), Qt.LeftButton, Qt.MouseButtons(), Qt.ShiftModifier)
        canvas.mouseReleaseEvent(mouse_release_event)
        self.assertEqual(tool.events[-1].type(), QEvent.MouseButtonRelease)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 6, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 50, 4)

        wheel_event = QWheelEvent(QPointF(300, 200), QPointF(300, 200), QPoint(1, 2), QPoint(3, 4), Qt.NoButton, Qt.NoModifier, Qt.ScrollBegin, False)
        canvas.wheelEvent(wheel_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Wheel)


if __name__ == '__main__':
    unittest.main()
