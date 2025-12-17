"""QGIS Unit tests for QgsNurbsCurve.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Loic Bartoletti"
__date__ = "10/09/2025"
__copyright__ = "Copyright 2025, The QGIS Project"


import qgis  # NOQA

from qgis.core import (
    QgsNurbsCurve,
    QgsCurve,
    QgsPoint,
    QgsLineString,
    QgsGeometry,
    QgsWkbTypes,
    QgsVertexId,
    Qgis,
)
from qgis.PyQt.QtCore import QByteArray
import binascii

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsNurbsCurve(unittest.TestCase):

    def testConstructor(self):
        """Test QgsNurbsCurve constructor"""
        # Test empty constructor
        curve = QgsNurbsCurve()
        self.assertEqual(curve.numPoints(), 0)
        self.assertEqual(curve.degree(), 0)
        self.assertEqual(len(curve.controlPoints()), 0)
        self.assertEqual(len(curve.knots()), 0)
        self.assertEqual(len(curve.weights()), 0)

        # Test constructor with parameters
        control_points = [
            QgsPoint(0, 0),
            QgsPoint(1, 1),
            QgsPoint(2, 0),
            QgsPoint(3, 1),
        ]
        degree = 2
        knots = [0, 0, 0, 1, 2, 2, 2]  # degree + 1 + control_points = 2 + 1 + 4 = 7
        weights = [1, 1, 1, 1]

        nurbs = QgsNurbsCurve(control_points, degree, knots, weights, False)
        self.assertEqual(nurbs.degree(), 2)
        self.assertEqual(len(nurbs.controlPoints()), 4)
        self.assertEqual(len(nurbs.knots()), 7)
        self.assertEqual(len(nurbs.weights()), 4)
        self.assertFalse(nurbs.isClosed())

    def testClone(self):
        """Test clone method"""
        # Test with a simple linear case first
        control_points = [QgsPoint(0, 0), QgsPoint(10, 10)]
        degree = 1
        knots = [0, 0, 1, 1]  # Simple linear knot vector
        weights = [1, 1]

        original = QgsNurbsCurve(control_points, degree, knots, weights)
        cloned = original.clone()

        self.assertIsInstance(cloned, QgsCurve)
        self.assertEqual(cloned.geometryType(), "NurbsCurve")

        if not original.isEmpty() and not cloned.isEmpty():
            self.assertAlmostEqual(original.length(), cloned.length(), places=5)

    def testGeometryType(self):
        """Test geometry type"""
        curve = QgsNurbsCurve()
        self.assertEqual(curve.geometryType(), "NurbsCurve")

    def testProperties(self):
        """Test NURBS curve properties"""
        # Test B-spline (all weights = 1)
        control_points = [QgsPoint(0, 0), QgsPoint(1, 1), QgsPoint(2, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        bspline = QgsNurbsCurve(control_points, degree, knots, weights)
        self.assertTrue(bspline.isBSpline())
        self.assertFalse(bspline.isRational())

        # Test rational curve (non-uniform weights)
        rational_weights = [1, 2, 1]
        rational = QgsNurbsCurve(control_points, degree, knots, rational_weights)
        self.assertFalse(rational.isBSpline())
        self.assertTrue(rational.isRational())

    def testBezierCurve(self):
        """Test Bézier curve detection"""
        # Cubic Bézier curve
        control_points = [
            QgsPoint(0, 0),
            QgsPoint(1, 2),
            QgsPoint(2, 2),
            QgsPoint(3, 0),
        ]
        degree = 3
        knots = [0, 0, 0, 0, 1, 1, 1, 1]  # Bézier knot vector
        weights = [1, 1, 1, 1]

        bezier = QgsNurbsCurve(control_points, degree, knots, weights)
        self.assertTrue(bezier.isBezier())

    def testEvaluation(self):
        """Test curve evaluation at parameter t"""
        # Simple linear case (degree 1)
        control_points = [QgsPoint(0, 0), QgsPoint(10, 10)]
        degree = 1
        knots = [0, 0, 1, 1]
        weights = [1, 1]

        linear = QgsNurbsCurve(control_points, degree, knots, weights)

        # Test evaluation at different parameters
        start_point = linear.evaluate(0.0)
        self.assertEqual(start_point.x(), 0.0)
        self.assertEqual(start_point.y(), 0.0)

        mid_point = linear.evaluate(0.5)
        self.assertEqual(mid_point.x(), 5.0)
        self.assertEqual(mid_point.y(), 5.0)

        end_point = linear.evaluate(1.0)
        self.assertEqual(end_point.x(), 10.0)
        self.assertEqual(end_point.y(), 10.0)

    def testStartEndPoints(self):
        """Test start and end points"""
        # Simple linear case
        control_points = [QgsPoint(0, 0), QgsPoint(10, 5)]
        degree = 1
        knots = [0, 0, 1, 1]
        weights = [1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)

        start = curve.startPoint()
        end = curve.endPoint()

        # For a linear NURBS curve, start/end should be at first/last control points
        self.assertAlmostEqual(start.x(), 0.0, places=5)
        self.assertAlmostEqual(start.y(), 0.0, places=5)
        self.assertAlmostEqual(end.x(), 10.0, places=5)
        self.assertAlmostEqual(end.y(), 5.0, places=5)

    def testNumPoints(self):
        """Test number of points"""
        control_points = [QgsPoint(0, 0), QgsPoint(1, 1), QgsPoint(2, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        # numPoints might return discretized points or control points count
        # depending on implementation
        self.assertGreater(curve.numPoints(), 0)

    def testToLineString(self):
        """Test conversion to line string"""
        control_points = [QgsPoint(0, 0), QgsPoint(1, 1), QgsPoint(2, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        line_string = curve.curveToLine()

        self.assertIsInstance(line_string, QgsLineString)
        self.assertGreater(line_string.numPoints(), 2)

        # Check that start and end points match
        self.assertEqual(line_string.startPoint().x(), curve.startPoint().x())
        self.assertEqual(line_string.startPoint().y(), curve.startPoint().y())
        self.assertEqual(line_string.endPoint().x(), curve.endPoint().x())
        self.assertEqual(line_string.endPoint().y(), curve.endPoint().y())

    def testCurveToLine(self):
        """Test curveToLine method"""
        control_points = [QgsPoint(0, 0), QgsPoint(5, 5), QgsPoint(10, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        line = curve.curveToLine()

        self.assertIsInstance(line, QgsLineString)
        self.assertGreater(line.numPoints(), 2)

    def testReversed(self):
        """Test reversed curve"""
        control_points = [QgsPoint(0, 0), QgsPoint(5, 5), QgsPoint(10, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        original = QgsNurbsCurve(control_points, degree, knots, weights)
        reversed_curve = original.reversed()

        # Start and end should be swapped
        self.assertEqual(original.startPoint().x(), reversed_curve.endPoint().x())
        self.assertEqual(original.startPoint().y(), reversed_curve.endPoint().y())
        self.assertEqual(original.endPoint().x(), reversed_curve.startPoint().x())
        self.assertEqual(original.endPoint().y(), reversed_curve.startPoint().y())

    def testBoundingBox(self):
        """Test bounding box calculation"""
        control_points = [QgsPoint(0, 0), QgsPoint(5, 10), QgsPoint(10, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        bbox = curve.boundingBox()

        self.assertLessEqual(bbox.xMinimum(), 0.0)
        self.assertGreaterEqual(bbox.xMaximum(), 10.0)
        self.assertLessEqual(bbox.yMinimum(), 0.0)
        self.assertGreaterEqual(bbox.yMaximum(), 10.0)

    def testEquals(self):
        """Test equality comparison"""
        control_points = [QgsPoint(0, 0), QgsPoint(1, 1), QgsPoint(2, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve1 = QgsNurbsCurve(control_points, degree, knots, weights)
        curve2 = QgsNurbsCurve(control_points, degree, knots, weights)
        curve3 = QgsNurbsCurve(
            [QgsPoint(0, 0), QgsPoint(2, 2)], 1, [0, 0, 1, 1], [1, 1]
        )

        self.assertTrue(curve1.equals(curve2))
        self.assertFalse(curve1.equals(curve3))

    def testIsEmpty(self):
        """Test isEmpty method"""
        empty_curve = QgsNurbsCurve()
        self.assertTrue(empty_curve.isEmpty())

        control_points = [QgsPoint(0, 0), QgsPoint(1, 1)]
        degree = 1
        knots = [0, 0, 1, 1]
        weights = [1, 1]
        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        self.assertFalse(curve.isEmpty())

    def testLength(self):
        """Test curve length calculation"""
        # Simple linear case
        control_points = [QgsPoint(0, 0), QgsPoint(3, 4)]  # 3-4-5 triangle
        degree = 1
        knots = [0, 0, 1, 1]
        weights = [1, 1]

        linear = QgsNurbsCurve(control_points, degree, knots, weights)
        length = linear.length()
        self.assertAlmostEqual(length, 5.0, places=2)

    def testHasCurvedSegments(self):
        """Test hasCurvedSegments method"""
        # Any NURBS curve with degree > 1 should have curved segments
        control_points = [QgsPoint(0, 0), QgsPoint(1, 1), QgsPoint(2, 0)]
        degree = 2
        knots = [0, 0, 0, 1, 1, 1]
        weights = [1, 1, 1]

        curve = QgsNurbsCurve(control_points, degree, knots, weights)
        self.assertTrue(curve.hasCurvedSegments())

    def testDimension(self):
        """Test dimension method"""
        curve = QgsNurbsCurve()
        self.assertEqual(curve.dimension(), 1)  # Curves are 1-dimensional


if __name__ == "__main__":
    unittest.main()
