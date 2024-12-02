"""QGIS Unit tests for QgsGeometryPaintDevice.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2024 by Nyall Dawson"
__date__ = "21/05/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

import os
import unittest

from qgis.PyQt.QtCore import Qt, QSize, QLine, QLineF, QPoint, QPointF, QRect, QRectF
from qgis.PyQt.QtGui import (
    QColor,
    QPainter,
    QPen,
    QTransform,
    QPaintDevice,
    QPolygon,
    QPolygonF,
    QPainterPath,
)
from qgis.core import QgsGeometryPaintDevice
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath, getTestFont

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

        painter.drawLines([QLineF(5.5, 10.7, 6.8, 12.9), QLineF(15.5, 12.7, 3.8, 42.9)])
        painter.drawLine(QLine(-4, -1, 2, 3))
        painter.end()

        result = device.geometry()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (LineString (15.5 12.7, 3.8 42.9),LineString (5.5 10.7, 6.8 12.9),LineString (-4 -1, 2 3))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 19)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 43)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawLine(QLineF(5.5, 10.7, 6.8, 12.9))
        painter.drawLine(QLineF(15.5, 12.7, 3.8, 42.9))
        painter.drawLine(QLine(-4, -1, 2, 3))

        painter.end()

        result = device.geometry()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (LineString (31 38.1, 7.6 128.7),LineString (11 32.1, 13.6 38.7),LineString (-8 -3, 4 9))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 39)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 131)

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

        painter.drawLines([QLineF(5.5, 10.7, 6.8, 12.9), QLineF(15.5, 12.7, 3.8, 42.9)])
        painter.drawLine(QLine(-4, -1, 2, 3))

        painter.end()

        result = device.geometry()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((4.85 11.08, 6.15 13.28, 7.45 12.52, 6.15 10.32, 4.85 11.08)),Polygon ((3.1 42.63, 4.5 43.17, 16.2 12.97, 14.8 12.43, 3.1 42.63)),Polygon ((-4.42 -0.38, 1.58 3.62, 2.42 2.38, -3.58 -1.62, -4.42 -0.38)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 20)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 44)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawLine(QLineF(5.5, 10.7, 6.8, 12.9))
        painter.drawLine(QLineF(15.5, 12.7, 3.8, 42.9))
        painter.drawLine(QLine(-4, -1, 2, 3))

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((9.71 33.24, 12.31 39.84, 14.89 37.56, 12.29 30.96, 9.71 33.24)),Polygon ((6.2 127.89, 9 129.51, 32.4 38.91, 29.6 37.29, 6.2 127.89)),Polygon ((-8.83 -1.13, 3.17 10.87, 4.83 7.13, -7.17 -4.87, -8.83 -1.13)))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 41)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 134)

        # with on the fly simplification
        device = QgsGeometryPaintDevice(usePathStroker=True)
        device.setSimplificationTolerance(5)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawLine(QLineF(5.5, 10.7, 6.8, 12.9))
        painter.drawLine(QLineF(15.5, 12.7, 3.8, 42.9))
        painter.drawLine(QLine(-4, -1, 2, 3))

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((12.29 30.96, 12.31 39.84, 12.29 30.96)),Polygon ((6.2 127.89, 29.6 37.29, 6.2 127.89)),Polygon ((-7.17 -4.87, 3.17 10.87, -7.17 -4.87)))",
        )

    def test_points(self):
        """
        Test drawing points
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawPoints(QPolygonF([QPointF(5.5, 10.7), QPointF(6.8, 12.9)]))
        painter.drawPoint(QPointF(15.5, 12.7))
        painter.drawPoint(QPoint(-4, -1))

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Point (15.5 12.7),Point (6.8 12.9),Point (5.5 10.7),Point (-4 -1))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 19)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 13)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPoints(QPolygonF([QPointF(5.5, 10.7), QPointF(6.8, 12.9)]))
        painter.drawPoint(QPointF(15.5, 12.7))
        painter.drawPoint(QPoint(-4, -1))

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Point (31 38.1),Point (13.6 38.7),Point (11 32.1),Point (-8 -3))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 39)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 41)

    def test_rects(self):
        """
        Test drawing rects
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawRects([QRectF(5.5, 10.7, 6.8, 12.9), QRectF(15.5, 12.7, 3.8, 42.9)])
        painter.drawRect(QRect(-4, -1, 2, 3))

        painter.end()

        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((15.5 12.7, 15.5 55.6, 19.3 55.6, 19.3 12.7, 15.5 12.7)),Polygon ((5.5 10.7, 5.5 23.6, 12.3 23.6, 12.3 10.7, 5.5 10.7)),Polygon ((-4 -1, -4 1, -3 1, -3 -1, -4 -1)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 23)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 56)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawRects([QRectF(5.5, 10.7, 6.8, 12.9), QRectF(15.5, 12.7, 3.8, 42.9)])
        painter.drawRect(QRect(-4, -1, 2, 3))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((31 38.1, 31 166.8, 38.6 166.8, 38.6 38.1, 31 38.1)),Polygon ((11 32.1, 11 70.8, 24.6 70.8, 24.6 32.1, 11 32.1)),Polygon ((-8 -3, -8 3, -6 3, -6 -3, -8 -3)))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 46)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 169)

    def test_polygons(self):
        """
        Test drawing polygons
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)

        painter.drawPolygon(
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((5.5 10.7, 6.8 12.9, 15.5 12.7, 5.5 10.7)),LineString (14 11, 22 35),LineString (-4 -1, 2 3))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 26)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 36)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPolygon(
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((11 32.1, 13.6 38.7, 31 38.1, 11 32.1)),LineString (28 33, 44 105),LineString (-8 -3, 4 9))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 52)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 108)

        # with on the fly simplification
        device = QgsGeometryPaintDevice()
        device.setSimplificationTolerance(10)
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPolygon(
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((11 32.1, 31 38.1, 11 32.1)),LineString (28 33, 44 105),LineString (-8 -3, 4 9))",
        )

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
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((4.85 11.08, 6.15 13.28, 6.82 13.65, 15.52 13.45, 15.65 11.96, 5.65 9.96, 4.85 11.08),(7 11.76, 8.71 12.11, 7.22 12.14, 7 11.76)),Polygon ((13.29 11.24, 21.29 35.24, 22.71 34.76, 14.71 10.76, 13.29 11.24)),Polygon ((-4.42 -0.38, 1.58 3.62, 2.42 2.38, -3.58 -1.62, -4.42 -0.38)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 27)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 36)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))
        painter.setPen(pen)

        painter.drawPolygon(
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((9.71 33.24, 12.31 39.84, 13.63 40.95, 31.03 40.35, 31.29 35.89, 11.29 29.89, 9.71 33.24),(14 35.29, 17.41 36.32, 14.44 36.42, 14 35.29)),Polygon ((26.58 33.71, 42.58 105.71, 45.42 104.29, 29.42 32.29, 26.58 33.71)),Polygon ((-8.83 -1.13, 3.17 10.87, 4.83 7.13, -7.17 -4.87, -8.83 -1.13)))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 54)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 110)

        # with on the fly simplification
        device = QgsGeometryPaintDevice(usePathStroker=True)
        device.setSimplificationTolerance(10)
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))
        painter.setPen(pen)

        painter.drawPolygon(
            QPolygonF(
                [
                    QPointF(5.5, 10.7),
                    QPointF(6.8, 12.9),
                    QPointF(15.5, 12.7),
                    QPointF(5.5, 10.7),
                ]
            )
        )
        painter.drawPolyline(QPolygonF([QPointF(-4, -1), QPointF(2, 3)]))
        painter.drawPolyline(QPolygon([QPoint(14, 11), QPoint(22, 35)]))

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((29.42 32.29, 42.58 105.71, 29.42 32.29)),Polygon ((10.71 34.31, 30.71 40.31, 10.71 34.31)),Polygon ((-7.17 -4.87, 3.17 10.87, -7.17 -4.87)))",
        )

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
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((5.5 10.7, 15.5 12.7, 13.07 3.39, 13.07 3.39, 12.99 3.27, 12.91 3.15, 12.82 3.03, 12.74 2.91, 12.65 2.8, 12.56 2.68, 12.47 2.57, 12.39 2.46, 12.3 2.35, 12.21 2.24, 12.11 2.13, 12.02 2.02, 11.93 1.92, 11.84 1.82, 11.74 1.72, 11.65 1.62, 11.55 1.52, 11.45 1.42, 11.35 1.33, 11.26 1.24, 11.16 1.14, 11.06 1.06, 10.95 0.97, 10.85 0.88, 10.75 0.8, 10.65 0.72, 10.54 0.63, 10.44 0.56, 10.33 0.48, 10.23 0.4, 5.5 10.7)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 10)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 12)

        # with transform
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((11 32.1, 31 38.1, 26.14 10.18, 26.14 10.18, 25.98 9.81, 25.81 9.45, 25.64 9.09, 25.47 8.74, 25.3 8.39, 25.13 8.05, 24.95 7.71, 24.77 7.37, 24.59 7.04, 24.41 6.71, 24.23 6.39, 24.04 6.07, 23.86 5.76, 23.67 5.45, 23.48 5.15, 23.29 4.85, 23.1 4.56, 22.9 4.27, 22.71 3.99, 22.51 3.71, 22.31 3.43, 22.11 3.17, 21.91 2.9, 21.71 2.65, 21.5 2.39, 21.3 2.15, 21.09 1.9, 20.88 1.67, 20.67 1.43, 20.46 1.21, 11 32.1)))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 20)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 36)

        # with on the fly simplification
        device = QgsGeometryPaintDevice()
        device.setSimplificationTolerance(10)
        painter = SafePainter(device)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((11 32.1, 31 38.1, 20.46 1.21, 11 32.1)))",
        )

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
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((4.82 10.39, 5.35 11.44, 15.35 13.44, 16.23 12.51, 13.8 3.2, 13.69 2.97, 13.61 2.85, 13.61 2.85, 13.52 2.72, 13.52 2.72, 13.43 2.6, 13.43 2.59, 13.34 2.47, 13.34 2.47, 13.25 2.35, 13.25 2.34, 13.16 2.23, 13.16 2.22, 13.07 2.11, 13.06 2.1, 12.97 1.99, 12.97 1.98, 12.88 1.87, 12.87 1.87, 12.78 1.76, 12.78 1.75, 12.69 1.64, 12.68 1.64, 12.59 1.53, 12.58 1.52, 12.49 1.42, 12.48 1.41, 12.39 1.31, 12.38 1.3, 12.29 1.2, 12.28 1.2, 12.19 1.1, 12.18 1.09, 12.08 0.99, 12.08 0.99, 11.98 0.89, 11.97 0.88, 11.87 0.79, 11.87 0.78, 11.77 0.69, 11.76 0.68, 11.66 0.59, 11.66 0.59, 11.56 0.5, 11.55 0.49, 11.45 0.4, 11.44 0.4, 11.34 0.31, 11.33 0.3, 11.23 0.22, 11.22 0.21, 11.12 0.13, 11.11 0.12, 11 0.04, 11 0.04, 10.89 -0.04, 10.88 -0.05, 10.78 -0.13, 10.77 -0.13, 10.66 -0.21, 9.55 0.09, 4.82 10.39),(6.58 10.15, 10.51 1.58, 10.56 1.62, 10.65 1.7, 10.75 1.79, 10.84 1.87, 10.93 1.96, 11.02 2.05, 11.11 2.14, 11.2 2.23, 11.29 2.33, 11.37 2.42, 11.46 2.52, 11.55 2.62, 11.63 2.72, 11.72 2.82, 11.8 2.93, 11.88 3.03, 11.97 3.14, 12.05 3.25, 12.13 3.36, 12.21 3.47, 12.29 3.58, 12.37 3.69, 12.38 3.71, 14.47 11.73, 6.58 10.15)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 11)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 13)

        # with transform
        device = QgsGeometryPaintDevice(usePathStroker=True)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((9.64 31.16, 10.71 34.31, 30.71 40.31, 32.45 37.53, 27.59 9.61, 27.39 8.92, 27.22 8.56, 27.21 8.54, 27.05 8.17, 27.04 8.15, 26.87 7.8, 26.86 7.78, 26.69 7.42, 26.68 7.4, 26.51 7.05, 26.5 7.03, 26.32 6.69, 26.31 6.67, 26.14 6.33, 26.12 6.31, 25.95 5.97, 25.94 5.95, 25.76 5.62, 25.75 5.6, 25.57 5.27, 25.55 5.25, 25.37 4.93, 25.36 4.91, 25.18 4.59, 25.16 4.57, 24.98 4.26, 24.97 4.24, 24.78 3.93, 24.77 3.91, 24.58 3.61, 24.56 3.59, 24.37 3.29, 24.36 3.27, 24.17 2.98, 24.15 2.96, 23.96 2.67, 23.95 2.65, 23.75 2.37, 23.74 2.35, 23.54 2.07, 23.52 2.05, 23.33 1.78, 23.31 1.76, 23.11 1.49, 23.1 1.47, 22.89 1.2, 22.88 1.19, 22.67 0.93, 22.66 0.91, 22.45 0.66, 22.44 0.64, 22.23 0.39, 22.22 0.37, 22.01 0.13, 21.99 0.11, 21.78 -0.13, 21.77 -0.15, 21.56 -0.38, 21.54 -0.4, 21.33 -0.62, 19.09 0.27, 9.64 31.16),(13.16 30.45, 21.03 4.73, 21.12 4.85, 21.31 5.1, 21.49 5.36, 21.67 5.62, 21.86 5.88, 22.04 6.15, 22.22 6.42, 22.39 6.7, 22.57 6.98, 22.75 7.27, 22.92 7.56, 23.09 7.86, 23.26 8.16, 23.43 8.47, 23.6 8.78, 23.77 9.09, 23.93 9.41, 24.1 9.74, 24.26 10.07, 24.42 10.4, 24.58 10.74, 24.74 11.08, 24.76 11.12, 28.94 35.19, 13.16 30.45)))",
        )
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 22)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 40)

        # with on the fly simplification
        device = QgsGeometryPaintDevice(usePathStroker=True)
        device.setSimplificationTolerance(10)
        painter = SafePainter(device)
        painter.setPen(pen)
        painter.setTransform(QTransform.fromScale(2, 3))

        painter.drawPath(path)

        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((9.64 31.16, 32.38 37.21, 21.83 0.32, 9.64 31.16),(13.16 30.45, 20.41 6.75, 28.5 35.05, 13.16 30.45)))",
        )

    def test_text(self):
        """
        Test drawing text.

        This also tests that drawing paths with holes works correctly
        """
        device = QgsGeometryPaintDevice()
        painter = SafePainter(device)
        font = getTestFont("bold")
        font.setPixelSize(40)
        painter.setFont(font)
        painter.drawText(0, 0, "abc")
        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((57.33 -10.92, 57.33 -10.56, 57.34 -10.21, 57.36 -9.86, 57.39 -9.52, 57.42 -9.19, 57.46 -8.85, 57.51 -8.53, 57.57 -8.21, 57.63 -7.89, 57.7 -7.58, 57.78 -7.28, 57.86 -6.98, 57.96 -6.68, 58.06 -6.39, 58.16 -6.11, 58.28 -5.83, 58.4 -5.55, 58.53 -5.29, 58.67 -5.02, 58.81 -4.77, 58.97 -4.51, 59.13 -4.27, 59.29 -4.02, 59.47 -3.79, 59.65 -3.56, 59.84 -3.33, 60.04 -3.11, 60.24 -2.89, 60.45 -2.68, 60.67 -2.48, 60.9 -2.28, 61.13 -2.08, 61.37 -1.9, 61.61 -1.72, 61.86 -1.55, 62.11 -1.38, 62.37 -1.22, 62.64 -1.07, 62.91 -0.93, 63.19 -0.79, 63.47 -0.66, 63.76 -0.53, 64.05 -0.41, 64.35 -0.3, 64.66 -0.2, 64.97 -0.1, 65.28 -0.01, 65.61 0.08, 65.94 0.15, 66.27 0.22, 66.61 0.29, 66.95 0.35, 67.31 0.4, 67.66 0.44, 68.02 0.48, 68.39 0.51, 68.77 0.53, 69.15 0.55, 69.53 0.56, 69.92 0.56, 70.04 0.56, 70.15 0.56, 70.26 0.56, 70.38 0.56, 70.49 0.55, 70.6 0.55, 70.72 0.55, 70.83 0.54, 70.94 0.54, 71.06 0.53, 71.17 0.52, 71.28 0.51, 71.39 0.51, 71.51 0.5, 71.62 0.49, 71.73 0.48, 71.85 0.46, 71.96 0.45, 72.07 0.44, 72.19 0.43, 72.3 0.41, 72.41 0.4, 72.52 0.38, 72.64 0.37, 72.75 0.35, 72.86 0.33, 72.97 0.32, 73.09 0.3, 73.2 0.28, 73.31 0.26, 73.42 0.24, 73.54 0.22, 73.65 0.19, 73.76 0.17, 73.87 0.15, 73.99 0.12, 74.1 0.1, 74.21 0.07, 74.32 0.05, 74.43 0.02, 74.54 0, 74.66 -0.03, 74.77 -0.06, 74.88 -0.09, 74.99 -0.12, 75.1 -0.15, 75.21 -0.18, 75.32 -0.21, 75.43 -0.24, 75.54 -0.28, 75.65 -0.31, 75.76 -0.34, 75.87 -0.38, 75.98 -0.41, 76.09 -0.45, 76.2 -0.49, 76.31 -0.52, 76.42 -0.56, 76.53 -0.6, 76.64 -0.64, 76.64 -6.38, 76.56 -6.31, 76.47 -6.25, 76.39 -6.19, 76.3 -6.13, 76.22 -6.07, 76.13 -6.01, 76.05 -5.95, 75.96 -5.89, 75.87 -5.84, 75.78 -5.78, 75.69 -5.73, 75.61 -5.68, 75.52 -5.63, 75.43 -5.58, 75.34 -5.53, 75.25 -5.48, 75.15 -5.43, 75.06 -5.39, 74.97 -5.34, 74.88 -5.3, 74.78 -5.26, 74.69 -5.21, 74.6 -5.17, 74.5 -5.14, 74.41 -5.1, 74.31 -5.06, 74.21 -5.02, 74.12 -4.99, 74.02 -4.95, 73.92 -4.92, 73.82 -4.89, 73.73 -4.86, 73.63 -4.83, 73.53 -4.8, 73.43 -4.77, 73.33 -4.75, 73.23 -4.72, 73.13 -4.7, 73.03 -4.67, 72.92 -4.65, 72.82 -4.63, 72.72 -4.61, 72.62 -4.59, 72.51 -4.58, 72.41 -4.56, 72.31 -4.54, 72.2 -4.53, 72.1 -4.52, 71.99 -4.5, 71.89 -4.49, 71.78 -4.48, 71.68 -4.47, 71.57 -4.46, 71.46 -4.46, 71.35 -4.45, 71.25 -4.45, 71.14 -4.44, 71.03 -4.44, 70.92 -4.44, 70.81 -4.44, 70.62 -4.44, 70.43 -4.45, 70.24 -4.45, 70.05 -4.47, 69.87 -4.49, 69.69 -4.51, 69.51 -4.53, 69.34 -4.56, 69.17 -4.59, 69 -4.63, 68.83 -4.67, 68.67 -4.71, 68.51 -4.76, 68.35 -4.81, 68.2 -4.87, 68.05 -4.92, 67.9 -4.99, 67.76 -5.05, 67.61 -5.12, 67.48 -5.2, 67.34 -5.28, 67.21 -5.36, 67.08 -5.44, 66.95 -5.53, 66.82 -5.63, 66.7 -5.72, 66.58 -5.82, 66.47 -5.93, 66.35 -6.04, 66.24 -6.15, 66.14 -6.26, 66.03 -6.38, 65.93 -6.5, 65.84 -6.63, 65.74 -6.76, 65.65 -6.89, 65.57 -7.02, 65.49 -7.16, 65.41 -7.3, 65.34 -7.44, 65.26 -7.58, 65.2 -7.73, 65.13 -7.89, 65.07 -8.04, 65.02 -8.2, 64.96 -8.36, 64.92 -8.52, 64.87 -8.69, 64.83 -8.86, 64.79 -9.03, 64.76 -9.21, 64.73 -9.38, 64.7 -9.57, 64.67 -9.75, 64.65 -9.94, 64.64 -10.13, 64.63 -10.32, 64.62 -10.52, 64.61 -10.72, 64.61 -10.92, 64.61 -11.12, 64.62 -11.32, 64.63 -11.52, 64.64 -11.71, 64.65 -11.91, 64.67 -12.09, 64.7 -12.28, 64.73 -12.46, 64.76 -12.64, 64.79 -12.81, 64.83 -12.99, 64.87 -13.16, 64.92 -13.32, 64.96 -13.49, 65.02 -13.65, 65.07 -13.8, 65.13 -13.96, 65.2 -14.11, 65.26 -14.26, 65.34 -14.4, 65.41 -14.55, 65.49 -14.69, 65.57 -14.82, 65.65 -14.96, 65.74 -15.09, 65.84 -15.22, 65.93 -15.34, 66.03 -15.46, 66.14 -15.58, 66.24 -15.7, 66.35 -15.81, 66.47 -15.92, 66.58 -16.02, 66.7 -16.12, 66.82 -16.22, 66.95 -16.31, 67.08 -16.4, 67.21 -16.49, 67.34 -16.57, 67.48 -16.65, 67.61 -16.72, 67.76 -16.79, 67.9 -16.86, 68.05 -16.92, 68.2 -16.98, 68.35 -17.03, 68.51 -17.08, 68.67 -17.13, 68.83 -17.18, 69 -17.22, 69.17 -17.25, 69.34 -17.28, 69.51 -17.31, 69.69 -17.34, 69.87 -17.36, 70.05 -17.38, 70.24 -17.39, 70.43 -17.4, 70.62 -17.4, 70.81 -17.41, 70.91 -17.41, 71.02 -17.4, 71.12 -17.4, 71.22 -17.4, 71.32 -17.39, 71.42 -17.39, 71.52 -17.38, 71.62 -17.37, 71.72 -17.36, 71.82 -17.35, 71.92 -17.34, 72.02 -17.33, 72.12 -17.32, 72.22 -17.3, 72.32 -17.29, 72.42 -17.27, 72.52 -17.26, 72.62 -17.24, 72.71 -17.22, 72.81 -17.2, 72.91 -17.18, 73.01 -17.15, 73.11 -17.13, 73.2 -17.11, 73.3 -17.08, 73.4 -17.05, 73.49 -17.03, 73.59 -17, 73.69 -16.97, 73.78 -16.94, 73.88 -16.91, 73.97 -16.87, 74.07 -16.84, 74.16 -16.8, 74.26 -16.77, 74.36 -16.73, 74.45 -16.69, 74.55 -16.65, 74.64 -16.61, 74.74 -16.57, 74.83 -16.52, 74.93 -16.48, 75.02 -16.43, 75.12 -16.39, 75.21 -16.34, 75.31 -16.29, 75.41 -16.24, 75.5 -16.19, 75.6 -16.14, 75.69 -16.08, 75.79 -16.03, 75.88 -15.97, 75.98 -15.92, 76.07 -15.86, 76.17 -15.8, 76.26 -15.74, 76.36 -15.68, 76.45 -15.61, 76.55 -15.55, 76.64 -15.48, 76.64 -21.19, 76.53 -21.23, 76.42 -21.27, 76.31 -21.31, 76.2 -21.34, 76.09 -21.38, 75.98 -21.42, 75.87 -21.46, 75.76 -21.49, 75.65 -21.53, 75.53 -21.56, 75.42 -21.59, 75.31 -21.63, 75.2 -21.66, 75.09 -21.69, 74.98 -21.72, 74.87 -21.75, 74.76 -21.78, 74.65 -21.81, 74.54 -21.84, 74.43 -21.86, 74.31 -21.89, 74.2 -21.92, 74.09 -21.94, 73.98 -21.97, 73.87 -21.99, 73.76 -22.01, 73.65 -22.04, 73.54 -22.06, 73.42 -22.08, 73.31 -22.1, 73.2 -22.12, 73.09 -22.14, 72.98 -22.16, 72.87 -22.18, 72.75 -22.19, 72.64 -22.21, 72.53 -22.23, 72.42 -22.24, 72.31 -22.26, 72.19 -22.27, 72.08 -22.28, 71.97 -22.3, 71.85 -22.31, 71.74 -22.32, 71.63 -22.33, 71.52 -22.34, 71.4 -22.35, 71.29 -22.36, 71.18 -22.37, 71.06 -22.37, 70.95 -22.38, 70.84 -22.38, 70.72 -22.39, 70.61 -22.39, 70.49 -22.4, 70.38 -22.4, 70.27 -22.4, 70.15 -22.4, 70.04 -22.41, 69.92 -22.41, 69.53 -22.4, 69.15 -22.39, 68.77 -22.38, 68.39 -22.35, 68.02 -22.32, 67.66 -22.28, 67.31 -22.24, 66.95 -22.19, 66.61 -22.13, 66.27 -22.07, 65.94 -22, 65.61 -21.92, 65.28 -21.84, 64.97 -21.74, 64.66 -21.65, 64.35 -21.54, 64.05 -21.43, 63.76 -21.31, 63.47 -21.19, 63.19 -21.06, 62.91 -20.92, 62.64 -20.77, 62.37 -20.62, 62.11 -20.46, 61.86 -20.3, 61.61 -20.12, 61.37 -19.94, 61.13 -19.76, 60.9 -19.57, 60.67 -19.37, 60.45 -19.16, 60.24 -18.95, 60.04 -18.74, 59.84 -18.51, 59.65 -18.29, 59.47 -18.06, 59.29 -17.82, 59.13 -17.58, 58.97 -17.33, 58.81 -17.08, 58.67 -16.82, 58.53 -16.56, 58.4 -16.29, 58.28 -16.02, 58.16 -15.74, 58.06 -15.45, 57.96 -15.16, 57.86 -14.87, 57.78 -14.57, 57.7 -14.26, 57.63 -13.95, 57.57 -13.64, 57.51 -13.32, 57.46 -12.99, 57.42 -12.66, 57.39 -12.32, 57.36 -11.98, 57.34 -11.63, 57.33 -11.28, 57.33 -10.92)),Polygon ((1.72 -6.56, 1.72 -6.35, 1.73 -6.15, 1.74 -5.94, 1.76 -5.74, 1.78 -5.54, 1.8 -5.35, 1.83 -5.15, 1.87 -4.96, 1.91 -4.77, 1.95 -4.59, 2 -4.41, 2.06 -4.23, 2.12 -4.05, 2.18 -3.87, 2.25 -3.7, 2.32 -3.53, 2.4 -3.36, 2.48 -3.2, 2.57 -3.03, 2.66 -2.88, 2.76 -2.72, 2.86 -2.56, 2.96 -2.41, 3.07 -2.26, 3.19 -2.12, 3.31 -1.97, 3.43 -1.83, 3.56 -1.69, 3.7 -1.56, 3.84 -1.42, 3.98 -1.29, 4.12 -1.17, 4.27 -1.04, 4.42 -0.93, 4.58 -0.82, 4.73 -0.71, 4.89 -0.6, 5.06 -0.5, 5.22 -0.41, 5.39 -0.32, 5.56 -0.23, 5.74 -0.15, 5.91 -0.07, 6.09 0, 6.28 0.07, 6.46 0.13, 6.65 0.19, 6.84 0.25, 7.03 0.3, 7.23 0.34, 7.43 0.38, 7.63 0.42, 7.84 0.45, 8.05 0.48, 8.26 0.51, 8.47 0.53, 8.69 0.54, 8.91 0.55, 9.13 0.56, 9.36 0.56, 9.53 0.56, 9.69 0.56, 9.85 0.55, 10.02 0.55, 10.18 0.54, 10.34 0.53, 10.49 0.51, 10.65 0.5, 10.8 0.48, 10.95 0.46, 11.1 0.44, 11.25 0.42, 11.4 0.39, 11.54 0.37, 11.69 0.34, 11.83 0.3, 11.97 0.27, 12.11 0.24, 12.24 0.2, 12.38 0.16, 12.51 0.12, 12.64 0.08, 12.77 0.03, 12.9 -0.02, 13.03 -0.07, 13.15 -0.12, 13.27 -0.17, 13.4 -0.23, 13.51 -0.28, 13.63 -0.34, 13.75 -0.41, 13.87 -0.47, 13.98 -0.54, 14.1 -0.6, 14.21 -0.68, 14.32 -0.75, 14.43 -0.83, 14.55 -0.9, 14.66 -0.99, 14.77 -1.07, 14.87 -1.16, 14.98 -1.24, 15.09 -1.33, 15.2 -1.43, 15.3 -1.52, 15.41 -1.62, 15.51 -1.72, 15.62 -1.83, 15.72 -1.93, 15.82 -2.04, 15.92 -2.15, 16.02 -2.26, 16.12 -2.38, 16.22 -2.49, 16.32 -2.61, 16.42 -2.74, 16.51 -2.86, 16.61 -2.99, 16.7 -3.12, 16.8 -3.25, 16.8 0, 23.84 0, 23.84 -12.48, 23.84 -12.83, 23.83 -13.17, 23.82 -13.51, 23.8 -13.83, 23.77 -14.15, 23.74 -14.47, 23.7 -14.78, 23.66 -15.08, 23.61 -15.37, 23.55 -15.66, 23.49 -15.94, 23.42 -16.22, 23.35 -16.49, 23.27 -16.75, 23.19 -17.01, 23.1 -17.26, 23 -17.5, 22.9 -17.74, 22.79 -17.97, 22.68 -18.19, 22.56 -18.41, 22.43 -18.62, 22.3 -18.82, 22.16 -19.02, 22.02 -19.21, 21.87 -19.4, 21.72 -19.57, 21.56 -19.75, 21.39 -19.91, 21.22 -20.07, 21.04 -20.22, 20.85 -20.37, 20.66 -20.51, 20.46 -20.65, 20.25 -20.78, 20.04 -20.91, 19.81 -21.03, 19.58 -21.15, 19.35 -21.26, 19.1 -21.37, 18.85 -21.47, 18.59 -21.57, 18.32 -21.66, 18.05 -21.74, 17.77 -21.82, 17.48 -21.9, 17.19 -21.97, 16.88 -22.03, 16.57 -22.09, 16.25 -22.15, 15.93 -22.2, 15.6 -22.24, 15.26 -22.28, 14.91 -22.31, 14.55 -22.34, 14.19 -22.36, 13.82 -22.38, 13.45 -22.4, 13.06 -22.4, 12.67 -22.41, 12.52 -22.41, 12.37 -22.41, 12.22 -22.4, 12.07 -22.4, 11.92 -22.4, 11.77 -22.4, 11.61 -22.39, 11.46 -22.39, 11.31 -22.38, 11.16 -22.38, 11.01 -22.37, 10.86 -22.36, 10.71 -22.35, 10.56 -22.35, 10.41 -22.34, 10.26 -22.33, 10.1 -22.32, 9.95 -22.31, 9.8 -22.29, 9.65 -22.28, 9.5 -22.27, 9.35 -22.26, 9.2 -22.24, 9.05 -22.23, 8.9 -22.21, 8.74 -22.2, 8.59 -22.18, 8.44 -22.16, 8.29 -22.14, 8.14 -22.13, 7.99 -22.11, 7.84 -22.09, 7.69 -22.07, 7.54 -22.05, 7.39 -22.02, 7.24 -22, 7.09 -21.98, 6.93 -21.96, 6.78 -21.93, 6.63 -21.91, 6.48 -21.88, 6.33 -21.86, 6.18 -21.83, 6.03 -21.8, 5.88 -21.78, 5.73 -21.75, 5.58 -21.72, 5.43 -21.69, 5.28 -21.66, 5.13 -21.63, 4.98 -21.6, 4.83 -21.57, 4.69 -21.54, 4.54 -21.51, 4.39 -21.47, 4.24 -21.44, 4.09 -21.4, 3.94 -21.37, 3.79 -21.33, 3.64 -21.3, 3.64 -15.95, 3.75 -16.01, 3.86 -16.07, 3.97 -16.13, 4.09 -16.19, 4.2 -16.24, 4.31 -16.3, 4.43 -16.35, 4.54 -16.4, 4.66 -16.46, 4.78 -16.51, 4.89 -16.56, 5.01 -16.6, 5.13 -16.65, 5.25 -16.7, 5.37 -16.74, 5.49 -16.79, 5.61 -16.83, 5.73 -16.87, 5.85 -16.92, 5.97 -16.96, 6.09 -17, 6.22 -17.03, 6.34 -17.07, 6.47 -17.11, 6.59 -17.14, 6.72 -17.18, 6.84 -17.21, 6.97 -17.24, 7.1 -17.27, 7.23 -17.3, 7.36 -17.33, 7.49 -17.36, 7.62 -17.39, 7.75 -17.42, 7.88 -17.44, 8.01 -17.47, 8.14 -17.49, 8.28 -17.51, 8.41 -17.53, 8.55 -17.55, 8.68 -17.57, 8.82 -17.59, 8.96 -17.61, 9.1 -17.62, 9.24 -17.64, 9.38 -17.65, 9.52 -17.67, 9.66 -17.68, 9.8 -17.69, 9.94 -17.7, 10.09 -17.71, 10.23 -17.72, 10.37 -17.73, 10.52 -17.73, 10.67 -17.74, 10.81 -17.74, 10.96 -17.75, 11.11 -17.75, 11.26 -17.75, 11.41 -17.75, 11.59 -17.75, 11.77 -17.75, 11.95 -17.74, 12.12 -17.74, 12.29 -17.73, 12.46 -17.72, 12.62 -17.71, 12.78 -17.7, 12.94 -17.68, 13.1 -17.66, 13.25 -17.65, 13.4 -17.63, 13.54 -17.61, 13.68 -17.58, 13.82 -17.56, 13.95 -17.53, 14.08 -17.5, 14.21 -17.47, 14.34 -17.44, 14.46 -17.41, 14.58 -17.37, 14.69 -17.34, 14.8 -17.3, 14.91 -17.26, 15.02 -17.22, 15.12 -17.17, 15.22 -17.13, 15.31 -17.08, 15.4 -17.03, 15.49 -16.98, 15.58 -16.93, 15.66 -16.88, 15.74 -16.82, 15.82 -16.76, 15.89 -16.7, 15.96 -16.64, 16.03 -16.58, 16.1 -16.51, 16.16 -16.44, 16.22 -16.37, 16.27 -16.3, 16.33 -16.23, 16.38 -16.15, 16.43 -16.07, 16.47 -15.99, 16.51 -15.91, 16.55 -15.83, 16.59 -15.74, 16.62 -15.65, 16.65 -15.56, 16.68 -15.47, 16.7 -15.37, 16.73 -15.28, 16.74 -15.18, 16.76 -15.08, 16.77 -14.98, 16.78 -14.87, 16.79 -14.77, 16.8 -14.66, 16.8 -14.55, 16.8 -14, 12.67 -14, 12.3 -14, 11.93 -13.99, 11.57 -13.98, 11.22 -13.97, 10.87 -13.95, 10.53 -13.93, 10.2 -13.9, 9.87 -13.87, 9.55 -13.84, 9.24 -13.8, 8.93 -13.76, 8.63 -13.71, 8.33 -13.66, 8.05 -13.61, 7.77 -13.55, 7.49 -13.49, 7.23 -13.42, 6.97 -13.35, 6.71 -13.28, 6.47 -13.2, 6.23 -13.12, 5.99 -13.03, 5.77 -12.94, 5.55 -12.85, 5.33 -12.75, 5.13 -12.65, 4.93 -12.54, 4.73 -12.43, 4.55 -12.32, 4.37 -12.2, 4.19 -12.08, 4.03 -11.95, 3.86 -11.82, 3.71 -11.69, 3.56 -11.55, 3.41 -11.4, 3.28 -11.25, 3.14 -11.1, 3.02 -10.94, 2.9 -10.78, 2.78 -10.61, 2.67 -10.44, 2.57 -10.26, 2.47 -10.08, 2.38 -9.89, 2.3 -9.7, 2.22 -9.51, 2.14 -9.31, 2.07 -9.11, 2.01 -8.9, 1.96 -8.68, 1.91 -8.47, 1.86 -8.24, 1.82 -8.02, 1.79 -7.79, 1.77 -7.55, 1.75 -7.31, 1.73 -7.07, 1.72 -6.82, 1.72 -6.56),(8.77 -6.92, 8.77 -7.02, 8.77 -7.11, 8.78 -7.2, 8.79 -7.29, 8.8 -7.38, 8.81 -7.47, 8.83 -7.55, 8.84 -7.64, 8.87 -7.72, 8.89 -7.8, 8.91 -7.88, 8.94 -7.96, 8.97 -8.04, 9.01 -8.11, 9.04 -8.19, 9.08 -8.26, 9.12 -8.33, 9.17 -8.4, 9.21 -8.46, 9.26 -8.53, 9.31 -8.59, 9.36 -8.66, 9.42 -8.72, 9.48 -8.78, 9.54 -8.83, 9.6 -8.89, 9.66 -8.95, 9.73 -9, 9.8 -9.05, 9.88 -9.1, 9.95 -9.15, 10.03 -9.2, 10.11 -9.24, 10.19 -9.29, 10.27 -9.33, 10.36 -9.37, 10.45 -9.41, 10.54 -9.44, 10.64 -9.48, 10.73 -9.51, 10.83 -9.55, 10.94 -9.58, 11.04 -9.61, 11.15 -9.63, 11.25 -9.66, 11.37 -9.68, 11.48 -9.7, 11.59 -9.73, 11.71 -9.74, 11.83 -9.76, 11.96 -9.78, 12.08 -9.79, 12.21 -9.8, 12.34 -9.81, 12.47 -9.82, 12.61 -9.83, 12.75 -9.84, 12.88 -9.84, 13.03 -9.84, 13.17 -9.84, 16.8 -9.84, 16.8 -9.05, 16.8 -8.91, 16.79 -8.77, 16.78 -8.63, 16.77 -8.49, 16.76 -8.36, 16.74 -8.23, 16.72 -8.09, 16.7 -7.96, 16.68 -7.84, 16.65 -7.71, 16.62 -7.58, 16.58 -7.46, 16.54 -7.34, 16.5 -7.22, 16.46 -7.1, 16.41 -6.98, 16.37 -6.86, 16.31 -6.75, 16.26 -6.64, 16.2 -6.53, 16.14 -6.42, 16.07 -6.31, 16.01 -6.2, 15.94 -6.1, 15.86 -5.99, 15.79 -5.89, 15.71 -5.79, 15.63 -5.69, 15.54 -5.6, 15.45 -5.5, 15.36 -5.41, 15.27 -5.32, 15.18 -5.23, 15.08 -5.15, 14.99 -5.07, 14.89 -4.99, 14.79 -4.91, 14.69 -4.84, 14.59 -4.77, 14.49 -4.71, 14.38 -4.65, 14.28 -4.59, 14.17 -4.53, 14.06 -4.48, 13.95 -4.43, 13.84 -4.39, 13.72 -4.35, 13.61 -4.31, 13.49 -4.27, 13.38 -4.24, 13.26 -4.21, 13.14 -4.18, 13.02 -4.16, 12.89 -4.14, 12.77 -4.12, 12.64 -4.1, 12.52 -4.09, 12.39 -4.08, 12.26 -4.08, 12.13 -4.08, 12.02 -4.08, 11.92 -4.08, 11.82 -4.09, 11.72 -4.09, 11.62 -4.1, 11.52 -4.11, 11.43 -4.12, 11.33 -4.13, 11.24 -4.15, 11.15 -4.16, 11.06 -4.18, 10.97 -4.2, 10.89 -4.22, 10.8 -4.24, 10.72 -4.27, 10.64 -4.29, 10.56 -4.32, 10.48 -4.35, 10.4 -4.38, 10.33 -4.41, 10.25 -4.45, 10.18 -4.49, 10.11 -4.52, 10.04 -4.56, 9.97 -4.6, 9.91 -4.65, 9.84 -4.69, 9.78 -4.74, 9.72 -4.79, 9.66 -4.84, 9.6 -4.89, 9.54 -4.94, 9.49 -4.99, 9.43 -5.05, 9.38 -5.1, 9.34 -5.16, 9.29 -5.22, 9.24 -5.28, 9.2 -5.34, 9.16 -5.4, 9.12 -5.47, 9.09 -5.53, 9.05 -5.6, 9.02 -5.67, 8.99 -5.74, 8.96 -5.81, 8.93 -5.88, 8.91 -5.95, 8.89 -6.02, 8.86 -6.1, 8.85 -6.18, 8.83 -6.25, 8.81 -6.33, 8.8 -6.41, 8.79 -6.5, 8.78 -6.58, 8.77 -6.66, 8.77 -6.75, 8.77 -6.83, 8.77 -6.92)),Polygon ((30.34 -30.39, 30.34 0, 37.34 0, 37.34 -3.17, 37.44 -3.04, 37.54 -2.92, 37.63 -2.8, 37.73 -2.68, 37.83 -2.56, 37.93 -2.44, 38.03 -2.33, 38.13 -2.22, 38.24 -2.11, 38.34 -2, 38.44 -1.9, 38.55 -1.8, 38.65 -1.7, 38.76 -1.6, 38.86 -1.5, 38.97 -1.41, 39.08 -1.32, 39.18 -1.23, 39.29 -1.14, 39.4 -1.06, 39.51 -0.98, 39.62 -0.9, 39.74 -0.82, 39.85 -0.75, 39.96 -0.67, 40.08 -0.6, 40.19 -0.54, 40.31 -0.47, 40.42 -0.41, 40.54 -0.34, 40.66 -0.28, 40.78 -0.23, 40.9 -0.17, 41.02 -0.12, 41.14 -0.07, 41.26 -0.02, 41.39 0.03, 41.51 0.08, 41.64 0.12, 41.77 0.16, 41.9 0.2, 42.03 0.24, 42.16 0.27, 42.29 0.3, 42.43 0.34, 42.56 0.37, 42.7 0.39, 42.84 0.42, 42.98 0.44, 43.12 0.46, 43.26 0.48, 43.4 0.5, 43.54 0.51, 43.69 0.53, 43.83 0.54, 43.98 0.55, 44.13 0.55, 44.28 0.56, 44.43 0.56, 44.58 0.56, 44.85 0.56, 45.11 0.55, 45.37 0.53, 45.63 0.51, 45.89 0.47, 46.14 0.43, 46.39 0.39, 46.63 0.33, 46.88 0.27, 47.12 0.21, 47.35 0.13, 47.58 0.05, 47.81 -0.04, 48.04 -0.14, 48.26 -0.24, 48.48 -0.35, 48.7 -0.47, 48.91 -0.59, 49.12 -0.73, 49.33 -0.86, 49.54 -1.01, 49.74 -1.16, 49.93 -1.32, 50.13 -1.49, 50.32 -1.67, 50.51 -1.85, 50.69 -2.04, 50.87 -2.23, 51.05 -2.44, 51.23 -2.65, 51.4 -2.86, 51.56 -3.08, 51.72 -3.31, 51.87 -3.54, 52.02 -3.77, 52.16 -4.01, 52.3 -4.25, 52.43 -4.49, 52.55 -4.74, 52.67 -4.99, 52.78 -5.25, 52.89 -5.51, 52.99 -5.78, 53.09 -6.05, 53.18 -6.32, 53.26 -6.6, 53.34 -6.88, 53.41 -7.17, 53.48 -7.46, 53.54 -7.75, 53.59 -8.05, 53.64 -8.35, 53.69 -8.66, 53.72 -8.97, 53.76 -9.29, 53.78 -9.6, 53.8 -9.93, 53.82 -10.26, 53.83 -10.59, 53.83 -10.92, 53.83 -11.26, 53.82 -11.59, 53.8 -11.92, 53.78 -12.24, 53.76 -12.56, 53.72 -12.87, 53.69 -13.18, 53.64 -13.49, 53.59 -13.79, 53.54 -14.09, 53.48 -14.39, 53.41 -14.68, 53.34 -14.96, 53.26 -15.24, 53.18 -15.52, 53.09 -15.8, 52.99 -16.06, 52.89 -16.33, 52.78 -16.59, 52.67 -16.85, 52.55 -17.1, 52.43 -17.35, 52.3 -17.6, 52.16 -17.84, 52.02 -18.07, 51.87 -18.31, 51.72 -18.53, 51.56 -18.76, 51.4 -18.98, 51.23 -19.2, 51.05 -19.41, 50.87 -19.61, 50.69 -19.81, 50.51 -19.99, 50.32 -20.18, 50.13 -20.35, 49.93 -20.52, 49.74 -20.68, 49.54 -20.83, 49.33 -20.98, 49.12 -21.12, 48.91 -21.25, 48.7 -21.38, 48.48 -21.49, 48.26 -21.6, 48.04 -21.71, 47.81 -21.8, 47.58 -21.89, 47.35 -21.97, 47.12 -22.05, 46.88 -22.12, 46.63 -22.18, 46.39 -22.23, 46.14 -22.28, 45.89 -22.32, 45.63 -22.35, 45.37 -22.37, 45.11 -22.39, 44.85 -22.4, 44.58 -22.41, 44.43 -22.41, 44.28 -22.4, 44.13 -22.4, 43.98 -22.39, 43.83 -22.38, 43.69 -22.37, 43.54 -22.36, 43.4 -22.34, 43.26 -22.32, 43.12 -22.31, 42.98 -22.28, 42.84 -22.26, 42.7 -22.24, 42.56 -22.21, 42.43 -22.18, 42.29 -22.15, 42.16 -22.12, 42.03 -22.08, 41.9 -22.04, 41.77 -22, 41.64 -21.96, 41.51 -21.92, 41.39 -21.87, 41.26 -21.83, 41.14 -21.78, 41.02 -21.73, 40.9 -21.67, 40.78 -21.62, 40.66 -21.56, 40.54 -21.5, 40.42 -21.44, 40.31 -21.37, 40.19 -21.31, 40.08 -21.24, 39.96 -21.17, 39.85 -21.1, 39.74 -21.02, 39.62 -20.94, 39.51 -20.86, 39.4 -20.78, 39.29 -20.7, 39.18 -20.61, 39.08 -20.52, 38.97 -20.43, 38.86 -20.34, 38.76 -20.24, 38.65 -20.15, 38.55 -20.05, 38.44 -19.94, 38.34 -19.84, 38.24 -19.73, 38.13 -19.62, 38.03 -19.51, 37.93 -19.4, 37.83 -19.28, 37.73 -19.17, 37.63 -19.05, 37.54 -18.92, 37.44 -18.8, 37.34 -18.67, 37.34 -30.39, 30.34 -30.39),(37.34 -10.92, 37.35 -11.13, 37.35 -11.33, 37.36 -11.53, 37.37 -11.73, 37.38 -11.92, 37.39 -12.11, 37.41 -12.29, 37.43 -12.48, 37.45 -12.66, 37.48 -12.83, 37.5 -13.01, 37.54 -13.18, 37.57 -13.34, 37.6 -13.51, 37.64 -13.67, 37.68 -13.82, 37.73 -13.98, 37.77 -14.13, 37.82 -14.27, 37.88 -14.42, 37.93 -14.56, 37.99 -14.7, 38.05 -14.83, 38.11 -14.96, 38.17 -15.09, 38.24 -15.21, 38.31 -15.34, 38.38 -15.45, 38.46 -15.57, 38.54 -15.68, 38.62 -15.79, 38.7 -15.89, 38.79 -15.99, 38.88 -16.09, 38.97 -16.18, 39.06 -16.27, 39.15 -16.36, 39.25 -16.44, 39.35 -16.52, 39.45 -16.6, 39.56 -16.67, 39.66 -16.73, 39.77 -16.8, 39.88 -16.86, 40 -16.92, 40.11 -16.97, 40.23 -17.02, 40.35 -17.06, 40.48 -17.11, 40.6 -17.14, 40.73 -17.18, 40.86 -17.21, 40.99 -17.24, 41.13 -17.26, 41.26 -17.28, 41.4 -17.3, 41.54 -17.31, 41.69 -17.32, 41.84 -17.33, 41.98 -17.33, 42.13 -17.33, 42.28 -17.32, 42.42 -17.31, 42.57 -17.3, 42.7 -17.28, 42.84 -17.26, 42.98 -17.24, 43.11 -17.21, 43.24 -17.18, 43.37 -17.15, 43.49 -17.11, 43.61 -17.07, 43.73 -17.02, 43.85 -16.97, 43.97 -16.92, 44.08 -16.86, 44.19 -16.8, 44.3 -16.74, 44.41 -16.67, 44.51 -16.6, 44.61 -16.52, 44.71 -16.45, 44.81 -16.36, 44.9 -16.28, 44.99 -16.19, 45.08 -16.1, 45.17 -16, 45.25 -15.9, 45.33 -15.8, 45.41 -15.69, 45.49 -15.58, 45.57 -15.46, 45.64 -15.34, 45.71 -15.22, 45.77 -15.1, 45.84 -14.97, 45.9 -14.84, 45.96 -14.71, 46.02 -14.57, 46.07 -14.43, 46.12 -14.28, 46.17 -14.14, 46.21 -13.99, 46.26 -13.83, 46.3 -13.68, 46.34 -13.52, 46.37 -13.35, 46.41 -13.18, 46.44 -13.01, 46.46 -12.84, 46.49 -12.66, 46.51 -12.48, 46.53 -12.3, 46.55 -12.11, 46.56 -11.92, 46.57 -11.73, 46.58 -11.53, 46.59 -11.33, 46.59 -11.13, 46.59 -10.92, 46.59 -10.72, 46.59 -10.51, 46.58 -10.31, 46.57 -10.11, 46.56 -9.92, 46.55 -9.73, 46.53 -9.54, 46.51 -9.36, 46.49 -9.18, 46.46 -9, 46.44 -8.83, 46.41 -8.66, 46.37 -8.49, 46.34 -8.33, 46.3 -8.17, 46.26 -8.01, 46.21 -7.86, 46.17 -7.71, 46.12 -7.56, 46.07 -7.41, 46.02 -7.27, 45.96 -7.14, 45.9 -7, 45.84 -6.87, 45.77 -6.74, 45.71 -6.62, 45.64 -6.5, 45.57 -6.38, 45.49 -6.27, 45.41 -6.16, 45.33 -6.05, 45.25 -5.94, 45.17 -5.84, 45.08 -5.75, 44.99 -5.65, 44.9 -5.57, 44.81 -5.48, 44.71 -5.4, 44.61 -5.32, 44.51 -5.24, 44.41 -5.17, 44.3 -5.11, 44.19 -5.04, 44.08 -4.98, 43.97 -4.93, 43.85 -4.87, 43.73 -4.82, 43.61 -4.78, 43.49 -4.74, 43.37 -4.7, 43.24 -4.66, 43.11 -4.63, 42.98 -4.6, 42.84 -4.58, 42.7 -4.56, 42.57 -4.54, 42.42 -4.53, 42.28 -4.52, 42.13 -4.52, 41.98 -4.52, 41.84 -4.52, 41.69 -4.52, 41.54 -4.53, 41.4 -4.54, 41.26 -4.56, 41.13 -4.58, 40.99 -4.61, 40.86 -4.63, 40.73 -4.66, 40.6 -4.7, 40.48 -4.74, 40.35 -4.78, 40.23 -4.83, 40.11 -4.87, 40 -4.93, 39.88 -4.98, 39.77 -5.04, 39.66 -5.11, 39.56 -5.18, 39.45 -5.25, 39.35 -5.32, 39.25 -5.4, 39.15 -5.48, 39.06 -5.57, 38.97 -5.66, 38.88 -5.75, 38.79 -5.85, 38.7 -5.95, 38.62 -6.06, 38.54 -6.16, 38.46 -6.28, 38.39 -6.39, 38.31 -6.51, 38.24 -6.63, 38.17 -6.75, 38.11 -6.88, 38.05 -7.01, 37.99 -7.15, 37.93 -7.28, 37.88 -7.43, 37.82 -7.57, 37.77 -7.72, 37.73 -7.87, 37.68 -8.02, 37.64 -8.18, 37.6 -8.34, 37.57 -8.5, 37.54 -8.67, 37.5 -8.84, 37.48 -9.01, 37.45 -9.19, 37.43 -9.37, 37.41 -9.55, 37.39 -9.74, 37.38 -9.93, 37.37 -10.12, 37.36 -10.31, 37.35 -10.51, 37.35 -10.72, 37.34 -10.92)))",
        )

        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmWidth), 74)
        self.assertEqual(device.metric(QPaintDevice.PaintDeviceMetric.PdmHeight), 30)

        # with on the fly simplification
        device = QgsGeometryPaintDevice()
        device.setSimplificationTolerance(2)
        painter = SafePainter(device)
        font = getTestFont("bold")
        font.setPixelSize(40)
        painter.setFont(font)
        painter.drawText(0, 0, "abc")
        painter.end()
        result = device.geometry().clone()
        result.normalize()
        self.assertEqual(
            result.asWkt(2),
            "GeometryCollection (Polygon ((2.25 -3.7, 7.63 0.42, 16.8 -3.25, 16.8 0, 23.84 0, 23.84 -12.48, 19.35 -21.26, 3.64 -21.3, 3.64 -15.95, 13.54 -17.61, 16.8 -14, 4.19 -12.08, 2.25 -3.7),(9.12 -8.33, 16.8 -9.84, 16.14 -6.42, 10.04 -4.56, 9.12 -8.33)),Polygon ((57.39 -9.52, 61.13 -2.08, 76.64 -0.64, 76.64 -6.38, 68.2 -4.87, 64.61 -10.92, 68.2 -16.98, 76.64 -15.48, 76.64 -21.19, 66.61 -22.13, 59.65 -18.29, 57.39 -9.52)),Polygon ((30.34 -30.39, 30.34 0, 37.34 0, 37.34 -3.17, 42.56 0.37, 49.12 -0.73, 53.64 -8.35, 51.87 -18.31, 44.85 -22.4, 37.34 -18.67, 37.34 -30.39, 30.34 -30.39),(37.34 -10.92, 41.98 -17.33, 46.59 -10.92, 41.98 -4.52, 37.34 -10.92)))",
        )


if __name__ == "__main__":
    unittest.main()
