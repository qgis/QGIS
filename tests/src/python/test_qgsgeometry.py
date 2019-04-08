# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGeometry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import csv
import math

from qgis.core import (
    QgsGeometry,
    QgsVectorLayer,
    QgsFeature,
    QgsPointXY,
    QgsPoint,
    QgsCircularString,
    QgsCompoundCurve,
    QgsCurvePolygon,
    QgsGeometryCollection,
    QgsLineString,
    QgsMultiCurve,
    QgsMultiLineString,
    QgsMultiPoint,
    QgsMultiPolygon,
    QgsMultiSurface,
    QgsPolygon,
    QgsCoordinateTransform,
    QgsRectangle,
    QgsWkbTypes,
    QgsRenderChecker,
    QgsCoordinateReferenceSystem,
    QgsProject
)
from qgis.PyQt.QtCore import QDir
from qgis.PyQt.QtGui import QImage, QPainter, QPen, QColor, QBrush, QPainterPath

from qgis.testing import (
    start_app,
    unittest,
)

from utilities import (
    compareWkt,
    unitTestDataPath,
    writeShape
)

# Convenience instances in case you may need them not used in this test

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometry(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsGeometry Tests</h1>\n"

    def testBool(self):
        """ Test boolean evaluation of QgsGeometry """
        g = QgsGeometry()
        self.assertFalse(g)
        myWKT = 'Point (10 10)'
        g = QgsGeometry.fromWkt(myWKT)
        self.assertTrue(g)
        g.set(None)
        self.assertFalse(g)

    def testIsEmpty(self):
        """
        the bulk of these tests are in testqgsgeometry.cpp for each QgsAbstractGeometry subclass
        this test just checks the QgsGeometry wrapper
        """
        g = QgsGeometry()
        self.assertTrue(g.isEmpty())
        g = QgsGeometry.fromWkt('Point(10 10 )')
        self.assertFalse(g.isEmpty())
        g = QgsGeometry.fromWkt('MultiPoint ()')
        self.assertTrue(g.isEmpty())

    def testVertexIterator(self):
        g = QgsGeometry.fromWkt('Linestring(11 12, 13 14)')
        it = g.vertices()
        self.assertEqual(next(it), QgsPoint(11, 12))
        self.assertEqual(next(it), QgsPoint(13, 14))
        with self.assertRaises(StopIteration):
            next(it)

    def testPartIterator(self):
        g = QgsGeometry()
        it = g.parts()
        with self.assertRaises(StopIteration):
            next(it)
        with self.assertRaises(StopIteration):
            next(it)

        # single point geometry
        g = QgsGeometry.fromWkt('Point (10 10)')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'Point (10 10)')
        with self.assertRaises(StopIteration):
            next(it)

        it = g.get().parts()
        self.assertEqual(next(it).asWkt(), 'Point (10 10)')
        with self.assertRaises(StopIteration):
            next(it)

        # multi point geometry
        g = QgsGeometry.fromWkt('MultiPoint (10 10, 20 20, 10 20)')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'Point (10 10)')
        self.assertEqual(next(it).asWkt(), 'Point (20 20)')
        self.assertEqual(next(it).asWkt(), 'Point (10 20)')
        with self.assertRaises(StopIteration):
            next(it)

        it = g.get().parts()
        self.assertEqual(next(it).asWkt(), 'Point (10 10)')
        self.assertEqual(next(it).asWkt(), 'Point (20 20)')
        self.assertEqual(next(it).asWkt(), 'Point (10 20)')
        with self.assertRaises(StopIteration):
            next(it)

        # empty multi point geometry
        g = QgsGeometry.fromWkt('MultiPoint ()')
        it = g.parts()
        with self.assertRaises(StopIteration):
            next(it)

        # single line geometry
        g = QgsGeometry.fromWkt('LineString (10 10, 20 10, 30 10)')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'LineString (10 10, 20 10, 30 10)')
        with self.assertRaises(StopIteration):
            next(it)

        # multi line geometry
        g = QgsGeometry.fromWkt('MultiLineString ((10 10, 20 20, 10 20),(5 7, 8 9))')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'LineString (10 10, 20 20, 10 20)')
        self.assertEqual(next(it).asWkt(), 'LineString (5 7, 8 9)')
        with self.assertRaises(StopIteration):
            next(it)

        # empty multi line geometry
        g = QgsGeometry.fromWkt('MultiLineString ()')
        it = g.parts()
        with self.assertRaises(StopIteration):
            next(it)

        # single polygon geometry
        g = QgsGeometry.fromWkt('Polygon ((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50))')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'Polygon ((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50))')
        with self.assertRaises(StopIteration):
            next(it)

        # multi polygon geometry
        g = QgsGeometry.fromWkt('MultiPolygon (((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50)),((20 2, 20 4, 22 4, 22 2, 20 2)))')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'Polygon ((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50))')
        self.assertEqual(next(it).asWkt(), 'Polygon ((20 2, 20 4, 22 4, 22 2, 20 2))')
        with self.assertRaises(StopIteration):
            next(it)

        # empty multi polygon geometry
        g = QgsGeometry.fromWkt('MultiPolygon ()')
        it = g.parts()
        with self.assertRaises(StopIteration):
            next(it)

        # geometry collection
        g = QgsGeometry.fromWkt('GeometryCollection( Point( 1 2), LineString( 4 5, 8 7 ))')
        it = g.parts()
        self.assertEqual(next(it).asWkt(), 'Point (1 2)')
        self.assertEqual(next(it).asWkt(), 'LineString (4 5, 8 7)')
        with self.assertRaises(StopIteration):
            next(it)

        # empty geometry collection
        g = QgsGeometry.fromWkt('GeometryCollection()')
        it = g.parts()
        with self.assertRaises(StopIteration):
            next(it)

    def testWktPointLoading(self):
        myWKT = 'Point (10 10)'
        myGeometry = QgsGeometry.fromWkt(myWKT)
        self.assertEqual(myGeometry.wkbType(), QgsWkbTypes.Point)

    def testWktMultiPointLoading(self):
        # Standard format
        wkt = 'MultiPoint ((10 15),(20 30))'
        geom = QgsGeometry.fromWkt(wkt)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
        self.assertEqual(geom.constGet().numGeometries(), 2)
        self.assertEqual(geom.constGet().geometryN(0).x(), 10)
        self.assertEqual(geom.constGet().geometryN(0).y(), 15)
        self.assertEqual(geom.constGet().geometryN(1).x(), 20)
        self.assertEqual(geom.constGet().geometryN(1).y(), 30)

        # Check MS SQL format
        wkt = 'MultiPoint (11 16, 21 31)'
        geom = QgsGeometry.fromWkt(wkt)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
        self.assertEqual(geom.constGet().numGeometries(), 2)
        self.assertEqual(geom.constGet().geometryN(0).x(), 11)
        self.assertEqual(geom.constGet().geometryN(0).y(), 16)
        self.assertEqual(geom.constGet().geometryN(1).x(), 21)
        self.assertEqual(geom.constGet().geometryN(1).y(), 31)

    def testFromPoint(self):
        myPoint = QgsGeometry.fromPointXY(QgsPointXY(10, 10))
        self.assertEqual(myPoint.wkbType(), QgsWkbTypes.Point)

    def testFromMultiPoint(self):
        myMultiPoint = QgsGeometry.fromMultiPointXY([
            (QgsPointXY(0, 0)), (QgsPointXY(1, 1))])
        self.assertEqual(myMultiPoint.wkbType(), QgsWkbTypes.MultiPoint)

    def testFromLine(self):
        myLine = QgsGeometry.fromPolylineXY([QgsPointXY(1, 1), QgsPointXY(2, 2)])
        self.assertEqual(myLine.wkbType(), QgsWkbTypes.LineString)

    def testFromMultiLine(self):
        myMultiPolyline = QgsGeometry.fromMultiPolylineXY(
            [[QgsPointXY(0, 0), QgsPointXY(1, 1)], [QgsPointXY(0, 1), QgsPointXY(2, 1)]])
        self.assertEqual(myMultiPolyline.wkbType(), QgsWkbTypes.MultiLineString)

    def testFromPolygon(self):
        myPolygon = QgsGeometry.fromPolygonXY(
            [[QgsPointXY(1, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)]])
        self.assertEqual(myPolygon.wkbType(), QgsWkbTypes.Polygon)

    def testFromMultiPolygon(self):
        myMultiPolygon = QgsGeometry.fromMultiPolygonXY([
            [[QgsPointXY(1, 1),
                QgsPointXY(2, 2),
                QgsPointXY(1, 2),
                QgsPointXY(1, 1)]],
            [[QgsPointXY(2, 2),
                QgsPointXY(3, 3),
                QgsPointXY(3, 1),
                QgsPointXY(2, 2)]]
        ])
        self.assertEqual(myMultiPolygon.wkbType(), QgsWkbTypes.MultiPolygon)

    def testLineStringPythonAdditions(self):
        """
        Tests Python specific additions to the QgsLineString API
        """
        ls = QgsLineString()
        self.assertTrue(bool(ls))
        self.assertEqual(len(ls), 0)
        ls = QgsLineString([QgsPoint(1, 2), QgsPoint(11, 12)])
        self.assertTrue(bool(ls))
        self.assertEqual(len(ls), 2)

        # pointN
        with self.assertRaises(IndexError):
            ls.pointN(-3)
        with self.assertRaises(IndexError):
            ls.pointN(2)
        self.assertEqual(ls.pointN(0), QgsPoint(1, 2))
        self.assertEqual(ls.pointN(1), QgsPoint(11, 12))
        self.assertEqual(ls.pointN(-2), QgsPoint(1, 2))
        self.assertEqual(ls.pointN(-1), QgsPoint(11, 12))

        # xAt
        with self.assertRaises(IndexError):
            ls.xAt(-3)
        with self.assertRaises(IndexError):
            ls.xAt(2)
        self.assertEqual(ls.xAt(0), 1)
        self.assertEqual(ls.xAt(1), 11)
        self.assertEqual(ls.xAt(-2), 1)
        self.assertEqual(ls.xAt(-1), 11)

        # yAt
        with self.assertRaises(IndexError):
            ls.yAt(-3)
        with self.assertRaises(IndexError):
            ls.yAt(2)
        self.assertEqual(ls.yAt(0), 2)
        self.assertEqual(ls.yAt(1), 12)
        self.assertEqual(ls.yAt(-2), 2)
        self.assertEqual(ls.yAt(-1), 12)

        # zAt
        with self.assertRaises(IndexError):
            ls.zAt(-3)
        with self.assertRaises(IndexError):
            ls.zAt(2)

        # mAt
        with self.assertRaises(IndexError):
            ls.mAt(-3)
        with self.assertRaises(IndexError):
            ls.mAt(2)

        ls = QgsLineString([QgsPoint(1, 2, 3, 4), QgsPoint(11, 12, 13, 14)])
        self.assertEqual(ls.zAt(0), 3)
        self.assertEqual(ls.zAt(1), 13)
        self.assertEqual(ls.zAt(-2), 3)
        self.assertEqual(ls.zAt(-1), 13)
        self.assertEqual(ls.mAt(0), 4)
        self.assertEqual(ls.mAt(1), 14)
        self.assertEqual(ls.mAt(-2), 4)
        self.assertEqual(ls.mAt(-1), 14)

        # setXAt
        with self.assertRaises(IndexError):
            ls.setXAt(-3, 55)
        with self.assertRaises(IndexError):
            ls.setXAt(2, 55)
        ls.setXAt(0, 5)
        ls.setXAt(1, 15)
        self.assertEqual(ls.xAt(0), 5)
        self.assertEqual(ls.xAt(1), 15)
        ls.setXAt(-2, 25)
        ls.setXAt(-1, 26)
        self.assertEqual(ls.xAt(0), 25)
        self.assertEqual(ls.xAt(1), 26)

        # setYAt
        with self.assertRaises(IndexError):
            ls.setYAt(-3, 66)
        with self.assertRaises(IndexError):
            ls.setYAt(2, 66)
        ls.setYAt(0, 6)
        ls.setYAt(1, 16)
        self.assertEqual(ls.yAt(0), 6)
        self.assertEqual(ls.yAt(1), 16)
        ls.setYAt(-2, 16)
        ls.setYAt(-1, 22)
        self.assertEqual(ls.yAt(0), 16)
        self.assertEqual(ls.yAt(1), 22)

        # setZAt
        with self.assertRaises(IndexError):
            ls.setZAt(-3, 77)
        with self.assertRaises(IndexError):
            ls.setZAt(2, 77)
        ls.setZAt(0, 7)
        ls.setZAt(1, 17)
        self.assertEqual(ls.zAt(0), 7)
        self.assertEqual(ls.zAt(1), 17)
        ls.setZAt(-2, 37)
        ls.setZAt(-1, 47)
        self.assertEqual(ls.zAt(0), 37)
        self.assertEqual(ls.zAt(1), 47)

        # setMAt
        with self.assertRaises(IndexError):
            ls.setMAt(-3, 88)
        with self.assertRaises(IndexError):
            ls.setMAt(2, 88)
        ls.setMAt(0, 8)
        ls.setMAt(1, 18)
        self.assertEqual(ls.mAt(0), 8)
        self.assertEqual(ls.mAt(1), 18)
        ls.setMAt(-2, 58)
        ls.setMAt(-1, 68)
        self.assertEqual(ls.mAt(0), 58)
        self.assertEqual(ls.mAt(1), 68)

        # get item
        with self.assertRaises(IndexError):
            ls[-3]
        with self.assertRaises(IndexError):
            ls[2]
        self.assertEqual(ls[0], QgsPoint(25, 16, 37, 58))
        self.assertEqual(ls[1], QgsPoint(26, 22, 47, 68))
        self.assertEqual(ls[-2], QgsPoint(25, 16, 37, 58))
        self.assertEqual(ls[-1], QgsPoint(26, 22, 47, 68))

        # set item
        with self.assertRaises(IndexError):
            ls[-3] = QgsPoint(33, 34)
        with self.assertRaises(IndexError):
            ls[2] = QgsPoint(33, 34)
        ls[0] = QgsPoint(33, 34, 35, 36)
        ls[1] = QgsPoint(43, 44, 45, 46)
        self.assertEqual(ls[0], QgsPoint(33, 34, 35, 36))
        self.assertEqual(ls[1], QgsPoint(43, 44, 45, 46))
        ls[-2] = QgsPoint(133, 134, 135, 136)
        ls[-1] = QgsPoint(143, 144, 145, 146)
        self.assertEqual(ls[0], QgsPoint(133, 134, 135, 136))
        self.assertEqual(ls[1], QgsPoint(143, 144, 145, 146))

        # set item, z/m handling
        ls[0] = QgsPoint(66, 67)
        self.assertEqual(ls[0], QgsPoint(66, 67, None, None, QgsWkbTypes.PointZM))
        ls[0] = QgsPoint(77, 78, 79)
        self.assertEqual(ls[0], QgsPoint(77, 78, 79, None, QgsWkbTypes.PointZM))
        ls[0] = QgsPoint(77, 78, None, 80, QgsWkbTypes.PointZM)
        self.assertEqual(ls[0], QgsPoint(77, 78, None, 80, QgsWkbTypes.PointZM))

        ls = QgsLineString([QgsPoint(1, 2), QgsPoint(11, 12)])
        ls[0] = QgsPoint(66, 67)
        self.assertEqual(ls[0], QgsPoint(66, 67))
        ls[0] = QgsPoint(86, 87, 89, 90)
        self.assertEqual(ls[0], QgsPoint(86, 87))

        # del item
        ls = QgsLineString([QgsPoint(1, 2), QgsPoint(11, 12), QgsPoint(33, 34)])
        with self.assertRaises(IndexError):
            del ls[-4]
        with self.assertRaises(IndexError):
            del ls[3]
        del ls[1]
        self.assertEqual(len(ls), 2)
        self.assertEqual(ls[0], QgsPoint(1, 2))
        self.assertEqual(ls[1], QgsPoint(33, 34))
        with self.assertRaises(IndexError):
            del ls[2]

        ls = QgsLineString([QgsPoint(1, 2), QgsPoint(11, 12), QgsPoint(33, 34)])
        del ls[-3]
        self.assertEqual(len(ls), 2)
        self.assertEqual(ls[0], QgsPoint(11, 12))
        self.assertEqual(ls[1], QgsPoint(33, 34))
        with self.assertRaises(IndexError):
            del ls[-3]

    def testGeometryCollectionPythonAdditions(self):
        """
        Tests Python specific additions to the QgsGeometryCollection API
        """
        g = QgsGeometryCollection()
        self.assertTrue(bool(g))
        self.assertEqual(len(g), 0)
        g = QgsGeometryCollection()
        g.fromWkt('GeometryCollection( Point(1  2), Point(11 12))')
        self.assertTrue(bool(g))
        self.assertEqual(len(g), 2)

        # pointN
        with self.assertRaises(IndexError):
            g.geometryN(-1)
        with self.assertRaises(IndexError):
            g.geometryN(2)
        self.assertEqual(g.geometryN(0), QgsPoint(1, 2))
        self.assertEqual(g.geometryN(1), QgsPoint(11, 12))

        # removeGeometry
        g.fromWkt('GeometryCollection( Point(1  2), Point(11 12), Point(33 34))')
        with self.assertRaises(IndexError):
            g.removeGeometry(-1)
        with self.assertRaises(IndexError):
            g.removeGeometry(3)
        g.removeGeometry(1)
        self.assertEqual(len(g), 2)
        self.assertEqual(g.geometryN(0), QgsPoint(1, 2))
        self.assertEqual(g.geometryN(1), QgsPoint(33, 34))
        with self.assertRaises(IndexError):
            g.removeGeometry(2)

        g.fromWkt('GeometryCollection( Point(25 16 37 58), Point(26 22 47 68))')
        # get item
        with self.assertRaises(IndexError):
            g[-3]
        with self.assertRaises(IndexError):
            g[2]
        self.assertEqual(g[0], QgsPoint(25, 16, 37, 58))
        self.assertEqual(g[1], QgsPoint(26, 22, 47, 68))
        self.assertEqual(g[-2], QgsPoint(25, 16, 37, 58))
        self.assertEqual(g[-1], QgsPoint(26, 22, 47, 68))

        # del item
        g.fromWkt('GeometryCollection( Point(1  2), Point(11 12), Point(33 34))')
        with self.assertRaises(IndexError):
            del g[-4]
        with self.assertRaises(IndexError):
            del g[3]
        del g[1]
        self.assertEqual(len(g), 2)
        self.assertEqual(g[0], QgsPoint(1, 2))
        self.assertEqual(g[1], QgsPoint(33, 34))
        with self.assertRaises(IndexError):
            del g[2]

        g.fromWkt('GeometryCollection( Point(1  2), Point(11 12), Point(33 34))')
        del g[-3]
        self.assertEqual(len(g), 2)
        self.assertEqual(g[0], QgsPoint(11, 12))
        self.assertEqual(g[1], QgsPoint(33, 34))
        with self.assertRaises(IndexError):
            del g[-3]

        # iteration
        g = QgsGeometryCollection()
        self.assertFalse([p for p in g])
        g.fromWkt('GeometryCollection( Point(1 2), Point(11 12), LineString(33 34, 44 45))')
        self.assertEqual([p.asWkt() for p in g], ['Point (1 2)', 'Point (11 12)', 'LineString (33 34, 44 45)'])

    def testCurvePolygonPythonAdditions(self):
        """
        Tests Python specific additions to the QgsCurvePolygon API
        """
        # interiorRing
        g = QgsPolygon()
        with self.assertRaises(IndexError):
            g.interiorRing(-1)
        with self.assertRaises(IndexError):
            g.interiorRing(0)

        g.fromWkt('Polygon((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1),(0.8 0.8, 0.9 0.8, 0.9 0.9, 0.8 0.8))')
        with self.assertRaises(IndexError):
            g.interiorRing(-1)
        with self.assertRaises(IndexError):
            g.interiorRing(2)
        self.assertEqual(g.interiorRing(0).asWkt(1), 'LineString (0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1)')
        self.assertEqual(g.interiorRing(1).asWkt(1), 'LineString (0.8 0.8, 0.9 0.8, 0.9 0.9, 0.8 0.8)')

        # removeInteriorRing
        g = QgsPolygon()
        with self.assertRaises(IndexError):
            g.removeInteriorRing(-1)
        with self.assertRaises(IndexError):
            g.removeInteriorRing(0)

        g.fromWkt(
            'Polygon((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1),(0.8 0.8, 0.9 0.8, 0.9 0.9, 0.8 0.8))')
        with self.assertRaises(IndexError):
            g.removeInteriorRing(-1)
        with self.assertRaises(IndexError):
            g.removeInteriorRing(2)

        g.removeInteriorRing(1)
        self.assertEqual(g.asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1))')
        with self.assertRaises(IndexError):
            g.removeInteriorRing(1)
        g.removeInteriorRing(0)
        self.assertEqual(g.asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 0))')
        with self.assertRaises(IndexError):
            g.removeInteriorRing(0)

    def testPointXY(self):
        """
        Test the QgsPointXY conversion methods
        """
        self.assertEqual(QgsGeometry.fromWkt('Point(11 13)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('PointZ(11 13 14)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('PointM(11 13 14)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('PointZM(11 13 14 15)').asPoint(), QgsPointXY(11, 13))
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13,14 15, 14 13, 11 13))').asPoint()
        with self.assertRaises(ValueError):
            QgsGeometry().asPoint()

        # as polyline
        self.assertEqual(QgsGeometry.fromWkt('LineString(11 13,14 15)').asPolyline(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringZ(11 13 1,14 15 2)').asPolyline(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringM(11 13 1,14 15 2)').asPolyline(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringZM(11 13 1 2,14 15 3 4)').asPolyline(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Point(11 13)').asPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiLineString((11 13, 14 15),(1 2, 3 4))').asPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13,14 15, 14 13, 11 13))').asPolyline()
        with self.assertRaises(ValueError):
            QgsGeometry().asPolyline()

        # as polygon
        self.assertEqual(QgsGeometry.fromWkt('Polygon((11 13,14 15, 11 15, 11 13))').asPolygon(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('PolygonZ((11 13 1,14 15 2, 11 15 3, 11 13 1))').asPolygon(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('PolygonM((11 13 1,14 15 2, 11 15 3, 11 13 1))').asPolygon(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('PolygonZM((11 13 1 11,14 15 2 12 , 11 15 3 13 , 11 13 1 11))').asPolygon(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Point(11 13)').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiLineString((11 13, 14 15),(1 2, 3 4))').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPolygon(((11 13,14 15, 11 15, 11 13)))').asPolygon()
        with self.assertRaises(ValueError):
            QgsGeometry().asPolygon()

        # as multipoint
        self.assertEqual(QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asMultiPoint(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZ(11 13 1,14 15 2)').asMultiPoint(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointM(11 13 1,14 15 2)').asMultiPoint(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZM(11 13 1 2,14 15 3 4)').asMultiPoint(), [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Point(11 13)').asMultiPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asMultiPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiLineString((11 13, 14 15),(1 2, 3 4))').asMultiPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13,14 15, 14 13, 11 13))').asMultiPoint()
        with self.assertRaises(ValueError):
            QgsGeometry().asMultiPoint()

        # as multilinestring
        self.assertEqual(QgsGeometry.fromWkt('MultiLineString((11 13,14 15, 11 15, 11 13))').asMultiPolyline(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('MultiLineStringZ((11 13 1,14 15 2, 11 15 3, 11 13 1))').asMultiPolyline(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('MultiLineStringM((11 13 1,14 15 2, 11 15 3, 11 13 1))').asMultiPolyline(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        self.assertEqual(QgsGeometry.fromWkt('MultiLineStringZM((11 13 1 11,14 15 2 12 , 11 15 3 13 , 11 13 1 11))').asMultiPolyline(),
                         [[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]])
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Point(11 13)').asMultiPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asMultiPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13, 14 15, 17 18, 11 13))').asMultiPolyline()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPolygon(((11 13,14 15, 11 15, 11 13)))').asMultiPolyline()
        with self.assertRaises(ValueError):
            QgsGeometry().asPolygon()

        # as multipolygon
        self.assertEqual(QgsGeometry.fromWkt('MultiPolygon(((11 13,14 15, 11 15, 11 13)))').asMultiPolygon(),
                         [[[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]]])
        self.assertEqual(
            QgsGeometry.fromWkt('MultiPolygonZ(((11 13 1,14 15 2, 11 15 3 , 11 13 1)))').asMultiPolygon(),
            [[[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]]])
        self.assertEqual(
            QgsGeometry.fromWkt('MultiPolygonM(((11 13 1,14 15 2, 11 15 3 , 11 13 1)))').asMultiPolygon(),
            [[[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]]])
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygonZM(((11 13 1 11,14 15 2 12, 11 15 3 13, 11 13 1 11)))').asMultiPolygon(),
            [[[QgsPointXY(11, 13), QgsPointXY(14, 15), QgsPointXY(11, 15), QgsPointXY(11, 13)]]])
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Point(11 13)').asMultiPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asMultiPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13, 14 15, 17 18, 11 13))').asMultiPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asPolygon()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiLineString((11 13,14 15, 11 15, 11 13))').asMultiPolygon()
        with self.assertRaises(ValueError):
            QgsGeometry().asPolygon()

    def testReferenceGeometry(self):
        """ Test parsing a whole range of valid reference wkt formats and variants, and checking
        expected values such as length, area, centroids, bounding boxes, etc of the resultant geometry.
        Note the bulk of this test data was taken from the PostGIS WKT test data """

        with open(os.path.join(TEST_DATA_DIR, 'geom_data.csv'), 'r') as f:
            reader = csv.DictReader(f)
            for i, row in enumerate(reader):

                # test that geometry can be created from WKT
                geom = QgsGeometry.fromWkt(row['wkt'])
                if row['valid_wkt']:
                    assert geom, "WKT conversion {} failed: could not create geom:\n{}\n".format(i + 1, row['wkt'])
                else:
                    assert not geom, "Corrupt WKT {} was incorrectly converted to geometry:\n{}\n".format(i + 1, row['wkt'])
                    continue

                # test exporting to WKT results in expected string
                result = geom.asWkt()
                exp = row['valid_wkt']
                assert compareWkt(result, exp, 0.000001), "WKT conversion {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test num points in geometry
                exp_nodes = int(row['num_points'])
                self.assertEqual(geom.constGet().nCoordinates(), exp_nodes, "Node count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_nodes, geom.constGet().nCoordinates()))

                # test num geometries in collections
                exp_geometries = int(row['num_geometries'])
                try:
                    self.assertEqual(geom.constGet().numGeometries(), exp_geometries, "Geometry count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_geometries, geom.constGet().numGeometries()))
                except:
                    # some geometry types don't have numGeometries()
                    assert exp_geometries <= 1, "Geometry count {}:  Expected:\n{} geometries but could not call numGeometries()\n".format(i + 1, exp_geometries)

                # test count of rings
                exp_rings = int(row['num_rings'])
                try:
                    self.assertEqual(geom.constGet().numInteriorRings(), exp_rings, "Ring count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_rings, geom.constGet().numInteriorRings()))
                except:
                    # some geometry types don't have numInteriorRings()
                    assert exp_rings <= 1, "Ring count {}:  Expected:\n{} rings but could not call numInteriorRings()\n{}".format(i + 1, exp_rings, geom.constGet())

                # test isClosed
                exp = (row['is_closed'] == '1')
                try:
                    self.assertEqual(geom.constGet().isClosed(), exp, "isClosed {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, True, geom.constGet().isClosed()))
                except:
                    # some geometry types don't have isClosed()
                    assert not exp, "isClosed {}:  Expected:\n isClosed() but could not call isClosed()\n".format(i + 1)

                # test geometry centroid
                exp = row['centroid']
                result = geom.centroid().asWkt()
                assert compareWkt(result, exp, 0.00001), "Centroid {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test bounding box limits
                bbox = geom.constGet().boundingBox()
                exp = float(row['x_min'])
                result = bbox.xMinimum()
                self.assertAlmostEqual(result, exp, 5, "Min X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['y_min'])
                result = bbox.yMinimum()
                self.assertAlmostEqual(result, exp, 5, "Min Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['x_max'])
                result = bbox.xMaximum()
                self.assertAlmostEqual(result, exp, 5, "Max X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['y_max'])
                result = bbox.yMaximum()
                self.assertAlmostEqual(result, exp, 5, "Max Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test area calculation
                exp = float(row['area'])
                result = geom.constGet().area()
                self.assertAlmostEqual(result, exp, 5, "Area {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test length calculation
                exp = float(row['length'])
                result = geom.constGet().length()
                self.assertAlmostEqual(result, exp, 5, "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test perimeter calculation
                exp = float(row['perimeter'])
                result = geom.constGet().perimeter()
                self.assertAlmostEqual(result, exp, 5, "Perimeter {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

    def testCollection(self):
        g = QgsGeometry.fromWkt('MultiLineString()')
        self.assertEqual(len(g.get()), 0)
        self.assertTrue(g.get())
        g = QgsGeometry.fromWkt('MultiLineString((0 0, 1 1),(13 2, 14 1))')
        self.assertEqual(len(g.get()), 2)
        self.assertTrue(g.get())
        self.assertEqual(g.get().geometryN(0).asWkt(), 'LineString (0 0, 1 1)')
        self.assertEqual(g.get().geometryN(1).asWkt(), 'LineString (13 2, 14 1)')
        with self.assertRaises(IndexError):
            g.get().geometryN(-1)
        with self.assertRaises(IndexError):
            g.get().geometryN(2)

    def testIntersection(self):
        myLine = QgsGeometry.fromPolylineXY([
            QgsPointXY(0, 0),
            QgsPointXY(1, 1),
            QgsPointXY(2, 2)])
        myPoint = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        intersectionGeom = QgsGeometry.intersection(myLine, myPoint)
        self.assertEqual(intersectionGeom.wkbType(), QgsWkbTypes.Point)

        layer = QgsVectorLayer("Point", "intersection", "memory")
        assert layer.isValid(), "Failed to create valid point memory layer"

        provider = layer.dataProvider()

        ft = QgsFeature()
        ft.setGeometry(intersectionGeom)
        provider.addFeatures([ft])

        self.assertEqual(layer.featureCount(), 1)

    def testBuffer(self):
        myPoint = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        bufferGeom = myPoint.buffer(10, 5)
        self.assertEqual(bufferGeom.wkbType(), QgsWkbTypes.Polygon)
        myTestPoint = QgsGeometry.fromPointXY(QgsPointXY(3, 3))
        self.assertTrue(bufferGeom.intersects(myTestPoint))

    def testContains(self):
        myPoly = QgsGeometry.fromPolygonXY(
            [[QgsPointXY(0, 0),
              QgsPointXY(2, 0),
              QgsPointXY(2, 2),
              QgsPointXY(0, 2),
              QgsPointXY(0, 0)]])
        myPoint = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        self.assertTrue(QgsGeometry.contains(myPoly, myPoint))

    def testTouches(self):
        myLine = QgsGeometry.fromPolylineXY([
            QgsPointXY(0, 0),
            QgsPointXY(1, 1),
            QgsPointXY(2, 2)])
        myPoly = QgsGeometry.fromPolygonXY([[
            QgsPointXY(0, 0),
            QgsPointXY(1, 1),
            QgsPointXY(2, 0),
            QgsPointXY(0, 0)]])
        touchesGeom = QgsGeometry.touches(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", touchesGeom))
        assert touchesGeom, myMessage

    def testOverlaps(self):
        myPolyA = QgsGeometry.fromPolygonXY([[
            QgsPointXY(0, 0),
            QgsPointXY(1, 3),
            QgsPointXY(2, 0),
            QgsPointXY(0, 0)]])
        myPolyB = QgsGeometry.fromPolygonXY([[
            QgsPointXY(0, 0),
            QgsPointXY(2, 0),
            QgsPointXY(2, 2),
            QgsPointXY(0, 2),
            QgsPointXY(0, 0)]])
        overlapsGeom = QgsGeometry.overlaps(myPolyA, myPolyB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", overlapsGeom))
        assert overlapsGeom, myMessage

    def testWithin(self):
        myLine = QgsGeometry.fromPolylineXY([
            QgsPointXY(0.5, 0.5),
            QgsPointXY(1, 1),
            QgsPointXY(1.5, 1.5)
        ])
        myPoly = QgsGeometry.fromPolygonXY([[
            QgsPointXY(0, 0),
            QgsPointXY(2, 0),
            QgsPointXY(2, 2),
            QgsPointXY(0, 2),
            QgsPointXY(0, 0)]])
        withinGeom = QgsGeometry.within(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", withinGeom))
        assert withinGeom, myMessage

    def testEquals(self):
        myPointA = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        myPointB = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        equalsGeom = QgsGeometry.equals(myPointA, myPointB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", equalsGeom))
        assert equalsGeom, myMessage

    def testCrosses(self):
        myLine = QgsGeometry.fromPolylineXY([
            QgsPointXY(0, 0),
            QgsPointXY(1, 1),
            QgsPointXY(3, 3)])
        myPoly = QgsGeometry.fromPolygonXY([[
            QgsPointXY(1, 0),
            QgsPointXY(2, 0),
            QgsPointXY(2, 2),
            QgsPointXY(1, 2),
            QgsPointXY(1, 0)]])
        crossesGeom = QgsGeometry.crosses(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", crossesGeom))
        assert crossesGeom, myMessage

    def testSimplifyIssue4189(self):
        """Test we can simplify a complex geometry.

        Note: there is a ticket related to this issue here:
        https://issues.qgis.org/issues/4189

        Backstory: Ole Nielson pointed out an issue to me
        (Tim Sutton) where simplify ftools was dropping
        features. This test replicates that issues.

        Interestingly we could replicate the issue in PostGIS too:
         - doing straight simplify returned no feature
         - transforming to UTM49, then simplify with e.g. 200 threshold is OK
         - as above with 500 threshold drops the feature

         pgsql2shp -f /tmp/dissolve500.shp gis 'select *,
           transform(simplify(transform(geom,32649),500), 4326) as
           simplegeom from dissolve;'
        """
        with open(os.path.join(unitTestDataPath('wkt'), 'simplify_error.wkt'), 'rt') as myWKTFile:
            myWKT = myWKTFile.readline()
        # print myWKT
        myGeometry = QgsGeometry().fromWkt(myWKT)
        assert myGeometry is not None
        myStartLength = len(myWKT)
        myTolerance = 0.00001
        mySimpleGeometry = myGeometry.simplify(myTolerance)
        myEndLength = len(mySimpleGeometry.asWkt())
        myMessage = 'Before simplify: %i\nAfter simplify: %i\n : Tolerance %e' % (
            myStartLength, myEndLength, myTolerance)
        myMinimumLength = len('Polygon(())')
        assert myEndLength > myMinimumLength, myMessage

    def testClipping(self):
        """Test that we can clip geometries using other geometries."""
        myMemoryLayer = QgsVectorLayer(
            ('LineString?crs=epsg:4326&field=name:string(20)&index=yes'),
            'clip-in',
            'memory')

        assert myMemoryLayer is not None, 'Provider not initialized'
        myProvider = myMemoryLayer.dataProvider()
        assert myProvider is not None

        myFeature1 = QgsFeature()
        myFeature1.setGeometry(QgsGeometry.fromPolylineXY([
            QgsPointXY(10, 10),
            QgsPointXY(20, 10),
            QgsPointXY(30, 10),
            QgsPointXY(40, 10),
        ]))
        myFeature1.setAttributes(['Johny'])

        myFeature2 = QgsFeature()
        myFeature2.setGeometry(QgsGeometry.fromPolylineXY([
            QgsPointXY(10, 10),
            QgsPointXY(20, 20),
            QgsPointXY(30, 30),
            QgsPointXY(40, 40),
        ]))
        myFeature2.setAttributes(['Be'])

        myFeature3 = QgsFeature()
        myFeature3.setGeometry(QgsGeometry.fromPolylineXY([
            QgsPointXY(10, 10),
            QgsPointXY(10, 20),
            QgsPointXY(10, 30),
            QgsPointXY(10, 40),
        ]))

        myFeature3.setAttributes(['Good'])

        myResult, myFeatures = myProvider.addFeatures(
            [myFeature1, myFeature2, myFeature3])
        assert myResult
        self.assertEqual(len(myFeatures), 3)

        myClipPolygon = QgsGeometry.fromPolygonXY([[
            QgsPointXY(20, 20),
            QgsPointXY(20, 30),
            QgsPointXY(30, 30),
            QgsPointXY(30, 20),
            QgsPointXY(20, 20),
        ]])
        print(('Clip: %s' % myClipPolygon.asWkt()))
        writeShape(myMemoryLayer, 'clipGeometryBefore.shp')
        fit = myProvider.getFeatures()
        myFeatures = []
        myFeature = QgsFeature()
        while fit.nextFeature(myFeature):
            myGeometry = myFeature.geometry()
            if myGeometry.intersects(myClipPolygon):
                # Adds nodes where the clip and the line intersec
                myCombinedGeometry = myGeometry.combine(myClipPolygon)
                # Gives you the areas inside the clip
                mySymmetricalGeometry = myGeometry.symDifference(
                    myCombinedGeometry)
                # Gives you areas outside the clip area
                # myDifferenceGeometry = myCombinedGeometry.difference(
                #    myClipPolygon)
                # print 'Original: %s' % myGeometry.asWkt()
                # print 'Combined: %s' % myCombinedGeometry.asWkt()
                # print 'Difference: %s' % myDifferenceGeometry.asWkt()
                print(('Symmetrical: %s' % mySymmetricalGeometry.asWkt()))

                myExpectedWkt = 'Polygon ((20 20, 20 30, 30 30, 30 20, 20 20))'

                # There should only be one feature that intersects this clip
                # poly so this assertion should work.
                assert compareWkt(myExpectedWkt,
                                  mySymmetricalGeometry.asWkt())

                myNewFeature = QgsFeature()
                myNewFeature.setAttributes(myFeature.attributes())
                myNewFeature.setGeometry(mySymmetricalGeometry)
                myFeatures.append(myNewFeature)

        myNewMemoryLayer = QgsVectorLayer(
            ('Polygon?crs=epsg:4326&field=name:string(20)&index=yes'),
            'clip-out',
            'memory')
        myNewProvider = myNewMemoryLayer.dataProvider()
        myResult, myFeatures = myNewProvider.addFeatures(myFeatures)
        self.assertTrue(myResult)
        self.assertEqual(len(myFeatures), 1)

        writeShape(myNewMemoryLayer, 'clipGeometryAfter.shp')

    def testClosestVertex(self):
        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        polyline = QgsGeometry.fromPolylineXY(
            [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
        )

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPointXY(6, 1))
        self.assertEqual(point, QgsPointXY(5, 1))
        self.assertEqual(beforeVertex, 3)
        self.assertEqual(atVertex, 4)
        self.assertEqual(afterVertex, 5)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex, leftOf) = polyline.closestSegmentWithContext(QgsPointXY(6, 2))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPointXY(5, 2))
        self.assertEqual(afterVertex, 4)
        self.assertEqual(leftOf, -1)

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPointXY(6, 0))
        self.assertEqual(point, QgsPointXY(5, 0))
        self.assertEqual(beforeVertex, -1)
        self.assertEqual(atVertex, 0)
        self.assertEqual(afterVertex, 1)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex, leftOf) = polyline.closestSegmentWithContext(QgsPointXY(6, 0))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPointXY(5, 0))
        self.assertEqual(afterVertex, 1)
        self.assertEqual(leftOf, 0)

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPointXY(0, -1))
        self.assertEqual(point, QgsPointXY(0, 0))
        self.assertEqual(beforeVertex, 0)
        self.assertEqual(atVertex, 1)
        self.assertEqual(afterVertex, 2)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex, leftOf) = polyline.closestSegmentWithContext(QgsPointXY(0, 1))
        self.assertEqual(dist, 0)
        self.assertEqual(minDistPoint, QgsPointXY(0, 1))
        self.assertEqual(afterVertex, 2)
        self.assertEqual(leftOf, 0)

        #   2-3 6-+-7 !
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromMultiPolylineXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
                [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPointXY(5, 2))
        self.assertEqual(point, QgsPointXY(5, 1))
        self.assertEqual(beforeVertex, 6)
        self.assertEqual(atVertex, 7)
        self.assertEqual(afterVertex, 8)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex, leftOf) = polyline.closestSegmentWithContext(QgsPointXY(7, 0))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPointXY(6, 0))
        self.assertEqual(afterVertex, 9)
        self.assertEqual(leftOf, 0)

        # 5---4
        # |!  |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPointXY(0.7, 1.1))
        self.assertEqual(point, QgsPointXY(1, 1))
        self.assertEqual(beforeVertex, 1)
        self.assertEqual(atVertex, 2)
        self.assertEqual(afterVertex, 3)
        assert abs(dist - 0.1) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.1)

        (dist, minDistPoint, afterVertex, leftOf) = polygon.closestSegmentWithContext(QgsPointXY(0.7, 1.1))
        self.assertEqual(afterVertex, 2)
        self.assertEqual(minDistPoint, QgsPointXY(1, 1))
        exp = 0.3 ** 2 + 0.1 ** 2
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)
        self.assertEqual(leftOf, -1)

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | |!| |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        polygon = QgsGeometry.fromPolygonXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(3, 0), QgsPointXY(3, 3), QgsPointXY(0, 3), QgsPointXY(0, 0)],
                [QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)],
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPointXY(1.1, 1.9))
        self.assertEqual(point, QgsPointXY(1, 2))
        self.assertEqual(beforeVertex, 7)
        self.assertEqual(atVertex, 8)
        self.assertEqual(afterVertex, 9)
        assert abs(dist - 0.02) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.02)

        (dist, minDistPoint, afterVertex, leftOf) = polygon.closestSegmentWithContext(QgsPointXY(1.2, 1.9))
        self.assertEqual(afterVertex, 8)
        self.assertEqual(minDistPoint, QgsPointXY(1.2, 2))
        exp = 0.01
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)
        self.assertEqual(leftOf, -1)

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2!+
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygonXY(
            [
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPointXY(4.1, 1.1))
        self.assertEqual(point, QgsPointXY(4, 1))
        self.assertEqual(beforeVertex, 11)
        self.assertEqual(atVertex, 12)
        self.assertEqual(afterVertex, 13)
        assert abs(dist - 0.02) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.02)

        (dist, minDistPoint, afterVertex, leftOf) = polygon.closestSegmentWithContext(QgsPointXY(4.1, 1.1))
        self.assertEqual(afterVertex, 12)
        self.assertEqual(minDistPoint, QgsPointXY(4, 1))
        exp = 0.02
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)
        self.assertEqual(leftOf, -1)

    def testAdjacentVertex(self):
        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        polyline = QgsGeometry.fromPolylineXY(
            [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
        )

        # don't crash
        (before, after) = polyline.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 10):
            (before, after) = polyline.adjacentVertices(i)
            if i == 0:
                self.assertEqual(before == -1 and after, 1, "Expected (0,1), Got:(%d,%d)" % (before, after))
            elif i == 9:
                self.assertEqual(before == i - 1 and after, -1, "Expected (0,1), Got:(%d,%d)" % (before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1, "Expected (0,1), Got:(%d,%d)" % (before, after))

        (before, after) = polyline.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromMultiPolylineXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
                [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
            ]
        )

        (before, after) = polyline.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 10):
            (before, after) = polyline.adjacentVertices(i)

            if i == 0 or i == 5:
                self.assertEqual(before == -1 and after, i + 1, "Expected (-1,%d), Got:(%d,%d)" % (i + 1, before, after))
            elif i == 4 or i == 9:
                self.assertEqual(before == i - 1 and after, -1, "Expected (%d,-1), Got:(%d,%d)" % (i - 1, before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polyline.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )

        (before, after) = polygon.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 7):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 6:
                self.assertEqual(before == 5 and after, 1, "Expected (5,1), Got:(%d,%d)" % (before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polygon.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        polygon = QgsGeometry.fromPolygonXY(
            [
                [QgsPointXY(0, 0), QgsPointXY(3, 0), QgsPointXY(3, 3), QgsPointXY(0, 3), QgsPointXY(0, 0)],
                [QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)],
            ]
        )

        (before, after) = polygon.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 8):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 4:
                self.assertEqual(before == 3 and after, 1, "Expected (3,1), Got:(%d,%d)" % (before, after))
            elif i == 5:
                self.assertEqual(before == 8 and after, 6, "Expected (2,0), Got:(%d,%d)" % (before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polygon.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygonXY(
            [
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
            ]
        )

        (before, after) = polygon.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 14):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 6:
                self.assertEqual(before == 5 and after, 1, "Expected (5,1), Got:(%d,%d)" % (before, after))
            elif i == 7 or i == 13:
                self.assertEqual(before == 12 and after, 8, "Expected (12,8), Got:(%d,%d)" % (before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polygon.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

    def testVertexAt(self):
        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        points = [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
        polyline = QgsGeometry.fromPolylineXY(points)

        for i in range(0, len(points)):
            self.assertEqual(QgsPoint(points[i]), polyline.vertexAt(i), "Mismatch at %d" % i)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
            [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolylineXY(points)

        p = polyline.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polyline.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(QgsPoint(points[j][k]), polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
        ]]
        polygon = QgsGeometry.fromPolygonXY(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(QgsPoint(points[j][k]), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        points = [
            [QgsPointXY(0, 0), QgsPointXY(3, 0), QgsPointXY(3, 3), QgsPointXY(0, 3), QgsPointXY(0, 0)],
            [QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)],
        ]
        polygon = QgsGeometry.fromPolygonXY(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(QgsPoint(points[j][k]), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
        ]

        polygon = QgsGeometry.fromMultiPolygonXY(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                for l in range(0, len(points[j][k])):
                    p = polygon.vertexAt(i)
                    self.assertEqual(QgsPoint(points[j][k][l]), p, "Got {},{} Expected {} at {} / {},{},{}".format(p.x(), p.y(), points[j][k][l].toString(), i, j, k, l))
                    i += 1

    def testMultipoint(self):
        # #9423
        points = [QgsPointXY(10, 30), QgsPointXY(40, 20), QgsPointXY(30, 10), QgsPointXY(20, 10)]
        wkt = "MultiPoint ((10 30),(40 20),(30 10),(20 10))"
        multipoint = QgsGeometry.fromWkt(wkt)
        assert multipoint.isMultipart(), "Expected MultiPoint to be multipart"
        self.assertEqual(multipoint.wkbType(), QgsWkbTypes.MultiPoint, "Expected wkbType to be WKBMultipoint")
        i = 0
        for p in multipoint.asMultiPoint():
            self.assertEqual(p, points[i], "Expected %s at %d, got %s" % (points[i].toString(), i, p.toString()))
            i += 1

        multipoint = QgsGeometry.fromWkt("MultiPoint ((5 5))")
        self.assertEqual(multipoint.vertexAt(0), QgsPoint(5, 5), "MULTIPOINT fromWkt failed")

        assert multipoint.insertVertex(4, 4, 0), "MULTIPOINT insert 4,4 at 0 failed"
        expwkt = "MultiPoint ((4 4),(5 5))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.insertVertex(7, 7, 2), "MULTIPOINT append 7,7 at 2 failed"
        expwkt = "MultiPoint ((4 4),(5 5),(7 7))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.insertVertex(6, 6, 2), "MULTIPOINT append 6,6 at 2 failed"
        expwkt = "MultiPoint ((4 4),(5 5),(6 6),(7 7))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not multipoint.deleteVertex(4), "MULTIPOINT delete at 4 unexpectedly succeeded"
        assert not multipoint.deleteVertex(-1), "MULTIPOINT delete at -1 unexpectedly succeeded"

        assert multipoint.deleteVertex(1), "MULTIPOINT delete at 1 failed"
        expwkt = "MultiPoint ((4 4),(6 6),(7 7))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.deleteVertex(2), "MULTIPOINT delete at 2 failed"
        expwkt = "MultiPoint ((4 4),(6 6))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.deleteVertex(0), "MULTIPOINT delete at 2 failed"
        expwkt = "MultiPoint ((6 6))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        multipoint = QgsGeometry.fromWkt("MultiPoint ((5 5))")
        self.assertEqual(multipoint.vertexAt(0), QgsPoint(5, 5), "MultiPoint fromWkt failed")

    def testMoveVertex(self):
        multipoint = QgsGeometry.fromWkt("MultiPoint ((5 0),(0 0),(0 4),(5 4),(5 1),(1 1),(1 3),(4 3),(4 2),(2 2))")

        # try moving invalid vertices
        assert not multipoint.moveVertex(9, 9, -1), "move vertex succeeded when it should have failed"
        assert not multipoint.moveVertex(9, 9, 10), "move vertex succeeded when it should have failed"
        assert not multipoint.moveVertex(9, 9, 11), "move vertex succeeded when it should have failed"

        for i in range(0, 10):
            assert multipoint.moveVertex(i + 1, -1 - i, i), "move vertex %d failed" % i
        expwkt = "MultiPoint ((1 -1),(2 -2),(3 -3),(4 -4),(5 -5),(6 -6),(7 -7),(8 -8),(9 -9),(10 -10))"
        wkt = multipoint.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        polyline = QgsGeometry.fromWkt("LineString (5 0, 0 0, 0 4, 5 4, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)")

        # try moving invalid vertices
        assert not polyline.moveVertex(9, 9, -1), "move vertex succeeded when it should have failed"
        assert not polyline.moveVertex(9, 9, 10), "move vertex succeeded when it should have failed"
        assert not polyline.moveVertex(9, 9, 11), "move vertex succeeded when it should have failed"

        assert polyline.moveVertex(5.5, 4.5, 3), "move vertex failed"
        expwkt = "LineString (5 0, 0 0, 0 4, 5.5 4.5, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5-+-4
        # |   |
        # 6 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))")

        assert not polygon.moveVertex(3, 4, -10), "move vertex unexpectedly succeeded"
        assert not polygon.moveVertex(3, 4, 7), "move vertex unexpectedly succeeded"
        assert not polygon.moveVertex(3, 4, 8), "move vertex unexpectedly succeeded"

        assert polygon.moveVertex(1, 2, 0), "move vertex failed"
        expwkt = "Polygon ((1 2, 1 0, 1 1, 2 1, 2 2, 0 2, 1 2))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(3, 4, 3), "move vertex failed"
        expwkt = "Polygon ((1 2, 1 0, 1 1, 3 4, 2 2, 0 2, 1 2))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(2, 3, 6), "move vertex failed"
        expwkt = "Polygon ((2 3, 1 0, 1 1, 3 4, 2 2, 0 2, 2 3))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2!+
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")

        assert not polygon.moveVertex(3, 4, -10), "move vertex unexpectedly succeeded"
        assert not polygon.moveVertex(3, 4, 14), "move vertex unexpectedly succeeded"
        assert not polygon.moveVertex(3, 4, 15), "move vertex unexpectedly succeeded"

        assert polygon.moveVertex(6, 2, 9), "move vertex failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 6 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(1, 2, 0), "move vertex failed"
        expwkt = "MultiPolygon (((1 2, 1 0, 1 1, 2 1, 2 2, 0 2, 1 2)),((4 0, 5 0, 6 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(2, 1, 7), "move vertex failed"
        expwkt = "MultiPolygon (((1 2, 1 0, 1 1, 2 1, 2 2, 0 2, 1 2)),((2 1, 5 0, 6 2, 3 2, 3 1, 4 1, 2 1)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testDeleteVertex(self):
        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4
        # |
        # 1-+-+-+-+-0
        polyline = QgsGeometry.fromWkt("LineString (5 0, 0 0, 0 4, 5 4, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)")
        assert polyline.deleteVertex(3), "Delete vertex 5 4 failed"
        expwkt = "LineString (5 0, 0 0, 0 4, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not polyline.deleteVertex(-5), "Delete vertex -5 unexpectedly succeeded"
        assert not polyline.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromWkt("MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not polyline.deleteVertex(-100), "Delete vertex -100 unexpectedly succeeded"
        assert not polyline.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        assert polyline.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "MultiLineString ((1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polyline = QgsGeometry.fromWkt("MultiLineString ((0 0, 1 0, 1 1, 2 1,2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        for i in range(4):
            assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))")

        assert polygon.deleteVertex(2), "Delete vertex 2 failed"
        expwkt = "Polygon ((0 0, 1 0, 2 1, 2 2, 0 2, 0 0))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "Polygon ((1 0, 2 1, 2 2, 0 2, 1 0))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(4), "Delete vertex 4 failed"
        # "Polygon ((2 1, 2 2, 0 2, 2 1))" #several possibilities are correct here
        expwkt = "Polygon ((0 2, 2 1, 2 2, 0 2))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not polygon.deleteVertex(-100), "Delete vertex -100 unexpectedly succeeded"
        assert not polygon.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2 +
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.deleteVertex(9), "Delete vertex 5 2 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "MultiPolygon (((1 0, 1 1, 2 1, 2 2, 0 2, 1 0)),((4 0, 5 0, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(6), "Delete vertex 6 failed"
        expwkt = "MultiPolygon (((1 0, 1 1, 2 1, 2 2, 0 2, 1 0)),((5 0, 3 2, 3 1, 4 1, 5 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        for i in range(4):
            assert polygon.deleteVertex(0), "Delete vertex 0 failed"

        expwkt = "MultiPolygon (((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 3-+-+-+-+-+-+-+-+-2
        # |                 |
        # + 8-7 3-2 8-7 3-2 +
        # | | | | | | | | | |
        # + 5-6 0-1 5-6 0-1 +
        # |                 |
        # 0-+-+-+-+---+-+-+-1
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1),(5 1, 6 1, 6 2, 5 2, 5 1),(7 1, 8 1, 8 2, 7 2, 7 1))")
        #                                         0   1    2    3    4     5    6    7    8    9     10   11   12   13   14    15   16   17   18   19    20  21   22   23   24

        for i in range(2):
            assert polygon.deleteVertex(16), "Delete vertex 16 failed" % i

        expwkt = "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1),(7 1, 8 1, 8 2, 7 2, 7 1))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        for i in range(3):
            for j in range(2):
                assert polygon.deleteVertex(5), "Delete vertex 5 failed" % i

        expwkt = "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # Remove whole outer ring, inner ring should become outer
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1))")
        for i in range(2):
            assert polygon.deleteVertex(0), "Delete vertex 16 failed" % i

        expwkt = "Polygon ((1 1, 2 1, 2 2, 1 2, 1 1))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testInsertVertex(self):
        linestring = QgsGeometry.fromWkt("LineString(1 0, 2 0)")

        assert linestring.insertVertex(0, 0, 0), "Insert vertex 0 0 at 0 failed"
        expwkt = "LineString (0 0, 1 0, 2 0)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert linestring.insertVertex(1.5, 0, 2), "Insert vertex 1.5 0 at 2 failed"
        expwkt = "LineString (0 0, 1 0, 1.5 0, 2 0)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not linestring.insertVertex(3, 0, 5), "Insert vertex 3 0 at 5 should have failed"

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.insertVertex(0, 0, 8), "Insert vertex 0 0 at 8 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 0 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.insertVertex(0, 0, 7), "Insert vertex 0 0 at 7 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((0 0, 4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 0 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testTranslate(self):
        point = QgsGeometry.fromWkt("Point (1 1)")
        self.assertEqual(point.translate(1, 2), 0, "Translate failed")
        expwkt = "Point (2 3)"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        self.assertEqual(point.translate(1, 2), 0, "Translate failed")
        expwkt = "MultiPoint ((2 3),(3 4),(4 5))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        self.assertEqual(linestring.translate(1, 2), 0, "Translate failed")
        expwkt = "LineString (2 2, 3 2)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        self.assertEqual(polygon.translate(1, 2), 0, "Translate failed")
        expwkt = "MultiPolygon (((1 2, 2 2, 2 3, 3 3, 3 4, 1 4, 1 2)),((5 2, 6 2, 6 4, 4 4, 4 3, 5 3, 5 2)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testTransform(self):
        # null transform
        ct = QgsCoordinateTransform()

        point = QgsGeometry.fromWkt("Point (1 1)")
        self.assertEqual(point.transform(ct), 0, "Transform failed")
        expwkt = "Point (1 1)"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        self.assertEqual(point.transform(ct), 0, "Transform failed")
        expwkt = "MultiPoint ((1 1),(2 2),(3 3))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        self.assertEqual(linestring.transform(ct), 0, "Transform failed")
        expwkt = "LineString (1 0, 2 0)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))")
        self.assertEqual(polygon.transform(ct), 0, "Transform failed")
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # valid transform
        ct = QgsCoordinateTransform(QgsCoordinateReferenceSystem(4326), QgsCoordinateReferenceSystem(3857), QgsProject.instance())

        point = QgsGeometry.fromWkt("Point (1 1)")
        self.assertEqual(point.transform(ct), 0, "Transform failed")
        expwkt = "Point (111319 111325)"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt, tol=100), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        self.assertEqual(point.transform(ct), 0, "Transform failed")
        expwkt = "MultiPoint ((111319 111325),(222638 222684),(333958 334111))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt, tol=100), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        self.assertEqual(linestring.transform(ct), 0, "Transform failed")
        expwkt = "LineString (111319 0, 222638 0)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt, tol=100), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))")
        self.assertEqual(polygon.transform(ct), 0, "Transform failed")
        expwkt = "MultiPolygon (((0 0, 111319 0, 111319 111325, 222638 111325, 222638 222684, 0 222684, 0 0)),((445277 0, 556597 0, 556597 222684, 333958 222684, 333958 111325, 445277 111325, 445277 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt, tol=100), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # reverse transform
        point = QgsGeometry.fromWkt("Point (111319 111325)")
        self.assertEqual(point.transform(ct, QgsCoordinateTransform.ReverseTransform), 0, "Transform failed")
        expwkt = "Point (1 1)"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt, tol=0.01), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((111319 111325),(222638 222684),(333958 334111))")
        self.assertEqual(point.transform(ct, QgsCoordinateTransform.ReverseTransform), 0, "Transform failed")
        expwkt = "MultiPoint ((1 1),(2 2),(3 3))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt, tol=0.01), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (111319 0, 222638 0)")
        self.assertEqual(linestring.transform(ct, QgsCoordinateTransform.ReverseTransform), 0, "Transform failed")
        expwkt = "LineString (1 0, 2 0)"
        wkt = linestring.asWkt()
        assert compareWkt(expwkt, wkt, tol=0.01), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 111319 0, 111319 111325, 222638 111325, 222638 222684, 0 222684, 0 0)),((445277 0, 556597 0, 556597 222684, 333958 222684, 333958 111325, 445277 111325, 445277 0)))")
        self.assertEqual(polygon.transform(ct, QgsCoordinateTransform.ReverseTransform), 0, "Transform failed")
        expwkt = "MultiPolygon(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt, tol=0.01), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testExtrude(self):
        # test with empty geometry
        g = QgsGeometry()
        self.assertTrue(g.extrude(1, 2).isNull())

        points = [QgsPointXY(1, 2), QgsPointXY(3, 2), QgsPointXY(4, 3)]
        line = QgsGeometry.fromPolylineXY(points)
        expected = QgsGeometry.fromWkt('Polygon ((1 2, 3 2, 4 3, 5 5, 4 4, 2 4, 1 2))')
        self.assertEqual(line.extrude(1, 2).asWkt(), expected.asWkt())

        points2 = [[QgsPointXY(1, 2), QgsPointXY(3, 2)], [QgsPointXY(4, 3), QgsPointXY(8, 3)]]
        multiline = QgsGeometry.fromMultiPolylineXY(points2)
        expected = QgsGeometry.fromWkt('MultiPolygon (((1 2, 3 2, 4 4, 2 4, 1 2)),((4 3, 8 3, 9 5, 5 5, 4 3)))')
        self.assertEqual(multiline.extrude(1, 2).asWkt(), expected.asWkt())

    def testNearestPoint(self):
        # test with empty geometries
        g1 = QgsGeometry()
        g2 = QgsGeometry()
        self.assertTrue(g1.nearestPoint(g2).isNull())
        g1 = QgsGeometry.fromWkt('LineString( 1 1, 5 1, 5 5 )')
        self.assertTrue(g1.nearestPoint(g2).isNull())
        self.assertTrue(g2.nearestPoint(g1).isNull())

        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'Point( 5 3 )'
        wkt = g1.nearestPoint(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        expWkt = 'Point( 6 3 )'
        wkt = g2.nearestPoint(g1).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g1 = QgsGeometry.fromWkt('Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))')
        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'Point( 5 3 )'
        wkt = g1.nearestPoint(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        expWkt = 'Point( 6 3 )'
        wkt = g2.nearestPoint(g1).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        g2 = QgsGeometry.fromWkt('Point( 2 3 )')
        expWkt = 'Point( 2 3 )'
        wkt = g1.nearestPoint(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

    def testShortestLine(self):
        # test with empty geometries
        g1 = QgsGeometry()
        g2 = QgsGeometry()
        self.assertTrue(g1.shortestLine(g2).isNull())
        g1 = QgsGeometry.fromWkt('LineString( 1 1, 5 1, 5 5 )')
        self.assertTrue(g1.shortestLine(g2).isNull())
        self.assertTrue(g2.shortestLine(g1).isNull())

        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'LineString( 5 3, 6 3 )'
        wkt = g1.shortestLine(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        expWkt = 'LineString( 6 3, 5 3 )'
        wkt = g2.shortestLine(g1).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g1 = QgsGeometry.fromWkt('Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))')
        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'LineString( 5 3, 6 3 )'
        wkt = g1.shortestLine(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        expWkt = 'LineString( 6 3, 5 3 )'
        wkt = g2.shortestLine(g1).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        g2 = QgsGeometry.fromWkt('Point( 2 3 )')
        expWkt = 'LineString( 2 3, 2 3 )'
        wkt = g1.shortestLine(g2).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

    def testBoundingBox(self):
        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        points = [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
        polyline = QgsGeometry.fromPolylineXY(points)
        expbb = QgsRectangle(0, 0, 5, 4)
        bb = polyline.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
            [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolylineXY(points)
        expbb = QgsRectangle(0, 0, 6, 1)
        bb = polyline.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0),
        ]]
        polygon = QgsGeometry.fromPolygonXY(points)
        expbb = QgsRectangle(0, 0, 2, 2)
        bb = polygon.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        points = [
            [QgsPointXY(0, 0), QgsPointXY(3, 0), QgsPointXY(3, 3), QgsPointXY(0, 3), QgsPointXY(0, 0)],
            [QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)],
        ]
        polygon = QgsGeometry.fromPolygonXY(points)
        expbb = QgsRectangle(0, 0, 3, 3)
        bb = polygon.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
        ]

        polygon = QgsGeometry.fromMultiPolygonXY(points)
        expbb = QgsRectangle(0, 0, 5, 2)
        bb = polygon.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # NULL
        points = []
        line = QgsGeometry.fromPolylineXY(points)
        assert line.boundingBox().isNull()

    def testCollectGeometry(self):
        # collect points
        geometries = [QgsGeometry.fromPointXY(QgsPointXY(0, 0)), QgsGeometry.fromPointXY(QgsPointXY(1, 1))]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiPoint ((0 0), (1 1))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # collect lines
        points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0)],
            [QgsPointXY(2, 0), QgsPointXY(3, 0)]
        ]
        geometries = [QgsGeometry.fromPolylineXY(points[0]), QgsGeometry.fromPolylineXY(points[1])]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiLineString ((0 0, 1 0), (2 0, 3 0))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # collect polygons
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(0, 1), QgsPointXY(0, 0)]],
            [[QgsPointXY(2, 0), QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(2, 1), QgsPointXY(2, 0)]]
        ]
        geometries = [QgsGeometry.fromPolygonXY(points[0]), QgsGeometry.fromPolygonXY(points[1])]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((2 0, 3 0, 3 1, 2 1, 2 0)))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test empty list
        geometries = []
        geometry = QgsGeometry.collectGeometry(geometries)
        assert geometry.isNull(), "Expected geometry to be empty"

        # check that the resulting geometry is multi
        geometry = QgsGeometry.collectGeometry([QgsGeometry.fromWkt('Point (0 0)')])
        assert geometry.isMultipart(), "Expected collected geometry to be multipart"

    def testAddPart(self):
        # add a part to a multipoint
        points = [QgsPointXY(0, 0), QgsPointXY(1, 0)]

        point = QgsGeometry.fromPointXY(points[0])
        self.assertEqual(point.addPointsXY([points[1]]), 0)
        expwkt = "MultiPoint ((0 0), (1 0))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        point = QgsGeometry.fromPointXY(points[0])
        point.get().addZValue(4.0)
        self.assertEqual(point.addPoints([QgsPoint(points[1][0], points[1][1], 3.0, wkbType=QgsWkbTypes.PointZ)]), 0)
        expwkt = "MultiPointZ ((0 0 4), (1 0 3))"
        wkt = point.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
            [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
        ]

        polyline = QgsGeometry.fromPolylineXY(points[0])
        self.assertEqual(polyline.addPointsXY(points[1][0:1]), QgsGeometry.InvalidInputGeometryType, "addPoints with one point line unexpectedly succeeded.")
        self.assertEqual(polyline.addPointsXY(points[1][0:2]), QgsGeometry.Success, "addPoints with two point line failed.")
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polyline = QgsGeometry.fromPolylineXY(points[0])
        self.assertEqual(polyline.addPointsXY(points[1]), QgsGeometry.Success, "addPoints with %d point line failed." % len(points[1]))
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        polyline = QgsGeometry.fromPolylineXY(points[0])
        polyline.get().addZValue(4.0)
        points2 = [QgsPoint(p[0], p[1], 3.0, wkbType=QgsWkbTypes.PointZ) for p in points[1]]
        self.assertEqual(polyline.addPoints(points2), QgsGeometry.Success)
        expwkt = "MultiLineStringZ ((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 0 4),(3 0 3, 3 1 3, 5 1 3, 5 0 3, 6 0 3))"
        wkt = polyline.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
        ]

        polygon = QgsGeometry.fromPolygonXY(points[0])

        self.assertEqual(polygon.addPointsXY(points[1][0][0:1]), QgsGeometry.InvalidInputGeometryType, "addPoints with one point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPointsXY(points[1][0][0:2]), QgsGeometry.InvalidInputGeometryType, "addPoints with two point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPointsXY(points[1][0][0:3]), QgsGeometry.InvalidInputGeometryType, "addPoints with unclosed three point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPointsXY([QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(4, 0)]), QgsGeometry.InvalidInputGeometryType, "addPoints with 'closed' three point ring unexpectedly succeeded.")

        self.assertEqual(polygon.addPointsXY(points[1][0]), QgsGeometry.Success, "addPoints failed")
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygonXY(points[:1])
        p = QgsGeometry.fromPolygonXY(points[1])

        self.assertEqual(mp.addPartGeometry(p), QgsGeometry.Success)
        wkt = mp.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygonXY(points[:1])
        mp2 = QgsGeometry.fromMultiPolygonXY(points[1:])
        self.assertEqual(mp.addPartGeometry(mp2), QgsGeometry.Success)
        wkt = mp.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        polygon = QgsGeometry.fromPolygonXY(points[0])
        polygon.get().addZValue(4.0)
        points2 = [QgsPoint(pi[0], pi[1], 3.0, wkbType=QgsWkbTypes.PointZ) for pi in points[1][0]]
        self.assertEqual(polygon.addPoints(points2), QgsGeometry.Success)
        expwkt = "MultiPolygonZ (((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 2 4, 0 2 4, 0 0 4)),((4 0 3, 5 0 3, 5 2 3, 3 2 3, 3 1 3, 4 1 3, 4 0 3)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part to a multisurface
        geom = QgsGeometry.fromWkt('MultiSurface(((0 0,0 1,1 1,0 0)))')
        g2 = QgsGeometry.fromWkt('CurvePolygon ((0 0,0 1,1 1,0 0))')
        geom.addPart(g2.get().clone())
        wkt = geom.asWkt()
        expwkt = 'MultiSurface (Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a multisurface to a multisurface
        geom = QgsGeometry.fromWkt('MultiSurface(((20 0,20 1,21 1,20 0)))')
        g2 = QgsGeometry.fromWkt('MultiSurface (Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))')
        geom.addPart(g2.get().clone())
        wkt = geom.asWkt()
        expwkt = 'MultiSurface (Polygon ((20 0, 20 1, 21 1, 20 0)),Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # Test adding parts to empty geometry, should become first part
        empty = QgsGeometry()
        # if not default type specified, addPart should fail
        result = empty.addPointsXY([QgsPointXY(4, 0)])
        assert result != QgsGeometry.Success, 'Got return code {}'.format(result)
        result = empty.addPointsXY([QgsPointXY(4, 0)], QgsWkbTypes.PointGeometry)
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiPoint ((4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPointsXY([QgsPointXY(5, 1)])
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiPoint ((4 0),(5 1))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        # next try with lines
        empty = QgsGeometry()
        result = empty.addPointsXY(points[0][0], QgsWkbTypes.LineGeometry)
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPointsXY(points[1][0])
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0),(4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        # finally try with polygons
        empty = QgsGeometry()
        result = empty.addPointsXY(points[0][0], QgsWkbTypes.PolygonGeometry)
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPointsXY(points[1][0])
        self.assertEqual(result, QgsGeometry.Success, 'Got return code {}'.format(result))
        wkt = empty.asWkt()
        expwkt = 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testConvertToType(self):
        # 5-+-4 0-+-9  13-+-+-12
        # |   | |   |  |       |
        # | 2-3 1-2 |  + 18-17 +
        # | |     | |  | |   | |
        # 0-1     7-8  + 15-16 +
        #              |       |
        #              10-+-+-11
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2), QgsPointXY(0, 0)], ],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1), QgsPointXY(4, 1), QgsPointXY(4, 0)], ],
            [[QgsPointXY(10, 0), QgsPointXY(13, 0), QgsPointXY(13, 3), QgsPointXY(10, 3), QgsPointXY(10, 0)], [QgsPointXY(11, 1), QgsPointXY(12, 1), QgsPointXY(12, 2), QgsPointXY(11, 2), QgsPointXY(11, 1)]]
        ]
        # ####### TO POINT ########
        # POINT TO POINT
        point = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, False).asWkt()
        expWkt = "Point (1 1)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to point. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # POINT TO MultiPoint
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((1 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MultiPoint
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPoint
        multiLine = QgsGeometry.fromMultiPolylineXY(points[2])
        wkt = multiLine.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPoint
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from poylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MultiPoint
        multiPolygon = QgsGeometry.fromMultiPolygonXY(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0),(4 0),(5 0),(5 2),(3 2),(3 1),(4 1),(4 0),(10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # ####### TO LINE ########
        # POINT TO LINE
        point = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        self.assertFalse(point.convertToType(QgsWkbTypes.LineGeometry, False)), "convertToType with a point should return a null geometry"
        # MultiPoint TO LINE
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MULTILINE
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO LINE
        multiLine = QgsGeometry.fromMultiPolylineXY(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MULTILINE
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO LINE
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MULTILINE
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon with ring TO MULTILINE
        polygon = QgsGeometry.fromPolygonXY(points[2])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon with ring to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO LINE
        multiPolygon = QgsGeometry.fromMultiPolygonXY([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MULTILINE
        multiPolygon = QgsGeometry.fromMultiPolygonXY(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0), (4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0), (10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # ####### TO Polygon ########
        # MultiPoint TO Polygon
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MultiPolygon
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO Polygon
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE ( 3 vertices, with first = last ) TO Polygon
        line = QgsGeometry.fromPolylineXY([QgsPointXY(1, 1), QgsPointXY(0, 0), QgsPointXY(1, 1)])
        self.assertFalse(line.convertToType(QgsWkbTypes.PolygonGeometry, False), "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # MULTILINE ( with a part of 3 vertices, with first = last ) TO MultiPolygon
        multiline = QgsGeometry.fromMultiPolylineXY([points[0][0], [QgsPointXY(1, 1), QgsPointXY(0, 0), QgsPointXY(1, 1)]])
        self.assertFalse(multiline.convertToType(QgsWkbTypes.PolygonGeometry, True), "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # LINE TO MultiPolygon
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO Polygon
        multiLine = QgsGeometry.fromMultiPolylineXY(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPolygon
        multiLine = QgsGeometry.fromMultiPolylineXY([points[0][0], points[1][0]])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPolygon
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO Polygon
        multiPolygon = QgsGeometry.fromMultiPolygonXY([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13053(self):
        """ See https://issues.qgis.org/issues/13053 """
        p = QgsGeometry.fromWkt('MULTIPOLYGON(((62.0 18.0, 62.0 19.0, 63.0 19.0, 63.0 18.0, 62.0 18.0)), ((63.0 19.0, 63.0 20.0, 64.0 20.0, 64.0 19.0, 63.0 19.0)))')
        assert p is not None

        expWkt = 'MultiPolygon (((62 18, 62 19, 63 19, 63 18, 62 18)),((63 19, 63 20, 64 20, 64 19, 63 19)))'
        wkt = p.asWkt()
        assert compareWkt(expWkt, wkt), "testRegression13053 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13055(self):
        """ See https://issues.qgis.org/issues/13055
            Testing that invalid WKT with z values but not using PolygonZ is still parsed
            by QGIS.
        """
        p = QgsGeometry.fromWkt('Polygon((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))')
        assert p is not None

        expWkt = 'PolygonZ ((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))'
        wkt = p.asWkt()
        assert compareWkt(expWkt, wkt), "testRegression13055 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13274(self):
        """ See https://issues.qgis.org/issues/13274
            Testing that two combined linestrings produce another line string if possible
        """
        a = QgsGeometry.fromWkt('LineString (0 0, 1 0)')
        b = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        c = a.combine(b)

        expWkt = 'LineString (0 0, 1 0, 2 0)'
        wkt = c.asWkt()
        assert compareWkt(expWkt, wkt), "testRegression13274 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testReshape(self):
        """ Test geometry reshaping """
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        g.reshapeGeometry(QgsLineString([QgsPoint(0, 1.5), QgsPoint(1.5, 0)]))
        expWkt = 'Polygon ((0.5 1, 0 1, 0 0, 1 0, 1 0.5, 0.5 1))'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a geometry involving the first/last vertex (https://issues.qgis.org/issues/14443)
        g.reshapeGeometry(QgsLineString([QgsPoint(0.5, 1), QgsPoint(0, 0.5)]))

        expWkt = 'Polygon ((0 0.5, 0 0, 1 0, 1 0.5, 0.5 1, 0 0.5))'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a polygon with a line starting or ending at the polygon's first vertex, no change expexted
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        expWkt = g.asWkt()
        g.reshapeGeometry(QgsLineString([QgsPoint(0, 0), QgsPoint(-1, -1)]))
        self.assertTrue(compareWkt(g.asWkt(), expWkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        # Test reshape a polygon with a line starting or ending at the polygon's first vertex
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(0, 0), QgsPoint(0.5, 0.5), QgsPoint(0, 1)])), QgsGeometry.Success)
        expWkt = 'Polygon ((0 0, 1 0, 1 1, 0 1, 0.5 0.5, 0 0))'
        self.assertTrue(compareWkt(g.asWkt(), expWkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        # Test reshape a line from first/last vertex
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1)')
        # extend start
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(0, 0), QgsPoint(-1, 0)])), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1)'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # extend end
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(5, 1), QgsPoint(10, 1), QgsPoint(10, 2)])), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1, 10 1, 10 2)'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test with reversed lines
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1)')
        # extend start
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(-1, 0), QgsPoint(0, 0)])), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1)'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # extend end
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(10, 2), QgsPoint(10, 1), QgsPoint(5, 1)])), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1, 10 1, 10 2)'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testConvertToMultiType(self):
        """ Test converting geometries to multi type """
        point = QgsGeometry.fromWkt('Point (1 2)')
        assert point.convertToMultiType()
        expWkt = 'MultiPoint ((1 2))'
        wkt = point.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiPoint
        assert point.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        assert line.convertToMultiType()
        expWkt = 'MultiLineString ((1 0, 2 0))'
        wkt = line.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiLineString
        assert line.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))')
        assert poly.convertToMultiType()
        expWkt = 'MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)))'
        wkt = poly.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiPolygon
        assert poly.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testConvertToSingleType(self):
        """ Test converting geometries to single type """
        point = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert point.convertToSingleType()
        expWkt = 'Point (1 2)'
        wkt = point.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of Point
        assert point.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('MultiLineString ((1 0, 2 0),(2 3, 4 5))')
        assert line.convertToSingleType()
        expWkt = 'LineString (1 0, 2 0)'
        wkt = line.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of LineString
        assert line.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)),((2 3,2 4, 3 4, 3 3, 2 3)))')
        assert poly.convertToSingleType()
        expWkt = 'Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))'
        wkt = poly.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of Polygon
        assert poly.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddZValue(self):
        """ Test adding z dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.constGet().addZValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CircularStringZ)
        expWkt = 'CircularStringZ (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.constGet().addZValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CompoundCurveZ)
        expWkt = 'CompoundCurveZ ((5 3 2, 5 13 2),CircularStringZ (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringZ (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.constGet().addZValue(3)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PolygonZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PolygonZ)
        expWkt = 'PolygonZ ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.constGet().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.MultiPointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPointZ)
        expWkt = 'MultiPointZ ((1 2 4),(2 3 4))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.constGet().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.LineStringZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.LineStringZ)
        expWkt = 'LineStringZ (1 2 4, 2 3 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.constGet().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PointZ)
        expWkt = 'PointZ (1 2 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddMValue(self):
        """ Test adding m dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.constGet().addMValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CircularStringM)
        expWkt = 'CircularStringM (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.constGet().addMValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CompoundCurveM)
        expWkt = 'CompoundCurveM ((5 3 2, 5 13 2),CircularStringM (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringM (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.constGet().addMValue(3)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PolygonM)
        expWkt = 'PolygonM ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.constGet().addMValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.MultiPointM)
        expWkt = 'MultiPointM ((1 2 4),(2 3 4))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.constGet().addMValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.LineStringM)
        expWkt = 'LineStringM (1 2 4, 2 3 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.constGet().addMValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PointM)
        expWkt = 'PointM (1 2 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testDistanceToVertex(self):
        """ Test distanceToVertex calculation """
        g = QgsGeometry()
        self.assertEqual(g.distanceToVertex(0), -1)

        g = QgsGeometry.fromWkt('LineString ()')
        self.assertEqual(g.distanceToVertex(0), -1)

        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        self.assertEqual(g.distanceToVertex(0), 0)
        self.assertEqual(g.distanceToVertex(1), 1)
        self.assertEqual(g.distanceToVertex(2), 2)
        self.assertEqual(g.distanceToVertex(3), 3)
        self.assertEqual(g.distanceToVertex(4), 4)
        self.assertEqual(g.distanceToVertex(5), -1)

    def testTypeInformation(self):
        """ Test type information """
        types = [
            (QgsCircularString, "CircularString", QgsWkbTypes.CircularString),
            (QgsCompoundCurve, "CompoundCurve", QgsWkbTypes.CompoundCurve),
            (QgsCurvePolygon, "CurvePolygon", QgsWkbTypes.CurvePolygon),
            (QgsGeometryCollection, "GeometryCollection", QgsWkbTypes.GeometryCollection),
            (QgsLineString, "LineString", QgsWkbTypes.LineString),
            (QgsMultiCurve, "MultiCurve", QgsWkbTypes.MultiCurve),
            (QgsMultiLineString, "MultiLineString", QgsWkbTypes.MultiLineString),
            (QgsMultiPoint, "MultiPoint", QgsWkbTypes.MultiPoint),
            (QgsMultiPolygon, "MultiPolygon", QgsWkbTypes.MultiPolygon),
            (QgsMultiSurface, "MultiSurface", QgsWkbTypes.MultiSurface),
            (QgsPoint, "Point", QgsWkbTypes.Point),
            (QgsPolygon, "Polygon", QgsWkbTypes.Polygon),
        ]

        for geomtype in types:
            geom = geomtype[0]()
            self.assertEqual(geom.geometryType(), geomtype[1])
            self.assertEqual(geom.wkbType(), geomtype[2])
            geom.clear()
            self.assertEqual(geom.geometryType(), geomtype[1])
            self.assertEqual(geom.wkbType(), geomtype[2])
            clone = geom.clone()
            self.assertEqual(clone.geometryType(), geomtype[1])
            self.assertEqual(clone.wkbType(), geomtype[2])

    def testRelates(self):
        """ Test relationships between geometries. Note the bulk of these tests were taken from the PostGIS relate testdata """
        with open(os.path.join(TEST_DATA_DIR, 'relates_data.csv'), 'r') as d:
            for i, t in enumerate(d):
                test_data = t.strip().split('|')
                geom1 = QgsGeometry.fromWkt(test_data[0])
                assert geom1, "Relates {} failed: could not create geom:\n{}\n".format(i + 1, test_data[0])
                geom2 = QgsGeometry.fromWkt(test_data[1])
                assert geom2, "Relates {} failed: could not create geom:\n{}\n".format(i + 1, test_data[1])
                result = QgsGeometry.createGeometryEngine(geom1.constGet()).relate(geom2.constGet())
                exp = test_data[2]
                self.assertEqual(result, exp, "Relates {} failed: mismatch Expected:\n{}\nGot:\n{}\nGeom1:\n{}\nGeom2:\n{}\n".format(i + 1, exp, result, test_data[0], test_data[1]))

    def testWkbTypes(self):
        """ Test QgsWkbTypes methods """

        # test singleType method
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPoint), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.LineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.LineStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiLineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CircularString), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularStringZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiSurface), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.singleType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.Polygon25D)

        # test multiType method
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Point), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.LineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.LineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.LineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.LineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Polygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.PolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CircularString), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CircularStringM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Point25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.LineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Polygon25D), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

        # test flatType method
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PointZ), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PointM), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PointZM), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.LineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.LineStringM), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PolygonZ), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PolygonM), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.PolygonZM), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CircularString), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.Point25D), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.flatType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon)

        # test geometryType method
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.Unknown), QgsWkbTypes.UnknownGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.Point), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PointZ), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PointM), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PointZM), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPoint), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPointM), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.LineString), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.LineStringM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiLineString), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.Polygon), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.UnknownGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.UnknownGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.UnknownGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.UnknownGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CircularString), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CircularStringM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiCurve), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiSurface), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NullGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.Point25D), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.LineString25D), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.Polygon25D), QgsWkbTypes.PolygonGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.PointGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.LineGeometry)
        self.assertEqual(QgsWkbTypes.geometryType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.PolygonGeometry)

        # test displayString method
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.Unknown), 'Unknown')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.Point), 'Point')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PointZ), 'PointZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PointM), 'PointM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PointZM), 'PointZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPoint), 'MultiPoint')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPointZ), 'MultiPointZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPointM), 'MultiPointM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPointZM), 'MultiPointZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.LineString), 'LineString')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.LineStringZ), 'LineStringZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.LineStringM), 'LineStringM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.LineStringZM), 'LineStringZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiLineString), 'MultiLineString')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiLineStringZ), 'MultiLineStringZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiLineStringM), 'MultiLineStringM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiLineStringZM), 'MultiLineStringZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.Polygon), 'Polygon')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PolygonZ), 'PolygonZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PolygonM), 'PolygonM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.PolygonZM), 'PolygonZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPolygon), 'MultiPolygon')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPolygonZ), 'MultiPolygonZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPolygonM), 'MultiPolygonM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPolygonZM), 'MultiPolygonZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.GeometryCollection), 'GeometryCollection')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.GeometryCollectionZ), 'GeometryCollectionZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.GeometryCollectionM), 'GeometryCollectionM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.GeometryCollectionZM), 'GeometryCollectionZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CircularString), 'CircularString')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CircularStringZ), 'CircularStringZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CircularStringM), 'CircularStringM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CircularStringZM), 'CircularStringZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CompoundCurve), 'CompoundCurve')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CompoundCurveZ), 'CompoundCurveZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CompoundCurveM), 'CompoundCurveM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CompoundCurveZM), 'CompoundCurveZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CurvePolygon), 'CurvePolygon')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CurvePolygonZ), 'CurvePolygonZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CurvePolygonM), 'CurvePolygonM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.CurvePolygonZM), 'CurvePolygonZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiCurve), 'MultiCurve')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiCurveZ), 'MultiCurveZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiCurveM), 'MultiCurveM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiCurveZM), 'MultiCurveZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiSurface), 'MultiSurface')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiSurfaceZ), 'MultiSurfaceZ')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiSurfaceM), 'MultiSurfaceM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiSurfaceZM), 'MultiSurfaceZM')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.NoGeometry), 'NoGeometry')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.Point25D), 'Point25D')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.LineString25D), 'LineString25D')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.Polygon25D), 'Polygon25D')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPoint25D), 'MultiPoint25D')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiLineString25D), 'MultiLineString25D')
        self.assertEqual(QgsWkbTypes.displayString(QgsWkbTypes.MultiPolygon25D), 'MultiPolygon25D')
        self.assertEqual(QgsWkbTypes.displayString(9999999), '')

        # test parseType method
        self.assertEqual(QgsWkbTypes.parseType('point( 1 2 )'), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.parseType('POINT( 1 2 )'), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.parseType('   point    ( 1 2 )   '), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.parseType('point'), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.parseType('LINE STRING( 1 2, 3 4 )'), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.parseType('POINTZ( 1 2 )'), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.parseType('POINT z m'), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.parseType('bad'), QgsWkbTypes.Unknown)

        # test wkbDimensions method
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.Unknown), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.Point), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PointZ), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PointM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PointZM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPoint), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPointZ), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPointM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPointZM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.LineString), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.LineStringZ), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.LineStringM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.LineStringZM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiLineString), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiLineStringZ), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiLineStringM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiLineStringZM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.Polygon), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PolygonZ), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PolygonM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.PolygonZM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPolygon), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPolygonZ), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPolygonM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPolygonZM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.GeometryCollection), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.GeometryCollectionZ), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.GeometryCollectionM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.GeometryCollectionZM), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CircularString), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CircularStringZ), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CircularStringM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CircularStringZM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CompoundCurve), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CompoundCurveZ), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CompoundCurveM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CompoundCurveZM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CurvePolygon), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CurvePolygonZ), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CurvePolygonM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.CurvePolygonZM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiCurve), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiCurveZ), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiCurveM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiCurveZM), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiSurface), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiSurfaceZ), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiSurfaceM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiSurfaceZM), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.NoGeometry), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.Point25D), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.LineString25D), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.Polygon25D), 2)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPoint25D), 0)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiLineString25D), 1)
        self.assertEqual(QgsWkbTypes.wkbDimensions(QgsWkbTypes.MultiPolygon25D), 2)

        # test coordDimensions method
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.Unknown), 0)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.Point), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PointZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PointM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PointZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPoint), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPointZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPointM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPointZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.LineString), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.LineStringZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.LineStringM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.LineStringZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiLineString), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiLineStringZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiLineStringM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiLineStringZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.Polygon), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PolygonZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PolygonM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.PolygonZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPolygon), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPolygonZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPolygonM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPolygonZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.GeometryCollection), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.GeometryCollectionZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.GeometryCollectionM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.GeometryCollectionZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CircularString), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CircularStringZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CircularStringM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CircularStringZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CompoundCurve), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CompoundCurveZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CompoundCurveM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CompoundCurveZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CurvePolygon), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CurvePolygonZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CurvePolygonM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.CurvePolygonZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiCurve), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiCurveZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiCurveM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiCurveZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiSurface), 2)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiSurfaceZ), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiSurfaceM), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiSurfaceZM), 4)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.NoGeometry), 0)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.Point25D), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.LineString25D), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.Polygon25D), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPoint25D), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiLineString25D), 3)
        self.assertEqual(QgsWkbTypes.coordDimensions(QgsWkbTypes.MultiPolygon25D), 3)

        # test isSingleType methods
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.Unknown)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.Point)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.LineString)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.Polygon)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPoint)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiLineString)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPolygon)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.GeometryCollection)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CircularString)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CompoundCurve)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CurvePolygon)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiCurve)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiSurface)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.NoGeometry)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PointZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.LineStringZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PolygonZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPointZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiLineStringZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPolygonZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.GeometryCollectionZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CircularStringZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CompoundCurveZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CurvePolygonZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiCurveZ)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiSurfaceZ)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PointM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.LineStringM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PolygonM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPointM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiLineStringM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPolygonM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.GeometryCollectionM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CircularStringM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CompoundCurveM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CurvePolygonM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiCurveM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiSurfaceM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PointZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.LineStringZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.PolygonZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPointZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiLineStringZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPolygonZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.GeometryCollectionZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CircularStringZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CompoundCurveZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.CurvePolygonZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiCurveZM)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiSurfaceZM)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.Point25D)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.LineString25D)
        assert QgsWkbTypes.isSingleType(QgsWkbTypes.Polygon25D)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPoint25D)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiLineString25D)
        assert not QgsWkbTypes.isSingleType(QgsWkbTypes.MultiPolygon25D)

        # test isMultiType methods
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.Unknown)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.Point)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.LineString)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.Polygon)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPoint)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiLineString)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPolygon)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.GeometryCollection)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CircularString)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CompoundCurve)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CurvePolygon)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiCurve)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiSurface)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.NoGeometry)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PointZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.LineStringZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PolygonZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPointZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiLineStringZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPolygonZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.GeometryCollectionZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CircularStringZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CompoundCurveZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CurvePolygonZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiCurveZ)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiSurfaceZ)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PointM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.LineStringM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PolygonM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPointM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiLineStringM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPolygonM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.GeometryCollectionM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CircularStringM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CompoundCurveM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CurvePolygonM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiCurveM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiSurfaceM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PointZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.LineStringZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.PolygonZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPointZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiLineStringZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPolygonZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.GeometryCollectionZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CircularStringZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CompoundCurveZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.CurvePolygonZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiCurveZM)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiSurfaceZM)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.Point25D)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.LineString25D)
        assert not QgsWkbTypes.isMultiType(QgsWkbTypes.Polygon25D)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPoint25D)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiLineString25D)
        assert QgsWkbTypes.isMultiType(QgsWkbTypes.MultiPolygon25D)

        # test isCurvedType methods
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.Unknown)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.Point)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.LineString)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.Polygon)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPoint)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiLineString)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPolygon)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.GeometryCollection)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CircularString)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CompoundCurve)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CurvePolygon)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiCurve)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiSurface)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.NoGeometry)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PointZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.LineStringZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PolygonZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPointZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiLineStringZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPolygonZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.GeometryCollectionZ)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CircularStringZ)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CompoundCurveZ)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CurvePolygonZ)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiCurveZ)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiSurfaceZ)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PointM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.LineStringM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PolygonM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPointM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiLineStringM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPolygonM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.GeometryCollectionM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CircularStringM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CompoundCurveM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CurvePolygonM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiCurveM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiSurfaceM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PointZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.LineStringZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.PolygonZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPointZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiLineStringZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPolygonZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.GeometryCollectionZM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CircularStringZM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CompoundCurveZM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.CurvePolygonZM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiCurveZM)
        assert QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiSurfaceZM)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.Point25D)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.LineString25D)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.Polygon25D)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPoint25D)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiLineString25D)
        assert not QgsWkbTypes.isCurvedType(QgsWkbTypes.MultiPolygon25D)

        # test hasZ methods
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.Unknown)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.Point)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.LineString)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.Polygon)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiPoint)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiLineString)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiPolygon)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.GeometryCollection)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CircularString)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CompoundCurve)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CurvePolygon)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiCurve)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiSurface)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.NoGeometry)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.PointZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.LineStringZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.PolygonZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPointZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiLineStringZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPolygonZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.GeometryCollectionZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CircularStringZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CompoundCurveZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CurvePolygonZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiCurveZ)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiSurfaceZ)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.PointM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.LineStringM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.PolygonM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiPointM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiLineStringM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiPolygonM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.GeometryCollectionM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CircularStringM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CompoundCurveM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.CurvePolygonM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiCurveM)
        assert not QgsWkbTypes.hasZ(QgsWkbTypes.MultiSurfaceM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.PointZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.LineStringZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.PolygonZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPointZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiLineStringZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPolygonZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.GeometryCollectionZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CircularStringZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CompoundCurveZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.CurvePolygonZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiCurveZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiSurfaceZM)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.Point25D)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.LineString25D)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.Polygon25D)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPoint25D)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiLineString25D)
        assert QgsWkbTypes.hasZ(QgsWkbTypes.MultiPolygon25D)

        # test hasM methods
        assert not QgsWkbTypes.hasM(QgsWkbTypes.Unknown)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.Point)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.LineString)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.Polygon)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPoint)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiLineString)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPolygon)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.GeometryCollection)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CircularString)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CompoundCurve)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CurvePolygon)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiCurve)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiSurface)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.NoGeometry)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.PointZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.LineStringZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.PolygonZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPointZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiLineStringZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPolygonZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.GeometryCollectionZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CircularStringZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CompoundCurveZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.CurvePolygonZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiCurveZ)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiSurfaceZ)
        assert QgsWkbTypes.hasM(QgsWkbTypes.PointM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.LineStringM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.PolygonM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiPointM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiLineStringM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiPolygonM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.GeometryCollectionM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CircularStringM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CompoundCurveM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CurvePolygonM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiCurveM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiSurfaceM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.PointZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.LineStringZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.PolygonZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiPointZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiLineStringZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiPolygonZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.GeometryCollectionZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CircularStringZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CompoundCurveZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.CurvePolygonZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiCurveZM)
        assert QgsWkbTypes.hasM(QgsWkbTypes.MultiSurfaceZM)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.Point25D)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.LineString25D)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.Polygon25D)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPoint25D)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiLineString25D)
        assert not QgsWkbTypes.hasM(QgsWkbTypes.MultiPolygon25D)

        # test adding z dimension to types
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.Point), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PointM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.LineString), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.LineStringM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.Polygon), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CircularString), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularStringZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.addZ(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

        # test to25D
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.Point), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PointZ), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PointM), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PointZM), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.LineString), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.LineStringM), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PolygonZ), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PolygonM), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.PolygonZM), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.GeometryCollection), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CircularString), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CircularStringZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CircularStringM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CircularStringZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CompoundCurve), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CurvePolygon), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiCurve), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiCurveM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiSurface), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.to25D(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

        # test adding m dimension to types
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Point), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PointZ), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineString), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Polygon), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CircularString), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        # can't be added to these types
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

        # test dropping z dimension from types
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PointZ), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PointZM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.LineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.LineStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PolygonZ), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CircularString), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.Point25D), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.dropZ(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon)

        # test dropping m dimension from types
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PointM), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PointZM), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.LineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.LineStringM), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PolygonM), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CircularString), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CircularStringM), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.dropM(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

        # Test QgsWkbTypes.zmType
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Point, False, False), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Point, True, False), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Point, False, True), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Point, True, True), QgsWkbTypes.PointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZ, False, False), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZ, True, False), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZ, False, True), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZ, True, True), QgsWkbTypes.PointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointM, False, False), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointM, True, False), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointM, False, True), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointM, True, True), QgsWkbTypes.PointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZM, False, False), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZM, True, False), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZM, False, True), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PointZM, True, True), QgsWkbTypes.PointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineString, False, False), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineString, True, False), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineString, False, True), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineString, True, True), QgsWkbTypes.LineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZ, False, False), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZ, True, False), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZ, False, True), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZ, True, True), QgsWkbTypes.LineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringM, False, False), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringM, True, False), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringM, False, True), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringM, True, True), QgsWkbTypes.LineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZM, False, False), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZM, True, False), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZM, False, True), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.LineStringZM, True, True), QgsWkbTypes.LineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Polygon, False, False), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Polygon, True, False), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Polygon, False, True), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.Polygon, True, True), QgsWkbTypes.PolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZ, False, False), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZ, True, False), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZ, False, True), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZ, True, True), QgsWkbTypes.PolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonM, False, False), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonM, True, False), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonM, False, True), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonM, True, True), QgsWkbTypes.PolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZM, False, False), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZM, True, False), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZM, False, True), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.PolygonZM, True, True), QgsWkbTypes.PolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPoint, False, False), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPoint, True, False), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPoint, False, True), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPoint, True, True), QgsWkbTypes.MultiPointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZ, False, False), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZ, True, False), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZ, False, True), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZ, True, True), QgsWkbTypes.MultiPointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointM, False, False), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointM, True, False), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointM, False, True), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointM, True, True), QgsWkbTypes.MultiPointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZM, False, False), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZM, True, False), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZM, False, True), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPointZM, True, True), QgsWkbTypes.MultiPointZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineString, False, False), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineString, True, False), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineString, False, True), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineString, True, True), QgsWkbTypes.MultiLineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZ, False, False), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZ, True, False), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZ, False, True), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZ, True, True), QgsWkbTypes.MultiLineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringM, False, False), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringM, True, False), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringM, False, True), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringM, True, True), QgsWkbTypes.MultiLineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZM, False, False), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZM, True, False), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZM, False, True), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiLineStringZM, True, True), QgsWkbTypes.MultiLineStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygon, False, False), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygon, True, False), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygon, False, True), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygon, True, True), QgsWkbTypes.MultiPolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZ, False, False), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZ, True, False), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZ, False, True), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZ, True, True), QgsWkbTypes.MultiPolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonM, False, False), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonM, True, False), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonM, False, True), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonM, True, True), QgsWkbTypes.MultiPolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZM, False, False), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZM, True, False), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZM, False, True), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiPolygonZM, True, True), QgsWkbTypes.MultiPolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, False, False), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, True, False), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, False, True), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, True, True), QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, False, False), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, True, False), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, False, True), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, True, True), QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, False, False), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, True, False), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, False, True), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, True, True), QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, False, False), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, True, False), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, False, True), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, True, True), QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularString, False, False), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularString, True, False), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularString, False, True), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularString, True, True), QgsWkbTypes.CircularStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZ, False, False), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZ, True, False), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZ, False, True), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZ, True, True), QgsWkbTypes.CircularStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringM, False, False), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringM, True, False), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringM, False, True), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringM, True, True), QgsWkbTypes.CircularStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZM, False, False), QgsWkbTypes.CircularString)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZM, True, False), QgsWkbTypes.CircularStringZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZM, False, True), QgsWkbTypes.CircularStringM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CircularStringZM, True, True), QgsWkbTypes.CircularStringZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurve, False, False), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurve, True, False), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurve, False, True), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurve, True, True), QgsWkbTypes.CompoundCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZ, False, False), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZ, True, False), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZ, False, True), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZ, True, True), QgsWkbTypes.CompoundCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveM, False, False), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveM, True, False), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveM, False, True), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveM, True, True), QgsWkbTypes.CompoundCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZM, False, False), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZM, True, False), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZM, False, True), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CompoundCurveZM, True, True), QgsWkbTypes.CompoundCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurve, False, False), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurve, True, False), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurve, False, True), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurve, True, True), QgsWkbTypes.MultiCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZ, False, False), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZ, True, False), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZ, False, True), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZ, True, True), QgsWkbTypes.MultiCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveM, False, False), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveM, True, False), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveM, False, True), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveM, True, True), QgsWkbTypes.MultiCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZM, False, False), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZM, True, False), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZM, False, True), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiCurveZM, True, True), QgsWkbTypes.MultiCurveZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygon, False, False), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygon, True, False), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygon, False, True), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygon, True, True), QgsWkbTypes.CurvePolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZ, False, False), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZ, True, False), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZ, False, True), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZ, True, True), QgsWkbTypes.CurvePolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonM, False, False), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonM, True, False), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonM, False, True), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonM, True, True), QgsWkbTypes.CurvePolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZM, False, False), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZM, True, False), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZM, False, True), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.CurvePolygonZM, True, True), QgsWkbTypes.CurvePolygonZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurface, False, False), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurface, True, False), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurface, False, True), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurface, True, True), QgsWkbTypes.MultiSurfaceZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZ, False, False), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZ, True, False), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZ, False, True), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZ, True, True), QgsWkbTypes.MultiSurfaceZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceM, False, False), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceM, True, False), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceM, False, True), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceM, True, True), QgsWkbTypes.MultiSurfaceZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZM, False, False), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZM, True, False), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZM, False, True), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.MultiSurfaceZM, True, True), QgsWkbTypes.MultiSurfaceZM)

    def testGeometryDisplayString(self):
        self.assertEqual(QgsWkbTypes.geometryDisplayString(QgsWkbTypes.PointGeometry), 'Point')
        self.assertEqual(QgsWkbTypes.geometryDisplayString(QgsWkbTypes.LineGeometry), 'Line')
        self.assertEqual(QgsWkbTypes.geometryDisplayString(QgsWkbTypes.PolygonGeometry), 'Polygon')
        self.assertEqual(QgsWkbTypes.geometryDisplayString(QgsWkbTypes.UnknownGeometry), 'Unknown geometry')
        self.assertEqual(QgsWkbTypes.geometryDisplayString(QgsWkbTypes.NullGeometry), 'No geometry')
        self.assertEqual(QgsWkbTypes.geometryDisplayString(999999), 'Invalid type')

    def testDeleteVertexCircularString(self):

        wkt = "CircularString ((0 0,1 1,2 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.asWkt(), QgsCircularString().asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CircularString (2 0, 3 -1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CircularString (0 0, 3 -1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CircularString (0 0,1 1,2 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert not geom.deleteVertex(-1)
        assert not geom.deleteVertex(5)

    def testDeleteVertexCompoundCurve(self):

        wkt = "CompoundCurve ((0 0,1 1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert not geom.deleteVertex(-1)
        assert not geom.deleteVertex(2)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.asWkt(), QgsCompoundCurve().asWkt())

        wkt = "CompoundCurve ((0 0,1 1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        self.assertEqual(geom.asWkt(), QgsCompoundCurve().asWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CompoundCurve ((1 1,2 2))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((0 0,2 2))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CompoundCurve ((0 0,1 1))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve ((0 0,1 1),CircularString(1 1,2 0,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((0 0,1 -1))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve (CircularString(0 0,1 1,2 0),CircularString(2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CompoundCurve ((0 0, 4 0))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CompoundCurve (CircularString (0 0, 1 1, 2 0),(2 0, 0 0))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve ((-1 0,0 0),CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((-1 0, 2 0),CircularString (2 0, 1.5 -0.5, 1 -1))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CompoundCurve (CircularString(-1 -1,-1.5 -0.5,-2 0,-1 1,0 0),CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CompoundCurve (CircularString (-1 -1, -1.5 -0.5, -2 0),(-2 0, 2 0),CircularString (2 0, 1.5 -0.5, 1 -1))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

    def testDeleteVertexCurvePolygon(self):

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert not geom.deleteVertex(-1)
        assert not geom.deleteVertex(4)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.asWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        self.assertEqual(geom.asWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        self.assertEqual(geom.asWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        self.assertEqual(geom.asWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (2 0, 1.5 -0.5, 1 -1),(1 -1, 2 0)))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1.5 -0.5, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 2 0),(2 0, 0 0)))"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

    def testSingleSidedBuffer(self):

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideLeft)
        result = out.asWkt()
        expected_wkt = "Polygon ((10 0, 0 0, 0 1, 10 1, 10 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleMiter)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleBevel)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

    def testMisc(self):

        # Test that we cannot add a CurvePolygon in a MultiPolygon
        multipolygon = QgsMultiPolygon()
        cp = QgsCurvePolygon()
        cp.fromWkt("CurvePolygon ((0 0,0 1,1 1,0 0))")
        assert not multipolygon.addGeometry(cp)

        # Test that importing an invalid WKB (a MultiPolygon with a CurvePolygon) fails
        geom = QgsGeometry.fromWkt('MultiSurface(((0 0,0 1,1 1,0 0)), CurvePolygon ((0 0,0 1,1 1,0 0)))')
        wkb = geom.asWkb()
        wkb = bytearray(wkb)
        if wkb[1] == QgsWkbTypes.MultiSurface:
            wkb[1] = QgsWkbTypes.MultiPolygon
        elif wkb[1 + 4] == QgsWkbTypes.MultiSurface:
            wkb[1 + 4] = QgsWkbTypes.MultiPolygon
        else:
            self.assertTrue(False)
        geom = QgsGeometry()
        geom.fromWkb(wkb)
        self.assertEqual(geom.asWkt(), QgsMultiPolygon().asWkt())

        # Test that fromWkt() on a GeometryCollection works with all possible geometries
        wkt = "GeometryCollection( "
        wkt += "Point(0 1)"
        wkt += ","
        wkt += "LineString(0 0,0 1)"
        wkt += ","
        wkt += "Polygon ((0 0,1 1,1 0,0 0))"
        wkt += ","
        wkt += "CurvePolygon ((0 0,1 1,1 0,0 0))"
        wkt += ","
        wkt += "CircularString (0 0,1 1,2 0)"
        wkt += ","
        wkt += "CompoundCurve ((0 0,0 1))"
        wkt += ","
        wkt += "MultiPoint ((0 0))"
        wkt += ","
        wkt += "MultiLineString((0 0,0 1))"
        wkt += ","
        wkt += "MultiCurve((0 0,0 1))"
        wkt += ","
        wkt += "MultiPolygon (((0 0,1 1,1 0,0 0)))"
        wkt += ","
        wkt += "MultiSurface (((0 0,1 1,1 0,0 0)))"
        wkt += ","
        wkt += "GeometryCollection (Point(0 0))"
        wkt += ")"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom is not None
        wkb1 = geom.asWkb()
        geom = QgsGeometry()
        geom.fromWkb(wkb1)
        wkb2 = geom.asWkb()
        self.assertEqual(wkb1, wkb2)

    def testMergeLines(self):
        """ test merging linestrings """

        # not a (multi)linestring
        geom = QgsGeometry.fromWkt('Point(1 2)')
        result = geom.mergeLines()
        self.assertTrue(result.isNull())

        # linestring should be returned intact
        geom = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        result = geom.mergeLines().asWkt()
        exp = 'LineString(0 0, 10 10)'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multilinestring
        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(10 10, 20 20))')
        result = geom.mergeLines().asWkt()
        exp = 'LineString(0 0, 10 10, 20 20)'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4),(10 10, 20 20))')
        result = geom.mergeLines().asWkt()
        exp = 'MultiLineString((0 0, 10 10, 20 20),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4))')
        result = geom.mergeLines().asWkt()
        exp = 'MultiLineString((0 0, 10 10),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testCurveSinuosity(self):
        """
        Test curve straightDistance2d() and sinuosity()
        """
        linestring = QgsGeometry.fromWkt('LineString()')
        self.assertEqual(linestring.constGet().straightDistance2d(), 0.0)
        self.assertTrue(math.isnan(linestring.constGet().sinuosity()))
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0)')
        self.assertEqual(linestring.constGet().straightDistance2d(), 10.0)
        self.assertEqual(linestring.constGet().sinuosity(), 1.0)
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 10, 5 0)')
        self.assertAlmostEqual(linestring.constGet().straightDistance2d(), 5.0, 4)
        self.assertAlmostEqual(linestring.constGet().sinuosity(), 5.06449510, 4)
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10, 0 0)')
        self.assertEqual(linestring.constGet().straightDistance2d(), 0.0)
        self.assertTrue(math.isnan(linestring.constGet().sinuosity()))

        curve = QgsGeometry.fromWkt('CircularString (20 30, 50 30, 50 90)')
        self.assertAlmostEqual(curve.constGet().straightDistance2d(), 67.08203932, 4)
        self.assertAlmostEqual(curve.constGet().sinuosity(), 1.57079632, 4)
        curve = QgsGeometry.fromWkt('CircularString (20 30, 50 30, 20 30)')
        self.assertAlmostEqual(curve.constGet().straightDistance2d(), 0.0, 4)
        self.assertTrue(math.isnan(curve.constGet().sinuosity()))

    def testLineLocatePoint(self):
        """ test QgsGeometry.lineLocatePoint() """

        # not a linestring
        point = QgsGeometry.fromWkt('Point(1 2)')
        self.assertEqual(point.lineLocatePoint(point), -1)

        # not a point
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0)')
        self.assertEqual(linestring.lineLocatePoint(linestring), -1)

        # valid
        self.assertEqual(linestring.lineLocatePoint(point), 1)
        point = QgsGeometry.fromWkt('Point(9 -2)')
        self.assertEqual(linestring.lineLocatePoint(point), 9)

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        point = QgsGeometry.fromWkt('Point(9 -2)')
        self.assertAlmostEqual(geom.lineLocatePoint(point), 7.377, places=3)

    def testInterpolateAngle(self):
        """ test QgsGeometry.interpolateAngle() """

        empty = QgsGeometry()
        # just test no crash
        self.assertEqual(empty.interpolateAngle(5), 0)

        # not a linestring
        point = QgsGeometry.fromWkt('Point(1 2)')
        # no meaning, just test no crash!
        self.assertEqual(point.interpolateAngle(5), 0)

        # linestring
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0, 20 10, 20 20, 10 20)')
        self.assertAlmostEqual(linestring.interpolateAngle(0), math.radians(90), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(5), math.radians(90), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(10), math.radians(67.5), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(15), math.radians(45), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(25), math.radians(0), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(35), math.radians(270), places=3)

        # test first and last points in a linestring - angle should be angle of
        # first/last segment
        linestring = QgsGeometry.fromWkt('LineString(20 0, 10 0, 10 -10)')
        self.assertAlmostEqual(linestring.interpolateAngle(0), math.radians(270), places=3)
        self.assertAlmostEqual(linestring.interpolateAngle(20), math.radians(180), places=3)

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 20 10, 20 20, 10 20, 0 0))')
        self.assertAlmostEqual(polygon.interpolateAngle(5), math.radians(90), places=3)
        self.assertAlmostEqual(polygon.interpolateAngle(10), math.radians(67.5), places=3)
        self.assertAlmostEqual(polygon.interpolateAngle(15), math.radians(45), places=3)
        self.assertAlmostEqual(polygon.interpolateAngle(25), math.radians(0), places=3)
        self.assertAlmostEqual(polygon.interpolateAngle(35), math.radians(270), places=3)

        # test first/last vertex in polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0))')
        self.assertAlmostEqual(polygon.interpolateAngle(0), math.radians(135), places=3)
        self.assertAlmostEqual(polygon.interpolateAngle(40), math.radians(135), places=3)

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        self.assertAlmostEqual(geom.interpolateAngle(5), 1.6919, places=3)

    def testInterpolate(self):
        """ test QgsGeometry.interpolate() """

        empty = QgsGeometry()
        # just test no crash
        self.assertFalse(empty.interpolate(5))

        # not a linestring
        point = QgsGeometry.fromWkt('Point(1 2)')  # NOQA
        # no meaning, just test no crash!
        self.assertFalse(empty.interpolate(5))

        # linestring
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10)')
        exp = 'Point(5 0)'
        result = linestring.interpolate(5).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001), "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 20 20, 10 20, 0 0))')  # NOQA
        exp = 'Point(10 5)'
        result = linestring.interpolate(15).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testAngleAtVertex(self):
        """ test QgsGeometry.angleAtVertex """

        empty = QgsGeometry()
        # just test no crash
        self.assertEqual(empty.angleAtVertex(0), 0)

        # not a linestring
        point = QgsGeometry.fromWkt('Point(1 2)')
        # no meaning, just test no crash!
        self.assertEqual(point.angleAtVertex(0), 0)

        # linestring
        linestring = QgsGeometry.fromWkt('LineString(0 0, 10 0, 20 10, 20 20, 10 20)')
        self.assertAlmostEqual(linestring.angleAtVertex(1), math.radians(67.5), places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(2), math.radians(22.5), places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(3), math.radians(315.0), places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(5), 0, places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(-1), 0, places=3)

        # test first and last points in a linestring - angle should be angle of
        # first/last segment
        linestring = QgsGeometry.fromWkt('LineString(20 0, 10 0, 10 -10)')
        self.assertAlmostEqual(linestring.angleAtVertex(0), math.radians(270), places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(2), math.radians(180), places=3)

        # closed linestring - angle at first/last vertex should be average angle
        linestring = QgsGeometry.fromWkt('LineString (-1007697 1334641, -1007697 1334643, -1007695 1334643, -1007695 1334641, -1007696 1334641, -1007697 1334641)')
        self.assertAlmostEqual(linestring.angleAtVertex(0), math.radians(315), places=3)
        self.assertAlmostEqual(linestring.angleAtVertex(5), math.radians(315), places=3)

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0))')
        self.assertAlmostEqual(polygon.angleAtVertex(0), math.radians(135.0), places=3)
        self.assertAlmostEqual(polygon.angleAtVertex(1), math.radians(45.0), places=3)
        self.assertAlmostEqual(polygon.angleAtVertex(2), math.radians(315.0), places=3)
        self.assertAlmostEqual(polygon.angleAtVertex(3), math.radians(225.0), places=3)
        self.assertAlmostEqual(polygon.angleAtVertex(4), math.radians(135.0), places=3)

    def testExtendLine(self):
        """ test QgsGeometry.extendLine """

        empty = QgsGeometry()
        self.assertFalse(empty.extendLine(1, 2))

        # not a linestring
        point = QgsGeometry.fromWkt('Point(1 2)')
        self.assertFalse(point.extendLine(1, 2))

        # linestring
        linestring = QgsGeometry.fromWkt('LineString(0 0, 1 0, 1 1)')
        extended = linestring.extendLine(1, 2)
        exp = 'LineString(-1 0, 1 0, 1 3)'
        result = extended.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Extend line: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multilinestring
        multilinestring = QgsGeometry.fromWkt('MultiLineString((0 0, 1 0, 1 1),(11 11, 11 10, 10 10))')
        extended = multilinestring.extendLine(1, 2)
        exp = 'MultiLineString((-1 0, 1 0, 1 3),(11 12, 11 10, 8 10))'
        result = extended.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Extend line: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testRemoveRings(self):
        empty = QgsGeometry()
        self.assertFalse(empty.removeInteriorRings())

        # not a polygon
        point = QgsGeometry.fromWkt('Point(1 2)')
        self.assertFalse(point.removeInteriorRings())

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1))')
        removed = polygon.removeInteriorRings()
        exp = 'Polygon((0 0, 1 0, 1 1, 0 0))'
        result = removed.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Extend line: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multipolygon
        multipolygon = QgsGeometry.fromWkt('MultiPolygon(((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1)),'
                                           '((10 0, 11 0, 11 1, 10 0),(10.1 10.1, 10.2 0.1, 10.2 0.2, 10.1 0.1)))')
        removed = multipolygon.removeInteriorRings()
        exp = 'MultiPolygon(((0 0, 1 0, 1 1, 0 0)),((10 0, 11 0, 11 1, 10 0)))'
        result = removed.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Extend line: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testMinimumOrientedBoundingBox(self):
        empty = QgsGeometry()
        bbox, area, angle, width, height = empty.orientedMinimumBoundingBox()
        self.assertFalse(bbox)

        # not a useful geometry
        point = QgsGeometry.fromWkt('Point(1 2)')
        bbox, area, angle, width, height = point.orientedMinimumBoundingBox()
        self.assertFalse(bbox)

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((-0.1 -1.3, 2.1 1, 3 2.8, 6.7 0.2, 3 -1.8, 0.3 -2.7, -0.1 -1.3))')
        bbox, area, angle, width, height = polygon.orientedMinimumBoundingBox()
        exp = 'Polygon ((-0.94905660 -1.571698, 2.3817055 -4.580453, 6.7000000 0.1999999, 3.36923 3.208754, -0.949056 -1.57169))'

        result = bbox.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Oriented MBBR: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        self.assertAlmostEqual(area, 28.9152, places=3)
        self.assertAlmostEqual(angle, 42.0922, places=3)
        self.assertAlmostEqual(width, 4.4884, places=3)
        self.assertAlmostEqual(height, 6.4420, places=3)

    def testOrthogonalize(self):
        empty = QgsGeometry()
        o = empty.orthogonalize()
        self.assertFalse(o)

        # not a useful geometry
        point = QgsGeometry.fromWkt('Point(1 2)')
        o = point.orthogonalize()
        self.assertFalse(o)

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((-0.699 0.892, -0.703 0.405, -0.022 0.361, 0.014 0.851, -0.699 0.892))')
        o = polygon.orthogonalize()
        exp = 'Polygon ((-0.69899999999999995 0.89200000000000002, -0.72568713635737736 0.38414056283699533, -0.00900222326098143 0.34648000752227009, 0.01768491457044956 0.85433944198378253, -0.69899999999999995 0.89200000000000002))'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # polygon with ring
        polygon = QgsGeometry.fromWkt('Polygon ((-0.698 0.892, -0.702 0.405, -0.022 0.360, 0.014 0.850, -0.698 0.892),(-0.619 0.777, -0.619 0.574, -0.515 0.567, -0.517 0.516, -0.411 0.499, -0.379 0.767, -0.619 0.777),(-0.322 0.506, -0.185 0.735, -0.046 0.428, -0.322 0.506))')
        o = polygon.orthogonalize()
        exp = 'Polygon ((-0.69799999999999995 0.89200000000000002, -0.72515703079591087 0.38373993222914216, -0.00901577368860811 0.34547552423418099, 0.01814125858957143 0.85373558928902782, -0.69799999999999995 0.89200000000000002),(-0.61899999999999999 0.77700000000000002, -0.63403125159063511 0.56020458713735533, -0.53071476068518508 0.55304126003523246, -0.5343108192220235 0.5011754225601015, -0.40493624158682306 0.49220537936424585, -0.3863089084840608 0.76086661681561074, -0.61899999999999999 0.77700000000000002),(-0.32200000000000001 0.50600000000000001, -0.185 0.73499999999999999, -0.046 0.42799999999999999, -0.32200000000000001 0.50600000000000001))'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multipolygon

        polygon = QgsGeometry.fromWkt('MultiPolygon(((-0.550 -1.553, -0.182 -0.954, -0.182 -0.954, 0.186 -1.538, -0.550 -1.553)),'
                                      '((0.506 -1.376, 0.433 -1.081, 0.765 -0.900, 0.923 -1.132, 0.923 -1.391, 0.506 -1.376)))')
        o = polygon.orthogonalize()
        exp = 'MultiPolygon (((-0.55000000000000004 -1.55299999999999994, -0.182 -0.95399999999999996, -0.182 -0.95399999999999996, 0.186 -1.53800000000000003, -0.55000000000000004 -1.55299999999999994)),((0.50600000000000001 -1.37599999999999989, 0.34888970623957499 -1.04704644438350125, 0.78332709454235683 -0.83955640656085295, 0.92300000000000004 -1.1319999999999999, 0.91737248858460974 -1.38514497083566535, 0.50600000000000001 -1.37599999999999989)))'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # line
        line = QgsGeometry.fromWkt('LineString (-1.07445631048298162 -0.91619958829825165, 0.04022568180912156 -0.95572731852137571, 0.04741254184968957 -0.61794489661467789, 0.68704308546024517 -0.66106605685808595)')
        o = line.orthogonalize()
        exp = 'LineString (-1.07445631048298162 -0.91619958829825165, 0.04812855116470245 -0.96433184892270418, 0.06228000950284909 -0.63427853851139493, 0.68704308546024517 -0.66106605685808595)'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testPolygonize(self):
        o = QgsGeometry.polygonize([])
        self.assertFalse(o)
        empty = QgsGeometry()
        o = QgsGeometry.polygonize([empty])
        self.assertFalse(o)
        line = QgsGeometry.fromWkt('LineString()')
        o = QgsGeometry.polygonize([line])
        self.assertFalse(o)

        l1 = QgsGeometry.fromWkt("LINESTRING (100 180, 20 20, 160 20, 100 180)")
        l2 = QgsGeometry.fromWkt("LINESTRING (100 180, 80 60, 120 60, 100 180)")
        o = QgsGeometry.polygonize([l1, l2])
        exp = "GeometryCollection(POLYGON ((100 180, 160 20, 20 20, 100 180), (100 180, 80 60, 120 60, 100 180)),POLYGON ((100 180, 120 60, 80 60, 100 180)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "polygonize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        lines = [QgsGeometry.fromWkt('LineString(0 0, 1 1)'),
                 QgsGeometry.fromWkt('LineString(0 0, 0 1)'),
                 QgsGeometry.fromWkt('LineString(0 1, 1 1)'),
                 QgsGeometry.fromWkt('LineString(1 1, 1 0)'),
                 QgsGeometry.fromWkt('LineString(1 0, 0 0)'),
                 QgsGeometry.fromWkt('LineString(5 5, 6 6)'),
                 QgsGeometry.fromWkt('Point(0, 0)')]
        o = QgsGeometry.polygonize(lines)
        exp = "GeometryCollection (Polygon ((0 0, 1 1, 1 0, 0 0)),Polygon ((1 1, 0 0, 0 1, 1 1)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "polygonize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testDelaunayTriangulation(self):
        empty = QgsGeometry()
        o = empty.delaunayTriangulation()
        self.assertFalse(o)
        line = QgsGeometry.fromWkt('LineString()')
        o = line.delaunayTriangulation()
        self.assertFalse(o)

        input = QgsGeometry.fromWkt("MULTIPOINT ((10 10), (10 20), (20 20))")
        o = input.delaunayTriangulation()
        exp = "GEOMETRYCOLLECTION (POLYGON ((10 20, 10 10, 20 20, 10 20)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.delaunayTriangulation(0, True)
        exp = "MultiLineString ((10 20, 20 20),(10 10, 10 20),(10 10, 20 20))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("MULTIPOINT ((50 40), (140 70), (80 100), (130 140), (30 150), (70 180), (190 110), (120 20))")
        o = input.delaunayTriangulation()
        exp = "GEOMETRYCOLLECTION (POLYGON ((30 150, 50 40, 80 100, 30 150)), POLYGON ((30 150, 80 100, 70 180, 30 150)), POLYGON ((70 180, 80 100, 130 140, 70 180)), POLYGON ((70 180, 130 140, 190 110, 70 180)), POLYGON ((190 110, 130 140, 140 70, 190 110)), POLYGON ((190 110, 140 70, 120 20, 190 110)), POLYGON ((120 20, 140 70, 80 100, 120 20)), POLYGON ((120 20, 80 100, 50 40, 120 20)), POLYGON ((80 100, 140 70, 130 140, 80 100)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.delaunayTriangulation(0, True)
        exp = "MultiLineString ((70 180, 190 110),(30 150, 70 180),(30 150, 50 40),(50 40, 120 20),(120 20, 190 110),(120 20, 140 70),(140 70, 190 110),(130 140, 140 70),(130 140, 190 110),(70 180, 130 140),(80 100, 130 140),(70 180, 80 100),(30 150, 80 100),(50 40, 80 100),(80 100, 120 20),(80 100, 140 70))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((10 10), (10 20), (20 20), (20 10), (20 0), (10 0), (0 0), (0 10), (0 20))")
        o = input.delaunayTriangulation()
        exp = "GEOMETRYCOLLECTION (POLYGON ((0 20, 0 10, 10 10, 0 20)), POLYGON ((0 20, 10 10, 10 20, 0 20)), POLYGON ((10 20, 10 10, 20 10, 10 20)), POLYGON ((10 20, 20 10, 20 20, 10 20)), POLYGON ((10 0, 20 0, 10 10, 10 0)), POLYGON ((10 0, 10 10, 0 10, 10 0)), POLYGON ((10 0, 0 10, 0 0, 10 0)), POLYGON ((10 10, 20 0, 20 10, 10 10)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.delaunayTriangulation(0, True)
        exp = "MultiLineString ((10 20, 20 20),(0 20, 10 20),(0 10, 0 20),(0 0, 0 10),(0 0, 10 0),(10 0, 20 0),(20 0, 20 10),(20 10, 20 20),(10 20, 20 10),(10 10, 20 10),(10 10, 10 20),(0 20, 10 10),(0 10, 10 10),(10 0, 10 10),(0 10, 10 0),(10 10, 20 0))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt(
            "POLYGON ((42 30, 41.96 29.61, 41.85 29.23, 41.66 28.89, 41.41 28.59, 41.11 28.34, 40.77 28.15, 40.39 28.04, 40 28, 39.61 28.04, 39.23 28.15, 38.89 28.34, 38.59 28.59, 38.34 28.89, 38.15 29.23, 38.04 29.61, 38 30, 38.04 30.39, 38.15 30.77, 38.34 31.11, 38.59 31.41, 38.89 31.66, 39.23 31.85, 39.61 31.96, 40 32, 40.39 31.96, 40.77 31.85, 41.11 31.66, 41.41 31.41, 41.66 31.11, 41.85 30.77, 41.96 30.39, 42 30))")
        o = input.delaunayTriangulation(0, True)
        exp = "MULTILINESTRING ((41.66 31.11, 41.85 30.77), (41.41 31.41, 41.66 31.11), (41.11 31.66, 41.41 31.41), (40.77 31.85, 41.11 31.66), (40.39 31.96, 40.77 31.85), (40 32, 40.39 31.96), (39.61 31.96, 40 32), (39.23 31.85, 39.61 31.96), (38.89 31.66, 39.23 31.85), (38.59 31.41, 38.89 31.66), (38.34 31.11, 38.59 31.41), (38.15 30.77, 38.34 31.11), (38.04 30.39, 38.15 30.77), (38 30, 38.04 30.39), (38 30, 38.04 29.61), (38.04 29.61, 38.15 29.23), (38.15 29.23, 38.34 28.89), (38.34 28.89, 38.59 28.59), (38.59 28.59, 38.89 28.34), (38.89 28.34, 39.23 28.15), (39.23 28.15, 39.61 28.04), (39.61 28.04, 40 28), (40 28, 40.39 28.04), (40.39 28.04, 40.77 28.15), (40.77 28.15, 41.11 28.34), (41.11 28.34, 41.41 28.59), (41.41 28.59, 41.66 28.89), (41.66 28.89, 41.85 29.23), (41.85 29.23, 41.96 29.61), (41.96 29.61, 42 30), (41.96 30.39, 42 30), (41.85 30.77, 41.96 30.39), (41.66 31.11, 41.96 30.39), (41.41 31.41, 41.96 30.39), (41.41 28.59, 41.96 30.39), (41.41 28.59, 41.41 31.41), (38.59 28.59, 41.41 28.59), (38.59 28.59, 41.41 31.41), (38.59 28.59, 38.59 31.41), (38.59 31.41, 41.41 31.41), (38.59 31.41, 39.61 31.96), (39.61 31.96, 41.41 31.41), (39.61 31.96, 40.39 31.96), (40.39 31.96, 41.41 31.41), (40.39 31.96, 41.11 31.66), (38.04 30.39, 38.59 28.59), (38.04 30.39, 38.59 31.41), (38.04 30.39, 38.34 31.11), (38.04 29.61, 38.59 28.59), (38.04 29.61, 38.04 30.39), (39.61 28.04, 41.41 28.59), (38.59 28.59, 39.61 28.04), (38.89 28.34, 39.61 28.04), (40.39 28.04, 41.41 28.59), (39.61 28.04, 40.39 28.04), (41.96 29.61, 41.96 30.39), (41.41 28.59, 41.96 29.61), (41.66 28.89, 41.96 29.61), (40.39 28.04, 41.11 28.34), (38.04 29.61, 38.34 28.89), (38.89 31.66, 39.61 31.96))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt(
            "POLYGON ((0 0, 0 200, 180 200, 180 0, 0 0), (20 180, 160 180, 160 20, 152.625 146.75, 20 180), (30 160, 150 30, 70 90, 30 160))")
        o = input.delaunayTriangulation(0, True)
        exp = "MultiLineString ((0 200, 180 200),(0 0, 0 200),(0 0, 180 0),(180 0, 180 200),(152.625 146.75, 180 0),(152.625 146.75, 180 200),(152.625 146.75, 160 180),(160 180, 180 200),(0 200, 160 180),(20 180, 160 180),(0 200, 20 180),(20 180, 30 160),(0 200, 30 160),(0 0, 30 160),(30 160, 70 90),(0 0, 70 90),(70 90, 150 30),(0 0, 150 30),(150 30, 160 20),(0 0, 160 20),(160 20, 180 0),(152.625 146.75, 160 20),(150 30, 152.625 146.75),(70 90, 152.625 146.75),(30 160, 152.625 146.75),(30 160, 160 180))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((10 10 1), (10 20 2), (20 20 3), (20 10 1.5), (20 0 2.5), (10 0 3.5), (0 0 0), (0 10 .5), (0 20 .25))")
        o = input.delaunayTriangulation()
        exp = "GeometryCollection (PolygonZ ((0 20 0.25, 0 10 0.5, 10 10 1, 0 20 0.25)),PolygonZ ((0 20 0.25, 10 10 1, 10 20 2, 0 20 0.25)),PolygonZ ((10 20 2, 10 10 1, 20 10 1.5, 10 20 2)),PolygonZ ((10 20 2, 20 10 1.5, 20 20 3, 10 20 2)),PolygonZ ((10 0 3.5, 20 0 2.5, 10 10 1, 10 0 3.5)),PolygonZ ((10 0 3.5, 10 10 1, 0 10 0.5, 10 0 3.5)),PolygonZ ((10 0 3.5, 0 10 0.5, 0 0 0, 10 0 3.5)),PolygonZ ((10 10 1, 20 0 2.5, 20 10 1.5, 10 10 1)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.delaunayTriangulation(0, True)
        exp = "MultiLineStringZ ((10 20 2, 20 20 3),(0 20 0.25, 10 20 2),(0 10 0.5, 0 20 0.25),(0 0 0, 0 10 0.5),(0 0 0, 10 0 3.5),(10 0 3.5, 20 0 2.5),(20 0 2.5, 20 10 1.5),(20 10 1.5, 20 20 3),(10 20 2, 20 10 1.5),(10 10 1, 20 10 1.5),(10 10 1, 10 20 2),(0 20 0.25, 10 10 1),(0 10 0.5, 10 10 1),(10 0 3.5, 10 10 1),(0 10 0.5, 10 0 3.5),(10 10 1, 20 0 2.5))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT((-118.3964065 56.0557),(-118.396406 56.0475),(-118.396407 56.04),(-118.3968 56))")
        o = input.delaunayTriangulation(0.001, True)
        exp = "MULTILINESTRING ((-118.3964065 56.0557, -118.396406 56.0475), (-118.396407 56.04, -118.396406 56.0475), (-118.3968 56, -118.396407 56.04))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testVoronoi(self):
        empty = QgsGeometry()
        o = empty.voronoiDiagram()
        self.assertFalse(o)
        line = QgsGeometry.fromWkt('LineString()')
        o = line.voronoiDiagram()
        self.assertFalse(o)

        input = QgsGeometry.fromWkt("MULTIPOINT ((150 200))")
        o = input.voronoiDiagram()
        self.assertTrue(o.isEmpty())

        input = QgsGeometry.fromWkt("MULTIPOINT ((150 200), (180 270), (275 163))")
        o = input.voronoiDiagram()
        exp = "GeometryCollection (Polygon ((170.02400000000000091 38, 25 38, 25 295, 221.20588235294115975 210.91176470588234793, 170.02400000000000091 38)),Polygon ((400 369.65420560747662648, 400 38, 170.02400000000000091 38, 221.20588235294115975 210.91176470588234793, 400 369.65420560747662648)),Polygon ((25 295, 25 395, 400 395, 400 369.65420560747662648, 221.20588235294115975 210.91176470588234793, 25 295)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("MULTIPOINT ((280 300), (420 330), (380 230), (320 160))")
        o = input.voronoiDiagram()
        exp = "GeometryCollection (Polygon ((110 175.71428571428572241, 110 500, 310.35714285714283278 500, 353.515625 298.59375, 306.875 231.96428571428572241, 110 175.71428571428572241)),Polygon ((590 204, 590 -10, 589.16666666666662877 -10, 306.875 231.96428571428572241, 353.515625 298.59375, 590 204)),Polygon ((589.16666666666662877 -10, 110 -10, 110 175.71428571428572241, 306.875 231.96428571428572241, 589.16666666666662877 -10)),Polygon ((310.35714285714283278 500, 590 500, 590 204, 353.515625 298.59375, 310.35714285714283278 500)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("MULTIPOINT ((320 170), (366 246), (530 230), (530 300), (455 277), (490 160))")
        o = input.voronoiDiagram()
        exp = "GeometryCollection (Polygon ((392.35294117647055145 -50, 110 -50, 110 349.02631578947364233, 405.31091180866962986 170.28550074738416242, 392.35294117647055145 -50)),Polygon ((740 63.57142857142859071, 740 -50, 392.35294117647055145 -50, 405.31091180866962986 170.28550074738416242, 429.91476778570188344 205.76082797008174907, 470.12061711079945781 217.78821879382888937, 740 63.57142857142859071)),Polygon ((110 349.02631578947364233, 110 510, 323.94382022471910432 510, 429.91476778570188344 205.76082797008174907, 405.31091180866962986 170.28550074738416242, 110 349.02631578947364233)),Polygon ((323.94382022471910432 510, 424.57333333333326664 510, 499.70666666666664923 265, 470.12061711079945781 217.78821879382888937, 429.91476778570188344 205.76082797008174907, 323.94382022471910432 510)),Polygon ((740 265, 740 63.57142857142859071, 470.12061711079945781 217.78821879382888937, 499.70666666666664923 265, 740 265)),Polygon ((424.57333333333326664 510, 740 510, 740 265, 499.70666666666664923 265, 424.57333333333326664 510)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("MULTIPOINT ((280 200), (406 285), (580 280), (550 190), (370 190), (360 90), (480 110), (440 160), (450 180), (480 180), (460 160), (360 210), (360 220), (370 210), (375 227))")
        o = input.voronoiDiagram()
        exp = "GeometryCollection (Polygon ((-20 -102.27272727272726627, -20 585, 111.94841269841269593 585, 293.54906542056073704 315.803738317756995, 318.75 215, 323.2352941176470722 179.1176470588235361, 319.39560439560437999 144.560439560439562, -20 -102.27272727272726627)),Polygon ((365 200, 365 215, 369.40909090909093493 219.40909090909090651, 414.21192052980131848 206.23178807947019209, 411.875 200, 365 200)),Polygon ((365 215, 365 200, 323.2352941176470722 179.1176470588235361, 318.75 215, 365 215)),Polygon ((471.66666666666674246 -210, -20 -210, -20 -102.27272727272726627, 319.39560439560437999 144.560439560439562, 388.97260273972602818 137.60273972602738013, 419.55882352941176805 102.64705882352942012, 471.66666666666674246 -210)),Polygon ((411.875 200, 410.29411764705884025 187.35294117647057988, 388.97260273972602818 137.60273972602738013, 319.39560439560437999 144.560439560439562, 323.2352941176470722 179.1176470588235361, 365 200, 411.875 200)),Polygon ((410.29411764705884025 187.35294117647057988, 411.875 200, 414.21192052980131848 206.23178807947019209, 431.62536593766145643 234.0192009643533595, 465 248.00476190476189231, 465 175, 450 167.5, 410.29411764705884025 187.35294117647057988)),Polygon ((293.54906542056073704 315.803738317756995, 339.65007656967839011 283.17840735068909908, 369.40909090909093493 219.40909090909090651, 365 215, 318.75 215, 293.54906542056073704 315.803738317756995)),Polygon ((111.94841269841269593 585, 501.69252873563215189 585, 492.56703910614521646 267.43296089385472669, 465 248.00476190476189231, 431.62536593766145643 234.0192009643533595, 339.65007656967839011 283.17840735068909908, 293.54906542056073704 315.803738317756995, 111.94841269841269593 585)),Polygon ((369.40909090909093493 219.40909090909090651, 339.65007656967839011 283.17840735068909908, 431.62536593766145643 234.0192009643533595, 414.21192052980131848 206.23178807947019209, 369.40909090909093493 219.40909090909090651)),Polygon ((388.97260273972602818 137.60273972602738013, 410.29411764705884025 187.35294117647057988, 450 167.5, 450 127, 419.55882352941176805 102.64705882352942012, 388.97260273972602818 137.60273972602738013)),Polygon ((465 175, 465 248.00476190476189231, 492.56703910614521646 267.43296089385472669, 505 255, 520.71428571428566556 145, 495 145, 465 175)),Polygon ((880 -169.375, 880 -210, 471.66666666666674246 -210, 419.55882352941176805 102.64705882352942012, 450 127, 495 145, 520.71428571428566556 145, 880 -169.375)),Polygon ((465 175, 495 145, 450 127, 450 167.5, 465 175)),Polygon ((501.69252873563215189 585, 880 585, 880 130.00000000000005684, 505 255, 492.56703910614521646 267.43296089385472669, 501.69252873563215189 585)),Polygon ((880 130.00000000000005684, 880 -169.375, 520.71428571428566556 145, 505 255, 880 130.00000000000005684)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((100 200), (105 202), (110 200), (140 230), (210 240), (220 190), (170 170), (170 260), (213 245), (220 190))")
        o = input.voronoiDiagram(QgsGeometry(), 6)
        exp = "GeometryCollection (Polygon ((77.1428571428571388 50, -20 50, -20 380, -3.75 380, 105 235, 105 115, 77.1428571428571388 50)),Polygon ((247 50, 77.1428571428571388 50, 105 115, 145 195, 178.33333333333334281 211.66666666666665719, 183.51851851851853326 208.70370370370369528, 247 50)),Polygon ((-3.75 380, 20.00000000000000711 380, 176.66666666666665719 223.33333333333334281, 178.33333333333334281 211.66666666666665719, 145 195, 105 235, -3.75 380)),Polygon ((105 115, 105 235, 145 195, 105 115)),Polygon ((20.00000000000000711 380, 255 380, 176.66666666666665719 223.33333333333334281, 20.00000000000000711 380)),Polygon ((255 380, 340 380, 340 240, 183.51851851851853326 208.70370370370369528, 178.33333333333334281 211.66666666666665719, 176.66666666666665719 223.33333333333334281, 255 380)),Polygon ((340 240, 340 50, 247 50, 183.51851851851853326 208.70370370370369528, 340 240)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((170 270), (177 275), (190 230), (230 250), (210 290), (240 280), (240 250))")
        o = input.voronoiDiagram(QgsGeometry(), 10)
        exp = "GeometryCollection (Polygon ((100 210, 100 360, 150 360, 200 260, 100 210)),Polygon ((150 360, 250 360, 220 270, 200 260, 150 360)),Polygon ((247 160, 100 160, 100 210, 200 260, 235 190, 247 160)),Polygon ((220 270, 235 265, 235 190, 200 260, 220 270)),Polygon ((250 360, 310 360, 310 265, 235 265, 220 270, 250 360)),Polygon ((310 265, 310 160, 247 160, 235 190, 235 265, 310 265)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))")
        o = input.voronoiDiagram(QgsGeometry(), 100)
        exp = "GeometryCollection (Polygon ((215 130, 20 130, 20 310, 205 310, 215 299, 215 130)),Polygon ((205 500, 410 500, 410 338, 215 299, 205 310, 205 500)),Polygon ((20 310, 20 500, 205 500, 205 310, 20 310)),Polygon ((410 338, 410 130, 215 130, 215 299, 410 338)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testDensifyByCount(self):

        empty = QgsGeometry()
        o = empty.densifyByCount(4)
        self.assertFalse(o)

        # point
        input = QgsGeometry.fromWkt("PointZ( 1 2 3 )")
        o = input.densifyByCount(100)
        exp = "PointZ( 1 2 3 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))")
        o = input.densifyByCount(100)
        exp = "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # line
        input = QgsGeometry.fromWkt("LineString( 0 0, 10 0, 10 10 )")
        o = input.densifyByCount(0)
        exp = "LineString( 0 0, 10 0, 10 10 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.densifyByCount(1)
        exp = "LineString( 0 0, 5 0, 10 0, 10 5, 10 10 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.densifyByCount(3)
        exp = "LineString( 0 0, 2.5 0, 5 0, 7.5 0, 10 0, 10 2.5, 10 5, 10 7.5, 10 10 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringZ( 0 0 1, 10 0 2, 10 10 0)")
        o = input.densifyByCount(1)
        exp = "LineStringZ( 0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringM( 0 0 0, 10 0 2, 10 10 0)")
        o = input.densifyByCount(1)
        exp = "LineStringM( 0 0 0, 5 0 1, 10 0 2, 10 5 1, 10 10 0 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringZM( 0 0 1 10, 10 0 2 8, 10 10 0 4)")
        o = input.densifyByCount(1)
        exp = "LineStringZM( 0 0 1 10, 5 0 1.5 9, 10 0 2 8, 10 5 1 6, 10 10 0 4 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # polygon
        input = QgsGeometry.fromWkt("Polygon(( 0 0, 10 0, 10 10, 0 0 ))")
        o = input.densifyByCount(0)
        exp = "Polygon(( 0 0, 10 0, 10 10, 0 0 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("PolygonZ(( 0 0 1, 10 0 2, 10 10 0, 0 0 1 ))")
        o = input.densifyByCount(1)
        exp = "PolygonZ(( 0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0, 5 5 0.5, 0 0 1 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("PolygonZM(( 0 0 1 4, 10 0 2 6, 10 10 0 8, 0 0 1 4 ))")
        o = input.densifyByCount(1)
        exp = "PolygonZM(( 0 0 1 4, 5 0 1.5 5, 10 0 2 6, 10 5 1 7, 10 10 0 8, 5 5 0.5 6, 0 0 1 4 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        # (not strictly valid, but shouldn't matter!
        input = QgsGeometry.fromWkt("PolygonZM(( 0 0 1 4, 10 0 2 6, 10 10 0 8, 0 0 1 4 ), ( 0 0 1 4, 10 0 2 6, 10 10 0 8, 0 0 1 4 ) )")
        o = input.densifyByCount(1)
        exp = "PolygonZM(( 0 0 1 4, 5 0 1.5 5, 10 0 2 6, 10 5 1 7, 10 10 0 8, 5 5 0.5 6, 0 0 1 4 ),( 0 0 1 4, 5 0 1.5 5, 10 0 2 6, 10 5 1 7, 10 10 0 8, 5 5 0.5 6, 0 0 1 4 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multi line
        input = QgsGeometry.fromWkt("MultiLineString(( 0 0, 5 0, 10 0, 10 5, 10 10), (20 0, 25 0, 30 0, 30 5, 30 10 ) )")
        o = input.densifyByCount(1)
        exp = "MultiLineString(( 0 0, 2.5 0, 5 0, 7.5 0, 10 0, 10 2.5, 10 5, 10 7.5, 10 10 ),( 20 0, 22.5 0, 25 0, 27.5 0, 30 0, 30 2.5, 30 5, 30 7.5, 30 10 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multipolygon
        input = QgsGeometry.fromWkt("MultiPolygonZ(((  0 0 1, 10 0 2, 10 10 0, 0 0 1)),((  0 0 1, 10 0 2, 10 10 0, 0 0 1 )))")
        o = input.densifyByCount(1)
        exp = "MultiPolygonZ((( 0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0, 5 5 0.5, 0 0 1 )),(( 0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0, 5 5 0.5, 0 0 1 )))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testDensifyByDistance(self):
        empty = QgsGeometry()
        o = empty.densifyByDistance(4)
        self.assertFalse(o)

        # point
        input = QgsGeometry.fromWkt("PointZ( 1 2 3 )")
        o = input.densifyByDistance(0.1)
        exp = "PointZ( 1 2 3 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))")
        o = input.densifyByDistance(0.1)
        exp = "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # line
        input = QgsGeometry.fromWkt("LineString( 0 0, 10 0, 10 10 )")
        o = input.densifyByDistance(100)
        exp = "LineString( 0 0, 10 0, 10 10 )"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        o = input.densifyByDistance(3)
        exp = "LineString (0 0, 2.5 0, 5 0, 7.5 0, 10 0, 10 2.5, 10 5, 10 7.5, 10 10)"

        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringZ( 0 0 1, 10 0 2, 10 10 0)")
        o = input.densifyByDistance(6)
        exp = "LineStringZ (0 0 1, 5 0 1.5, 10 0 2, 10 5 1, 10 10 0)"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringM( 0 0 0, 10 0 2, 10 10 0)")
        o = input.densifyByDistance(3)
        exp = "LineStringM (0 0 0, 2.5 0 0.5, 5 0 1, 7.5 0 1.5, 10 0 2, 10 2.5 1.5, 10 5 1, 10 7.5 0.5, 10 10 0)"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        input = QgsGeometry.fromWkt("LineStringZM( 0 0 1 10, 10 0 2 8, 10 10 0 4)")
        o = input.densifyByDistance(6)
        exp = "LineStringZM (0 0 1 10, 5 0 1.5 9, 10 0 2 8, 10 5 1 6, 10 10 0 4)"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # polygon
        input = QgsGeometry.fromWkt("Polygon(( 0 0, 20 0, 20 20, 0 0 ))")
        o = input.densifyByDistance(110)
        exp = "Polygon(( 0 0, 20 0, 20 20, 0 0 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("PolygonZ(( 0 0 1, 20 0 2, 20 20 0, 0 0 1 ))")
        o = input.densifyByDistance(6)
        exp = "PolygonZ ((0 0 1, 5 0 1.25, 10 0 1.5, 15 0 1.75, 20 0 2, 20 5 1.5, 20 10 1, 20 15 0.5, 20 20 0, 16 16 0.2, 12 12 0.4, 8 8 0.6, 4 4 0.8, 0 0 1))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testCentroid(self):
        tests = [["POINT(10 0)", "POINT(10 0)"],
                 ["POINT(10 10)", "POINT(10 10)"],
                 ["MULTIPOINT((10 10), (20 20) )", "POINT(15 15)"],
                 [" MULTIPOINT((10 10), (20 20), (10 20), (20 10))", "POINT(15 15)"],
                 ["LINESTRING(10 10, 20 20)", "POINT(15 15)"],
                 ["LINESTRING(0 0, 10 0)", "POINT(5 0 )"],
                 ["LINESTRING (10 10, 10 10)", "POINT (10 10)"], # zero length line
                 ["MULTILINESTRING ((10 10, 10 10), (20 20, 20 20))", "POINT (15 15)"], # zero length multiline
                 ["LINESTRING (60 180, 120 100, 180 180)", "POINT (120 140)"],
                 ["LINESTRING (80 0, 80 120, 120 120, 120 0))", "POINT (100 68.57142857142857)"],
                 ["MULTILINESTRING ((0 0, 0 100), (100 0, 100 100))", "POINT (50 50)"],
                 [" MULTILINESTRING ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180))", "POINT (90 110)"],
                 ["MULTILINESTRING ((20 20, 60 60),(20 -20, 60 -60),(-20 -20, -60 -60),(-20 20, -60 60),(-80 0, 0 80, 80 0, 0 -80, -80 0),(-40 20, -40 -20),(-20 40, 20 40),(40 20, 40 -20),(20 -40, -20 -40))", "POINT (0 0)"],
                 ["POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT (5 5)"],
                 ["POLYGON ((40 160, 160 160, 160 40, 40 40, 40 160))", "POINT (100 100)"],
                 ["POLYGON ((0 200, 200 200, 200 0, 0 0, 0 200), (20 180, 80 180, 80 20, 20 20, 20 180))", "POINT (115.78947368421052 100)"],
                 ["POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180))", "POINT (102.5 97.5)"],
                 ["POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180),(180 60, 140 60, 140 20, 180 20, 180 60))", "POINT (100 100)"],
                 ["MULTIPOLYGON (((0 40, 0 140, 140 140, 140 120, 20 120, 20 40, 0 40)),((0 0, 0 20, 120 20, 120 100, 140 100, 140 0, 0 0)))", "POINT (70 70)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((0 200, 20 180, 20 140, 60 140, 200 0, 0 0, 0 200)),POLYGON ((200 200, 0 200, 20 180, 60 180, 60 140, 200 0, 200 200)))", "POINT (102.5 97.5)"],
                 ["GEOMETRYCOLLECTION (LINESTRING (80 0, 80 120, 120 120, 120 0),MULTIPOINT ((20 60), (40 80), (60 60)))", "POINT (100 68.57142857142857)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((0 40, 40 40, 40 0, 0 0, 0 40)),LINESTRING (80 0, 80 80, 120 40))", "POINT (20 20)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((0 40, 40 40, 40 0, 0 0, 0 40)),LINESTRING (80 0, 80 80, 120 40),MULTIPOINT ((20 60), (40 80), (60 60)))", "POINT (20 20)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 30 30))", "POINT (25 25)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20))", "POINT (15 15)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20),MULTIPOINT ((20 10), (10 20)) )", "POINT (15 15)"],
                 # ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20),POINT EMPTY )","POINT (15 15)"],
                 # ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING EMPTY,POINT EMPTY )","POINT (10 10)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((20 100, 20 -20, 60 -20, 60 100, 20 100)),POLYGON ((-20 60, 100 60, 100 20, -20 20, -20 60)))", "POINT (40 40)"],
                 ["POLYGON ((40 160, 160 160, 160 160, 40 160, 40 160))", "POINT (100 160)"],
                 ["POLYGON ((10 10, 100 100, 100 100, 10 10))", "POINT (55 55)"],
                 # ["POLYGON EMPTY","POINT EMPTY"],
                 # ["MULTIPOLYGON(EMPTY,((0 0,1 0,1 1,0 1, 0 0)))","POINT (0.5 0.5)"],
                 ["POLYGON((56.528666666700 25.2101666667,56.529000000000 25.2105000000,56.528833333300 25.2103333333,56.528666666700 25.2101666667))", "POINT (56.52883333335 25.21033333335)"],
                 ["POLYGON((56.528666666700 25.2101666667,56.529000000000 25.2105000000,56.528833333300 25.2103333333,56.528666666700 25.2101666667))", "POINT (56.528833 25.210333)"]
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            o = input.centroid()
            exp = t[1]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.00001),
                            "centroid: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

            # QGIS native algorithms are bad!
            if False:
                result = QgsGeometry(input.get().centroid()).asWkt()
                self.assertTrue(compareWkt(result, exp, 0.00001),
                                "centroid: mismatch using QgsAbstractGeometry methods Input {} \n Expected:\n{}\nGot:\n{}\n".format(t[0], exp, result))

    def testCompare(self):
        lp = [QgsPointXY(1, 1), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)]
        lp2 = [QgsPointXY(1, 1.0000001), QgsPointXY(2, 2), QgsPointXY(1, 2), QgsPointXY(1, 1)]
        self.assertTrue(QgsGeometry.compare(lp, lp))  # line-line
        self.assertTrue(QgsGeometry.compare([lp], [lp]))  # pylygon-polygon
        self.assertTrue(QgsGeometry.compare([[lp]], [[lp]]))  # multipyolygon-multipolygon
        # handling empty values
        self.assertFalse(QgsGeometry.compare(None, None))
        self.assertFalse(QgsGeometry.compare(lp, []))  # line-line
        self.assertFalse(QgsGeometry.compare([lp], [[]]))  # pylygon-polygon
        self.assertFalse(QgsGeometry.compare([[lp]], [[[]]]))  # multipolygon-multipolygon
        # tolerance
        self.assertFalse(QgsGeometry.compare(lp, lp2))
        self.assertTrue(QgsGeometry.compare(lp, lp2, 1e-6))

    def testPoint(self):
        point = QgsPoint(1, 2)
        self.assertEqual(point.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(point.x(), 1)
        self.assertEqual(point.y(), 2)

        point = QgsPoint(1, 2, wkbType=QgsWkbTypes.Point)
        self.assertEqual(point.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(point.x(), 1)
        self.assertEqual(point.y(), 2)

        point_z = QgsPoint(1, 2, 3)
        self.assertEqual(point_z.wkbType(), QgsWkbTypes.PointZ)
        self.assertEqual(point_z.x(), 1)
        self.assertEqual(point_z.y(), 2)
        self.assertEqual(point_z.z(), 3)

        point_z = QgsPoint(1, 2, 3, 4, wkbType=QgsWkbTypes.PointZ)
        self.assertEqual(point_z.wkbType(), QgsWkbTypes.PointZ)
        self.assertEqual(point_z.x(), 1)
        self.assertEqual(point_z.y(), 2)
        self.assertEqual(point_z.z(), 3)

        point_m = QgsPoint(1, 2, m=3)
        self.assertEqual(point_m.wkbType(), QgsWkbTypes.PointM)
        self.assertEqual(point_m.x(), 1)
        self.assertEqual(point_m.y(), 2)
        self.assertEqual(point_m.m(), 3)

        point_zm = QgsPoint(1, 2, 3, 4)
        self.assertEqual(point_zm.wkbType(), QgsWkbTypes.PointZM)
        self.assertEqual(point_zm.x(), 1)
        self.assertEqual(point_zm.y(), 2)
        self.assertEqual(point_zm.z(), 3)
        self.assertEqual(point_zm.m(), 4)

    def testSubdivide(self):
        tests = [["LINESTRING (1 1,1 9,9 9,9 1)", 8, "MULTILINESTRING ((1 1,1 9,9 9,9 1))"],
                 ["Point (1 1)", 8, "MultiPoint ((1 1))"],
                 ["GeometryCollection ()", 8, "GeometryCollection ()"],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 8, "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5),(1 5, 1 6, 1 7, 1 8, 1 9))"],
                 ["LINESTRING(0 0, 100 100, 150 150)", 8, 'MultiLineString ((0 0, 100 100, 150 150))'],
                 ['POLYGON((132 10,119 23,85 35,68 29,66 28,49 42,32 56,22 64,32 110,40 119,36 150,57 158,75 171,92 182,114 184,132 186,146 178,176 184,179 162,184 141,190 122,190 100,185 79,186 56,186 52,178 34,168 18,147 13,132 10))',
                  10, None],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 1,
                  "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5),(1 5, 1 6, 1 7, 1 8, 1 9))"],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 16,
                  "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5, 1 6, 1 7, 1 8, 1 9))"],
                 ["POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180),(180 60, 140 60, 140 20, 180 20, 180 60))", 8, "MultiPolygon (((0 0, 0 100, 100 100, 100 0, 0 0)),((100 0, 100 50, 140 50, 140 20, 150 20, 150 0, 100 0)),((150 0, 150 20, 180 20, 180 50, 200 50, 200 0, 150 0)),((100 50, 100 100, 150 100, 150 60, 140 60, 140 50, 100 50)),((150 60, 150 100, 200 100, 200 50, 180 50, 180 60, 150 60)),((0 100, 0 150, 20 150, 20 140, 50 140, 50 100, 0 100)),((50 100, 50 140, 60 140, 60 150, 100 150, 100 100, 50 100)),((0 150, 0 200, 50 200, 50 180, 20 180, 20 150, 0 150)),((50 180, 50 200, 100 200, 100 150, 60 150, 60 180, 50 180)),((100 100, 100 200, 200 200, 200 100, 100 100)))"],
                 ["POLYGON((132 10,119 23,85 35,68 29,66 28,49 42,32 56,22 64,32 110,40 119,36 150, 57 158,75 171,92 182,114 184,132 186,146 178,176 184,179 162,184 141,190 122,190 100,185 79,186 56,186 52,178 34,168 18,147 13,132 10))", 10, None]
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            o = input.subdivide(t[1])
            # make sure area is unchanged
            self.assertAlmostEqual(input.area(), o.area(), 5)
            max_points = 999999
            for p in range(o.constGet().numGeometries()):
                part = o.constGet().geometryN(p)
                self.assertLessEqual(part.nCoordinates(), max(t[1], 8))

            if t[2]:
                exp = t[2]
                result = o.asWkt()
                self.assertTrue(compareWkt(result, exp, 0.00001),
                                "clipped: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testClipped(self):
        tests = [["LINESTRING (1 1,1 9,9 9,9 1)", QgsRectangle(0, 0, 10, 10), "LINESTRING (1 1,1 9,9 9,9 1)"],
                 ["LINESTRING (-1 -9,-1 11,9 11)", QgsRectangle(0, 0, 10, 10), "GEOMETRYCOLLECTION ()"],
                 ["LINESTRING (-1 5,5 5,9 9)", QgsRectangle(0, 0, 10, 10), "LINESTRING (0 5,5 5,9 9)"],
                 ["LINESTRING (5 5,8 5,12 5)", QgsRectangle(0, 0, 10, 10), "LINESTRING (5 5,8 5,10 5)"],
                 ["LINESTRING (5 -1,5 5,1 2,-3 2,1 6)", QgsRectangle(0, 0, 10, 10), "MULTILINESTRING ((5 0,5 5,1 2,0 2),(0 5,1 6))"],
                 ["LINESTRING (0 3,0 5,0 7)", QgsRectangle(0, 0, 10, 10), "GEOMETRYCOLLECTION ()"],
                 ["LINESTRING (0 3,0 5,-1 7)", QgsRectangle(0, 0, 10, 10), "GEOMETRYCOLLECTION ()"],
                 ["LINESTRING (0 3,0 5,2 7)", QgsRectangle(0, 0, 10, 10), "LINESTRING (0 5,2 7)"],
                 ["LINESTRING (2 1,0 0,1 2)", QgsRectangle(0, 0, 10, 10), "LINESTRING (2 1,0 0,1 2)"],
                 ["LINESTRING (3 3,0 3,0 5,2 7)", QgsRectangle(0, 0, 10, 10), "MULTILINESTRING ((3 3,0 3),(0 5,2 7))"],
                 ["LINESTRING (5 5,10 5,20 5)", QgsRectangle(0, 0, 10, 10), "LINESTRING (5 5,10 5)"],
                 ["LINESTRING (3 3,0 6,3 9)", QgsRectangle(0, 0, 10, 10), "LINESTRING (3 3,0 6,3 9)"],
                 ["POLYGON ((5 5,5 6,6 6,6 5,5 5))", QgsRectangle(0, 0, 10, 10), "POLYGON ((5 5,5 6,6 6,6 5,5 5))"],
                 ["POLYGON ((15 15,15 16,16 16,16 15,15 15))", QgsRectangle(0, 0, 10, 10), "GEOMETRYCOLLECTION ()"],
                 ["POLYGON ((-1 -1,-1 11,11 11,11 -1,-1 -1))", QgsRectangle(0, 0, 10, 10), "Polygon ((0 0, 0 10, 10 10, 10 0, 0 0))"],
                 ["POLYGON ((-1 -1,-1 5,5 5,5 -1,-1 -1))", QgsRectangle(0, 0, 10, 10), "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))"],
                 ["POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (3 3,4 4,4 2,3 3))", QgsRectangle(0, 0, 10, 10), "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0),(3 3, 4 4, 4 2, 3 3))"]
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            o = input.clipped(t[1])
            exp = t[2]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.00001),
                            "clipped: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testCreateWedgeBuffer(self):
        tests = [[QgsPoint(1, 11), 0, 45, 2, 0, 'CurvePolygon (CompoundCurve (CircularString (0.23463313526982044 12.84775906502257392, 1 13, 1.76536686473017967 12.84775906502257392),(1.76536686473017967 12.84775906502257392, 1 11),(1 11, 0.23463313526982044 12.84775906502257392)))'],
                 [QgsPoint(1, 11), 90, 45, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (2.84775906502257348 11.76536686473017923, 3 11, 2.84775906502257348 10.23463313526982077),(2.84775906502257348 10.23463313526982077, 1 11),(1 11, 2.84775906502257348 11.76536686473017923)))'],
                 [QgsPoint(1, 11), 180, 90, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (2.41421356237309492 9.58578643762690419, 1.00000000000000022 9, -0.41421356237309492 9.58578643762690419),(-0.41421356237309492 9.58578643762690419, 1 11),(1 11, 2.41421356237309492 9.58578643762690419)))'],
                 [QgsPoint(1, 11), 0, 200, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (-0.96961550602441604 10.65270364466613984, 0.99999999999999956 13, 2.96961550602441626 10.65270364466613984),(2.96961550602441626 10.65270364466613984, 1 11),(1 11, -0.96961550602441604 10.65270364466613984)))'],
                 [QgsPoint(1, 11), 0, 45, 2, 1,
                  'CurvePolygon (CompoundCurve (CircularString (0.23463313526982044 12.84775906502257392, 1 13, 1.76536686473017967 12.84775906502257392),(1.76536686473017967 12.84775906502257392, 1.38268343236508984 11.92387953251128607),CircularString (1.38268343236508984 11.92387953251128607, 0.99999999999999978 12, 0.61731656763491016 11.92387953251128607),(0.61731656763491016 11.92387953251128607, 0.23463313526982044 12.84775906502257392)))'],
                 [QgsPoint(1, 11), 0, 200, 2, 1,
                  'CurvePolygon (CompoundCurve (CircularString (-0.96961550602441604 10.65270364466613984, 0.99999999999999956 13, 2.96961550602441626 10.65270364466613984),(2.96961550602441626 10.65270364466613984, 1.98480775301220813 10.82635182233306992),CircularString (1.98480775301220813 10.82635182233306992, 0.99999999999999978 12, 0.01519224698779198 10.82635182233306992),(0.01519224698779198 10.82635182233306992, -0.96961550602441604 10.65270364466613984)))'],
                 [QgsPoint(1, 11, 3), 0, 45, 2, 0,
                  'CurvePolygonZ (CompoundCurveZ (CircularStringZ (0.23463313526982044 12.84775906502257392 3, 1 13 3, 1.76536686473017967 12.84775906502257392 3),(1.76536686473017967 12.84775906502257392 3, 1 11 3),(1 11 3, 0.23463313526982044 12.84775906502257392 3)))'],
                 [QgsPoint(1, 11, m=3), 0, 45, 2, 0,
                  'CurvePolygonM (CompoundCurveM (CircularStringM (0.23463313526982044 12.84775906502257392 3, 1 13 3, 1.76536686473017967 12.84775906502257392 3),(1.76536686473017967 12.84775906502257392 3, 1 11 3),(1 11 3, 0.23463313526982044 12.84775906502257392 3)))'],
                 [QgsPoint(1, 11), 0, 360, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (1 13, 3 11, 1 9, -1 11, 1 13)))'],
                 [QgsPoint(1, 11), 0, -1000, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (1 13, 3 11, 1 9, -1 11, 1 13)))'],
                 [QgsPoint(1, 11), 0, 360, 2, 1,
                  'CurvePolygon (CompoundCurve (CircularString (1 13, 3 11, 1 9, -1 11, 1 13)),CompoundCurve (CircularString (1 12, 2 11, 1 10, 0 11, 1 12)))']
                 ]
        for t in tests:
            input = t[0]
            azimuth = t[1]
            width = t[2]
            outer = t[3]
            inner = t[4]
            o = QgsGeometry.createWedgeBuffer(input, azimuth, width, outer, inner)
            exp = t[5]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.01),
                            "wedge buffer: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testTaperedBuffer(self):
        tests = [['LineString (6 2, 9 2, 9 3, 11 5)', 1, 2, 3, 'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.23175255221825708 2.66341629715358597, 8.20710678118654791 3, 8.31333433001913669 3.39644660940672605, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.76603613070954424 2.63321666219915951, 9.71966991411008863 1.99999999999999978, 9.62325242795870217 1.64016504294495569, 9.35983495705504431 1.37674757204129761, 9 1.28033008588991049, 6 1.5)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 1, 1, 3,
                  'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.5 2.5, 8.5 3, 8.56698729810778126 3.24999999999999956, 8.75 3.43301270189221919, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 11.25 4.56698729810778037, 9.5 2.81698729810778081, 9.5 2, 9.43301270189221874 1.75000000000000022, 9.25 1.56698729810778081, 9 1.5, 6 1.5)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 2, 1, 3,
                  'MultiPolygon (((6 1, 5.5 1.13397459621556118, 5.13397459621556163 1.49999999999999978, 5 2, 5.13397459621556074 2.49999999999999956, 5.5 2.86602540378443837, 6 3, 8.28066508549441238 2.83300216551852069, 8.29289321881345209 3, 8.38762756430420531 3.35355339059327351, 8.64644660940672694 3.61237243569579425, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 9.72358625835961909 2.77494218213953703, 9.78033008588991137 1.99999999999999978, 9.67578567771795583 1.60983495705504498, 9.39016504294495569 1.32421432228204461, 9 1.21966991411008951, 6 1)))'],
                 ['MultiLineString ((2 0, 2 2, 3 2, 3 3),(2.94433781190019195 4.04721689059500989, 5.45950095969289784 4.11976967370441471),(3 3, 5.5804222648752404 2.94683301343570214))', 1, 2, 3,
                  'MultiPolygon (((2.5 -0.00000000000000012, 2.43301270189221963 -0.24999999999999983, 2.25 -0.4330127018922193, 2 -0.5, 1.75 -0.43301270189221935, 1.56698729810778081 -0.25000000000000011, 1.5 0.00000000000000006, 1.25 2, 1.35048094716167078 2.37499999999999956, 1.62499999999999978 2.649519052838329, 2 2.75, 2.03076923076923066 2.75384615384615383, 2 3, 2.13397459621556118 3.49999999999999956, 2.5 3.86602540378443837, 3.00000000000000044 4, 3.50000000000000044 3.86602540378443837, 3.86602540378443837 3.5, 4 3, 3.875 1.99999999999999978, 3.75777222831138413 1.56250000000000044, 3.4375 1.24222777168861631, 3 1.125, 2.64615384615384608 1.1692307692307693, 2.5 -0.00000000000000012)),((2.94433781190019195 3.54721689059500989, 2.69433781190019195 3.6142041887027907, 2.51132511000797276 3.79721689059500989, 2.44433781190019195 4.04721689059500989, 2.51132511000797232 4.29721689059500989, 2.69433781190019195 4.48022959248722952, 2.94433781190019195 4.54721689059500989, 5.45950095969289784 5.11976967370441471, 5.95950095969289784 4.98579507748885309, 6.32552636347733621 4.61976967370441471, 6.45950095969289784 4.11976967370441471, 6.3255263634773371 3.61976967370441516, 5.95950095969289784 3.25374426991997634, 5.45950095969289784 3.11976967370441471, 2.94433781190019195 3.54721689059500989)),((5.5804222648752404 3.94683301343570214, 6.0804222648752404 3.81285841722014052, 6.44644766865967878 3.44683301343570214, 6.5804222648752404 2.94683301343570214, 6.44644766865967966 2.44683301343570259, 6.0804222648752404 2.08080760965126377, 5.5804222648752404 1.94683301343570214, 3 2.5, 2.75 2.56698729810778081, 2.56698729810778081 2.75, 2.5 3, 2.56698729810778037 3.24999999999999956, 2.75 3.43301270189221919, 3 3.5, 5.5804222648752404 3.94683301343570214)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 2, 7, 3,
                  'MultiPolygon (((10.86920158655618174 1.1448080812814232, 10.81722403411685463 0.95082521472477743, 10.04917478527522334 0.18277596588314599, 9 -0.09834957055044669, 7.95082521472477666 0.18277596588314587, 5.5 1.13397459621556118, 5.13397459621556163 1.49999999999999978, 5 2, 5.13397459621556074 2.49999999999999956, 5.5 2.86602540378443837, 6.61565808125483201 3.29902749321661304, 6.86570975577233966 4.23223304703362935, 7.96891108675446347 6.74999999999999822, 9.25 8.03108891324553475, 11 8.5, 12.75000000000000178 8.03108891324553475, 14.03108891324553475 6.75, 14.5 4.99999999999999911, 14.03108891324553653 3.25000000000000133, 12.75 1.9689110867544648, 10.86920158655618174 1.1448080812814232)))'],
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            start = t[1]
            end = t[2]
            segments = t[3]
            o = QgsGeometry.taperedBuffer(input, start, end, segments)
            exp = t[4]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.01),
                            "tapered buffer: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testVariableWidthBufferByM(self):
        tests = [['LineString (6 2, 9 2, 9 3, 11 5)', 3, 'GeometryCollection ()'],
                 ['LineStringM (6 2 1, 9 2 1.5, 9 3 0.5, 11 5 2)', 3, 'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.54510095773215994 2.71209174647768014, 8.78349364905388974 3.125, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.34232758349701164 2.90707123255090094, 9.649519052838329 2.375, 9.75 1.99999999999999978, 9.649519052838329 1.62500000000000022, 9.375 1.350480947161671, 9 1.25, 6 1.5)))'],
                 ['MultiLineStringM ((6 2 1, 9 2 1.5, 9 3 0.5, 11 5 2),(1 2 0.5, 3 2 0.2))', 3,
                  'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.54510095773215994 2.71209174647768014, 8.78349364905388974 3.125, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.34232758349701164 2.90707123255090094, 9.649519052838329 2.375, 9.75 1.99999999999999978, 9.649519052838329 1.62500000000000022, 9.375 1.350480947161671, 9 1.25, 6 1.5)),((1 1.75, 0.875 1.78349364905389041, 0.78349364905389041 1.875, 0.75 2, 0.7834936490538903 2.125, 0.875 2.21650635094610982, 1 2.25, 3 2.10000000000000009, 3.04999999999999982 2.08660254037844384, 3.08660254037844384 2.04999999999999982, 3.10000000000000009 2, 3.08660254037844384 1.94999999999999996, 3.04999999999999982 1.91339745962155616, 3 1.89999999999999991, 1 1.75)))']
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            segments = t[1]
            o = QgsGeometry.variableWidthBufferByM(input, segments)
            exp = t[2]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.01),
                            "tapered buffer: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testHausdorff(self):
        tests = [["POLYGON((0 0, 0 2, 1 2, 2 2, 2 0, 0 0))", "POLYGON((0.5 0.5, 0.5 2.5, 1.5 2.5, 2.5 2.5, 2.5 0.5, 0.5 0.5))", 0.707106781186548],
                 ["LINESTRING (0 0, 2 1)", "LINESTRING (0 0, 2 0)", 1],
                 ["LINESTRING (0 0, 2 0)", "LINESTRING (0 1, 1 2, 2 1)", 2],
                 ["LINESTRING (0 0, 2 0)", "MULTIPOINT (0 1, 1 0, 2 1)", 1],
                 ["LINESTRING (130 0, 0 0, 0 150)", "LINESTRING (10 10, 10 150, 130 10)", 14.142135623730951]
                 ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            g2 = QgsGeometry.fromWkt(t[1])
            o = g1.hausdorffDistance(g2)
            exp = t[2]
            self.assertAlmostEqual(o, exp, 5,
                                   "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], exp, o))

    def testHausdorffDensify(self):
        tests = [
            ["LINESTRING (130 0, 0 0, 0 150)", "LINESTRING (10 10, 10 150, 130 10)", 0.5, 70.0]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            g2 = QgsGeometry.fromWkt(t[1])
            densify = t[2]
            o = g1.hausdorffDistanceDensify(g2, densify)
            exp = t[3]
            self.assertAlmostEqual(o, exp, 5,
                                   "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], exp, o))

    def testBoundingBoxIntersects(self):
        tests = [
            ["LINESTRING (0 0, 100 100)", "LINESTRING (90 0, 100 0)", True],
            ["LINESTRING (0 0, 100 100)", "LINESTRING (101 0, 102 0)", False],
            ["POINT(20 1)", "LINESTRING( 0 0, 100 100 )", True],
            ["POINT(20 1)", "POINT(21 1)", False],
            ["POINT(20 1)", "POINT(20 1)", True]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            g2 = QgsGeometry.fromWkt(t[1])
            res = g1.boundingBoxIntersects(g2)
            self.assertEqual(res, t[2], "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(g1.asWkt(), g2.asWkt(), t[2], res))

    def testBoundingBoxIntersectsRectangle(self):
        tests = [
            ["LINESTRING (0 0, 100 100)", QgsRectangle(90, 0, 100, 10), True],
            ["LINESTRING (0 0, 100 100)", QgsRectangle(101, 0, 102, 10), False],
            ["POINT(20 1)", QgsRectangle(0, 0, 100, 100), True],
            ["POINT(20 1)", QgsRectangle(21, 1, 21, 1), False],
            ["POINT(20 1)", QgsRectangle(20, 1, 20, 1), True]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.boundingBoxIntersects(t[1])
            self.assertEqual(res, t[2], "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(g1.asWkt(), t[1].toString(), t[2], res))

    def testOffsetCurve(self):
        tests = [
            ["LINESTRING (0 0, 0 100, 100 100)", 1, "LineString (-1 0, -1 101, 100 101)"],
            ["LINESTRING (0 0, 0 100, 100 100)", -1, "LineString (1 0, 1 99, 100 99)"],
            ["LINESTRING (100 100, 0 100, 0 0)", 1, "LineString (100 99, 1 99, 1 0)"],
            ["LINESTRING (100 100, 0 100, 0 0)", -1, "LineString (100 101, -1 101, -1 0)"],
            # linestring which becomes multilinestring -- the actual offset curve calculated by GEOS looks bad, but we shouldn't crash here
            ["LINESTRING (259329.820 5928370.79, 259324.337 5928371.758, 259319.678 5928372.33, 259317.064 5928372.498 )", 100, "MultiLineString ((259313.3 5928272.5, 259312.5 5928272.6),(259312.4 5928272.3, 259309.5 5928272.8, 259307.5 5928273.1))"],
            ["MULTILINESTRING ((0 0, 0 100, 100 100),(100 100, 0 100, 0 0))", 1, "MultiLineString ((-1 0, -1 101, 100 101),(100 99, 1 99, 1 0))"]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.offsetCurve(t[1], 2, QgsGeometry.JoinStyleMiter, 5)

            self.assertEqual(res.asWkt(1), t[2],
                             "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1],
                                                                                       t[2], res.asWkt(1)))

    def testForceRHR(self):
        tests = [
            ["", ""],
            ["Point (100 100)", "Point (100 100)"],
            ["LINESTRING (0 0, 0 100, 100 100)", "LineString (0 0, 0 100, 100 100)"],
            ["LINESTRING (100 100, 0 100, 0 0)", "LineString (100 100, 0 100, 0 0)"],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", "Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1))"],
            ["MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))", "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))"],
            [
                "GeometryCollection(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "GeometryCollection (Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1)),Polygon ((100 100, 100 200, 200 200, 200 100, 100 100)))"]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.forceRHR()
            self.assertEqual(res.asWkt(1), t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res.asWkt(1)))

    def testIsGeosValid(self):
        tests = [
            ["", False, False, ''],
            ["Point (100 100)", True, True, ''],
            ["MultiPoint (100 100, 100 200)", True, True, ''],
            ["LINESTRING (0 0, 0 100, 100 100)", True, True, ''],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", True, True, ''],
            ["MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))", True, True, ''],
            ['MultiPolygon (((159865.14786298031685874 6768656.31838363595306873, 159858.97975336571107619 6769211.44824895076453686, 160486.07089751763851382 6769211.44824895076453686, 160481.95882444124436006 6768658.37442017439752817, 160163.27316101978067309 6768658.37442017439752817, 160222.89822062765597366 6769116.87056819349527359, 160132.43261294672265649 6769120.98264127038419247, 160163.27316101978067309 6768658.37442017439752817, 159865.14786298031685874 6768656.31838363595306873)))', False, True, 'Ring self-intersection'],
            ['Polygon((0 3, 3 0, 3 3, 0 0, 0 3))', False, False, 'Self-intersection'],
        ]
        for t in tests:
            # run each check 2 times to allow for testing of cached value
            g1 = QgsGeometry.fromWkt(t[0])
            for i in range(2):
                res = g1.isGeosValid()
                self.assertEqual(res, t[1],
                                 "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res))
                if not res:
                    self.assertEqual(g1.lastError(), t[3], t[0])
            for i in range(2):
                res = g1.isGeosValid(QgsGeometry.FlagAllowSelfTouchingHoles)
                self.assertEqual(res, t[2],
                                 "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[2], res))

    def testValidateGeometry(self):
        tests = [
            ["", [], [], []],
            ["Point (100 100)", [], [], []],
            ["MultiPoint (100 100, 100 200)", [], [], []],
            ["LINESTRING (0 0, 0 100, 100 100)", [], [], []],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", [], [], []],
            ["MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))", [], [], []],
            ['POLYGON ((200 400, 400 400, 400 200, 300 200, 350 250, 250 250, 300 200, 200 200, 200 400))', [QgsGeometry.Error('Ring self-intersection', QgsPointXY(300, 200))], [], []],
            ['MultiPolygon (((159865.14786298031685874 6768656.31838363595306873, 159858.97975336571107619 6769211.44824895076453686, 160486.07089751763851382 6769211.44824895076453686, 160481.95882444124436006 6768658.37442017439752817, 160163.27316101978067309 6768658.37442017439752817, 160222.89822062765597366 6769116.87056819349527359, 160132.43261294672265649 6769120.98264127038419247, 160163.27316101978067309 6768658.37442017439752817, 159865.14786298031685874 6768656.31838363595306873)))', [QgsGeometry.Error('Ring self-intersection', QgsPointXY(160163.27316101978067309, 6768658.37442017439752817))], [], []],
            ['Polygon((0 3, 3 0, 3 3, 0 0, 0 3))', [QgsGeometry.Error('Self-intersection', QgsPointXY(1.5, 1.5))], [QgsGeometry.Error('Self-intersection', QgsPointXY(1.5, 1.5))], [QgsGeometry.Error('segments 0 and 2 of line 0 intersect at 1.5, 1.5', QgsPointXY(1.5, 1.5))]],
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.validateGeometry(QgsGeometry.ValidatorGeos)
            self.assertEqual(res, t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res[0].where() if res else ''))
            res = g1.validateGeometry(QgsGeometry.ValidatorGeos, QgsGeometry.FlagAllowSelfTouchingHoles)
            self.assertEqual(res, t[2],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[2], res[0].where() if res else ''))
            res = g1.validateGeometry(QgsGeometry.ValidatorQgisInternal)
            self.assertEqual(res, t[3],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[3], res[0].where() if res else ''))

    def renderGeometry(self, geom, use_pen, as_polygon=False, as_painter_path=False):
        image = QImage(200, 200, QImage.Format_RGB32)
        image.fill(QColor(0, 0, 0))

        painter = QPainter(image)
        if use_pen:
            painter.setPen(QPen(QColor(255, 255, 255), 4))
        else:
            painter.setBrush(QBrush(QColor(255, 255, 255)))

        if as_painter_path:
            path = QPainterPath()
            geom.constGet().addToPainterPath(path)
            painter.drawPath(path)
        else:
            if as_polygon:
                geom.constGet().drawAsPolygon(painter)
            else:
                geom.draw(painter)
        painter.end()
        return image

    def testGeometryDraw(self):
        '''Tests drawing geometries'''

        tests = [{'name': 'Point',
                  'wkt': 'Point (40 60)',
                  'reference_image': 'point',
                  'use_pen': False},
                 {'name': 'LineString',
                  'wkt': 'LineString (20 30, 50 30, 50 90)',
                  'reference_image': 'linestring',
                  'as_polygon_reference_image': 'linestring_aspolygon',
                  'use_pen': True},
                 {'name': 'CircularString',
                  'wkt': 'CircularString (20 30, 50 30, 50 90)',
                  'reference_image': 'circularstring',
                  'as_polygon_reference_image': 'circularstring_aspolygon',
                  'use_pen': True},
                 {'name': 'CurvePolygon',
                  'wkt': 'CurvePolygon(CircularString (20 30, 50 30, 50 90, 10 50, 20 30))',
                  'reference_image': 'curvepolygon_circularstring',
                  'use_pen': False},
                 {'name': 'CurvePolygonInteriorRings',
                  'wkt': 'CurvePolygon(CircularString (20 30, 50 30, 50 90, 10 50, 20 30),LineString(30 45, 55 45, 30 75, 30 45))',
                  'reference_image': 'curvepolygon_circularstring_interiorrings',
                  'use_pen': False},
                 {'name': 'CompoundCurve',
                  'wkt': 'CompoundCurve(CircularString (20 30, 50 30, 50 90),LineString(50 90, 10 90))',
                  'reference_image': 'compoundcurve',
                  'use_pen': True,
                  'as_polygon_reference_image': 'compoundcurve_aspolygon'},
                 {'name': 'GeometryCollection',
                  'wkt': 'GeometryCollection(LineString (20 30, 50 30, 50 70),LineString(10 90, 90 90))',
                  'reference_image': 'geometrycollection',
                  'use_pen': True}
                 ]

        for test in tests:
            geom = QgsGeometry.fromWkt(test['wkt'])
            self.assertTrue(geom and not geom.isNull(), 'Could not create geometry {}'.format(test['wkt']))
            rendered_image = self.renderGeometry(geom, test['use_pen'])
            assert self.imageCheck(test['name'], test['reference_image'], rendered_image)

            if hasattr(geom.constGet(), 'addToPainterPath'):
                # also check using painter path
                rendered_image = self.renderGeometry(geom, test['use_pen'], as_painter_path=True)
                assert self.imageCheck(test['name'], test['reference_image'], rendered_image)

            if 'as_polygon_reference_image' in test:
                rendered_image = self.renderGeometry(geom, False, True)
                assert self.imageCheck(test['name'] + '_aspolygon', test['as_polygon_reference_image'], rendered_image)

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'geometry_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("geometry")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
