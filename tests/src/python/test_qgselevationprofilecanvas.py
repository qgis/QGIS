"""QGIS Unit tests for QgsElevationProfileCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "28/3/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from qgis.PyQt.QtCore import QEvent, QPoint, QPointF, Qt
from qgis.PyQt.QtGui import QKeyEvent, QMouseEvent, QWheelEvent
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsPoint,
    QgsPointXY,
    QgsProject,
)
from qgis.gui import QgsElevationProfileCanvas, QgsPlotMouseEvent, QgsPlotTool
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestTool(QgsPlotTool):

    def __init__(self, canvas):
        super().__init__(canvas, "Test")
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


class TestQgsElevationProfileCanvas(QgisTestCase):

    def testGettersSetters(self):
        canvas = QgsElevationProfileCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(canvas.crs().authid(), "EPSG:3111")

        ls = QgsLineString()
        ls.fromWkt("LineString(1 2, 3 4)")
        canvas.setProfileCurve(ls)
        self.assertEqual(canvas.profileCurve().asWkt(), "LineString (1 2, 3 4)")

    def testToFromMapCoordinates(self):
        """
        Test converting canvas coordinates to map coordinates
        """
        canvas = QgsElevationProfileCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.setProject(QgsProject.instance())
        canvas.show()
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        # no profile curve set
        self.assertTrue(canvas.toMapCoordinates(QgsPointXY(300, 200)).isEmpty())
        self.assertTrue(canvas.toCanvasCoordinates(QgsPoint(6, 3)).isEmpty())

        ls = QgsLineString()
        ls.fromWkt("LineString(0 2, 10 2, 10 4)")
        canvas.setProfileCurve(ls)

        canvas.setVisiblePlotRange(0, ls.length(), 0, 100)

        res = canvas.toMapCoordinates(QgsPointXY(300, 200))
        self.assertAlmostEqual(res.x(), 5.927, delta=1)
        self.assertAlmostEqual(res.y(), 2, 4)
        self.assertAlmostEqual(res.z(), 49.165, delta=5)

        res = canvas.toCanvasCoordinates(QgsPoint(6, 2, 50))
        self.assertAlmostEqual(res.x(), 303.578, delta=25)
        self.assertAlmostEqual(res.y(), 196.75, delta=25)

        # point outside plot area
        res = canvas.toMapCoordinates(QgsPointXY(0, 0))
        self.assertTrue(res.isEmpty())

        # just inside top left of plot area
        res = canvas.toMapCoordinates(QgsPointXY(75, 300))
        self.assertAlmostEqual(res.x(), 0.1190, delta=1)
        self.assertAlmostEqual(res.y(), 2, 4)
        self.assertAlmostEqual(res.z(), 2.95, delta=20)

        res = canvas.toCanvasCoordinates(QgsPoint(0, 2, 0))
        self.assertAlmostEqual(res.x(), 9.156, delta=60)
        self.assertAlmostEqual(res.y(), 391.5, delta=50)

        res = canvas.toCanvasCoordinates(QgsPoint(10, 4, 100))
        self.assertAlmostEqual(res.x(), 598, delta=15)
        self.assertAlmostEqual(res.y(), 2, delta=15)

        res = canvas.toMapCoordinates(QgsPointXY(540, 50))
        self.assertAlmostEqual(res.x(), 10, 1)
        self.assertAlmostEqual(res.y(), 3.83, delta=1)
        self.assertAlmostEqual(res.z(), 97.946, delta=20)

    def test_tool(self):
        """
        Test some plot tool logic
        """
        canvas = QgsElevationProfileCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        canvas.setProject(QgsProject.instance())
        canvas.show()
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)
        ls = QgsLineString()
        ls.fromWkt("LineString(0 2, 10 2, 10 4)")
        canvas.setProfileCurve(ls)
        canvas.setVisiblePlotRange(0, ls.length(), 0, 100)

        self.assertFalse(canvas.tool())
        tool = TestTool(canvas)
        self.assertFalse(tool.isActive())
        canvas.setTool(tool)
        self.assertEqual(canvas.tool(), tool)

        key_press_event = QKeyEvent(
            QEvent.Type.KeyPress, 54, Qt.KeyboardModifier.ShiftModifier
        )
        canvas.keyPressEvent(key_press_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.KeyPress)

        key_release_event = QKeyEvent(
            QEvent.Type.KeyRelease, 54, Qt.KeyboardModifier.ShiftModifier
        )
        canvas.keyReleaseEvent(key_release_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.KeyRelease)

        mouse_dbl_click_event = QMouseEvent(
            QEvent.Type.MouseButtonDblClick,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mouseDoubleClickEvent(mouse_dbl_click_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.MouseButtonDblClick)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 5.92, delta=0.6)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 49.165, delta=5)

        mouse_move_event = QMouseEvent(
            QEvent.Type.MouseMove,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mouseMoveEvent(mouse_move_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.MouseMove)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 5.92, delta=10)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 49.165, delta=5)

        mouse_press_event = QMouseEvent(
            QEvent.Type.MouseButtonPress,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mousePressEvent(mouse_press_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.MouseButtonPress)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 5.927, delta=1)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 49.165, delta=5)

        mouse_release_event = QMouseEvent(
            QEvent.Type.MouseButtonRelease,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mouseReleaseEvent(mouse_release_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.MouseButtonRelease)
        self.assertIsInstance(tool.events[-1], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[-1].mapPoint().x(), 5.927, delta=1)
        self.assertAlmostEqual(tool.events[-1].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[-1].mapPoint().z(), 49.165, delta=5)

        wheel_event = QWheelEvent(
            QPointF(300, 200),
            QPointF(300, 200),
            QPoint(1, 2),
            QPoint(3, 4),
            Qt.MouseButton.NoButton,
            Qt.KeyboardModifier.NoModifier,
            Qt.ScrollPhase.ScrollBegin,
            False,
        )
        canvas.wheelEvent(wheel_event)
        self.assertEqual(tool.events[-1].type(), QEvent.Type.Wheel)


if __name__ == "__main__":
    unittest.main()
