# -*- coding: utf-8 -*-
"""QGIS Unit tests for core additions

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '05.06.2018'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from PyQt5.QtCore import QVariant
from qgis.testing import unittest, start_app
from qgis.core import QgsGeometry, QgsPoint, QgsPointXY, QgsCircle, QgsCircularString, QgsCompoundCurve,\
    QgsCurvePolygon, QgsEllipse, QgsLineString, QgsMultiCurve, QgsRectangle, QgsExpression, QgsField

start_app()


class TestPython__repr__(unittest.TestCase):

    def testQgsGeometryRepr(self):
        p = QgsPointXY(123.456, 987.654)
        g = QgsGeometry.fromPointXY(p)
        self.assertTrue(g.__repr__().startswith('<QgsGeometry: Point (123.456'))

    def testQgsPointRepr(self):
        p = QgsPoint(123.456, 987.654, 100)
        self.assertTrue(p.__repr__().startswith('<QgsPoint: PointZ (123.456'))

    def testQgsPointXYRepr(self):
        p = QgsPointXY(123.456, 987.654)
        self.assertTrue(p.__repr__().startswith('<QgsPointXY: POINT(123.456'))

    def testQgsCircleRepr(self):
        c = QgsCircle(QgsPoint(1, 1), 2.0)
        self.assertEqual(c.__repr__(), '<QgsCircle: Circle (Center: Point (1 1), Radius: 2, Azimuth: 0)>')

    def testQgsCircularstringRepr(self):
        cs = QgsCircularString(QgsPoint(1, 2), QgsPoint(2, 3), QgsPoint(3, 4))
        self.assertEqual(cs.__repr__(), '<QgsCircularString: CircularString (1 2, 2 3, 3 4)>')

    def testQgsCompoundcurveRepr(self):
        cs = QgsCircularString(QgsPoint(1, 2), QgsPoint(2, 3), QgsPoint(3, 4))
        cc = QgsCompoundCurve()
        cc.addCurve(cs)
        self.assertEqual(cc.__repr__(), '<QgsCompoundCurve: CompoundCurve (CircularString (1 2, 2 3, 3 4))>')

    def testQgsCurvepolygonRepr(self):
        cp = QgsCurvePolygon()
        cs = QgsCircularString(QgsPoint(1, 10), QgsPoint(2, 11), QgsPoint(1, 10))
        cp.setExteriorRing(cs)
        self.assertEqual(cp.__repr__(), '<QgsCurvePolygon: CurvePolygon (CircularString (1 10, 2 11, 1 10))>')

    def testQgsEllipseRepr(self):
        e = QgsEllipse(QgsPoint(1, 2), 2.0, 3.0)
        self.assertEqual(e.__repr__(), '<QgsEllipse: Ellipse (Center: Point (1 2), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 180)>')

    def testQgsLineStringRepr(self):
        ls = QgsLineString([QgsPoint(10, 2), QgsPoint(10, 1), QgsPoint(5, 1)])
        self.assertEqual(ls.__repr__(), '<QgsLineString: LineString (10 2, 10 1, 5 1)>')

    def testQgsMulticurveRepr(self):
        mc = QgsMultiCurve()
        cs = QgsCircularString(QgsPoint(1, 10), QgsPoint(2, 11), QgsPoint(3, 12))
        mc.addGeometry(cs)
        cs2 = QgsCircularString(QgsPoint(4, 20), QgsPoint(5, 22), QgsPoint(6, 24))
        mc.addGeometry(cs2)
        self.assertEqual(mc.__repr__(), '<QgsMultiCurve: MultiCurve (CircularString (1 10, 2 11, 3 12),CircularString (4 20, 5 22, 6 24))>')

    def testQgsMultilineStringRepr(self):
        ml = QgsGeometry.fromMultiPolylineXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
                [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
            ]
        )
        self.assertEqual(ml.constGet().__repr__(), '<QgsMultiLineString: MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0),(3 0, 3 1, 5 1, 5 0, 6 0))>')

    def testQgsMultiPointRepr(self):
        wkt = "MultiPoint ((10 30),(40 20),(30 10),(20 10))"
        mp = QgsGeometry.fromWkt(wkt)
        self.assertEqual(mp.constGet().__repr__(), '<QgsMultiPoint: MultiPoint ((10 30),(40 20),(30 10),(20 10))>')

    def testQgsMultipolygonRepr(self):
        mp = QgsGeometry.fromMultiPolygonXY([
            [[QgsPointXY(1, 1),
              QgsPointXY(2, 2),
              QgsPointXY(1, 2),
              QgsPointXY(1, 1)]],
            [[QgsPointXY(2, 2),
              QgsPointXY(3, 3),
              QgsPointXY(3, 1),
              QgsPointXY(2, 2)]]
        ])
        self.assertEqual(mp.constGet().__repr__(), '<QgsMultiPolygon: MultiPolygon (((1 1, 2 2, 1 2, 1 1)),((2 2, 3 3, 3 1, 2 2)))>')

    def testQgsPolygonRepr(self):
        p = QgsGeometry.fromPolygonXY(
            [[QgsPointXY(0, 0),
              QgsPointXY(2, 0),
              QgsPointXY(2, 2),
              QgsPointXY(0, 2),
              QgsPointXY(0, 0)]])
        self.assertEqual(p.constGet().__repr__(), '<QgsPolygon: Polygon ((0 0, 2 0, 2 2, 0 2, 0 0))>')

    def testQgsRectangleRepr(self):
        r = QgsRectangle(1, 2, 3, 4)
        self.assertEqual(r.__repr__(), '<QgsRectangle: 1 2, 3 4>')

    def testQgsExpressionRepr(self):
        e = QgsExpression('my expression')
        self.assertEqual(e.__repr__(), "<QgsExpression: 'my expression'>")

    def testQgsFieldRepr(self):
        f = QgsField('field_name', QVariant.Double, 'double')
        self.assertEqual(f.__repr__(), "<QgsField: field_name (double)>")


if __name__ == "__main__":
    unittest.main()
