"""QGIS Unit tests for QgsElevationProfileCanvas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "28/3/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import unittest

from qgis.core import (
    QgsAbstractProfileSource,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsPluginLayer,
    QgsPoint,
    QgsPointXY,
    QgsProfileRequest,
    QgsProject,
    QgsRasterLayer,
    QgsVectorLayer,
)
from qgis.gui import QgsElevationProfileCanvas, QgsPlotMouseEvent, QgsPlotTool
from qgis.PyQt.QtCore import QEvent, QPoint, QPointF, Qt
from qgis.PyQt.QtGui import QKeyEvent, QMouseEvent, QWheelEvent
from qgis.testing import QgisTestCase, start_app

app = start_app()


class TestTool(QgsPlotTool):
    def __init__(self, canvas):
        super().__init__(canvas, "Test")
        self.events = []

    def plotMoveEvent(self, event):
        self.events.append(QgsPlotMouseEvent(self.canvas(), event))

    def plotDoubleClickEvent(self, event):
        self.events.append(QgsPlotMouseEvent(self.canvas(), event))

    def plotPressEvent(self, event):
        self.events.append(QgsPlotMouseEvent(self.canvas(), event))

    def plotReleaseEvent(self, event):
        self.events.append(QgsPlotMouseEvent(self.canvas(), event))

    def wheelEvent(self, event):
        self.events.append(
            QWheelEvent(
                event.position(),
                event.globalPosition(),
                event.pixelDelta(),
                event.angleDelta(),
                event.buttons(),
                event.modifiers(),
                event.phase(),
                event.inverted(),
            )
        )

    def keyPressEvent(self, event):
        self.events.append(
            QKeyEvent(
                event.type(),
                event.key(),
                event.modifiers(),
                event.text(),
                event.isAutoRepeat(),
                event.count(),
            )
        )

    def keyReleaseEvent(self, event):
        self.events.append(
            QKeyEvent(
                event.type(),
                event.key(),
                event.modifiers(),
                event.text(),
                event.isAutoRepeat(),
                event.count(),
            )
        )


class MyPluginLayerNoSource(QgsPluginLayer):
    LAYER_TYPE = "MyPluginLayerNoSource"

    def __init__(self):
        super().__init__(MyPluginLayerNoSource.LAYER_TYPE, "new_plugin_layer")


class MyPluginLayer(QgsPluginLayer):
    LAYER_TYPE = "MyPluginLayer"

    def __init__(self):
        super().__init__(MyPluginLayer.LAYER_TYPE, "new_plugin_layer")
        self._profile_source = MyPluginLayerProfileSource(self)

    def profileSource(self):
        return self._profile_source


class MyPluginLayerProfileSource(QgsAbstractProfileSource):
    def __init__(self, layer: MyPluginLayer):
        super().__init__()
        self._layer = layer

    def profileSourceId(self):
        return self._layer.id()

    def profileSourceName(self):
        return self._layer.name()

    def createProfileGenerator(self, request: QgsProfileRequest):
        return None


class MyDummyProfileSource(QgsAbstractProfileSource):
    def __init__(self):
        QgsAbstractProfileSource.__init__(self)

    def createProfileGenerator(self, request):
        return None  # MyDummyProfileGenerator(request)

    def profileSourceName(self):
        return "My Dummy Profile"

    def profileSourceId(self):
        return "my-dummy-profile"


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

        self.assertFalse(tool.events)
        key_press_event = QKeyEvent(
            QEvent.Type.KeyPress, 54, Qt.KeyboardModifier.ShiftModifier
        )
        canvas.keyPressEvent(key_press_event)
        self.assertEqual(len(tool.events), 1)
        self.assertEqual(tool.events[0].type(), QEvent.Type.KeyPress)

        key_release_event = QKeyEvent(
            QEvent.Type.KeyRelease, 54, Qt.KeyboardModifier.ShiftModifier
        )
        canvas.keyReleaseEvent(key_release_event)
        self.assertEqual(len(tool.events), 2)
        self.assertEqual(tool.events[1].type(), QEvent.Type.KeyRelease)

        mouse_dbl_click_event = QMouseEvent(
            QEvent.Type.MouseButtonDblClick,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        self.assertEqual(mouse_dbl_click_event.type(), QEvent.Type.MouseButtonDblClick)
        canvas.mouseDoubleClickEvent(mouse_dbl_click_event)
        self.assertEqual(len(tool.events), 3)
        self.assertEqual(tool.events[2].type(), QEvent.Type.MouseButtonDblClick)
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
        self.assertEqual(len(tool.events), 4)
        self.assertEqual(tool.events[3].type(), QEvent.Type.MouseMove)
        self.assertIsInstance(tool.events[3], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[3].mapPoint().x(), 5.92, delta=10)
        self.assertAlmostEqual(tool.events[3].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[3].mapPoint().z(), 49.165, delta=5)

        mouse_press_event = QMouseEvent(
            QEvent.Type.MouseButtonPress,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mousePressEvent(mouse_press_event)
        self.assertEqual(len(tool.events), 5)
        self.assertEqual(tool.events[4].type(), QEvent.Type.MouseButtonPress)
        self.assertIsInstance(tool.events[4], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[4].mapPoint().x(), 5.927, delta=1)
        self.assertAlmostEqual(tool.events[4].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[4].mapPoint().z(), 49.165, delta=5)

        mouse_release_event = QMouseEvent(
            QEvent.Type.MouseButtonRelease,
            QPointF(300, 200),
            Qt.MouseButton.LeftButton,
            Qt.MouseButtons(),
            Qt.KeyboardModifier.ShiftModifier,
        )
        canvas.mouseReleaseEvent(mouse_release_event)
        self.assertEqual(len(tool.events), 6)
        self.assertEqual(tool.events[5].type(), QEvent.Type.MouseButtonRelease)
        self.assertIsInstance(tool.events[5], QgsPlotMouseEvent)
        self.assertAlmostEqual(tool.events[5].mapPoint().x(), 5.927, delta=1)
        self.assertAlmostEqual(tool.events[5].mapPoint().y(), 2, 4)
        self.assertAlmostEqual(tool.events[5].mapPoint().z(), 49.165, delta=5)

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
        self.assertEqual(len(tool.events), 7)
        self.assertEqual(tool.events[6].type(), QEvent.Type.Wheel)

    def test_ratio(self):
        """
        Test axis scale ratio logic
        """
        canvas = QgsElevationProfileCanvas()
        canvas.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        canvas.setFrameStyle(0)
        # make a 2:1 canvas, to make the maths easier!
        canvas.resize(800, 400)
        canvas.setProject(QgsProject.instance())
        canvas.show()
        self.assertEqual(canvas.width(), 800)
        self.assertEqual(canvas.height(), 400)

        canvas.setVisiblePlotRange(100, 200, 50, 150)
        # showing 100m distance, 100m elevation
        # distance:elevation ratio is 1:2, as canvas is twice as wide as high
        self.assertAlmostEqual(canvas.axisScaleRatio(), 0.5, 1)

        canvas.setVisiblePlotRange(100, 300, 50, 150)
        # showing 200m distance, 100m elevation
        # distance:elevation ratio is 1:1, as canvas is twice as wide as high
        self.assertAlmostEqual(canvas.axisScaleRatio(), 1.0, 1)

        canvas.setVisiblePlotRange(100, 500, 50, 150)
        # showing 400m distance, 100m elevation
        # distance:elevation ratio is 2:1, as canvas is twice as wide as high
        self.assertAlmostEqual(canvas.axisScaleRatio(), 2.0, delta=0.1)

        canvas.setAxisScaleRatio(0.5)
        self.assertAlmostEqual(canvas.axisScaleRatio(), 0.5, 1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().lower(), 248.024, delta=1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().upper(), 351.976, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().lower(), 50.0, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().upper(), 150.0, delta=1)

        canvas.setAxisScaleRatio(1.0)
        self.assertAlmostEqual(canvas.axisScaleRatio(), 1.0, 1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().lower(), 248.024, delta=1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().upper(), 351.976, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().lower(), 75.0, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().upper(), 125.0, delta=1)

        canvas.setAxisScaleRatio(2.0)
        self.assertAlmostEqual(canvas.axisScaleRatio(), 2.0, 1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().lower(), 248.024, delta=1)
        self.assertAlmostEqual(canvas.visibleDistanceRange().upper(), 351.976, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().lower(), 87.5, delta=1)
        self.assertAlmostEqual(canvas.visibleElevationRange().upper(), 112.5, delta=1)

    def testSourceGettersSetters(self):
        canvas = QgsElevationProfileCanvas()
        vl1 = QgsVectorLayer()

        # Not allowed types
        def _source_tests(sources):
            with self.assertRaises(TypeError):
                canvas.setSources(sources)
            self.assertEqual(len(canvas.sources()), 0)

        _source_tests(3)
        _source_tests([3])
        _source_tests([])
        _source_tests([None])
        _source_tests([QgsPoint(0, 0)])
        _source_tests(vl1)
        _source_tests([vl1, 3])
        _source_tests([vl1, QgsPoint(0, 0)])

        # Allowed types
        canvas.setSources([vl1])
        self.assertEqual(len(canvas.sources()), 1)

        rl1 = QgsRasterLayer()
        canvas.setSources([vl1, rl1])
        self.assertEqual(len(canvas.sources()), 2)

        pl1 = MyPluginLayerNoSource()  # Plugin layer with no profile source
        with self.assertRaises(ValueError):
            canvas.setSources([vl1, rl1, pl1])
        self.assertEqual(len(canvas.sources()), 2)

        pl2 = MyPluginLayer()  # Plugin layer with valid profile source
        canvas.setSources([vl1, rl1, pl2])
        self.assertEqual(len(canvas.sources()), 3)

        ps1 = MyDummyProfileSource()  # Custom profile source
        QgsApplication.profileSourceRegistry().registerProfileSource(ps1)
        canvas.setSources([vl1, rl1, pl2, ps1])
        self.assertEqual(len(canvas.sources()), 4)
        QgsApplication.profileSourceRegistry().unregisterProfileSource(
            ps1.profileSourceId()
        )


if __name__ == "__main__":
    unittest.main()
