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
import os

from qgis.testing import unittest, start_app
from qgis.core import QgsGeometry, QgsPoint, QgsPointXY, QgsCircle, QgsCircularString, QgsCompoundCurve, QgsCurve,\
    QgsCurvePolygon, QgsEllipse, QgsLineString, QgsMultiCurve, QgsMultiLineString, QgsMultiPoint, QgsMultiPolygon,\
    QgsPolygon, QgsRectangle

import sip

start_app()


class TestCoreAdditions(unittest.TestCase):

    def TestQgsPointRepr(self):
        p = QgsPoint(123.456, 987.654, 100)
        print(p)

    def TestQgsPointXYRepr(self):
        p = QgsPointXY(123.456, 987.654)
        print(p)

    def TestQgsCircleRepr(self):
        c = QgsCircle(QgsPoint(1, 1), 2.0)
        print(c)

    def TestQgsCircularstringRepr(self):
        cs = QgsCircularString(QgsPoint(1, 2), QgsPoint(2, 3), QgsPoint(3, 4))
        print(cs)

    def TestQgsCompoundcurveRepr(self):
        cs = QgsCircularString(QgsPoint(1, 2), QgsPoint(2, 3), QgsPoint(3, 4))
        cc = QgsCompoundCurve()
        cc.addCurve(cs)
        print(cc)

    def TestQgsCurvepolygonRepr(self):
        cp = QgsCurvePolygon()
        cs = QgsCircularString(QgsPoint(1, 10), QgsPoint(2, 11), QgsPoint(1, 10))
        cp.setExteriorRing(cs)
        print(cp)

    def TestQgsEllipseRepr(self):
        e = QgsEllipse(QgsPoint(1, 2), 2.0, 3.0)
        print(e)

    def TestQgsLineStringRepr(self):
        ls = QgsLineString([QgsPoint(10, 2), QgsPoint(10, 1), QgsPoint(5, 1)])
        print(ls)

    def TestQgsMulticurveRepr(self):
        mc = QgsMultiCurve()
        cs = QgsCircularString(QgsPoint(1, 10), QgsPoint(2, 11), QgsPoint(3, 12))
        mc.addGeometry(cs)
        cs2 = QgsCircularString(QgsPoint(4, 20), QgsPoint(5, 22), QgsPoint(6, 24))
        mc.addGeometry(cs2)
        print(mc)

    def TestQgsMultilineStringRepr(self):
        ml = QgsGeometry.fromMultiPolylineXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
                [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
            ]
        )
        print(ml)

    def TestQgsMultiPointRepr(self):
        wkt = "MultiPoint ((10 30),(40 20),(30 10),(20 10))"
        mp = QgsGeometry.fromWkt(wkt)
        print(mp)

    def TestQgsMultipolygonRepr(self):
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
        print(mp)

    def TestQgsPolygonRepr(self):
        p = QgsGeometry.fromPolygonXY(
            [[QgsPointXY(0, 0),
              QgsPointXY(2, 0),
              QgsPointXY(2, 2),
              QgsPointXY(0, 2),
              QgsPointXY(0, 0)]])
        print(p)

    def TestQgsRectangleRepr(self):
        r = QgsRectangle(1, 2, 3, 4)
        print(r)


if __name__ == "__main__":
    unittest.main()
