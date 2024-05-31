"""QGIS Unit tests for QgsGeometryPaintDevice.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2024 by Nyall Dawson'
__date__ = '21/05/2024'
__copyright__ = 'Copyright 2024, The QGIS Project'

import os
import unittest

from qgis.PyQt.QtCore import (
    Qt,
    QSize,
    QLine,
    QLineF,
    QPoint,
    QPointF,
    QRect,
    QRectF
)
from qgis.PyQt.QtGui import (
    QColor,
    QPainter,
    QPen,
    QTransform,
    QPaintDevice,
    QPolygon,
    QPolygonF,
    QPainterPath
)
from qgis.core import (
    QgsGeometryPaintDevice
)
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class SafePainter(QPainter):
    """
    A painter which doesn't cause a segfault when a test fails
    """

    def __init__(self, device):
        super().__init__(device)
        self._device = device

    def __del__(self):
        if self.isActive():
            self.end()
        del self._device


class TestQgsGeometryPaintDevice(QgisTestCase):

    def test_lines(self):
        """
        Test drawing lines
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawLines(
            [QLineF(5.5, 10.7, 6.8, 12.9),
             QLineF(15.5, 12.7, 3.8, 42.9)]
        )
        painter.drawLine(
            QLine(-4, -1, 2, 3)
        )
        painter.end()

        self.assertEqual(device.geometry().asWkt(2),
                         'GeometryCollection (LineString (5.5 10.7, 6.8 12.9),LineString (15.5 12.7, 3.8 42.9),LineString (-4 -1, 2 3))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            19)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            43)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawLine(
            QLineF(5.5, 10.7, 6.8, 12.9)
        )
        painter.drawLine(
            QLineF(15.5, 12.7, 3.8, 42.9)
        )
        painter.drawLine(
            QLine(-4, -1, 2, 3)
        )

        painter.end()

        self.assertEqual(device.geometry().asWkt(2),
                         'GeometryCollection (LineString (11 32.1, 13.6 38.7),LineString (31 38.1, 7.6 128.7),LineString (-8 -3, 4 9))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            39)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            131)

    def test_stroked_lines(self):
        """
        Test drawing lines
        """
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        pen = QPen(QColor(0, 0, 0))
        pen.setWidthF(1.5)
        pen.setCapStyle(Qt.PenCapStyle.FlatCap)
        painter.setPen(pen)

        painter.drawLines(
            [QLineF(5.5, 10.7, 6.8, 12.9),
             QLineF(15.5, 12.7, 3.8, 42.9)]
        )
        painter.drawLine(
            QLine(-4, -1, 2, 3)
        )

        painter.end()

        self.assertEqual(device.geometry().asWkt(2),
                         'GeometryCollection (Polygon ((6.15 10.32, 7.45 12.52, 6.15 13.28, 4.85 11.08, 6.15 10.32)),Polygon ((16.2 12.97, 4.5 43.17, 3.1 42.63, 14.8 12.43, 16.2 12.97)),Polygon ((-3.58 -1.62, 2.42 2.38, 1.58 3.62, -4.42 -0.38, -3.58 -1.62)))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            20)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            44)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawLine(
            QLineF(5.5, 10.7, 6.8, 12.9)
        )
        painter.drawLine(
            QLineF(15.5, 12.7, 3.8, 42.9)
        )
        painter.drawLine(
            QLine(-4, -1, 2, 3)
        )

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((9.71 33.24, 12.31 39.84, 14.89 37.56, 12.29 30.96, 9.71 33.24)),Polygon ((6.2 127.89, 9 129.51, 32.4 38.91, 29.6 37.29, 6.2 127.89)),Polygon ((-8.83 -1.13, 3.17 10.87, 4.83 7.13, -7.17 -4.87, -8.83 -1.13)))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            41)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            134)

    def test_points(self):
        """
        Test drawing points
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawPoints(
            [QPointF(5.5, 10.7),
             QPointF(6.8, 12.9)]
        )
        painter.drawPoint(
            QPointF(15.5, 12.7)
        )
        painter.drawPoint(
            QPoint(-4, -1)
        )

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Point (15.5 12.7),Point (6.8 12.9),Point (5.5 10.7),Point (-4 -1))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            19)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            13)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawPoints(
            [QPointF(5.5, 10.7),
             QPointF(6.8, 12.9)]
        )
        painter.drawPoint(
            QPointF(15.5, 12.7)
        )
        painter.drawPoint(
            QPoint(-4, -1)
        )

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Point (31 38.1),Point (13.6 38.7),Point (11 32.1),Point (-8 -3))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            39)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            41)

    def test_rects(self):
        """
        Test drawing rects
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawRects(
            [QRectF(5.5, 10.7, 6.8, 12.9),
             QRectF(15.5, 12.7, 3.8, 42.9)]
        )
        painter.drawRect(
            QRect(-4, -1, 2, 3)
        )

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((15.5 12.7, 15.5 55.6, 19.3 55.6, 19.3 12.7, 15.5 12.7)),Polygon ((5.5 10.7, 5.5 23.6, 12.3 23.6, 12.3 10.7, 5.5 10.7)),Polygon ((-4 -1, -4 1, -3 1, -3 -1, -4 -1)))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            23)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            56)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawRects(
            [QRectF(5.5, 10.7, 6.8, 12.9),
             QRectF(15.5, 12.7, 3.8, 42.9)]
        )
        painter.drawRect(
            QRect(-4, -1, 2, 3)
        )

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((31 38.1, 31 166.8, 38.6 166.8, 38.6 38.1, 31 38.1)),Polygon ((11 32.1, 11 70.8, 24.6 70.8, 24.6 32.1, 11 32.1)),Polygon ((-8 -3, -8 3, -6 3, -6 -3, -8 -3)))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            46)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            169)

    def test_polygons(self):
        """
        Test drawing polygons
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawPolygon(
            [QPointF(5.5, 10.7), QPointF(6.8, 12.9),
             QPointF(15.5, 12.7), QPointF(5.5, 10.7)])
        painter.drawPolyline(
            [QPointF(-4, -1), QPointF(2, 3)]
        )
        painter.drawPolyline(
            QPolygon([QPoint(14, 11), QPoint(22, 35)])
        )

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((5.5 10.7, 6.8 12.9, 15.5 12.7, 5.5 10.7)),LineString (14 11, 22 35),LineString (-4 -1, 2 3))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            26)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            36)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawPolygon(
            [QPointF(5.5, 10.7), QPointF(6.8, 12.9),
             QPointF(15.5, 12.7), QPointF(5.5, 10.7)])
        painter.drawPolyline(
            [QPointF(-4, -1), QPointF(2, 3)]
        )
        painter.drawPolyline(
            QPolygon([QPoint(14, 11), QPoint(22, 35)])
        )

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((11 32.1, 13.6 38.7, 31 38.1, 11 32.1)),LineString (28 33, 44 105),LineString (-8 -3, 4 9))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            52)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            108)

    def test_stroked_polygons(self):
        """
        Test drawing stroked polygons
        """
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        pen = QPen(QColor(0, 0, 0))
        pen.setWidthF(1.5)
        pen.setCapStyle(Qt.PenCapStyle.FlatCap)
        painter.setPen(pen)

        painter.drawPolygon(
            [QPointF(5.5, 10.7), QPointF(6.8, 12.9),
             QPointF(15.5, 12.7), QPointF(5.5, 10.7)])
        painter.drawPolyline(
            [QPointF(-4, -1), QPointF(2, 3)]
        )
        painter.drawPolyline(
            QPolygon([QPoint(14, 11), QPoint(22, 35)])
        )

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((4.85 11.08, 6.15 13.28, 6.82 13.65, 15.52 13.45, 15.65 11.96, 5.65 9.96, 4.85 11.08),(8.71 12.11, 15.48 11.95, 15.5 12.7, 15.35 13.44, 8.71 12.11),(6.78 12.15, 7.22 12.14, 7.45 12.52, 6.8 12.9, 6.78 12.15),(5.35 11.44, 5.5 10.7, 6.15 10.32, 7 11.76, 5.35 11.44),(7 11.76, 8.71 12.11, 7.22 12.14, 7 11.76)),Polygon ((13.29 11.24, 21.29 35.24, 22.71 34.76, 14.71 10.76, 13.29 11.24)),Polygon ((-4.42 -0.38, 1.58 3.62, 2.42 2.38, -3.58 -1.62, -4.42 -0.38)))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            27)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            36)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )
        painter.setPen(pen)

        painter.drawPolygon(
            [QPointF(5.5, 10.7), QPointF(6.8, 12.9),
             QPointF(15.5, 12.7), QPointF(5.5, 10.7)])
        painter.drawPolyline(
            [QPointF(-4, -1), QPointF(2, 3)]
        )
        painter.drawPolyline(
            QPolygon([QPoint(14, 11), QPoint(22, 35)])
        )

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((9.71 33.24, 12.31 39.84, 13.63 40.95, 31.03 40.35, 31.29 35.89, 11.29 29.89, 9.71 33.24),(17.41 36.32, 30.97 35.85, 31 38.1, 30.71 40.31, 17.41 36.32),(13.57 36.45, 14.44 36.42, 14.89 37.56, 13.6 38.7, 13.57 36.45),(10.71 34.31, 11 32.1, 12.29 30.96, 14 35.29, 10.71 34.31),(14 35.29, 17.41 36.32, 14.44 36.42, 14 35.29)),Polygon ((26.58 33.71, 42.58 105.71, 45.42 104.29, 29.42 32.29, 26.58 33.71)),Polygon ((-8.83 -1.13, 3.17 10.87, 4.83 7.13, -7.17 -4.87, -8.83 -1.13)))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            54)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            110)

    def test_paths(self):
        """
        Test drawing QPainterPaths
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        path = QPainterPath()
        path.moveTo(QPointF(5.5, 10.7))
        path.lineTo(QPointF(15.5, 12.7))
        path.arcTo(-4, -1, 20, 30, 45, 20)
        path.lineTo(QPointF(5.5, 10.7))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((5.5 10.7, 15.5 12.7, 13.07 3.39, 13.07 3.39, 12.99 3.27, 12.91 3.15, 12.82 3.03, 12.74 2.91, 12.65 2.8, 12.56 2.68, 12.47 2.57, 12.39 2.46, 12.3 2.35, 12.21 2.24, 12.11 2.13, 12.02 2.02, 11.93 1.92, 11.84 1.82, 11.74 1.72, 11.65 1.62, 11.55 1.52, 11.45 1.42, 11.35 1.33, 11.26 1.24, 11.16 1.14, 11.06 1.06, 10.95 0.97, 10.85 0.88, 10.75 0.8, 10.65 0.72, 10.54 0.63, 10.44 0.56, 10.33 0.48, 10.23 0.4, 5.5 10.7)))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            10)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            12)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((11 32.1, 31 38.1, 26.14 10.18, 26.14 10.18, 25.98 9.81, 25.81 9.45, 25.64 9.09, 25.47 8.74, 25.3 8.39, 25.13 8.05, 24.95 7.71, 24.77 7.37, 24.59 7.04, 24.41 6.71, 24.23 6.39, 24.04 6.07, 23.86 5.76, 23.67 5.45, 23.48 5.15, 23.29 4.85, 23.1 4.56, 22.9 4.27, 22.71 3.99, 22.51 3.71, 22.31 3.43, 22.11 3.17, 21.91 2.9, 21.71 2.65, 21.5 2.39, 21.3 2.15, 21.09 1.9, 20.88 1.67, 20.67 1.43, 20.46 1.21, 11 32.1)))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            20)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            36)

    def test_stroked_paths(self):
        """
        Test drawing stroked QPainterPaths
        """
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        pen = QPen(QColor(0, 0, 0))
        pen.setWidthF(1.5)
        pen.setCapStyle(Qt.PenCapStyle.FlatCap)
        painter.setPen(pen)

        path = QPainterPath()
        path.moveTo(QPointF(5.5, 10.7))
        path.lineTo(QPointF(15.5, 12.7))
        path.arcTo(-4, -1, 20, 30, 45, 20)
        path.lineTo(QPointF(5.5, 10.7))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((4.82 10.39, 5.35 11.44, 15.35 13.44, 16.23 12.51, 13.8 3.2, 13.7 2.98, 13.61 2.85, 13.52 2.72, 13.43 2.6, 13.34 2.47, 13.25 2.35, 13.16 2.23, 13.07 2.11, 12.97 1.99, 12.87 1.87, 12.78 1.76, 12.68 1.64, 12.58 1.53, 12.48 1.42, 12.38 1.31, 12.28 1.2, 12.18 1.09, 12.07 0.99, 11.97 0.88, 11.86 0.78, 11.76 0.68, 11.65 0.58, 11.54 0.49, 11.43 0.39, 11.32 0.3, 11.21 0.21, 11.1 0.12, 10.99 0.03, 10.88 -0.05, 10.77 -0.13, 10.66 -0.21, 9.55 0.09, 4.82 10.39),(6.58 10.15, 10.51 1.58, 10.57 1.62, 10.66 1.71, 10.75 1.79, 10.85 1.88, 10.94 1.96, 11.03 2.05, 11.12 2.14, 11.2 2.23, 11.29 2.33, 11.38 2.42, 11.46 2.52, 11.55 2.62, 11.63 2.72, 11.72 2.82, 11.8 2.92, 11.88 3.03, 11.97 3.13, 12.05 3.24, 12.13 3.35, 12.21 3.46, 12.29 3.58, 12.37 3.69, 12.38 3.71, 14.47 11.73, 6.58 10.15),(9.8 1.02, 10.23 0.4, 10.91 0.72, 10.51 1.58, 10.48 1.54, 10.38 1.46, 10.29 1.39, 10.19 1.31, 10.09 1.23, 10 1.16, 9.9 1.09, 9.8 1.02),(14.47 11.73, 15.65 11.96, 15.5 12.7, 14.77 12.89, 14.47 11.73),(12.35 3.58, 13.07 3.39, 12.45 3.81, 12.38 3.71, 12.35 3.58),(5.5 10.7, 5.65 9.96, 6.58 10.15, 6.18 11.01, 5.5 10.7)))')

        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            11)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            13)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(
            QTransform.fromScale(2, 3)
        )

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(result.asWkt(2),
                         'GeometryCollection (Polygon ((9.64 31.16, 10.71 34.31, 30.71 40.31, 32.45 37.53, 27.59 9.61, 27.39 8.93, 27.22 8.55, 27.04 8.17, 26.87 7.79, 26.69 7.42, 26.5 7.05, 26.32 6.69, 26.13 6.33, 25.94 5.97, 25.75 5.62, 25.56 5.27, 25.36 4.92, 25.16 4.58, 24.96 4.25, 24.76 3.92, 24.56 3.59, 24.35 3.27, 24.14 2.96, 23.93 2.65, 23.72 2.34, 23.51 2.04, 23.3 1.75, 23.08 1.46, 22.87 1.18, 22.65 0.9, 22.43 0.63, 22.21 0.36, 21.99 0.1, 21.77 -0.15, 21.54 -0.39, 21.32 -0.63, 19.09 0.27, 9.64 31.16),(13.16 30.45, 21.03 4.73, 21.14 4.87, 21.33 5.12, 21.51 5.37, 21.69 5.63, 21.87 5.89, 22.05 6.16, 22.23 6.43, 22.41 6.7, 22.58 6.99, 22.76 7.27, 22.93 7.56, 23.1 7.86, 23.27 8.16, 23.44 8.46, 23.6 8.77, 23.77 9.08, 23.93 9.4, 24.1 9.73, 24.26 10.06, 24.42 10.39, 24.58 10.73, 24.74 11.08, 24.76 11.12, 28.94 35.19, 13.16 30.45),(19.6 3.05, 20.46 1.21, 21.82 2.15, 21.03 4.73, 20.95 4.63, 20.76 4.39, 20.57 4.16, 20.38 3.93, 20.19 3.7, 19.99 3.48, 19.79 3.26, 19.6 3.05),(28.94 35.19, 31.29 35.89, 31 38.1, 29.55 38.67, 28.94 35.19),(24.69 10.75, 26.14 10.18, 24.89 11.43, 24.76 11.12, 24.69 10.75),(11 32.1, 11.29 29.89, 13.16 30.45, 12.36 33.04, 11 32.1)))')
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth),
            22)
        self.assertEqual(
            device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight),
            40)


if __name__ == '__main__':
    unittest.main()
