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
    QgsTriangle,
    QgsRenderChecker,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsVertexId,
    QgsAbstractGeometryTransformer,
    QgsCircle,
    Qgis
)
from qgis.PyQt.QtCore import QDir, QPointF, QRectF
from qgis.PyQt.QtGui import QImage, QPainter, QPen, QColor, QBrush, QPainterPath, QPolygonF, QTransform

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
        self.geos39 = Qgis.geosVersionInt() >= 30900

    def testBool(self):
        """ Test boolean evaluation of QgsGeometry """
        g = QgsGeometry()
        self.assertFalse(g)
        myWKT = 'Point (10 10)'
        g = QgsGeometry.fromWkt(myWKT)
        self.assertTrue(g)
        g = QgsGeometry(None)
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
        self.assertEqual(next(it).asWkt(),
                         'Polygon ((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50))')
        with self.assertRaises(StopIteration):
            next(it)

        # multi polygon geometry
        g = QgsGeometry.fromWkt(
            'MultiPolygon (((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50)),((20 2, 20 4, 22 4, 22 2, 20 2)))')
        it = g.parts()
        self.assertEqual(next(it).asWkt(),
                         'Polygon ((10 10, 100 10, 100 100, 10 100, 10 10),(50 50, 55 50, 55 55, 50 55, 50 50))')
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
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint,
                         ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
        self.assertEqual(geom.constGet().numGeometries(), 2)
        self.assertEqual(geom.constGet().geometryN(0).x(), 10)
        self.assertEqual(geom.constGet().geometryN(0).y(), 15)
        self.assertEqual(geom.constGet().geometryN(1).x(), 20)
        self.assertEqual(geom.constGet().geometryN(1).y(), 30)

        # Check MS SQL format
        wkt = 'MultiPoint (11 16, 21 31)'
        geom = QgsGeometry.fromWkt(wkt)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint,
                         ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
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

    def testQgsLineStringPythonConstructors(self):
        """
        Test various constructors for QgsLineString in Python
        """
        line = QgsLineString()
        self.assertEqual(line.asWkt(), 'LineString EMPTY')

        # empty array
        line = QgsLineString([])
        self.assertEqual(line.asWkt(), 'LineString EMPTY')

        # invalid array
        with self.assertRaises(TypeError):
            line = QgsLineString([1, 2, 3])

        # array of QgsPoint
        line = QgsLineString([QgsPoint(1, 2), QgsPoint(3, 4), QgsPoint(11, 12)])
        self.assertEqual(line.asWkt(), 'LineString (1 2, 3 4, 11 12)')

        # array of QgsPoint with Z
        line = QgsLineString([QgsPoint(1, 2, 11), QgsPoint(3, 4, 13), QgsPoint(11, 12, 14)])
        self.assertEqual(line.asWkt(), 'LineStringZ (1 2 11, 3 4 13, 11 12 14)')

        # array of QgsPoint with Z, only first has z
        line = QgsLineString([QgsPoint(1, 2, 11), QgsPoint(3, 4), QgsPoint(11, 12)])
        self.assertEqual(line.asWkt(), 'LineStringZ (1 2 11, 3 4 nan, 11 12 nan)')

        # array of QgsPoint with M
        line = QgsLineString([QgsPoint(1, 2, None, 11), QgsPoint(3, 4, None, 13), QgsPoint(11, 12, None, 14)])
        self.assertEqual(line.asWkt(), 'LineStringM (1 2 11, 3 4 13, 11 12 14)')

        # array of QgsPoint with M, only first has M
        line = QgsLineString([QgsPoint(1, 2, None, 11), QgsPoint(3, 4), QgsPoint(11, 12)])
        self.assertEqual(line.asWkt(), 'LineStringM (1 2 11, 3 4 nan, 11 12 nan)')

        # array of QgsPoint with ZM
        line = QgsLineString([QgsPoint(1, 2, 22, 11), QgsPoint(3, 4, 23, 13), QgsPoint(11, 12, 24, 14)])
        self.assertEqual(line.asWkt(), 'LineStringZM (1 2 22 11, 3 4 23 13, 11 12 24 14)')

        # array of QgsPoint with ZM, only first has ZM
        line = QgsLineString([QgsPoint(1, 2, 33, 11), QgsPoint(3, 4), QgsPoint(11, 12)])
        self.assertEqual(line.asWkt(), 'LineStringZM (1 2 33 11, 3 4 nan nan, 11 12 nan nan)')

        # array of QgsPointXY
        line = QgsLineString([QgsPointXY(1, 2), QgsPointXY(3, 4), QgsPointXY(11, 12)])
        self.assertEqual(line.asWkt(), 'LineString (1 2, 3 4, 11 12)')

        # array of array of bad values
        with self.assertRaises(TypeError):
            line = QgsLineString([[QgsPolygon(), QgsPoint()]])

        with self.assertRaises(TypeError):
            line = QgsLineString([[1, 2], [QgsPolygon(), QgsPoint()]])

        # array of array of 1d floats
        with self.assertRaises(TypeError):
            line = QgsLineString([[1], [3], [5]])

        # array of array of floats
        line = QgsLineString([[1, 2], [3, 4], [5, 6]])
        self.assertEqual(line.asWkt(), 'LineString (1 2, 3 4, 5 6)')

        # tuple of tuple of floats
        line = QgsLineString(((1, 2), (3, 4), (5, 6)))
        self.assertEqual(line.asWkt(), 'LineString (1 2, 3 4, 5 6)')

        # sequence
        line = QgsLineString([[c + 10, c + 11] for c in range(5)])
        self.assertEqual(line.asWkt(), 'LineString (10 11, 11 12, 12 13, 13 14, 14 15)')

        # array of array of 3d floats
        line = QgsLineString([[1, 2, 11], [3, 4, 12], [5, 6, 13]])
        self.assertEqual(line.asWkt(), 'LineStringZ (1 2 11, 3 4 12, 5 6 13)')

        # array of array of inconsistent 3d floats
        line = QgsLineString([[1, 2, 11], [3, 4], [5, 6]])
        self.assertEqual(line.asWkt(), 'LineStringZ (1 2 11, 3 4 nan, 5 6 nan)')

        # array of array of 4d floats
        line = QgsLineString([[1, 2, 11, 21], [3, 4, 12, 22], [5, 6, 13, 23]])
        self.assertEqual(line.asWkt(), 'LineStringZM (1 2 11 21, 3 4 12 22, 5 6 13 23)')

        # array of array of inconsistent 4d floats
        line = QgsLineString([[1, 2, 11, 21], [3, 4, 12], [5, 6]])
        self.assertEqual(line.asWkt(), 'LineStringZM (1 2 11 21, 3 4 12 nan, 5 6 nan nan)')

        # array of array of 5 floats
        with self.assertRaises(TypeError):
            line = QgsLineString([[1, 2, 11, 21, 22], [3, 4, 12, 22, 23], [5, 6, 13, 23, 24]])

        # mixed array, because hey, why not?? :D
        line = QgsLineString([QgsPoint(1, 2), QgsPointXY(3, 4), [5, 6], (7, 8)])
        self.assertEqual(line.asWkt(), 'LineString (1 2, 3 4, 5 6, 7 8)')

    def testGeometryCollectionPythonAdditions(self):
        """
        Tests Python specific additions to the QgsGeometryCollection API
        """
        g = QgsGeometryCollection()
        self.assertTrue(bool(g))
        self.assertEqual(len(g), 0)
        g = QgsMultiPoint()
        g.fromWkt('MultiPoint( (1  2), (11 12))')
        self.assertTrue(bool(g))
        self.assertEqual(len(g), 2)

        # geometryN
        with self.assertRaises(IndexError):
            g.geometryN(-1)
        with self.assertRaises(IndexError):
            g.geometryN(2)
        self.assertEqual(g.geometryN(0), QgsPoint(1, 2))
        self.assertEqual(g.geometryN(1), QgsPoint(11, 12))

        # pointN
        with self.assertRaises(IndexError):
            g.pointN(-1)
        with self.assertRaises(IndexError):
            g.pointN(2)
        self.assertEqual(g.pointN(0), QgsPoint(1, 2))
        self.assertEqual(g.pointN(1), QgsPoint(11, 12))

        # removeGeometry
        g = QgsGeometryCollection()
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

        g = QgsGeometryCollection()
        g.fromWkt('GeometryCollection( Point(1  2), Point(11 12))')
        self.assertTrue(bool(g))
        self.assertEqual(len(g), 2)

        # lineStringN
        g = QgsMultiLineString()
        g.fromWkt('MultiLineString( (1  2, 3 4), (11 12, 13 14))')
        with self.assertRaises(IndexError):
            g.lineStringN(-1)
        with self.assertRaises(IndexError):
            g.lineStringN(2)
        self.assertEqual(g.lineStringN(0).asWkt(), 'LineString (1 2, 3 4)')
        self.assertEqual(g.lineStringN(1).asWkt(), 'LineString (11 12, 13 14)')

        # curveN
        g = QgsMultiCurve()
        g.fromWkt('MultiCurve( LineString(1  2, 3 4), LineString(11 12, 13 14))')
        with self.assertRaises(IndexError):
            g.curveN(-1)
        with self.assertRaises(IndexError):
            g.curveN(2)
        self.assertEqual(g.curveN(0).asWkt(), 'LineString (1 2, 3 4)')
        self.assertEqual(g.curveN(1).asWkt(), 'LineString (11 12, 13 14)')

        # polygonN
        g = QgsMultiPolygon()
        g.fromWkt('MultiPolygon( ((1  2, 3 4, 3 6, 1 2)), ((11 12, 13 14, 13 16, 11 12)))')
        with self.assertRaises(IndexError):
            g.polygonN(-1)
        with self.assertRaises(IndexError):
            g.polygonN(2)
        self.assertEqual(g.polygonN(0).asWkt(), 'Polygon ((1 2, 3 4, 3 6, 1 2))')
        self.assertEqual(g.polygonN(1).asWkt(), 'Polygon ((11 12, 13 14, 13 16, 11 12))')

        # surfaceN
        g = QgsMultiSurface()
        g.fromWkt('MultiSurface( Polygon((1  2, 3 4, 3 6, 1 2)), Polygon((11 12, 13 14, 13 16, 11 12)))')
        with self.assertRaises(IndexError):
            g.surfaceN(-1)
        with self.assertRaises(IndexError):
            g.surfaceN(2)
        self.assertEqual(g.surfaceN(0).asWkt(), 'Polygon ((1 2, 3 4, 3 6, 1 2))')
        self.assertEqual(g.surfaceN(1).asWkt(), 'Polygon ((11 12, 13 14, 13 16, 11 12))')

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

        g.fromWkt(
            'Polygon((0 0, 1 0, 1 1, 0 0),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.1),(0.8 0.8, 0.9 0.8, 0.9 0.9, 0.8 0.8))')
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
        # multipoint with single point should work too!
        self.assertEqual(QgsGeometry.fromWkt('MultiPoint(11 13)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZ(11 13 14)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('MultiPointM(11 13 14)').asPoint(), QgsPointXY(11, 13))
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZM(11 13 14 15)').asPoint(), QgsPointXY(11, 13))
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('LineString(11 13,14 15)').asPoint()
        with self.assertRaises(TypeError):
            QgsGeometry.fromWkt('Polygon((11 13,14 15, 14 13, 11 13))').asPoint()
        with self.assertRaises(ValueError):
            QgsGeometry().asPoint()

        # as polyline
        self.assertEqual(QgsGeometry.fromWkt('LineString(11 13,14 15)').asPolyline(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringZ(11 13 1,14 15 2)').asPolyline(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringM(11 13 1,14 15 2)').asPolyline(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('LineStringZM(11 13 1 2,14 15 3 4)').asPolyline(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
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
        self.assertEqual(
            QgsGeometry.fromWkt('PolygonZM((11 13 1 11,14 15 2 12 , 11 15 3 13 , 11 13 1 11))').asPolygon(),
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
        self.assertEqual(QgsGeometry.fromWkt('MultiPoint(11 13,14 15)').asMultiPoint(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZ(11 13 1,14 15 2)').asMultiPoint(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointM(11 13 1,14 15 2)').asMultiPoint(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
        self.assertEqual(QgsGeometry.fromWkt('MultiPointZM(11 13 1 2,14 15 3 4)').asMultiPoint(),
                         [QgsPointXY(11, 13), QgsPointXY(14, 15)])
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
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiLineStringZM((11 13 1 11,14 15 2 12 , 11 15 3 13 , 11 13 1 11))').asMultiPolyline(),
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
                    assert not geom, "Corrupt WKT {} was incorrectly converted to geometry:\n{}\n".format(i + 1,
                                                                                                          row['wkt'])
                    continue

                # test exporting to WKT results in expected string
                result = geom.asWkt()
                exp = row['valid_wkt']
                assert compareWkt(result, exp,
                                  0.000001), "WKT conversion {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp,
                                                                                                            result)

                # test num points in geometry
                exp_nodes = int(row['num_points'])
                self.assertEqual(geom.constGet().nCoordinates(), exp_nodes,
                                 "Node count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_nodes,
                                                                                            geom.constGet().nCoordinates()))

                # test num geometries in collections
                exp_geometries = int(row['num_geometries'])
                try:
                    self.assertEqual(geom.constGet().numGeometries(), exp_geometries,
                                     "Geometry count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1,
                                                                                                    exp_geometries,
                                                                                                    geom.constGet().numGeometries()))
                except:
                    # some geometry types don't have numGeometries()
                    assert exp_geometries <= 1, "Geometry count {}:  Expected:\n{} geometries but could not call numGeometries()\n".format(
                        i + 1, exp_geometries)

                # test count of rings
                exp_rings = int(row['num_rings'])
                try:
                    self.assertEqual(geom.constGet().numInteriorRings(), exp_rings,
                                     "Ring count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_rings,
                                                                                                geom.constGet().numInteriorRings()))
                except:
                    # some geometry types don't have numInteriorRings()
                    assert exp_rings <= 1, "Ring count {}:  Expected:\n{} rings but could not call numInteriorRings()\n{}".format(
                        i + 1, exp_rings, geom.constGet())

                # test isClosed
                exp = (row['is_closed'] == '1')
                try:
                    self.assertEqual(geom.constGet().isClosed(), exp,
                                     "isClosed {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, True,
                                                                                              geom.constGet().isClosed()))
                except:
                    # some geometry types don't have isClosed()
                    assert not exp, "isClosed {}:  Expected:\n isClosed() but could not call isClosed()\n".format(i + 1)

                # test geometry centroid
                exp = row['centroid']
                result = geom.centroid().asWkt()
                assert compareWkt(result, exp, 0.00001), "Centroid {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1,
                                                                                                                  exp,
                                                                                                                  result)

                # test bounding box limits
                bbox = geom.constGet().boundingBox()
                exp = float(row['x_min'])
                result = bbox.xMinimum()
                self.assertAlmostEqual(result, exp, 5,
                                       "Min X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['y_min'])
                result = bbox.yMinimum()
                self.assertAlmostEqual(result, exp, 5,
                                       "Min Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['x_max'])
                result = bbox.xMaximum()
                self.assertAlmostEqual(result, exp, 5,
                                       "Max X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                exp = float(row['y_max'])
                result = bbox.yMaximum()
                self.assertAlmostEqual(result, exp, 5,
                                       "Max Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test area calculation
                exp = float(row['area'])
                result = geom.constGet().area()
                self.assertAlmostEqual(result, exp, 5,
                                       "Area {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                result = geom.area()
                self.assertAlmostEqual(result, exp, 5,
                                       "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test length calculation
                exp = float(row['length'])
                result = geom.constGet().length()
                self.assertAlmostEqual(result, exp, 5,
                                       "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                if geom.type() != QgsWkbTypes.PolygonGeometry:
                    result = geom.length()
                    self.assertAlmostEqual(result, exp, 5,
                                           "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

                # test perimeter calculation
                exp = float(row['perimeter'])
                result = geom.constGet().perimeter()
                self.assertAlmostEqual(result, exp, 5,
                                       "Perimeter {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))
                if geom.type() == QgsWkbTypes.PolygonGeometry:
                    result = geom.length()
                    self.assertAlmostEqual(result, exp, 5,
                                           "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result))

    def testCollection(self):
        g = QgsGeometry.fromWkt('MultiLineString EMPTY')
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
        https://github.com/qgis/QGIS/issues/14164

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

                if self.geos39:
                    myExpectedWkt = 'Polygon ((20 30, 30 30, 30 20, 20 20, 20 30))'
                else:
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
            [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1),
             QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
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
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                QgsPointXY(0, 2), QgsPointXY(0, 0),
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
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                  QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
                  QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
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

        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPointXY())
        self.assertTrue(point.isEmpty())
        self.assertEqual(dist, -1)

        (point, atVertex, beforeVertex, afterVertex, dist) = QgsGeometry().closestVertex(QgsPointXY(42, 42))
        self.assertTrue(point.isEmpty())

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
            [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1), QgsPointXY(1, 1),
             QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
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
                self.assertEqual(before == -1 and after, i + 1,
                                 "Expected (-1,%d), Got:(%d,%d)" % (i + 1, before, after))
            elif i == 4 or i == 9:
                self.assertEqual(before == i - 1 and after, -1,
                                 "Expected (%d,-1), Got:(%d,%d)" % (i - 1, before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1,
                                 "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polyline.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygonXY(
            [[
                QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                QgsPointXY(0, 2), QgsPointXY(0, 0),
            ]]
        )

        (before, after) = polygon.adjacentVertices(-100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        for i in range(0, 7):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 6:
                self.assertEqual(before == 5 and after, 1, "Expected (5,1), Got:(%d,%d)" % (before, after))
            else:
                self.assertEqual(before == i - 1 and after, i + 1,
                                 "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

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
                self.assertEqual(before == i - 1 and after, i + 1,
                                 "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

        (before, after) = polygon.adjacentVertices(100)
        self.assertEqual(before == -1 and after, -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after))

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygonXY(
            [
                [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
                  QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
                [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
                  QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
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
                self.assertEqual(before == i - 1 and after, i + 1,
                                 "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after))

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
        points = [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1),
                  QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
        polyline = QgsGeometry.fromPolylineXY(points)

        for i in range(0, len(points)):
            # WORKAROUND to avoid a system error
            # self.assertEqual(QgsPoint(points[i]), polyline.vertexAt(i), "Mismatch at %d" % i)
            self.assertEqual(QgsPoint(points[i].x(), points[i].y()), polyline.vertexAt(i), "Mismatch at %d" % i)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
            [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolylineXY(points)

        p = polyline.vertexAt(-100)
        self.assertEqual(p, QgsPoint(math.nan, math.nan), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polyline.vertexAt(100)
        self.assertEqual(p, QgsPoint(math.nan, math.nan), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                # WORKAROUND
                # self.assertEqual(QgsPoint(points[j][k]), polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                pt = points[j][k]
                self.assertEqual(QgsPoint(pt.x(), pt.y()), polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2),
            QgsPointXY(0, 0),
        ]]
        polygon = QgsGeometry.fromPolygonXY(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                # WORKAROUND
                # self.assertEqual(QgsPoint(points[j][k]), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                pt = points[j][k]
                self.assertEqual(QgsPoint(pt.x(), pt.y()), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
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
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                # WORKAROUND
                # self.assertEqual(QgsPoint(points[j][k]), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                pt = points[j][k]
                self.assertEqual(QgsPoint(pt.x(), pt.y()), polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
              QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
              QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
        ]

        polygon = QgsGeometry.fromMultiPolygonXY(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(), "Expected 0,0, Got {}.{}".format(p.x(), p.y()))

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                for l in range(0, len(points[j][k])):
                    p = polygon.vertexAt(i)
                    # WORKAROUND
                    # self.assertEqual(QgsPoint(points[j][k][l]), p, "Got {},{} Expected {} at {} / {},{},{}".format(p.x(), p.y(), points[j][k][l].toString(), i, j, k, l))
                    pt = points[j][k][l]
                    self.assertEqual(QgsPoint(pt.x(), pt.y()), p,
                                     "Got {},{} Expected {} at {} / {},{},{}".format(p.x(), p.y(), pt.toString(), i, j,
                                                                                     k, l))
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
        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")

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
        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
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

        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
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
        polygon = QgsGeometry.fromWkt(
            "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1),(5 1, 6 1, 6 2, 5 2, 5 1),(7 1, 8 1, 8 2, 7 2, 7 1))")
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

        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.insertVertex(0, 0, 8), "Insert vertex 0 0 at 8 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 0 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
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

        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
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
        ct = QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'),
                                    QgsCoordinateReferenceSystem('EPSG:3857'),
                                    QgsProject.instance())

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

        polygon = QgsGeometry.fromWkt(
            "MultiPolygon (((0 0, 111319 0, 111319 111325, 222638 111325, 222638 222684, 0 222684, 0 0)),((445277 0, 556597 0, 556597 222684, 333958 222684, 333958 111325, 445277 111325, 445277 0)))")
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

        # trivial point case
        expWkt = 'Point (3 4)'
        wkt = QgsGeometry.fromWkt('Point(3 4)').nearestPoint(QgsGeometry.fromWkt('Point(-1 -8)')).asWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        wkt = QgsGeometry.fromWkt('Point(3 4)').nearestPoint(QgsGeometry.fromWkt('LineString( 1 1, 5 1, 5 5 )')).asWkt()
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

        # trivial point to point case
        expWkt = 'LineString (3 4, -1 -8)'
        wkt = QgsGeometry.fromWkt('Point(3 4)').shortestLine(QgsGeometry.fromWkt('Point(-1 -8)')).asWkt()
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
        points = [QgsPointXY(5, 0), QgsPointXY(0, 0), QgsPointXY(0, 4), QgsPointXY(5, 4), QgsPointXY(5, 1),
                  QgsPointXY(1, 1), QgsPointXY(1, 3), QgsPointXY(4, 3), QgsPointXY(4, 2), QgsPointXY(2, 2)]
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
            QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2), QgsPointXY(0, 2),
            QgsPointXY(0, 0),
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
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
              QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
              QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
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

        # collect some geometries which are already multipart
        geometries = [QgsGeometry.fromWkt('LineString( 0 0, 1 1)'),
                      QgsGeometry.fromWkt('MultiLineString((2 2, 3 3),(4 4, 5 5))')]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiLineString ((0 0, 1 1),(2 2, 3 3),(4 4, 5 5))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        geometries = [QgsGeometry.fromWkt('MultiLineString((2 2, 3 3),(4 4, 5 5))'),
                      QgsGeometry.fromWkt('LineString( 0 0, 1 1)')]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiLineString ((2 2, 3 3),(4 4, 5 5),(0 0, 1 1))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        geometries = [QgsGeometry.fromWkt('Polygon((100 100, 101 100, 101 101, 100 100))'),
                      QgsGeometry.fromWkt('MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((2 0, 3 0, 3 1, 2 1, 2 0)))')]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiPolygon (((100 100, 101 100, 101 101, 100 100)),((0 0, 1 0, 1 1, 0 1, 0 0)),((2 0, 3 0, 3 1, 2 1, 2 0)))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        geometries = [QgsGeometry.fromWkt('MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((2 0, 3 0, 3 1, 2 1, 2 0)))'),
                      QgsGeometry.fromWkt('Polygon((100 100, 101 100, 101 101, 100 100))')]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((2 0, 3 0, 3 1, 2 1, 2 0)),((100 100, 101 100, 101 101, 100 100)))"
        wkt = geometry.asWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        geometries = [QgsGeometry(QgsTriangle(QgsPoint(0, 0, 5), QgsPoint(1, 0, 6), QgsPoint(1, 1, 7))),
                      QgsGeometry(QgsTriangle(QgsPoint(100, 100, 9), QgsPoint(101, 100, -1), QgsPoint(101, 101, 4)))]
        geometry = QgsGeometry.collectGeometry(geometries)
        expwkt = "MultiPolygonZ (((0 0 5, 1 0 6, 1 1 7, 0 0 5)),((100 100 9, 101 100 -1, 101 101 4, 100 100 9)))"
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
        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        line_points = [
            [QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 0), ],
            [QgsPointXY(3, 0), QgsPointXY(3, 1), QgsPointXY(5, 1), QgsPointXY(5, 0), QgsPointXY(6, 0), ]
        ]
        def polyline1_geom(): return QgsGeometry.fromPolylineXY(line_points[0]) # noqa: E704,E261
        def polyline2_geom(): return QgsGeometry.fromPolylineXY(line_points[1]) # noqa: E704,E261

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        poly_points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
              QgsPointXY(0, 2), QgsPointXY(0, 0), ]],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
              QgsPointXY(4, 1), QgsPointXY(4, 0), ]]
        ]
        def polygon1_geom(): return QgsGeometry.fromPolygonXY(poly_points[0]) # noqa: E704,E261
        def polygon2_geom(): return QgsGeometry.fromPolygonXY(poly_points[1]) # noqa: E704,E261
        def multi_polygon_geom(): return QgsGeometry.fromMultiPolygonXY(poly_points) # noqa: E704,E261
        def multi_polygon1_geom(): return QgsGeometry.fromMultiPolygonXY(poly_points[:1]) # noqa: E704,E261
        def multi_polygon2_geom(): return QgsGeometry.fromMultiPolygonXY(poly_points[1:]) # noqa: E704,E261

        def multi_surface_geom():
            ms = QgsMultiSurface()
            p = polygon1_geom()
            ms.addGeometry(p.constGet().clone())
            return QgsGeometry(ms)

        def curve():
            cs = QgsCircularString()
            cs.setPoints([QgsPoint(31, 32), QgsPoint(34, 36), QgsPoint(37, 39)])
            return cs.toCurveType()

        circle = QgsCircle(QgsPoint(10, 10), 5)

        def circle_polygon():
            p = QgsPolygon()
            p.setExteriorRing(circle.toCircularString())
            return p

        def circle_curvepolygon():
            p = QgsCurvePolygon()
            p.setExteriorRing(circle.toCircularString())
            return p

        geoms = {}  # initial geometry
        parts = {}  # part to add
        expec = {}  # expected WKT result
        types = {}  # optional geometry types for points added
        resul = {}  # expected GeometryOperationResult

        T = 'point_add_point'
        geoms[T] = QgsGeometry.fromPointXY(QgsPointXY(0, 0))
        parts[T] = [QgsPointXY(1, 0)]
        expec[T] = "MultiPoint ((0 0), (1 0))"

        T = 'point_add_point_with_Z'
        geoms[T] = QgsGeometry(QgsPoint(0, 0, 4))
        parts[T] = [QgsPoint(1, 0, 3, wkbType=QgsWkbTypes.PointZ)]
        expec[T] = "MultiPointZ ((0 0 4), (1 0 3))"

        T = 'line_add_1_point_fails'
        geoms[T] = polyline1_geom()
        parts[T] = line_points[1][0:1]
        resul[T] = QgsGeometry.InvalidInputGeometryType

        T = 'line_add_2_point'
        geoms[T] = polyline1_geom()
        parts[T] = line_points[1][0:2]
        expec[T] = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1))"

        T = 'add_point_with_more_points'
        geoms[T] = polyline1_geom()
        parts[T] = line_points[1]
        expec[T] = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1, 5 1, 5 0, 6 0))"

        T = 'line_add_points_with_Z'
        geoms[T] = polyline1_geom()
        geoms[T].get().addZValue(4.0)
        parts[T] = [QgsPoint(p[0], p[1], 3.0, wkbType=QgsWkbTypes.PointZ) for p in line_points[1]]
        expec[T] = "MultiLineStringZ ((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 0 4),(3 0 3, 3 1 3, 5 1 3, 5 0 3, 6 0 3))"

        T = 'linestring_add_curve'
        geoms[T] = polyline1_geom()
        parts[T] = curve()
        expec[T] = 'MultiLineString ({},{})'.format(polyline1_geom().asWkt()[len('LineString '):], curve().curveToLine().asWkt()[len('LineString '):])

        T = 'polygon_add_ring_1_point'
        geoms[T] = polygon1_geom()
        parts[T] = poly_points[1][0][0:1]
        resul[T] = QgsGeometry.InvalidInputGeometryType

        T = 'polygon_add_ring_2_points'
        geoms[T] = polygon1_geom()
        parts[T] = poly_points[1][0][0:2]
        resul[T] = QgsGeometry.InvalidInputGeometryType

        T = 'polygon_add_ring_3_points'
        geoms[T] = polygon1_geom()
        parts[T] = poly_points[1][0][0:3]
        resul[T] = QgsGeometry.InvalidInputGeometryType

        T = 'polygon_add_ring_3_points_closed'
        geoms[T] = polygon1_geom()
        parts[T] = [QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(4, 0)]
        resul[T] = QgsGeometry.InvalidInputGeometryType

        T = 'polygon_add_polygon'
        geoms[T] = polygon1_geom()
        parts[T] = poly_points[1][0]
        expec[T] = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"

        T = 'multipolygon_add_polygon'
        geoms[T] = multi_polygon1_geom()
        parts[T] = polygon2_geom()
        expec[T] = multi_polygon_geom().asWkt()

        T = 'multipolygon_add_multipolygon'
        geoms[T] = multi_polygon1_geom()
        parts[T] = multi_polygon2_geom()
        expec[T] = multi_polygon_geom().asWkt()

        T = 'polygon_add_point_with_Z'
        geoms[T] = polygon1_geom()
        geoms[T].get().addZValue(4.0)
        parts[T] = [QgsPoint(pi[0], pi[1], 3.0, wkbType=QgsWkbTypes.PointZ) for pi in poly_points[1][0]]
        expec[T] = "MultiPolygonZ (((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 2 4, 0 2 4, 0 0 4)),((4 0 3, 5 0 3, 5 2 3, 3 2 3, 3 1 3, 4 1 3, 4 0 3)))"

        T = 'multisurface_add_curvepolygon'
        geoms[T] = QgsGeometry.fromWkt('MultiSurface(((0 0,0 1,1 1,0 0)))')
        parts[T] = QgsGeometry.fromWkt('CurvePolygon ((0 0,0 1,1 1,0 0))')
        expec[T] = 'MultiSurface (Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))'

        T = 'multisurface_add_multisurface'
        geoms[T] = QgsGeometry.fromWkt('MultiSurface(((20 0,20 1,21 1,20 0)))')
        parts[T] = QgsGeometry.fromWkt('MultiSurface (Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))')
        expec[T] = 'MultiSurface (Polygon ((20 0, 20 1, 21 1, 20 0)),Polygon ((0 0, 0 1, 1 1, 0 0)),CurvePolygon ((0 0, 0 1, 1 1, 0 0)))'

        T = 'empty_geom_add_point_with_no_default_type'
        # if not default type specified, addPart should fail
        geoms[T] = QgsGeometry()
        parts[T] = [QgsPointXY(4, 0)]
        resul[T] = Qgis.GeometryOperationResult.AddPartNotMultiGeometry

        T = 'empty_geom_add_point'
        geoms[T] = QgsGeometry()
        parts[T] = [QgsPointXY(4, 0)]
        types[T] = QgsWkbTypes.PointGeometry
        expec[T] = 'MultiPoint ((4 0))'

        T = 'empty_geom_add_line'
        geoms[T] = QgsGeometry()
        parts[T] = poly_points[0][0]
        types[T] = QgsWkbTypes.LineGeometry
        expec[T] = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))'

        T = 'empty_geom_add_polygon'
        geoms[T] = QgsGeometry()
        parts[T] = poly_points[0][0]
        types[T] = QgsWkbTypes.PolygonGeometry
        expec[T] = 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))'

        T = 'multipolygon_add_curvepolygon'
        geoms[T] = multi_polygon1_geom()
        parts[T] = circle_curvepolygon()
        expec[T] = 'MultiPolygon ({},{})'.format(polygon1_geom().asWkt()[len('Polygon '):], circle_polygon().asWkt()[len('Polygon '):])

        T = 'multisurface_add_curvepolygon'
        geoms[T] = multi_surface_geom()
        parts[T] = circle_curvepolygon()
        expec[T] = 'MultiSurface (Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),CurvePolygon (CircularString (10 15, 15 10, 10 5, 5 10, 10 15)))'

        for t in parts.keys():
            with self.subTest(t=t):
                expected_result = resul.get(t, Qgis.GeometryOperationResult.Success)
                geom_type = types.get(t, QgsWkbTypes.UnknownGeometry)
                message = '\n' + t
                if expected_result != Qgis.Success:
                    message += ' unexpectedly succeeded'
                else:
                    message += ' failed'
                message_with_wkt = message + '\nOriginal geom: {}'.format(geoms[t].asWkt())
                if type(parts[t]) is list:
                    if type(parts[t][0]) == QgsPointXY:
                        self.assertEqual(geoms[t].addPointsXY(parts[t], geom_type), expected_result, message_with_wkt)
                    elif type(parts[t][0]) == QgsPoint:
                        self.assertEqual(geoms[t].addPoints(parts[t]), expected_result, message_with_wkt)
                    else:
                        self.fail(message_with_wkt + '\n could not detect what Python method to use for add part')
                else:
                    if type(parts[t]) == QgsGeometry:
                        self.assertEqual(geoms[t].addPartGeometry(parts[t]), expected_result, message)
                    else:
                        self.assertEqual(geoms[t].addPart(parts[t], geom_type), expected_result, message_with_wkt)

                if expected_result == Qgis.GeometryOperationResult.Success:
                    wkt = geoms[t].asWkt()
                    assert compareWkt(expec[t], wkt), message + "\nExpected:\n%s\nGot:\n%s\n" % (expec[t], wkt)

    def testConvertToType(self):
        # 5-+-4 0-+-9  13-+-+-12
        # |   | |   |  |       |
        # | 2-3 1-2 |  + 18-17 +
        # | |     | |  | |   | |
        # 0-1     7-8  + 15-16 +
        #              |       |
        #              10-+-+-11
        points = [
            [[QgsPointXY(0, 0), QgsPointXY(1, 0), QgsPointXY(1, 1), QgsPointXY(2, 1), QgsPointXY(2, 2),
              QgsPointXY(0, 2), QgsPointXY(0, 0)], ],
            [[QgsPointXY(4, 0), QgsPointXY(5, 0), QgsPointXY(5, 2), QgsPointXY(3, 2), QgsPointXY(3, 1),
              QgsPointXY(4, 1), QgsPointXY(4, 0)], ],
            [[QgsPointXY(10, 0), QgsPointXY(13, 0), QgsPointXY(13, 3), QgsPointXY(10, 3), QgsPointXY(10, 0)],
             [QgsPointXY(11, 1), QgsPointXY(12, 1), QgsPointXY(12, 2), QgsPointXY(11, 2), QgsPointXY(11, 1)]]
        ]
        # ####### TO POINT ########
        # POINT TO POINT
        point = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, False).asWkt()
        expWkt = "Point (1 1)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to point. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # POINT TO MultiPoint
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((1 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to multipoint. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # LINE TO MultiPoint
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipoint. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MULTILINE TO MultiPoint
        multiLine = QgsGeometry.fromMultiPolylineXY(points[2])
        wkt = multiLine.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multiline to multipoint. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # Polygon TO MultiPoint
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from poylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPolygon TO MultiPoint
        multiPolygon = QgsGeometry.fromMultiPolygonXY(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.PointGeometry, True).asWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0),(4 0),(5 0),(5 2),(3 2),(3 1),(4 1),(4 0),(10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipoylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # ####### TO LINE ########
        # POINT TO LINE
        point = QgsGeometry.fromPointXY(QgsPointXY(1, 1))
        self.assertFalse(point.convertToType(QgsWkbTypes.LineGeometry,
                                             False)), "convertToType with a point should return a null geometry"
        # MultiPoint TO LINE
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to line. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPoint TO MULTILINE
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipoint to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MULTILINE (which has a single part) TO LINE
        multiLine = QgsGeometry.fromMultiPolylineXY(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to line. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # LINE TO MULTILINE
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # Polygon TO LINE
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to line. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # Polygon TO MULTILINE
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # Polygon with ring TO MULTILINE
        polygon = QgsGeometry.fromPolygonXY(points[2])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from polygon with ring to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPolygon (which has a single part) TO LINE
        multiPolygon = QgsGeometry.fromMultiPolygonXY([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, False).asWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPolygon TO MULTILINE
        multiPolygon = QgsGeometry.fromMultiPolygonXY(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, True).asWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0), (4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0), (10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # ####### TO Polygon ########
        # MultiPoint TO Polygon
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipoint to polygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPoint TO MultiPolygon
        multipoint = QgsGeometry.fromMultiPointXY(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multipoint to multipolygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # LINE TO Polygon
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to polygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # LINE ( 3 vertices, with first = last ) TO Polygon
        line = QgsGeometry.fromPolylineXY([QgsPointXY(1, 1), QgsPointXY(0, 0), QgsPointXY(1, 1)])
        self.assertFalse(line.convertToType(QgsWkbTypes.PolygonGeometry, False),
                         "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # MULTILINE ( with a part of 3 vertices, with first = last ) TO MultiPolygon
        multiline = QgsGeometry.fromMultiPolylineXY(
            [points[0][0], [QgsPointXY(1, 1), QgsPointXY(0, 0), QgsPointXY(1, 1)]])
        self.assertFalse(multiline.convertToType(QgsWkbTypes.PolygonGeometry, True),
                         "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # LINE TO MultiPolygon
        line = QgsGeometry.fromPolylineXY(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipolygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MULTILINE (which has a single part) TO Polygon
        multiLine = QgsGeometry.fromMultiPolylineXY(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MULTILINE TO MultiPolygon
        multiLine = QgsGeometry.fromMultiPolylineXY([points[0][0], points[1][0]])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from multiline to multipolygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # Polygon TO MultiPolygon
        polygon = QgsGeometry.fromPolygonXY(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PolygonGeometry, True).asWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt,
                          wkt), "convertToType failed: from polygon to multipolygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # MultiPolygon (which has a single part) TO Polygon
        multiPolygon = QgsGeometry.fromMultiPolygonXY([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.PolygonGeometry, False).asWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

    def testRegression13053(self):
        """ See https://github.com/qgis/QGIS/issues/21125 """
        p = QgsGeometry.fromWkt(
            'MULTIPOLYGON(((62.0 18.0, 62.0 19.0, 63.0 19.0, 63.0 18.0, 62.0 18.0)), ((63.0 19.0, 63.0 20.0, 64.0 20.0, 64.0 19.0, 63.0 19.0)))')
        assert p is not None

        expWkt = 'MultiPolygon (((62 18, 62 19, 63 19, 63 18, 62 18)),((63 19, 63 20, 64 20, 64 19, 63 19)))'
        wkt = p.asWkt()
        assert compareWkt(expWkt, wkt), "testRegression13053 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13055(self):
        """ See https://github.com/qgis/QGIS/issues/21127
            Testing that invalid WKT with z values but not using PolygonZ is still parsed
            by QGIS.
        """
        p = QgsGeometry.fromWkt('Polygon((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))')
        assert p is not None

        expWkt = 'PolygonZ ((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))'
        wkt = p.asWkt()
        assert compareWkt(expWkt, wkt), "testRegression13055 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13274(self):
        """ See https://github.com/qgis/QGIS/issues/21334
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

        # no overlap
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1, 6 1, 6 0, 7 0)')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(4, 2), QgsPoint(7, 2)])), 0)
        expWkt = 'LineString (0 0, 5 0, 5 1, 6 1, 6 0, 7 0)'
        wkt = g.asWkt()
        self.assertTrue(compareWkt(expWkt, wkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        g.reshapeGeometry(QgsLineString([QgsPoint(0, 1.5), QgsPoint(1.5, 0)]))
        expWkt = 'Polygon ((0.5 1, 0 1, 0 0, 1 0, 1 0.5, 0.5 1))'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a geometry involving the first/last vertex (https://github.com/qgis/QGIS/issues/22422)
        g.reshapeGeometry(QgsLineString([QgsPoint(0.5, 1), QgsPoint(0, 0.5)]))

        expWkt = 'Polygon ((0 0.5, 0 0, 1 0, 1 0.5, 0.5 1, 0 0.5))'
        wkt = g.asWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a polygon with a line starting or ending at the polygon's first vertex, no change expexted
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        expWkt = g.asWkt()
        g.reshapeGeometry(QgsLineString([QgsPoint(0, 0), QgsPoint(-1, -1)]))
        self.assertTrue(compareWkt(g.asWkt(), expWkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        # Test reshape a polygon with a line starting or ending at the polygon's first vertex
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(0, 0), QgsPoint(0.5, 0.5), QgsPoint(0, 1)])),
                         QgsGeometry.Success)
        expWkt = 'Polygon ((0 0, 1 0, 1 1, 0 1, 0.5 0.5, 0 0))'
        wkt = g.asWkt()
        self.assertTrue(compareWkt(wkt, expWkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

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

        # reshape where reshape line exactly overlaps some portions of geometry
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1, 6 1, 6 0, 7 0)')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(2, 0), QgsPoint(6, 0)])), 0)
        expWkt = 'LineString (0 0, 2 0, 5 0, 6 0, 7 0)'
        wkt = g.asWkt()
        self.assertTrue(compareWkt(expWkt, wkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1, 6 1, 6 0, 7 0)')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(5, 0), QgsPoint(7, 0)])), 0)
        expWkt = 'LineString (0 0, 5 0, 6 0, 7 0)'
        wkt = g.asWkt()
        self.assertTrue(compareWkt(expWkt, wkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        # reshape line overlaps at both start and end
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1, 6 1, 6 0, 7 0)')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(4, 0), QgsPoint(7, 0)])), 0)
        expWkt = 'LineString (0 0, 4 0, 5 0, 6 0, 7 0)'
        wkt = g.asWkt()
        self.assertTrue(compareWkt(expWkt, wkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        # test that tolerance is correctly handled
        g = QgsGeometry.fromWkt(
            'LineString(152.96370660521466789 -25.60915858374441356, 152.96370887800003402 -25.60912889999996978, 152.9640088780000724 -25.60858889999996535, 152.96423077601289719 -25.60858080133134962, 152.96423675797717578 -25.60854355430449658, 152.96427575123991005 -25.60857916087011432, 152.96537884400004259 -25.60853889999992106, 152.96576355343805176 -25.60880035169972047)')
        self.assertEqual(g.reshapeGeometry(QgsLineString([QgsPoint(152.9634281, -25.6079985),
                                                          QgsPoint(152.9640088780000724, -25.60858889999996535),
                                                          QgsPoint(152.96537884400004259, -25.60853889999992106),
                                                          QgsPoint(152.9655739, -25.6083169)])), 0)
        expWkt = 'LineString (152.96371 -25.60916, 152.96371 -25.60913, 152.96401 -25.60859, 152.96423 -25.60858, 152.96423 -25.60858, 152.96428 -25.60858, 152.96538 -25.60854, 152.96576 -25.6088)'
        wkt = g.asWkt(5)
        self.assertTrue(compareWkt(expWkt, wkt),
                        "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

    def testConvertToMultiType(self):
        """ Test converting geometries to multi type """
        point = QgsGeometry.fromWkt('Point (1 2)')
        assert point.convertToMultiType()
        expWkt = 'MultiPoint ((1 2))'
        wkt = point.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of MultiPoint
        assert point.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        line = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        assert line.convertToMultiType()
        expWkt = 'MultiLineString ((1 0, 2 0))'
        wkt = line.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of MultiLineString
        assert line.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        poly = QgsGeometry.fromWkt('Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))')
        assert poly.convertToMultiType()
        expWkt = 'MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)))'
        wkt = poly.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of MultiPolygon
        assert poly.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

    def testConvertToSingleType(self):
        """ Test converting geometries to single type """
        point = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert point.convertToSingleType()
        expWkt = 'Point (1 2)'
        wkt = point.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of Point
        assert point.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        line = QgsGeometry.fromWkt('MultiLineString ((1 0, 2 0),(2 3, 4 5))')
        assert line.convertToSingleType()
        expWkt = 'LineString (1 0, 2 0)'
        wkt = line.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of LineString
        assert line.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        poly = QgsGeometry.fromWkt('MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)),((2 3,2 4, 3 4, 3 3, 2 3)))')
        assert poly.convertToSingleType()
        expWkt = 'Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))'
        wkt = poly.asWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)
        # test conversion of Polygon
        assert poly.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

    def testAddZValue(self):
        """ Test adding z dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.get().addZValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CircularStringZ)
        expWkt = 'CircularStringZ (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt(
            'CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.get().addZValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CompoundCurveZ)
        expWkt = 'CompoundCurveZ ((5 3 2, 5 13 2),CircularStringZ (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringZ (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.get().addZValue(3)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PolygonZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PolygonZ)
        expWkt = 'PolygonZ ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.get().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.MultiPointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPointZ)
        expWkt = 'MultiPointZ ((1 2 4),(2 3 4))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.get().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.LineStringZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.LineStringZ)
        expWkt = 'LineStringZ (1 2 4, 2 3 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.get().addZValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PointZ)
        expWkt = 'PointZ (1 2 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addZValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddMValue(self):
        """ Test adding m dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.get().addMValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CircularStringM)
        expWkt = 'CircularStringM (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt(
            'CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.get().addMValue(2)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.CompoundCurveM)
        expWkt = 'CompoundCurveM ((5 3 2, 5 13 2),CircularStringM (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringM (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.get().addMValue(3)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.PolygonM)
        expWkt = 'PolygonM ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.get().addMValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.MultiPointM)
        expWkt = 'MultiPointM ((1 2 4),(2 3 4))'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.get().addMValue(4)
        self.assertEqual(geom.constGet().wkbType(), QgsWkbTypes.LineStringM)
        expWkt = 'LineStringM (1 2 4, 2 3 4)'
        wkt = geom.asWkt()
        assert compareWkt(expWkt, wkt), "addMValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (
            expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.get().addMValue(4)
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
                self.assertEqual(result, exp,
                                 "Relates {} failed: mismatch Expected:\n{}\nGot:\n{}\nGeom1:\n{}\nGeom2:\n{}\n".format(
                                     i + 1, exp, result, test_data[0], test_data[1]))

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
        # until we have tin types, these should return multipolygons
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.Triangle), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.TriangleZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.TriangleM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.multiType(QgsWkbTypes.TriangleZM), QgsWkbTypes.MultiPolygonZM)

        # test promoteNonPointTypesToMulti method
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.LineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.LineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.LineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.LineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Polygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.PolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CircularString), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CircularStringZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CircularStringM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CircularStringZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CompoundCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CurvePolygon), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.LineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Polygon25D), QgsWkbTypes.MultiPolygon25D)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)
        # until we have tin types, these should return multipolygons
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.Triangle), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.TriangleZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.TriangleM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.promoteNonPointTypesToMulti(QgsWkbTypes.TriangleZM), QgsWkbTypes.MultiPolygonZM)

        # test curveType method
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.LineString), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.LineStringZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.LineStringM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.LineStringZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.Polygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PolygonZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PolygonM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.PolygonZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CircularString), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CircularStringM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.CompoundCurve)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.CompoundCurveM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.CompoundCurveZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.CurvePolygon)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.CurvePolygonM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.CurvePolygonZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiCurve)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiCurveM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiCurveZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiSurface)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiSurfaceZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiSurfaceM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiSurfaceZM)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.Point25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.LineString25D), QgsWkbTypes.CompoundCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.Polygon25D), QgsWkbTypes.CurvePolygonZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiCurveZ)
        self.assertEqual(QgsWkbTypes.curveType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiSurfaceZ)

        # test linearType method
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.Unknown), QgsWkbTypes.Unknown)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.Point), QgsWkbTypes.Point)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PointZ), QgsWkbTypes.PointZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PointM), QgsWkbTypes.PointM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PointZM), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPoint), QgsWkbTypes.MultiPoint)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPointZ), QgsWkbTypes.MultiPointZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPointM), QgsWkbTypes.MultiPointM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPointZM), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.LineString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.LineStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.LineStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.LineStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiLineString), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiLineStringZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiLineStringM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiLineStringZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.Polygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.PolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPolygon), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPolygonZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPolygonM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPolygonZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.GeometryCollection), QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.GeometryCollectionZ), QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.GeometryCollectionM), QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.GeometryCollectionZM), QgsWkbTypes.GeometryCollectionZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CircularString), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CircularStringZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CircularStringM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CircularStringZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CompoundCurve), QgsWkbTypes.LineString)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CompoundCurveZ), QgsWkbTypes.LineStringZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CompoundCurveM), QgsWkbTypes.LineStringM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CompoundCurveZM), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CurvePolygon), QgsWkbTypes.Polygon)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CurvePolygonZ), QgsWkbTypes.PolygonZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CurvePolygonM), QgsWkbTypes.PolygonM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.CurvePolygonZM), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiCurve), QgsWkbTypes.MultiLineString)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiCurveZ), QgsWkbTypes.MultiLineStringZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiCurveM), QgsWkbTypes.MultiLineStringM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiCurveZM), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiSurface), QgsWkbTypes.MultiPolygon)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiSurfaceZ), QgsWkbTypes.MultiPolygonZ)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiSurfaceM), QgsWkbTypes.MultiPolygonM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiSurfaceZM), QgsWkbTypes.MultiPolygonZM)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.NoGeometry), QgsWkbTypes.NoGeometry)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.Point25D), QgsWkbTypes.Point25D)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.LineString25D), QgsWkbTypes.LineString25D)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.Polygon25D), QgsWkbTypes.Polygon25D)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPoint25D)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineString25D)
        self.assertEqual(QgsWkbTypes.linearType(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygon25D)

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
        # we force upgrade 25D types to "Z" before adding the M value
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Point25D), QgsWkbTypes.PointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.LineString25D), QgsWkbTypes.LineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.Polygon25D), QgsWkbTypes.PolygonZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPoint25D), QgsWkbTypes.MultiPointZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiLineString25D), QgsWkbTypes.MultiLineStringZM)
        self.assertEqual(QgsWkbTypes.addM(QgsWkbTypes.MultiPolygon25D), QgsWkbTypes.MultiPolygonZM)

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

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, False, False),
                         QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, True, False),
                         QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, False, True),
                         QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollection, True, True),
                         QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, False, False),
                         QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, True, False),
                         QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, False, True),
                         QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZ, True, True),
                         QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, False, False),
                         QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, True, False),
                         QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, False, True),
                         QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionM, True, True),
                         QgsWkbTypes.GeometryCollectionZM)

        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, False, False),
                         QgsWkbTypes.GeometryCollection)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, True, False),
                         QgsWkbTypes.GeometryCollectionZ)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, False, True),
                         QgsWkbTypes.GeometryCollectionM)
        self.assertEqual(QgsWkbTypes.zmType(QgsWkbTypes.GeometryCollectionZM, True, True),
                         QgsWkbTypes.GeometryCollectionZM)

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

        wkt = "CircularString (0 0,1 1,2 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.asWkt(), QgsCircularString().asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CircularString (2 0, 3 -1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CircularString (0 0, 3 -1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CircularString (0 0,1 1,2 0)"
        self.assertEqual(geom.asWkt(), QgsGeometry.fromWkt(expected_wkt).asWkt())

        wkt = "CircularString (0 0,1 1,2 0,3 -1,4 0)"
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

    def testConvertVertex(self):

        # WKT BEFORE -> WKT AFTER A CONVERT ON POINT AT 10,10
        test_setup = {
            # Curve
            'LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0)': 'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))',
            'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))': 'COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0))',
            'COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0))': 'COMPOUNDCURVE((0 0, 10 0, 10 10, 0 10, 0 0))',

            # Multicurve
            'MULTICURVE(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))',
            'MULTICURVE(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))',
            'MULTICURVE(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))': 'MULTICURVE(COMPOUNDCURVE((0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(5 15, 10 20, 0 20, 5 15))',

            # Polygon
            'CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))': 'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3))',
            'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3)))',
            'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))': 'CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3)))',

            # Multipolygon
            'MULTISURFACE(CURVEPOLYGON(LINESTRING(0 0, 10 0, 10 10, 0 10, 0 0), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3)), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), LINESTRING(3 3, 7 3, 7 7, 3 7, 3 3)), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
            'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE(CIRCULARSTRING(3 3, 7 3, 7 7, 3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
            'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0), CIRCULARSTRING(10 0, 10 10, 0 10), (0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))': 'MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE((0 0, 10 0, 10 10, 0 10, 0 0)), COMPOUNDCURVE((3 3, 7 3), CIRCULARSTRING(7 3, 7 7, 3 7), (3 7, 3 3))), CURVEPOLYGON(LINESTRING(5 15, 10 20, 0 20, 5 15)))',
        }

        for wkt_before, wkt_expected in test_setup.items():
            geom = QgsGeometry.fromWkt(wkt_before)
            geom.toggleCircularAtVertex(geom.closestVertex(QgsPointXY(10, 10))[1])
            self.assertTrue(QgsGeometry.equals(geom, QgsGeometry.fromWkt(wkt_expected)),
                            f'toggleCircularAtVertex() did not create expected geometry.\nconverted wkt : {geom.asWkt()}\nexpected wkt :  {wkt_expected}\ninput wkt :     {wkt_before}).')

    def testSingleSidedBuffer(self):

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideLeft)
        result = out.asWkt()
        expected_wkt = "Polygon ((10 0, 0 0, 0 1, 10 1, 10 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleMiter)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleBevel)
        result = out.asWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

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
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multilinestring
        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(10 10, 20 20))')
        result = geom.mergeLines().asWkt()
        exp = 'LineString(0 0, 10 10, 20 20)'
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4),(10 10, 20 20))')
        result = geom.mergeLines().asWkt()
        exp = 'MultiLineString((0 0, 10 10, 20 20),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4))')
        result = geom.mergeLines().asWkt()
        exp = 'MultiLineString((0 0, 10 10),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testCurveSinuosity(self):
        """
        Test curve straightDistance2d() and sinuosity()
        """
        linestring = QgsGeometry.fromWkt('LineString EMPTY')
        self.assertTrue(math.isnan(linestring.constGet().straightDistance2d()))
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
        self.assertEqual(point.interpolateAngle(0), 0)

        collection_with_point = QgsGeometry.fromWkt('MultiPoint((0 -49))')
        # no meaning, just test no crash!
        self.assertEqual(collection_with_point.interpolateAngle(5), 0)
        self.assertEqual(collection_with_point.interpolateAngle(0), 0)

        collection_with_point = QgsGeometry.fromWkt('MultiPoint((0 -49), (10 10))')
        # no meaning, just test no crash!
        self.assertEqual(collection_with_point.interpolateAngle(5), 0)
        self.assertEqual(collection_with_point.interpolateAngle(0), 0)

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
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        self.assertTrue(linestring.interpolate(25).isNull())

        # multilinestring
        linestring = QgsGeometry.fromWkt('MultiLineString((0 0, 10 0, 10 10),(20 0, 30 0, 30 10))')
        exp = 'Point(5 0)'
        result = linestring.interpolate(5).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        exp = 'Point(10 5)'
        result = linestring.interpolate(15).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        exp = 'Point(10 10)'
        result = linestring.interpolate(20).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        exp = 'Point(25 0)'
        result = linestring.interpolate(25).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        exp = 'Point(30 0)'
        result = linestring.interpolate(30).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        exp = 'Point(30 5)'
        result = linestring.interpolate(35).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        self.assertTrue(linestring.interpolate(50).isNull())

        # polygon
        polygon = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 20 20, 10 20, 0 0))')  # NOQA
        exp = 'Point(10 5)'
        result = polygon.interpolate(15).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Interpolate: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        self.assertTrue(polygon.interpolate(68).isNull())

        # polygon with ring
        polygon = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 20 20, 10 20, 0 0),(5 5, 6 5, 6 6, 5 6, 5 5))')  # NOQA
        exp = 'Point (6 5.5)'
        result = polygon.interpolate(68).asWkt()
        self.assertTrue(compareWkt(result, exp, 0.1),
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
        linestring = QgsGeometry.fromWkt(
            'LineString (-1007697 1334641, -1007697 1334643, -1007695 1334643, -1007695 1334641, -1007696 1334641, -1007697 1334641)')
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
        exp = 'Polygon ((-0.628 -1.9983, 2.9769 -4.724, 6.7 0.2, 3.095 2.9257, -0.628 -1.9983))'

        result = bbox.asWkt(4)
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "Oriented MBBR: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        self.assertAlmostEqual(area, 27.89886071158214, places=3)
        self.assertAlmostEqual(angle, 37.09283729704157, places=3)
        self.assertAlmostEqual(width, 4.519421040409892, places=3)
        self.assertAlmostEqual(height, 6.173105019896937, places=3)

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
        polygon = QgsGeometry.fromWkt(
            'Polygon ((-0.698 0.892, -0.702 0.405, -0.022 0.360, 0.014 0.850, -0.698 0.892),(-0.619 0.777, -0.619 0.574, -0.515 0.567, -0.517 0.516, -0.411 0.499, -0.379 0.767, -0.619 0.777),(-0.322 0.506, -0.185 0.735, -0.046 0.428, -0.322 0.506))')
        o = polygon.orthogonalize()
        exp = 'Polygon ((-0.69799999999999995 0.89200000000000002, -0.72515703079591087 0.38373993222914216, -0.00901577368860811 0.34547552423418099, 0.01814125858957143 0.85373558928902782, -0.69799999999999995 0.89200000000000002),(-0.61899999999999999 0.77700000000000002, -0.63403125159063511 0.56020458713735533, -0.53071476068518508 0.55304126003523246, -0.5343108192220235 0.5011754225601015, -0.40493624158682306 0.49220537936424585, -0.3863089084840608 0.76086661681561074, -0.61899999999999999 0.77700000000000002),(-0.32200000000000001 0.50600000000000001, -0.185 0.73499999999999999, -0.046 0.42799999999999999, -0.32200000000000001 0.50600000000000001))'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multipolygon

        polygon = QgsGeometry.fromWkt(
            'MultiPolygon(((-0.550 -1.553, -0.182 -0.954, -0.182 -0.954, 0.186 -1.538, -0.550 -1.553)),'
            '((0.506 -1.376, 0.433 -1.081, 0.765 -0.900, 0.923 -1.132, 0.923 -1.391, 0.506 -1.376)))')
        o = polygon.orthogonalize()
        exp = 'MultiPolygon (((-0.55000000000000004 -1.55299999999999994, -0.182 -0.95399999999999996, -0.182 -0.95399999999999996, 0.186 -1.53800000000000003, -0.55000000000000004 -1.55299999999999994)),((0.50600000000000001 -1.37599999999999989, 0.34888970623957499 -1.04704644438350125, 0.78332709454235683 -0.83955640656085295, 0.92300000000000004 -1.1319999999999999, 0.91737248858460974 -1.38514497083566535, 0.50600000000000001 -1.37599999999999989)))'
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "orthogonalize: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # line
        line = QgsGeometry.fromWkt(
            'LineString (-1.07445631048298162 -0.91619958829825165, 0.04022568180912156 -0.95572731852137571, 0.04741254184968957 -0.61794489661467789, 0.68704308546024517 -0.66106605685808595)')
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
        line = QgsGeometry.fromWkt('LineString EMPTY')
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
        line = QgsGeometry.fromWkt('LineString EMPTY')
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
        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((50 40), (140 70), (80 100), (130 140), (30 150), (70 180), (190 110), (120 20))")
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
        # depending on GEOS version, the result will be one of these two. Either is correct.
        # older GEOS
        exp = "MULTILINESTRING ((41.66 31.11, 41.85 30.77), (41.41 31.41, 41.66 31.11), (41.11 31.66, 41.41 31.41), (40.77 31.85, 41.11 31.66), (40.39 31.96, 40.77 31.85), (40 32, 40.39 31.96), (39.61 31.96, 40 32), (39.23 31.85, 39.61 31.96), (38.89 31.66, 39.23 31.85), (38.59 31.41, 38.89 31.66), (38.34 31.11, 38.59 31.41), (38.15 30.77, 38.34 31.11), (38.04 30.39, 38.15 30.77), (38 30, 38.04 30.39), (38 30, 38.04 29.61), (38.04 29.61, 38.15 29.23), (38.15 29.23, 38.34 28.89), (38.34 28.89, 38.59 28.59), (38.59 28.59, 38.89 28.34), (38.89 28.34, 39.23 28.15), (39.23 28.15, 39.61 28.04), (39.61 28.04, 40 28), (40 28, 40.39 28.04), (40.39 28.04, 40.77 28.15), (40.77 28.15, 41.11 28.34), (41.11 28.34, 41.41 28.59), (41.41 28.59, 41.66 28.89), (41.66 28.89, 41.85 29.23), (41.85 29.23, 41.96 29.61), (41.96 29.61, 42 30), (41.96 30.39, 42 30), (41.85 30.77, 41.96 30.39), (41.66 31.11, 41.96 30.39), (41.41 31.41, 41.96 30.39), (41.41 28.59, 41.96 30.39), (41.41 28.59, 41.41 31.41), (38.59 28.59, 41.41 28.59), (38.59 28.59, 41.41 31.41), (38.59 28.59, 38.59 31.41), (38.59 31.41, 41.41 31.41), (38.59 31.41, 39.61 31.96), (39.61 31.96, 41.41 31.41), (39.61 31.96, 40.39 31.96), (40.39 31.96, 41.41 31.41), (40.39 31.96, 41.11 31.66), (38.04 30.39, 38.59 28.59), (38.04 30.39, 38.59 31.41), (38.04 30.39, 38.34 31.11), (38.04 29.61, 38.59 28.59), (38.04 29.61, 38.04 30.39), (39.61 28.04, 41.41 28.59), (38.59 28.59, 39.61 28.04), (38.89 28.34, 39.61 28.04), (40.39 28.04, 41.41 28.59), (39.61 28.04, 40.39 28.04), (41.96 29.61, 41.96 30.39), (41.41 28.59, 41.96 29.61), (41.66 28.89, 41.96 29.61), (40.39 28.04, 41.11 28.34), (38.04 29.61, 38.34 28.89), (38.89 31.66, 39.61 31.96))"
        # newer GEOS
        exp2 = "MultiLineString ((41.66 31.11, 41.85 30.77),(41.41 31.41, 41.66 31.11),(41.11 31.66, 41.41 31.41),(40.77 31.85, 41.11 31.66),(40.39 31.96, 40.77 31.85),(40 32, 40.39 31.96),(39.61 31.96, 40 32),(39.23 31.85, 39.61 31.96),(38.89 31.66, 39.23 31.85),(38.59 31.41, 38.89 31.66),(38.34 31.11, 38.59 31.41),(38.15 30.77, 38.34 31.11),(38.04 30.39, 38.15 30.77),(38 30, 38.04 30.39),(38 30, 38.04 29.61),(38.04 29.61, 38.15 29.23),(38.15 29.23, 38.34 28.89),(38.34 28.89, 38.59 28.59),(38.59 28.59, 38.89 28.34),(38.89 28.34, 39.23 28.15),(39.23 28.15, 39.61 28.04),(39.61 28.04, 40 28),(40 28, 40.39 28.04),(40.39 28.04, 40.77 28.15),(40.77 28.15, 41.11 28.34),(41.11 28.34, 41.41 28.59),(41.41 28.59, 41.66 28.89),(41.66 28.89, 41.85 29.23),(41.85 29.23, 41.96 29.61),(41.96 29.61, 42 30),(41.96 30.39, 42 30),(41.85 30.77, 41.96 30.39),(41.66 31.11, 41.96 30.39),(41.41 31.41, 41.96 30.39),(41.96 29.61, 41.96 30.39),(41.41 31.41, 41.96 29.61),(41.41 28.59, 41.96 29.61),(41.41 28.59, 41.41 31.41),(38.59 31.41, 41.41 28.59),(38.59 31.41, 41.41 31.41),(38.59 31.41, 40.39 31.96),(40.39 31.96, 41.41 31.41),(40.39 31.96, 41.11 31.66),(38.59 31.41, 39.61 31.96),(39.61 31.96, 40.39 31.96),(38.59 28.59, 41.41 28.59),(38.59 28.59, 38.59 31.41),(38.04 29.61, 38.59 28.59),(38.04 29.61, 38.59 31.41),(38.04 29.61, 38.04 30.39),(38.04 30.39, 38.59 31.41),(38.04 30.39, 38.34 31.11),(40.39 28.04, 41.41 28.59),(38.59 28.59, 40.39 28.04),(39.61 28.04, 40.39 28.04),(38.59 28.59, 39.61 28.04),(38.89 28.34, 39.61 28.04),(41.66 28.89, 41.96 29.61),(40.39 28.04, 41.11 28.34),(38.04 29.61, 38.34 28.89),(38.89 31.66, 39.61 31.96))"
        result = o.asWkt(2)
        if compareWkt(result, exp, 0.00001):
            self.assertTrue(compareWkt(result, exp, 0.00001),
                            "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))
        else:
            self.assertTrue(compareWkt(result, exp2, 0.00001),
                            "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp2, result))

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
        line = QgsGeometry.fromWkt('LineString EMPTY')
        o = line.voronoiDiagram()
        self.assertFalse(o)

        input = QgsGeometry.fromWkt("MULTIPOINT ((150 200))")
        o = input.voronoiDiagram()
        self.assertTrue(o.isEmpty())

        input = QgsGeometry.fromWkt("MULTIPOINT ((150 200), (180 270), (275 163))")
        o = input.voronoiDiagram()
        if self.geos39:
            exp = "GeometryCollection (Polygon ((25 38, 25 295, 221.20588235294118817 210.91176470588234793, 170.02400000000000091 38, 25 38)),Polygon ((400 38, 170.02400000000000091 38, 221.20588235294118817 210.91176470588234793, 400 369.65420560747656964, 400 38)),Polygon ((25 395, 400 395, 400 369.65420560747656964, 221.20588235294118817 210.91176470588234793, 25 295, 25 395)))"
        else:
            exp = "GeometryCollection (Polygon ((170.02400000000000091 38, 25 38, 25 295, 221.20588235294115975 210.91176470588234793, 170.02400000000000091 38)),Polygon ((400 369.65420560747662648, 400 38, 170.02400000000000091 38, 221.20588235294115975 210.91176470588234793, 400 369.65420560747662648)),Polygon ((25 295, 25 395, 400 395, 400 369.65420560747662648, 221.20588235294115975 210.91176470588234793, 25 295)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("MULTIPOINT ((280 300), (420 330), (380 230), (320 160))")
        o = input.voronoiDiagram()
        if self.geos39:
            exp = "GeometryCollection (Polygon ((110 500, 310.35714285714283278 500, 353.515625 298.59375, 306.875 231.96428571428572241, 110 175.71428571428572241, 110 500)),Polygon ((590 -10, 589.16666666666662877 -10, 306.875 231.96428571428572241, 353.515625 298.59375, 590 204, 590 -10)),Polygon ((110 -10, 110 175.71428571428572241, 306.875 231.96428571428572241, 589.16666666666662877 -10, 110 -10)),Polygon ((590 500, 590 204, 353.515625 298.59375, 310.35714285714283278 500, 590 500)))"
        else:
            exp = "GeometryCollection (Polygon ((110 175.71428571428572241, 110 500, 310.35714285714283278 500, 353.515625 298.59375, 306.875 231.96428571428572241, 110 175.71428571428572241)),Polygon ((590 204, 590 -10, 589.16666666666662877 -10, 306.875 231.96428571428572241, 353.515625 298.59375, 590 204)),Polygon ((589.16666666666662877 -10, 110 -10, 110 175.71428571428572241, 306.875 231.96428571428572241, 589.16666666666662877 -10)),Polygon ((310.35714285714283278 500, 590 500, 590 204, 353.515625 298.59375, 310.35714285714283278 500)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt("MULTIPOINT ((320 170), (366 246), (530 230), (530 300), (455 277), (490 160))")
        o = input.voronoiDiagram()
        if self.geos39:
            exp = "GeometryCollection (Polygon ((110 -50, 110 349.02631578947364233, 405.31091180866962986 170.28550074738416242, 392.35294117647055145 -50, 110 -50)),Polygon ((740 -50, 392.35294117647055145 -50, 405.31091180866962986 170.28550074738416242, 429.91476778570188344 205.76082797008174907, 470.12061711079945781 217.78821879382888937, 740 63.57142857142859071, 740 -50)),Polygon ((110 510, 323.94382022471910432 510, 429.91476778570188344 205.76082797008174907, 405.31091180866962986 170.28550074738416242, 110 349.02631578947364233, 110 510)),Polygon ((424.57333333333326664 510, 499.70666666666664923 265, 470.12061711079945781 217.78821879382888937, 429.91476778570188344 205.76082797008174907, 323.94382022471910432 510, 424.57333333333326664 510)),Polygon ((740 63.57142857142859071, 470.12061711079945781 217.78821879382888937, 499.70666666666664923 265, 740 265, 740 63.57142857142859071)),Polygon ((740 510, 740 265, 499.70666666666664923 265, 424.57333333333326664 510, 740 510)))"
        else:
            exp = "GeometryCollection (Polygon ((392.35294117647055145 -50, 110 -50, 110 349.02631578947364233, 405.31091180866962986 170.28550074738416242, 392.35294117647055145 -50)),Polygon ((740 63.57142857142859071, 740 -50, 392.35294117647055145 -50, 405.31091180866962986 170.28550074738416242, 429.91476778570188344 205.76082797008174907, 470.12061711079945781 217.78821879382888937, 740 63.57142857142859071)),Polygon ((110 349.02631578947364233, 110 510, 323.94382022471910432 510, 429.91476778570188344 205.76082797008174907, 405.31091180866962986 170.28550074738416242, 110 349.02631578947364233)),Polygon ((323.94382022471910432 510, 424.57333333333326664 510, 499.70666666666664923 265, 470.12061711079945781 217.78821879382888937, 429.91476778570188344 205.76082797008174907, 323.94382022471910432 510)),Polygon ((740 265, 740 63.57142857142859071, 470.12061711079945781 217.78821879382888937, 499.70666666666664923 265, 740 265)),Polygon ((424.57333333333326664 510, 740 510, 740 265, 499.70666666666664923 265, 424.57333333333326664 510)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((280 200), (406 285), (580 280), (550 190), (370 190), (360 90), (480 110), (440 160), (450 180), (480 180), (460 160), (360 210), (360 220), (370 210), (375 227))")
        o = input.voronoiDiagram()
        if self.geos39:
            exp = "GeometryCollection (Polygon ((-20 585, 111.94841269841269593 585, 293.54906542056073704 315.803738317756995, 318.75 215, 323.2352941176470722 179.1176470588235361, 319.39560439560437999 144.560439560439562, -20 -102.27272727272726627, -20 585)),Polygon ((365 200, 365 215, 369.40909090909093493 219.40909090909090651, 414.21192052980131848 206.23178807947019209, 411.875 200, 365 200)),Polygon ((365 215, 365 200, 323.2352941176470722 179.1176470588235361, 318.75 215, 365 215)),Polygon ((-20 -210, -20 -102.27272727272726627, 319.39560439560437999 144.560439560439562, 388.97260273972602818 137.60273972602740855, 419.55882352941176805 102.64705882352940591, 471.66666666666674246 -210, -20 -210)),Polygon ((411.875 200, 410.29411764705884025 187.35294117647057988, 388.97260273972602818 137.60273972602740855, 319.39560439560437999 144.560439560439562, 323.2352941176470722 179.1176470588235361, 365 200, 411.875 200)),Polygon ((410.29411764705884025 187.35294117647057988, 411.875 200, 414.21192052980131848 206.23178807947019209, 431.62536593766145643 234.0192009643533595, 465 248.00476190476189231, 465 175, 450 167.5, 410.29411764705884025 187.35294117647057988)),Polygon ((293.54906542056073704 315.803738317756995, 339.65007656967839011 283.17840735068909908, 369.40909090909093493 219.40909090909090651, 365 215, 318.75 215, 293.54906542056073704 315.803738317756995)),Polygon ((501.69252873563220874 585, 492.56703910614527331 267.43296089385472669, 465 248.00476190476189231, 431.62536593766145643 234.0192009643533595, 339.65007656967839011 283.17840735068909908, 293.54906542056073704 315.803738317756995, 111.94841269841269593 585, 501.69252873563220874 585)),Polygon ((369.40909090909093493 219.40909090909090651, 339.65007656967839011 283.17840735068909908, 431.62536593766145643 234.0192009643533595, 414.21192052980131848 206.23178807947019209, 369.40909090909093493 219.40909090909090651)),Polygon ((388.97260273972602818 137.60273972602740855, 410.29411764705884025 187.35294117647057988, 450 167.5, 450 127, 419.55882352941176805 102.64705882352940591, 388.97260273972602818 137.60273972602740855)),Polygon ((465 175, 465 248.00476190476189231, 492.56703910614527331 267.43296089385472669, 505 255, 520.71428571428566556 145, 495 145, 465 175)),Polygon ((880 -210, 471.66666666666674246 -210, 419.55882352941176805 102.64705882352940591, 450 127, 495 145, 520.71428571428566556 145, 880 -169.375, 880 -210)),Polygon ((465 175, 495 145, 450 127, 450 167.5, 465 175)),Polygon ((880 585, 880 130, 505 255, 492.56703910614527331 267.43296089385472669, 501.69252873563220874 585, 880 585)),Polygon ((880 -169.375, 520.71428571428566556 145, 505 255, 880 130, 880 -169.375)))"
        else:
            exp = "GeometryCollection (Polygon ((-20 -102.27272727272726627, -20 585, 111.94841269841269593 585, 293.54906542056073704 315.803738317756995, 318.75 215, 323.2352941176470722 179.1176470588235361, 319.39560439560437999 144.560439560439562, -20 -102.27272727272726627)),Polygon ((365 200, 365 215, 369.40909090909093493 219.40909090909090651, 414.21192052980131848 206.23178807947019209, 411.875 200, 365 200)),Polygon ((365 215, 365 200, 323.2352941176470722 179.1176470588235361, 318.75 215, 365 215)),Polygon ((471.66666666666674246 -210, -20 -210, -20 -102.27272727272726627, 319.39560439560437999 144.560439560439562, 388.97260273972602818 137.60273972602738013, 419.55882352941176805 102.64705882352942012, 471.66666666666674246 -210)),Polygon ((411.875 200, 410.29411764705884025 187.35294117647057988, 388.97260273972602818 137.60273972602738013, 319.39560439560437999 144.560439560439562, 323.2352941176470722 179.1176470588235361, 365 200, 411.875 200)),Polygon ((410.29411764705884025 187.35294117647057988, 411.875 200, 414.21192052980131848 206.23178807947019209, 431.62536593766145643 234.0192009643533595, 465 248.00476190476189231, 465 175, 450 167.5, 410.29411764705884025 187.35294117647057988)),Polygon ((293.54906542056073704 315.803738317756995, 339.65007656967839011 283.17840735068909908, 369.40909090909093493 219.40909090909090651, 365 215, 318.75 215, 293.54906542056073704 315.803738317756995)),Polygon ((111.94841269841269593 585, 501.69252873563215189 585, 492.56703910614521646 267.43296089385472669, 465 248.00476190476189231, 431.62536593766145643 234.0192009643533595, 339.65007656967839011 283.17840735068909908, 293.54906542056073704 315.803738317756995, 111.94841269841269593 585)),Polygon ((369.40909090909093493 219.40909090909090651, 339.65007656967839011 283.17840735068909908, 431.62536593766145643 234.0192009643533595, 414.21192052980131848 206.23178807947019209, 369.40909090909093493 219.40909090909090651)),Polygon ((388.97260273972602818 137.60273972602738013, 410.29411764705884025 187.35294117647057988, 450 167.5, 450 127, 419.55882352941176805 102.64705882352942012, 388.97260273972602818 137.60273972602738013)),Polygon ((465 175, 465 248.00476190476189231, 492.56703910614521646 267.43296089385472669, 505 255, 520.71428571428566556 145, 495 145, 465 175)),Polygon ((880 -169.375, 880 -210, 471.66666666666674246 -210, 419.55882352941176805 102.64705882352942012, 450 127, 495 145, 520.71428571428566556 145, 880 -169.375)),Polygon ((465 175, 495 145, 450 127, 450 167.5, 465 175)),Polygon ((501.69252873563215189 585, 880 585, 880 130.00000000000005684, 505 255, 492.56703910614521646 267.43296089385472669, 501.69252873563215189 585)),Polygon ((880 130.00000000000005684, 880 -169.375, 520.71428571428566556 145, 505 255, 880 130.00000000000005684)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((100 200), (105 202), (110 200), (140 230), (210 240), (220 190), (170 170), (170 260), (213 245), (220 190))")
        o = input.voronoiDiagram(QgsGeometry(), 6)
        if self.geos39:
            exp = "GeometryCollection (Polygon ((-20 50, -20 380, -3.75 380, 105 235, 105 115, 77.1428571428571388 50, -20 50)),Polygon ((77.1428571428571388 50, 105 115, 145 195, 178.33333333333334281 211.66666666666665719, 183.51851851851850483 208.70370370370369528, 246.99999999999997158 50, 77.1428571428571388 50)),Polygon ((20.00000000000000711 380, 176.66666666666665719 223.33333333333334281, 178.33333333333334281 211.66666666666665719, 145 195, 105 235, -3.75 380, 20.00000000000000711 380)),Polygon ((105 115, 105 235, 145 195, 105 115)),Polygon ((255 380, 176.66666666666665719 223.33333333333334281, 20.00000000000000711 380, 255 380)),Polygon ((340 380, 340 240, 183.51851851851850483 208.70370370370369528, 178.33333333333334281 211.66666666666665719, 176.66666666666665719 223.33333333333334281, 255 380, 340 380)),Polygon ((340 50, 246.99999999999997158 50, 183.51851851851850483 208.70370370370369528, 340 240, 340 50)))"
        else:
            exp = "GeometryCollection (Polygon ((77.1428571428571388 50, -20 50, -20 380, -3.75 380, 105 235, 105 115, 77.1428571428571388 50)),Polygon ((247 50, 77.1428571428571388 50, 105 115, 145 195, 178.33333333333334281 211.66666666666665719, 183.51851851851853326 208.70370370370369528, 247 50)),Polygon ((-3.75 380, 20.00000000000000711 380, 176.66666666666665719 223.33333333333334281, 178.33333333333334281 211.66666666666665719, 145 195, 105 235, -3.75 380)),Polygon ((105 115, 105 235, 145 195, 105 115)),Polygon ((20.00000000000000711 380, 255 380, 176.66666666666665719 223.33333333333334281, 20.00000000000000711 380)),Polygon ((255 380, 340 380, 340 240, 183.51851851851853326 208.70370370370369528, 178.33333333333334281 211.66666666666665719, 176.66666666666665719 223.33333333333334281, 255 380)),Polygon ((340 240, 340 50, 247 50, 183.51851851851853326 208.70370370370369528, 340 240)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((170 270), (177 275), (190 230), (230 250), (210 290), (240 280), (240 250))")
        o = input.voronoiDiagram(QgsGeometry(), 10)
        if self.geos39:
            exp = "GeometryCollection (Polygon ((100 360, 150 360, 200 260, 100 210, 100 360)),Polygon ((250 360, 220 270, 200 260, 150 360, 250 360)),Polygon ((100 160, 100 210, 200 260, 235 190, 247 160, 100 160)),Polygon ((220 270, 235 265, 235 190, 200 260, 220 270)),Polygon ((310 360, 310 265, 235 265, 220 270, 250 360, 310 360)),Polygon ((310 160, 247 160, 235 190, 235 265, 310 265, 310 160)))"
        else:
            exp = "GeometryCollection (Polygon ((100 210, 100 360, 150 360, 200 260, 100 210)),Polygon ((150 360, 250 360, 220 270, 200 260, 150 360)),Polygon ((247 160, 100 160, 100 210, 200 260, 235 190, 247 160)),Polygon ((220 270, 235 265, 235 190, 200 260, 220 270)),Polygon ((250 360, 310 360, 310 265, 235 265, 220 270, 250 360)),Polygon ((310 265, 310 160, 247 160, 235 190, 235 265, 310 265)))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "delaunay: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        input = QgsGeometry.fromWkt(
            "MULTIPOINT ((155 271), (150 360), (260 360), (271 265), (280 260), (270 370), (154 354), (150 260))")
        o = input.voronoiDiagram(QgsGeometry(), 100)
        if self.geos39:
            exp = "GeometryCollection (Polygon ((20 130, 20 310, 205 310, 215 299, 215 130, 20 130)),Polygon ((410 500, 410 338, 215 299, 205 310, 205 500, 410 500)),Polygon ((20 500, 205 500, 205 310, 20 310, 20 500)),Polygon ((410 130, 215 130, 215 299, 410 338, 410 130)))"
        else:
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
        input = QgsGeometry.fromWkt(
            "PolygonZM(( 0 0 1 4, 10 0 2 6, 10 10 0 8, 0 0 1 4 ), ( 0 0 1 4, 10 0 2 6, 10 10 0 8, 0 0 1 4 ) )")
        o = input.densifyByCount(1)
        exp = "PolygonZM(( 0 0 1 4, 5 0 1.5 5, 10 0 2 6, 10 5 1 7, 10 10 0 8, 5 5 0.5 6, 0 0 1 4 ),( 0 0 1 4, 5 0 1.5 5, 10 0 2 6, 10 5 1 7, 10 10 0 8, 5 5 0.5 6, 0 0 1 4 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multi line
        input = QgsGeometry.fromWkt(
            "MultiLineString(( 0 0, 5 0, 10 0, 10 5, 10 10), (20 0, 25 0, 30 0, 30 5, 30 10 ) )")
        o = input.densifyByCount(1)
        exp = "MultiLineString(( 0 0, 2.5 0, 5 0, 7.5 0, 10 0, 10 2.5, 10 5, 10 7.5, 10 10 ),( 20 0, 22.5 0, 25 0, 27.5 0, 30 0, 30 2.5, 30 5, 30 7.5, 30 10 ))"
        result = o.asWkt()
        self.assertTrue(compareWkt(result, exp, 0.00001),
                        "densify by count: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multipolygon
        input = QgsGeometry.fromWkt(
            "MultiPolygonZ(((  0 0 1, 10 0 2, 10 10 0, 0 0 1)),((  0 0 1, 10 0 2, 10 10 0, 0 0 1 )))")
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
                 ["LINESTRING (10 10, 10 10)", "POINT (10 10)"],  # zero length line
                 ["MULTILINESTRING ((10 10, 10 10), (20 20, 20 20))", "POINT (15 15)"],  # zero length multiline
                 ["LINESTRING (60 180, 120 100, 180 180)", "POINT (120 140)"],
                 ["LINESTRING (80 0, 80 120, 120 120, 120 0)", "POINT (100 68.57142857142857)"],
                 ["MULTILINESTRING ((0 0, 0 100), (100 0, 100 100))", "POINT (50 50)"],
                 [" MULTILINESTRING ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180))",
                  "POINT (90 110)"],
                 [
                     "MULTILINESTRING ((20 20, 60 60),(20 -20, 60 -60),(-20 -20, -60 -60),(-20 20, -60 60),(-80 0, 0 80, 80 0, 0 -80, -80 0),(-40 20, -40 -20),(-20 40, 20 40),(40 20, 40 -20),(20 -40, -20 -40))",
                     "POINT (0 0)"],
                 ["POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))", "POINT (5 5)"],
                 ["POLYGON ((40 160, 160 160, 160 40, 40 40, 40 160))", "POINT (100 100)"],
                 ["POLYGON ((0 200, 200 200, 200 0, 0 0, 0 200), (20 180, 80 180, 80 20, 20 20, 20 180))",
                  "POINT (115.78947368421052 100)"],
                 ["POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180))",
                  "POINT (102.5 97.5)"],
                 [
                     "POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180),(180 60, 140 60, 140 20, 180 20, 180 60))",
                     "POINT (100 100)"],
                 [
                     "MULTIPOLYGON (((0 40, 0 140, 140 140, 140 120, 20 120, 20 40, 0 40)),((0 0, 0 20, 120 20, 120 100, 140 100, 140 0, 0 0)))",
                     "POINT (70 70)"],
                 [
                     "GEOMETRYCOLLECTION (POLYGON ((0 200, 20 180, 20 140, 60 140, 200 0, 0 0, 0 200)),POLYGON ((200 200, 0 200, 20 180, 60 180, 60 140, 200 0, 200 200)))",
                     "POINT (102.5 97.5)"],
                 [
                     "GEOMETRYCOLLECTION (LINESTRING (80 0, 80 120, 120 120, 120 0),MULTIPOINT ((20 60), (40 80), (60 60)))",
                     "POINT (100 68.57142857142857)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((0 40, 40 40, 40 0, 0 0, 0 40)),LINESTRING (80 0, 80 80, 120 40))",
                  "POINT (20 20)"],
                 [
                     "GEOMETRYCOLLECTION (POLYGON ((0 40, 40 40, 40 0, 0 0, 0 40)),LINESTRING (80 0, 80 80, 120 40),MULTIPOINT ((20 60), (40 80), (60 60)))",
                     "POINT (20 20)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 30 30))",
                  "POINT (25 25)"],
                 ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20))",
                  "POINT (15 15)"],
                 [
                     "GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20),MULTIPOINT ((20 10), (10 20)) )",
                     "POINT (15 15)"],
                 # ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING (20 20, 20 20),POINT EMPTY )","POINT (15 15)"],
                 # ["GEOMETRYCOLLECTION (POLYGON ((10 10, 10 10, 10 10, 10 10)),LINESTRING EMPTY,POINT EMPTY )","POINT (10 10)"],
                 [
                     "GEOMETRYCOLLECTION (POLYGON ((20 100, 20 -20, 60 -20, 60 100, 20 100)),POLYGON ((-20 60, 100 60, 100 20, -20 20, -20 60)))",
                     "POINT (40 40)"],
                 ["POLYGON ((40 160, 160 160, 160 160, 40 160, 40 160))", "POINT (100 160)"],
                 ["POLYGON ((10 10, 100 100, 100 100, 10 10))", "POINT (55 55)"],
                 # ["POLYGON EMPTY","POINT EMPTY"],
                 # ["MULTIPOLYGON(EMPTY,((0 0,1 0,1 1,0 1, 0 0)))","POINT (0.5 0.5)"],
                 [
                     "POLYGON((56.528666666700 25.2101666667,56.529000000000 25.2105000000,56.528833333300 25.2103333333,56.528666666700 25.2101666667))",
                     "POINT (56.52883333335 25.21033333335)"],
                 [
                     "POLYGON((56.528666666700 25.2101666667,56.529000000000 25.2105000000,56.528833333300 25.2103333333,56.528666666700 25.2101666667))",
                     "POINT (56.528833 25.210333)"]
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
                                "centroid: mismatch using QgsAbstractGeometry methods Input {} \n Expected:\n{}\nGot:\n{}\n".format(
                                    t[0], exp, result))

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
                 ["GeometryCollection ()", 8, "GeometryCollection EMPTY"],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 8,
                  "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5),(1 5, 1 6, 1 7, 1 8, 1 9))"],
                 ["LINESTRING(0 0, 100 100, 150 150)", 8, 'MultiLineString ((0 0, 100 100, 150 150))'],
                 [
                     'POLYGON((132 10,119 23,85 35,68 29,66 28,49 42,32 56,22 64,32 110,40 119,36 150,57 158,75 171,92 182,114 184,132 186,146 178,176 184,179 162,184 141,190 122,190 100,185 79,186 56,186 52,178 34,168 18,147 13,132 10))',
                     10, None],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 1,
                  "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5),(1 5, 1 6, 1 7, 1 8, 1 9))"],
                 ["LINESTRING (1 1,1 2,1 3,1 4,1 5,1 6,1 7,1 8,1 9)", 16,
                  "MultiLineString ((1 1, 1 2, 1 3, 1 4, 1 5, 1 6, 1 7, 1 8, 1 9))"],
                 [
                     "POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0),(60 180, 20 180, 20 140, 60 140, 60 180),(180 60, 140 60, 140 20, 180 20, 180 60))",
                     8,
                     "MultiPolygon (((0 0, 0 100, 100 100, 100 0, 0 0)),((100 0, 100 50, 140 50, 140 20, 150 20, 150 0, 100 0)),((150 0, 150 20, 180 20, 180 50, 200 50, 200 0, 150 0)),((100 50, 100 100, 150 100, 150 60, 140 60, 140 50, 100 50)),((150 60, 150 100, 200 100, 200 50, 180 50, 180 60, 150 60)),((0 100, 0 150, 20 150, 20 140, 50 140, 50 100, 0 100)),((50 100, 50 140, 60 140, 60 150, 100 150, 100 100, 50 100)),((0 150, 0 200, 50 200, 50 180, 20 180, 20 150, 0 150)),((50 180, 50 200, 100 200, 100 150, 60 150, 60 180, 50 180)),((100 100, 100 200, 200 200, 200 100, 100 100)))"],
                 [
                     "POLYGON((132 10,119 23,85 35,68 29,66 28,49 42,32 56,22 64,32 110,40 119,36 150, 57 158,75 171,92 182,114 184,132 186,146 178,176 184,179 162,184 141,190 122,190 100,185 79,186 56,186 52,178 34,168 18,147 13,132 10))",
                     10, None]
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
                 ["LINESTRING (-1 -9,-1 11,9 11)", QgsRectangle(0, 0, 10, 10),
                  "GEOMETRYCOLLECTION EMPTY"],
                 ["LINESTRING (-1 5,5 5,9 9)", QgsRectangle(0, 0, 10, 10), "LINESTRING (0 5,5 5,9 9)"],
                 ["LINESTRING (5 5,8 5,12 5)", QgsRectangle(0, 0, 10, 10), "LINESTRING (5 5,8 5,10 5)"],
                 ["LINESTRING (5 -1,5 5,1 2,-3 2,1 6)", QgsRectangle(0, 0, 10, 10),
                  "MULTILINESTRING ((5 0,5 5,1 2,0 2),(0 5,1 6))"],
                 ["LINESTRING (0 3,0 5,0 7)", QgsRectangle(0, 0, 10, 10),
                  "GEOMETRYCOLLECTION EMPTY"],
                 ["LINESTRING (0 3,0 5,-1 7)", QgsRectangle(0, 0, 10, 10),
                  "GEOMETRYCOLLECTION EMPTY"],
                 ["LINESTRING (0 3,0 5,2 7)", QgsRectangle(0, 0, 10, 10), "LINESTRING (0 5,2 7)"],
                 ["LINESTRING (2 1,0 0,1 2)", QgsRectangle(0, 0, 10, 10), "LINESTRING (2 1,0 0,1 2)"],
                 ["LINESTRING (3 3,0 3,0 5,2 7)", QgsRectangle(0, 0, 10, 10), "MULTILINESTRING ((3 3,0 3),(0 5,2 7))"],
                 ["LINESTRING (5 5,10 5,20 5)", QgsRectangle(0, 0, 10, 10), "LINESTRING (5 5,10 5)"],
                 ["LINESTRING (3 3,0 6,3 9)", QgsRectangle(0, 0, 10, 10), "LINESTRING (3 3,0 6,3 9)"],
                 ["POLYGON ((5 5,5 6,6 6,6 5,5 5))", QgsRectangle(0, 0, 10, 10), "POLYGON ((5 5,5 6,6 6,6 5,5 5))"],
                 ["POLYGON ((15 15,15 16,16 16,16 15,15 15))", QgsRectangle(0, 0, 10, 10),
                  "GEOMETRYCOLLECTION EMPTY"],
                 ["POLYGON ((-1 -1,-1 11,11 11,11 -1,-1 -1))", QgsRectangle(0, 0, 10, 10),
                  "Polygon ((0 0, 0 10, 10 10, 10 0, 0 0))"],
                 ["POLYGON ((-1 -1,-1 5,5 5,5 -1,-1 -1))", QgsRectangle(0, 0, 10, 10),
                  "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))"],
                 ["POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (3 3,4 4,4 2,3 3))", QgsRectangle(0, 0, 10, 10),
                  "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0),(3 3, 4 4, 4 2, 3 3))"]
                 ]
        for t in tests:
            input = QgsGeometry.fromWkt(t[0])
            o = input.clipped(t[1])
            exp = t[2]
            result = o.asWkt()
            self.assertTrue(compareWkt(result, exp, 0.00001),
                            "clipped: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

    def testCreateWedgeBuffer(self):
        tests = [[QgsPoint(1, 11), 0, 45, 2, 0,
                  'CurvePolygon (CompoundCurve (CircularString (0.23463313526982044 12.84775906502257392, 1 13, 1.76536686473017967 12.84775906502257392),(1.76536686473017967 12.84775906502257392, 1 11),(1 11, 0.23463313526982044 12.84775906502257392)))'],
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
        tests = [['LineString (6 2, 9 2, 9 3, 11 5)', 1, 2, 3,
                  'MultiPolygon (((5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.23175255221825708 2.66341629715358597, 8.20710678118654791 3, 8.31333433001913669 3.39644660940672605, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.76603613070954424 2.63321666219915951, 9.71966991411008863 1.99999999999999978, 9.62325242795870217 1.64016504294495569, 9.35983495705504431 1.37674757204129761, 9 1.28033008588991049, 6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2)))'
                  if self.geos39 else
                  'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.23175255221825708 2.66341629715358597, 8.20710678118654791 3, 8.31333433001913669 3.39644660940672605, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.76603613070954424 2.63321666219915951, 9.71966991411008863 1.99999999999999978, 9.62325242795870217 1.64016504294495569, 9.35983495705504431 1.37674757204129761, 9 1.28033008588991049, 6 1.5)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 1, 1, 3,
                  'MultiPolygon (((5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.5 2.5, 8.5 3, 8.56698729810778126 3.24999999999999956, 8.75 3.43301270189221919, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 11.25 4.56698729810778037, 9.5 2.81698729810778081, 9.5 2, 9.43301270189221874 1.75000000000000022, 9.25 1.56698729810778081, 9 1.5, 6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2)))'
                  if self.geos39 else
                  'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.5 2.5, 8.5 3, 8.56698729810778126 3.24999999999999956, 8.75 3.43301270189221919, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 11.25 4.56698729810778037, 9.5 2.81698729810778081, 9.5 2, 9.43301270189221874 1.75000000000000022, 9.25 1.56698729810778081, 9 1.5, 6 1.5)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 2, 1, 3,
                  'MultiPolygon (((5 2, 5.13397459621556074 2.49999999999999956, 5.5 2.86602540378443837, 6 3, 8.28066508549441238 2.83300216551852069, 8.29289321881345209 3, 8.38762756430420531 3.35355339059327351, 8.64644660940672694 3.61237243569579425, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 9.72358625835961909 2.77494218213953703, 9.78033008588991137 1.99999999999999978, 9.67578567771795583 1.60983495705504498, 9.39016504294495569 1.32421432228204461, 9 1.21966991411008951, 6 1, 5.5 1.13397459621556118, 5.13397459621556163 1.49999999999999978, 5 2)))'
                  if self.geos39 else
                  'MultiPolygon (((6 1, 5.5 1.13397459621556118, 5.13397459621556163 1.49999999999999978, 5 2, 5.13397459621556074 2.49999999999999956, 5.5 2.86602540378443837, 6 3, 8.28066508549441238 2.83300216551852069, 8.29289321881345209 3, 8.38762756430420531 3.35355339059327351, 8.64644660940672694 3.61237243569579425, 10.75 5.43301270189221963, 11 5.5, 11.25 5.43301270189221874, 11.43301270189221874 5.25, 11.5 5, 11.43301270189221874 4.75, 9.72358625835961909 2.77494218213953703, 9.78033008588991137 1.99999999999999978, 9.67578567771795583 1.60983495705504498, 9.39016504294495569 1.32421432228204461, 9 1.21966991411008951, 6 1)))'
                  ],
                 [
                     'MultiLineString ((2 0, 2 2, 3 2, 3 3),(2.94433781190019195 4.04721689059500989, 5.45950095969289784 4.11976967370441471),(3 3, 5.5804222648752404 2.94683301343570214))',
                     1, 2, 3,
                     'MultiPolygon (((2 -0.5, 1.75 -0.43301270189221935, 1.56698729810778081 -0.25000000000000011, 1.5 0.00000000000000006, 1.25 2, 1.35048094716167078 2.37499999999999956, 1.62499999999999978 2.649519052838329, 2 2.75, 2.03076923076923066 2.75384615384615383, 2 3, 2.13397459621556118 3.49999999999999956, 2.5 3.86602540378443837, 3.00000000000000044 4, 3.50000000000000044 3.86602540378443837, 3.86602540378443837 3.5, 4 3, 3.875 1.99999999999999978, 3.75777222831138413 1.56250000000000044, 3.4375 1.24222777168861631, 3 1.125, 2.64615384615384608 1.1692307692307693, 2.5 -0.00000000000000012, 2.43301270189221963 -0.24999999999999983, 2.25 -0.4330127018922193, 2 -0.5)),((2.69433781190019195 3.6142041887027907, 2.51132511000797276 3.79721689059500989, 2.44433781190019195 4.04721689059500989, 2.51132511000797232 4.29721689059500989, 2.69433781190019195 4.48022959248722952, 2.94433781190019195 4.54721689059500989, 5.45950095969289784 5.11976967370441471, 5.95950095969289784 4.98579507748885309, 6.32552636347733621 4.61976967370441471, 6.45950095969289784 4.11976967370441471, 6.3255263634773371 3.61976967370441516, 5.95950095969289784 3.25374426991997634, 5.45950095969289784 3.11976967370441471, 2.94433781190019195 3.54721689059500989, 2.69433781190019195 3.6142041887027907)),((5.5804222648752404 3.94683301343570214, 6.0804222648752404 3.81285841722014052, 6.44644766865967878 3.44683301343570214, 6.5804222648752404 2.94683301343570214, 6.44644766865967966 2.44683301343570259, 6.0804222648752404 2.08080760965126377, 5.5804222648752404 1.94683301343570214, 3 2.5, 2.75 2.56698729810778081, 2.56698729810778081 2.75, 2.5 3, 2.56698729810778037 3.24999999999999956, 2.75 3.43301270189221919, 3 3.5, 5.5804222648752404 3.94683301343570214)))'
                     if self.geos39 else
                     'MultiPolygon (((2.5 -0.00000000000000012, 2.43301270189221963 -0.24999999999999983, 2.25 -0.4330127018922193, 2 -0.5, 1.75 -0.43301270189221935, 1.56698729810778081 -0.25000000000000011, 1.5 0.00000000000000006, 1.25 2, 1.35048094716167078 2.37499999999999956, 1.62499999999999978 2.649519052838329, 2 2.75, 2.03076923076923066 2.75384615384615383, 2 3, 2.13397459621556118 3.49999999999999956, 2.5 3.86602540378443837, 3.00000000000000044 4, 3.50000000000000044 3.86602540378443837, 3.86602540378443837 3.5, 4 3, 3.875 1.99999999999999978, 3.75777222831138413 1.56250000000000044, 3.4375 1.24222777168861631, 3 1.125, 2.64615384615384608 1.1692307692307693, 2.5 -0.00000000000000012)),((2.94433781190019195 3.54721689059500989, 2.69433781190019195 3.6142041887027907, 2.51132511000797276 3.79721689059500989, 2.44433781190019195 4.04721689059500989, 2.51132511000797232 4.29721689059500989, 2.69433781190019195 4.48022959248722952, 2.94433781190019195 4.54721689059500989, 5.45950095969289784 5.11976967370441471, 5.95950095969289784 4.98579507748885309, 6.32552636347733621 4.61976967370441471, 6.45950095969289784 4.11976967370441471, 6.3255263634773371 3.61976967370441516, 5.95950095969289784 3.25374426991997634, 5.45950095969289784 3.11976967370441471, 2.94433781190019195 3.54721689059500989)),((5.5804222648752404 3.94683301343570214, 6.0804222648752404 3.81285841722014052, 6.44644766865967878 3.44683301343570214, 6.5804222648752404 2.94683301343570214, 6.44644766865967966 2.44683301343570259, 6.0804222648752404 2.08080760965126377, 5.5804222648752404 1.94683301343570214, 3 2.5, 2.75 2.56698729810778081, 2.56698729810778081 2.75, 2.5 3, 2.56698729810778037 3.24999999999999956, 2.75 3.43301270189221919, 3 3.5, 5.5804222648752404 3.94683301343570214)))'],
                 ['LineString (6 2, 9 2, 9 3, 11 5)', 2, 7, 3,
                  'MultiPolygon (((5.13397459621556163 1.49999999999999978, 5 2, 5.13397459621556074 2.49999999999999956, 5.5 2.86602540378443837, 6.61565808125483201 3.29902749321661304, 6.86570975577233966 4.23223304703362935, 7.96891108675446347 6.74999999999999822, 9.25 8.03108891324553475, 11 8.5, 12.75000000000000178 8.03108891324553475, 14.03108891324553475 6.75, 14.5 4.99999999999999911, 14.03108891324553653 3.25000000000000133, 12.75 1.9689110867544648, 10.86920158655618174 1.1448080812814232, 10.81722403411685463 0.95082521472477743, 10.04917478527522334 0.18277596588314599, 9 -0.09834957055044669, 7.95082521472477666 0.18277596588314587, 5.5 1.13397459621556118, 5.13397459621556163 1.49999999999999978)))'
                  if self.geos39 else
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
        tests = [['LineString (6 2, 9 2, 9 3, 11 5)', 3, 'GeometryCollection EMPTY'],
                 ['LineStringM (6 2 1, 9 2 1.5, 9 3 0.5, 11 5 2)', 3,
                  'MultiPolygon (((5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.54510095773215994 2.71209174647768014, 8.78349364905388974 3.125, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.34232758349701164 2.90707123255090094, 9.649519052838329 2.375, 9.75 1.99999999999999978, 9.649519052838329 1.62500000000000022, 9.375 1.350480947161671, 9 1.25, 6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2)))'
                  if self.geos39 else
                  'MultiPolygon (((6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.54510095773215994 2.71209174647768014, 8.78349364905388974 3.125, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.34232758349701164 2.90707123255090094, 9.649519052838329 2.375, 9.75 1.99999999999999978, 9.649519052838329 1.62500000000000022, 9.375 1.350480947161671, 9 1.25, 6 1.5)))'],
                 ['MultiLineStringM ((6 2 1, 9 2 1.5, 9 3 0.5, 11 5 2),(1 2 0.5, 3 2 0.2))', 3,
                  'MultiPolygon (((5.5 2, 5.56698729810778037 2.24999999999999956, 5.75 2.43301270189221919, 6 2.5, 8.54510095773215994 2.71209174647768014, 8.78349364905388974 3.125, 10.13397459621556074 5.49999999999999911, 10.5 5.86602540378443837, 11 6, 11.5 5.86602540378443837, 11.86602540378443926 5.5, 12 5, 11.86602540378443926 4.5, 11.5 4.13397459621556163, 9.34232758349701164 2.90707123255090094, 9.649519052838329 2.375, 9.75 1.99999999999999978, 9.649519052838329 1.62500000000000022, 9.375 1.350480947161671, 9 1.25, 6 1.5, 5.75 1.56698729810778059, 5.56698729810778037 1.75, 5.5 2)),((0.875 1.78349364905389041, 0.78349364905389041 1.875, 0.75 2, 0.7834936490538903 2.125, 0.875 2.21650635094610982, 1 2.25, 3 2.10000000000000009, 3.04999999999999982 2.08660254037844384, 3.08660254037844384 2.04999999999999982, 3.10000000000000009 2, 3.08660254037844384 1.94999999999999996, 3.04999999999999982 1.91339745962155616, 3 1.89999999999999991, 1 1.75, 0.875 1.78349364905389041)))'
                  if self.geos39 else
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
        tests = [["POLYGON((0 0, 0 2, 1 2, 2 2, 2 0, 0 0))",
                  "POLYGON((0.5 0.5, 0.5 2.5, 1.5 2.5, 2.5 2.5, 2.5 0.5, 0.5 0.5))", 0.707106781186548],
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

    def testConvertToCurves(self):
        tests = [
            [
                "LINESTRING Z (3 3 3,2.4142135623731 1.58578643762691 3,1 1 3,-0.414213562373092 1.5857864376269 3,-1 2.99999999999999 3,-0.414213562373101 4.41421356237309 3,0.999999999999991 5 3,2.41421356237309 4.4142135623731 3,3 3 3)",
                "CompoundCurveZ (CircularStringZ (3 3 3, -1 2.99999999999998979 3, 3 3 3))", 0.00000001, 0.0000001],
            ["LINESTRING(0 0,10 0,10 10,0 10,0 0)", "CompoundCurve((0 0,10 0,10 10,0 10,0 0))", 0.00000001, 0.00000001],
            ["LINESTRING(0 0,10 0,10 10,0 10)", "CompoundCurve((0 0,10 0,10 10,0 10))", 0.00000001, 0.00000001],
            ["LINESTRING(10 10,0 10,0 0,10 0)", "CompoundCurve((10 10,0 10,0 0,10 0))", 0.0000001, 0.00000001],
            ["LINESTRING(0 0, 1 1)", "CompoundCurve((0 0, 1 1))", 0.00000001, 0.00000001],
            ["GEOMETRYCOLLECTION(LINESTRING(10 10,10 11),LINESTRING(10 11,11 11),LINESTRING(11 11,10 10))",
             "MultiCurve (CompoundCurve ((10 10, 10 11)),CompoundCurve ((10 11, 11 11)),CompoundCurve ((11 11, 10 10)))",
             0.000001, 0.000001],
            ["GEOMETRYCOLLECTION(LINESTRING(4 4,4 8),CIRCULARSTRING(4 8,6 10,8 8),LINESTRING(8 8,8 4))",
             "MultiCurve (CompoundCurve ((4 4, 4 8)),CompoundCurve (CircularString (4 8, 6 10, 8 8)),CompoundCurve ((8 8, 8 4)))",
             0.0000001, 0.0000001],
            [
                "LINESTRING(-13151357.927248 3913656.64539871,-13151419.0845266 3913664.12016378,-13151441.323537 3913666.61175286,-13151456.8908442 3913666.61175286,-13151476.9059536 3913666.61175286,-13151496.921063 3913666.61175287,-13151521.3839744 3913666.61175287,-13151591.4368571 3913665.36595828)",
                "CompoundCurve ((-13151357.92724799923598766 3913656.64539870992302895, -13151419.08452660031616688 3913664.12016378017142415, -13151441.32353699952363968 3913666.61175285978242755, -13151456.8908441998064518 3913666.61175285978242755, -13151476.90595359914004803 3913666.61175285978242755, -13151496.92106300033628941 3913666.61175287002697587, -13151521.38397439941763878 3913666.61175287002697587, -13151591.43685710057616234 3913665.36595827993005514))",
                0.000001, 0.0000001],
            ["Point( 1 2 )", "Point( 1 2 )", 0.00001, 0.00001],
            ["MultiPoint( 1 2, 3 4 )", "MultiPoint( (1 2 ), (3 4 ))", 0.00001, 0.00001],
            # A polygon converts to curve
            [
                "POLYGON((3 3,2.4142135623731 1.58578643762691,1 1,-0.414213562373092 1.5857864376269,-1 2.99999999999999,-0.414213562373101 4.41421356237309,0.999999999999991 5,2.41421356237309 4.4142135623731,3 3))",
                "CURVEPOLYGON(COMPOUNDCURVE(CircularString(3 3, -1 2.99999999999998979, 3 3)))", 0.00000001,
                0.00000001],
            # The same polygon, even if already CURVEPOLYGON, still converts to curve
            [
                "CURVEPOLYGON((3 3,2.4142135623731 1.58578643762691,1 1,-0.414213562373092 1.5857864376269,-1 2.99999999999999,-0.414213562373101 4.41421356237309,0.999999999999991 5,2.41421356237309 4.4142135623731,3 3))",
                "CURVEPOLYGON(COMPOUNDCURVE(CircularString(3 3, -1 2.99999999999998979, 3 3)))", 0.00000001,
                0.00000001],
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            distance_tolerance = t[2]
            angle_tolerance = t[3]
            o = g1.convertToCurves(distance_tolerance, angle_tolerance)
            self.assertTrue(compareWkt(o.asWkt(), t[1], 0.00001),
                            "clipped: mismatch Expected:\n{}\nGot:\n{}\n".format(t[1], o.asWkt()))

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
            self.assertEqual(res, t[2],
                             "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(g1.asWkt(), g2.asWkt(), t[2],
                                                                                       res))

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
            self.assertEqual(res, t[2],
                             "mismatch for {} to {}, expected:\n{}\nGot:\n{}\n".format(g1.asWkt(), t[1].toString(),
                                                                                       t[2], res))

    def testOffsetCurve(self):
        tests = [
            ["LINESTRING (0 0, 0 100, 100 100)", 1, "LineString (-1 0, -1 101, 100 101)"],
            ["LINESTRING (0 0, 0 100, 100 100)", -1, "LineString (1 0, 1 99, 100 99)"],
            ["LINESTRING (100 100, 0 100, 0 0)", 1, "LineString (100 99, 1 99, 1 0)"],
            ["LINESTRING (100 100, 0 100, 0 0)", -1, "LineString (100 101, -1 101, -1 0)"],
            # linestring which becomes multilinestring -- the actual offset curve calculated by GEOS looks bad, but we shouldn't crash here
            [
                "LINESTRING (259329.820 5928370.79, 259324.337 5928371.758, 259319.678 5928372.33, 259317.064 5928372.498 )",
                100,
                "MultiLineString ((259313.3 5928272.5, 259312.5 5928272.6),(259312.4 5928272.3, 259309.5 5928272.8, 259307.5 5928273.1))"],
            ["MULTILINESTRING ((0 0, 0 100, 100 100),(100 100, 0 100, 0 0))", 1,
             "MultiLineString ((-1 0, -1 101, 100 101),(100 99, 1 99, 1 0))"]
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
            [
                "MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))"],
            [
                "GeometryCollection(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "GeometryCollection (Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1)),Polygon ((100 100, 100 200, 200 200, 200 100, 100 100)))"]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.forceRHR()
            self.assertEqual(res.asWkt(1), t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res.asWkt(1)))

    def testForceCW(self):
        tests = [
            ["", ""],
            ["Point (100 100)", "Point (100 100)"],
            ["LINESTRING (0 0, 0 100, 100 100)", "LineString (0 0, 0 100, 100 100)"],
            ["LINESTRING (100 100, 0 100, 0 0)", "LineString (100 100, 0 100, 0 0)"],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", "Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1))"],
            [
                "MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "MultiPolygon (((-1 -1, 0 2, 4 2, 4 0, -1 -1)),((100 100, 100 200, 200 200, 200 100, 100 100)))"],
            [
                "GeometryCollection(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "GeometryCollection (Polygon ((-1 -1, 0 2, 4 2, 4 0, -1 -1)),Polygon ((100 100, 100 200, 200 200, 200 100, 100 100)))"]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.forcePolygonClockwise()
            self.assertEqual(res.asWkt(1), t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res.asWkt(1)))

    def testForceCCW(self):
        tests = [
            ["", ""],
            ["Point (100 100)", "Point (100 100)"],
            ["LINESTRING (0 0, 0 100, 100 100)", "LineString (0 0, 0 100, 100 100)"],
            ["LINESTRING (100 100, 0 100, 0 0)", "LineString (100 100, 0 100, 0 0)"],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", "Polygon ((-1 -1, 4 0, 4 2, 0 2, -1 -1))"],
            [
                "MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "MultiPolygon (((-1 -1, 4 0, 4 2, 0 2, -1 -1)),((100 100, 200 100, 200 200, 100 200, 100 100)))"],
            [
                "GeometryCollection(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                "GeometryCollection (Polygon ((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon ((100 100, 200 100, 200 200, 100 200, 100 100)))"]
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.forcePolygonCounterClockwise()
            self.assertEqual(res.asWkt(1), t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1], res.asWkt(1)))

    def testLineStringFromBezier(self):
        tests = [
            [QgsPoint(1, 1), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10), 5,
             'LineString (1 1, 5.5 1.9, 8.7 4.2, 11.6 6.8, 15 9.1, 20 10)'],
            [QgsPoint(1, 1), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(1, 1), 10,
             'LineString (1 1, 3.4 1.2, 5.3 1.9, 6.7 2.7, 7.5 3.6, 7.8 4.4, 7.5 4.9, 6.7 5, 5.3 4.5, 3.4 3.2, 1 1)'],
            [QgsPoint(1, 1), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10), 10,
             'LineString (1 1, 3.4 1.3, 5.5 1.9, 7.2 2.9, 8.7 4.2, 10.1 5.5, 11.6 6.8, 13.2 8.1, 15 9.1, 17.3 9.7, 20 10)'],
            [QgsPoint(1, 1), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10), 1,
             'LineString (1 1, 20 10)'],
            [QgsPoint(1, 1), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10), 0,
             'LineString EMPTY'],
            [QgsPoint(1, 1, 2), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10), 5,
             'LineString (1 1, 5.5 1.9, 8.7 4.2, 11.6 6.8, 15 9.1, 20 10)'],
            [QgsPoint(1, 1), QgsPoint(10, 1, 2), QgsPoint(10, 10), QgsPoint(20, 10), 5,
             'LineString (1 1, 5.5 1.9, 8.7 4.2, 11.6 6.8, 15 9.1, 20 10)'],
            [QgsPoint(1, 1, 2), QgsPoint(10, 1), QgsPoint(10, 10, 2), QgsPoint(20, 10), 5,
             'LineString (1 1, 5.5 1.9, 8.7 4.2, 11.6 6.8, 15 9.1, 20 10)'],
            [QgsPoint(1, 1, 2), QgsPoint(10, 1), QgsPoint(10, 10), QgsPoint(20, 10, 2), 5,
             'LineString (1 1, 5.5 1.9, 8.7 4.2, 11.6 6.8, 15 9.1, 20 10)'],
            [QgsPoint(1, 1, 1), QgsPoint(10, 1, 2), QgsPoint(10, 10, 3), QgsPoint(20, 10, 4), 5,
             'LineStringZ (1 1 1, 5.5 1.9 1.6, 8.7 4.2 2.2, 11.6 6.8 2.8, 15 9.1 3.4, 20 10 4)'],
            [QgsPoint(1, 1, 1, 10), QgsPoint(10, 1, 2, 9), QgsPoint(10, 10, 3, 2), QgsPoint(20, 10, 4, 1), 5,
             'LineStringZM (1 1 1 10, 5.5 1.9 1.6 8.8, 8.7 4.2 2.2 6.7, 11.6 6.8 2.8 4.3, 15 9.1 3.4 2.2, 20 10 4 1)']
        ]
        for t in tests:
            res = QgsLineString.fromBezierCurve(t[0], t[1], t[2], t[3], t[4])
            self.assertEqual(res.asWkt(1), t[5],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[5], res.asWkt(1)))

    def testIsGeosValid(self):
        tests = [
            ["", False, False, ''],
            ["Point (100 100)", True, True, ''],
            ["MultiPoint (100 100, 100 200)", True, True, ''],
            ["LINESTRING (0 0, 0 100, 100 100)", True, True, ''],
            ["POLYGON((-1 -1, 4 0, 4 2, 0 2, -1 -1))", True, True, ''],
            [
                "MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                True, True, ''],
            [
                'MultiPolygon (((159865.14786298031685874 6768656.31838363595306873, 159858.97975336571107619 6769211.44824895076453686, 160486.07089751763851382 6769211.44824895076453686, 160481.95882444124436006 6768658.37442017439752817, 160163.27316101978067309 6768658.37442017439752817, 160222.89822062765597366 6769116.87056819349527359, 160132.43261294672265649 6769120.98264127038419247, 160163.27316101978067309 6768658.37442017439752817, 159865.14786298031685874 6768656.31838363595306873)))',
                False, True, 'Ring self-intersection'],
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
            [
                "MULTIPOLYGON(Polygon((-1 -1, 4 0, 4 2, 0 2, -1 -1)),Polygon((100 100, 200 100, 200 200, 100 200, 100 100)))",
                [], [], []],
            ['POLYGON ((200 400, 400 400, 400 200, 300 200, 350 250, 250 250, 300 200, 200 200, 200 400))',
             [QgsGeometry.Error('Ring self-intersection', QgsPointXY(300, 200))], [], []],
            [
                'MultiPolygon (((159865.14786298031685874 6768656.31838363595306873, 159858.97975336571107619 6769211.44824895076453686, 160486.07089751763851382 6769211.44824895076453686, 160481.95882444124436006 6768658.37442017439752817, 160163.27316101978067309 6768658.37442017439752817, 160222.89822062765597366 6769116.87056819349527359, 160132.43261294672265649 6769120.98264127038419247, 160163.27316101978067309 6768658.37442017439752817, 159865.14786298031685874 6768656.31838363595306873)))',
                [QgsGeometry.Error('Ring self-intersection',
                                   QgsPointXY(160163.27316101978067309, 6768658.37442017439752817))], [], []],
            ['Polygon((0 3, 3 0, 3 3, 0 0, 0 3))', [QgsGeometry.Error('Self-intersection', QgsPointXY(1.5, 1.5))],
             [QgsGeometry.Error('Self-intersection', QgsPointXY(1.5, 1.5))],
             [QgsGeometry.Error('segments 0 and 2 of line 0 intersect at 1.5, 1.5', QgsPointXY(1.5, 1.5))]],
        ]
        for t in tests:
            g1 = QgsGeometry.fromWkt(t[0])
            res = g1.validateGeometry(QgsGeometry.ValidatorGeos)
            self.assertEqual(res, t[1],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[1],
                                                                                 res[0].where() if res else ''))
            res = g1.validateGeometry(QgsGeometry.ValidatorGeos, QgsGeometry.FlagAllowSelfTouchingHoles)
            self.assertEqual(res, t[2],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[2],
                                                                                 res[0].where() if res else ''))
            res = g1.validateGeometry(QgsGeometry.ValidatorQgisInternal)
            self.assertEqual(res, t[3],
                             "mismatch for {}, expected:\n{}\nGot:\n{}\n".format(t[0], t[3],
                                                                                 res[0].where() if res else ''))

    def testCollectDuplicateNodes(self):
        g = QgsGeometry.fromWkt("LineString (1 1, 1 1, 1 1, 1 2, 1 3, 1 3, 1 3, 1 4, 1 5, 1 6, 1 6)")
        res = g.constGet().collectDuplicateNodes()
        self.assertCountEqual(res, [QgsVertexId(-1, -1, 1), QgsVertexId(-1, -1, 2), QgsVertexId(-1, -1, 5),
                                    QgsVertexId(-1, -1, 6), QgsVertexId(-1, -1, 10)])

        g = QgsGeometry.fromWkt("LineString (1 1, 1 2, 1 3, 1 4, 1 5, 1 6)")
        res = g.constGet().collectDuplicateNodes()
        self.assertFalse(res)

        g = QgsGeometry.fromWkt(
            "LineStringZ (1 1 1, 1 1 2, 1 1 3, 1 2 1, 1 3 1, 1 3 1, 1 3 2, 1 4 1, 1 5 1, 1 6 1, 1 6 2)")
        res = g.constGet().collectDuplicateNodes()
        self.assertCountEqual(res, [QgsVertexId(-1, -1, 1), QgsVertexId(-1, -1, 2), QgsVertexId(-1, -1, 5),
                                    QgsVertexId(-1, -1, 6), QgsVertexId(-1, -1, 10)])

        # consider z values
        res = g.constGet().collectDuplicateNodes(useZValues=True)
        self.assertEqual(res, [QgsVertexId(-1, -1, 5)])

        # tolerance
        g = QgsGeometry.fromWkt("LineString (1 1, 1 1.1, 1 2, 1 3, 1 3, 1 4, 1 5)")
        res = g.constGet().collectDuplicateNodes()
        self.assertCountEqual(res, [QgsVertexId(-1, -1, 4)])
        res = g.constGet().collectDuplicateNodes(epsilon=0.5)
        self.assertCountEqual(res, [QgsVertexId(-1, -1, 1), QgsVertexId(-1, -1, 4)])

    def testRandomPoints(self):
        """
        Test QgsGeometry.randomPointsInPolygon.

        This just test the Python operation of this function -- more tests in testqgsgeometry.cpp
        """

        # no random points inside null geometry
        g = QgsGeometry()
        with self.assertRaises(ValueError):
            res = g.randomPointsInPolygon(100)
        # no random points inside linestring
        g = QgsGeometry.fromWkt('LineString(4 5, 6 6)')
        with self.assertRaises(TypeError):
            res = g.randomPointsInPolygon(100)
        # good!
        g = QgsGeometry.fromWkt('Polygon(( 5 15, 10 15, 10 20, 5 20, 5 15 ), (6 16, 8 16, 8 18, 6 16 ))')
        res = g.randomPointsInPolygon(100)
        self.assertEqual(len(res), 100)
        g = QgsGeometry.fromWkt(
            'MultiPolygon((( 5 15, 10 15, 10 20, 5 20, 5 15 ), (6 16, 8 16, 8 18, 6 16 )), (( 105 115, 110 115, 110 120, 105 120, 105 115 ), (106 116, 108 116, 108 118, 106 116 )))')
        res = g.randomPointsInPolygon(100)
        self.assertEqual(len(res), 100)
        res2 = g.randomPointsInPolygon(100)
        self.assertNotEqual(res, res2)

        # with seed
        res = g.randomPointsInPolygon(100, seed=123123)
        res2 = g.randomPointsInPolygon(100, seed=123123)
        self.assertEqual(res, res2)

    def testLineStringFromQPolygonF(self):
        line = QgsLineString.fromQPolygonF(QPolygonF())
        self.assertEqual(line.asWkt(0), 'LineString EMPTY')
        line = QgsLineString.fromQPolygonF(QPolygonF([QPointF(1, 2), QPointF(3, 4)]))
        self.assertEqual(line.asWkt(1), 'LineString (1 2, 3 4)')
        line = QgsLineString.fromQPolygonF(
            QPolygonF([QPointF(1.5, 2.5), QPointF(3, 4), QPointF(3, 6.5), QPointF(1.5, 2.5)]))
        self.assertEqual(line.asWkt(1), 'LineString (1.5 2.5, 3 4, 3 6.5, 1.5 2.5)')

    def testCoerce(self):
        """Test coerce function"""

        def coerce_to_wkt(wkt, type, defaultZ=None, defaultM=None):
            geom = QgsGeometry.fromWkt(wkt)
            if defaultZ is not None or defaultM is not None:
                return [g.asWkt(2) for g in geom.coerceToType(type, defaultZ or 0, defaultM or 0)]
            else:
                return [g.asWkt(2) for g in geom.coerceToType(type)]

        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.Point), ['Point (1 1)'])
        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3)', QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 3 3)'])
        self.assertEqual(coerce_to_wkt('Polygon((1 1, 2 2, 1 2, 1 1))', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 2 2, 1 2, 1 1))'])

        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3)', QgsWkbTypes.Point),
                         ['Point (1 1)', 'Point (2 2)', 'Point (3 3)'])
        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3)', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 2 2, 3 3, 1 1))'])
        self.assertEqual(coerce_to_wkt('Polygon((1 1, 2 2, 1 2, 1 1))', QgsWkbTypes.Point),
                         ['Point (1 1)', 'Point (2 2)', 'Point (1 2)'])
        self.assertEqual(coerce_to_wkt('Polygon((1 1, 2 2, 1 2, 1 1))', QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 1 2, 1 1)'])

        self.assertEqual(coerce_to_wkt('Point z (1 1 3)', QgsWkbTypes.Point), ['Point (1 1)'])
        self.assertEqual(coerce_to_wkt('Point z (1 1 3)', QgsWkbTypes.PointZ), ['PointZ (1 1 3)'])

        # Adding Z back
        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.PointZ), ['PointZ (1 1 0)'])

        # Adding Z/M with defaults
        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.PointZ, defaultZ=222), ['PointZ (1 1 222)'])
        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.PointM, defaultM=333), ['PointM (1 1 333)'])
        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.PointZM, defaultZ=222, defaultM=333), ['PointZM (1 1 222 333)'])

        # Adding M back
        self.assertEqual(coerce_to_wkt('Point (1 1)', QgsWkbTypes.PointM), ['PointM (1 1 0)'])
        self.assertEqual(coerce_to_wkt('Point m (1 1 3)', QgsWkbTypes.Point), ['Point (1 1)'])
        self.assertEqual(coerce_to_wkt('Point(1 3)', QgsWkbTypes.MultiPoint), ['MultiPoint ((1 3))'])
        self.assertEqual(coerce_to_wkt('MultiPoint((1 3), (2 2))', QgsWkbTypes.MultiPoint),
                         ['MultiPoint ((1 3),(2 2))'])

        self.assertEqual(coerce_to_wkt('Polygon((1 1, 2 2, 3 3, 1 1))', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 2 2, 3 3, 1 1))'])
        self.assertEqual(coerce_to_wkt('Polygon z ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 2 2, 3 3, 1 1))'])
        self.assertEqual(coerce_to_wkt('Polygon z ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', QgsWkbTypes.PolygonZ),
                         ['PolygonZ ((1 1 1, 2 2 2, 3 3 3, 1 1 1))'])

        # Adding Z back
        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 2 2, 3 3, 1 1))', QgsWkbTypes.PolygonZ),
                         ['PolygonZ ((1 1 0, 2 2 0, 3 3 0, 1 1 0))'])

        # Adding M back
        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 2 2, 3 3, 1 1))', QgsWkbTypes.PolygonM),
                         ['PolygonM ((1 1 0, 2 2 0, 3 3 0, 1 1 0))'])

        self.assertEqual(coerce_to_wkt('Polygon m ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 2 2, 3 3, 1 1))'])
        self.assertEqual(coerce_to_wkt('Polygon m ((1 1 1, 2 2 2, 3 3 3, 1 1 1))', QgsWkbTypes.PolygonM),
                         ['PolygonM ((1 1 1, 2 2 2, 3 3 3, 1 1 1))'])
        self.assertEqual(coerce_to_wkt('Polygon((1 1, 2 2, 3 3, 1 1))', QgsWkbTypes.MultiPolygon),
                         ['MultiPolygon (((1 1, 2 2, 3 3, 1 1)))'])
        self.assertEqual(
            coerce_to_wkt('MultiPolygon(((1 1, 2 2, 3 3, 1 1)), ((1 1, 2 2, 3 3, 1 1)))', QgsWkbTypes.MultiPolygon),
            ['MultiPolygon (((1 1, 2 2, 3 3, 1 1)),((1 1, 2 2, 3 3, 1 1)))'])

        self.assertEqual(coerce_to_wkt('LineString(1 1, 2 2, 3 3, 1 1)', QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 3 3, 1 1)'])
        self.assertEqual(coerce_to_wkt('LineString z (1 1 1, 2 2 2, 3 3 3, 1 1 1)', QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 3 3, 1 1)'])
        self.assertEqual(coerce_to_wkt('LineString z (1 1 1, 2 2 2, 3 3 3, 1 1 1)', QgsWkbTypes.LineStringZ),
                         ['LineStringZ (1 1 1, 2 2 2, 3 3 3, 1 1 1)'])
        self.assertEqual(coerce_to_wkt('LineString m (1 1 1, 2 2 2, 3 3 3, 1 1 1)', QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 3 3, 1 1)'])
        self.assertEqual(coerce_to_wkt('LineString m (1 1 1, 2 2 2, 3 3 3, 1 1 1)', QgsWkbTypes.LineStringM),
                         ['LineStringM (1 1 1, 2 2 2, 3 3 3, 1 1 1)'])

        # Adding Z back
        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3, 1 1)', QgsWkbTypes.LineStringZ),
                         ['LineStringZ (1 1 0, 2 2 0, 3 3 0, 1 1 0)'])

        # Adding M back
        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3, 1 1)', QgsWkbTypes.LineStringM),
                         ['LineStringM (1 1 0, 2 2 0, 3 3 0, 1 1 0)'])

        self.assertEqual(coerce_to_wkt('LineString(1 1, 2 2, 3 3, 1 1)', QgsWkbTypes.MultiLineString),
                         ['MultiLineString ((1 1, 2 2, 3 3, 1 1))'])
        self.assertEqual(coerce_to_wkt('MultiLineString((1 1, 2 2, 3 3, 1 1), (1 1, 2 2, 3 3, 1 1))',
                                       QgsWkbTypes.MultiLineString),
                         ['MultiLineString ((1 1, 2 2, 3 3, 1 1),(1 1, 2 2, 3 3, 1 1))'])

        # Test Multi -> Single
        self.assertEqual(coerce_to_wkt('MultiLineString((1 1, 2 2, 3 3, 1 1), (10 1, 20 2, 30 3, 10 1))',
                                       QgsWkbTypes.LineString),
                         ['LineString (1 1, 2 2, 3 3, 1 1)', 'LineString (10 1, 20 2, 30 3, 10 1)'])

        # line -> points
        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3)', QgsWkbTypes.Point),
                         ['Point (1 1)', 'Point (2 2)', 'Point (3 3)'])

        self.assertEqual(coerce_to_wkt('LineString (1 1, 2 2, 3 3)', QgsWkbTypes.MultiPoint),
                         ['MultiPoint ((1 1),(2 2),(3 3))'])

        self.assertEqual(coerce_to_wkt('MultiLineString ((1 1, 2 2),(4 4, 3 3))', QgsWkbTypes.Point),
                         ['Point (1 1)', 'Point (2 2)', 'Point (4 4)', 'Point (3 3)'])

        self.assertEqual(coerce_to_wkt('MultiLineString ((1 1, 2 2),(4 4, 3 3))', QgsWkbTypes.MultiPoint),
                         ['MultiPoint ((1 1),(2 2),(4 4),(3 3))'])

        # line -> polygon
        self.assertEqual(coerce_to_wkt('LineString (1 1, 1 2, 2 2)', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 1 2, 2 2, 1 1))'])

        self.assertEqual(coerce_to_wkt('LineString (1 1, 1 2, 2 2)', QgsWkbTypes.MultiPolygon),
                         ['MultiPolygon (((1 1, 1 2, 2 2, 1 1)))'])

        self.assertEqual(coerce_to_wkt('MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4))', QgsWkbTypes.Polygon),
                         ['Polygon ((1 1, 1 2, 2 2, 1 1))', 'Polygon ((3 3, 4 3, 4 4, 3 3))'])

        self.assertEqual(coerce_to_wkt('MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4))',
                                       QgsWkbTypes.MultiPolygon),
                         ['MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))'])

        self.assertEqual(coerce_to_wkt('CircularString (1 1, 1 2, 2 2, 2 0, 1 1)', QgsWkbTypes.CurvePolygon),
                         ['CurvePolygon (CircularString (1 1, 1 2, 2 2, 2 0, 1 1))'])
        self.assertEqual(coerce_to_wkt('CircularString (1 1, 1 2, 2 2, 2 0, 1 1)', QgsWkbTypes.LineString)[0][:100],
                         'LineString (1 1, 0.99 1.01, 0.98 1.02, 0.97 1.03, 0.97 1.04, 0.96 1.05, 0.95 1.06, 0.94 1.06, 0.94 1')
        self.assertEqual(coerce_to_wkt('CircularString (1 1, 1 2, 2 2, 2 0, 1 1)', QgsWkbTypes.Polygon)[0][:100],
                         'Polygon ((1 1, 0.99 1.01, 0.98 1.02, 0.97 1.03, 0.97 1.04, 0.96 1.05, 0.95 1.06, 0.94 1.06, 0.94 1.0')

        # polygon -> points
        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 1 2, 2 2, 1 1))', QgsWkbTypes.Point),
                         ['Point (1 1)', 'Point (1 2)', 'Point (2 2)'])

        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 1 2, 2 2, 1 1))', QgsWkbTypes.MultiPoint),
                         ['MultiPoint ((1 1),(1 2),(2 2))'])

        self.assertEqual(
            coerce_to_wkt('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))', QgsWkbTypes.Point),
            ['Point (1 1)', 'Point (1 2)', 'Point (2 2)', 'Point (3 3)', 'Point (4 3)', 'Point (4 4)'])

        self.assertEqual(coerce_to_wkt('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                       QgsWkbTypes.MultiPoint), ['MultiPoint ((1 1),(1 2),(2 2),(3 3),(4 3),(4 4))'])

        # polygon -> lines
        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 1 2, 2 2, 1 1))', QgsWkbTypes.LineString),
                         ['LineString (1 1, 1 2, 2 2, 1 1)'])

        self.assertEqual(coerce_to_wkt('Polygon ((1 1, 1 2, 2 2, 1 1))', QgsWkbTypes.MultiLineString),
                         ['MultiLineString ((1 1, 1 2, 2 2, 1 1))'])

        self.assertEqual(coerce_to_wkt('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                       QgsWkbTypes.LineString),
                         ['LineString (1 1, 1 2, 2 2, 1 1)', 'LineString (3 3, 4 3, 4 4, 3 3)'])

        self.assertEqual(coerce_to_wkt('MultiPolygon (((1 1, 1 2, 2 2, 1 1)),((3 3, 4 3, 4 4, 3 3)))',
                                       QgsWkbTypes.MultiLineString),
                         ['MultiLineString ((1 1, 1 2, 2 2, 1 1),(3 3, 4 3, 4 4, 3 3))'])

    def testTriangularWaves(self):
        """Test triangular waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').triangularWaves(1, 2).asWkt(3), 'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').triangularWaves(1, 2).asWkt(3), '')  # don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(1, 2).asWkt(3),
                         'LineString (1 1, 1.25 3, 1.75 -1, 2.25 3, 2.75 -1, 3.25 3, 3.75 -1, 4.25 3, 4.75 -1, 5.25 3, 5.75 -1, 6.25 3, 6.75 -1, 7.25 3, 7.75 -1, 8.25 3, 8.75 -1, 9.25 3, 9.75 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(5, 2).asWkt(3),
                         'LineString (1 1, 2.125 3, 4.375 -1, 6.625 3, 8.875 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(8, 2).asWkt(3),
                         'LineString (1 1, 2.125 3, 4.375 -1, 6.625 3, 8.875 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(8, 2, True).asWkt(3),
                         'LineString (1 1, 3 3, 7 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(10, 2).asWkt(3),
                         'LineString (1 1, 3.25 3, 7.75 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWaves(20, 2).asWkt(3),
                         'LineString (1 1, 3.25 3, 7.75 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').triangularWaves(5, 2).asWkt(3),
                         'LineString (1 1, 2.125 3, 4.375 -1, 6.625 3, 8.875 -1, 8 2.125, 12 4.375, 8 6.625, 12 8.875, 10 10)')
        self.assertEqual(
            QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').triangularWaves(5, 2).asWkt(3),
            'MultiLineString ((1 1, 2.125 3, 4.375 -1, 6.625 3, 8.875 -1, 10 1),(10 10, 8.75 8, 6.25 12, 3.75 8, 1.25 12, 0 10))')
        self.assertEqual(
            QgsGeometry.fromWkt('Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').triangularWaves(5,
                                                                                                                     .5).asWkt(
                3),
            'Polygon ((1 1, 2.125 1.5, 4.375 0.5, 6.625 1.5, 8.875 0.5, 9.5 2.125, 10.5 4.375, 9.5 6.625, 10.5 8.875, 8.875 9.5, 6.625 10.5, 4.375 9.5, 2.125 10.5, 1.5 8.875, 0.5 6.625, 1.5 4.375, 0.5 2.125, 1 1),(3 4, 4.13 4.5, 6.39 3.5, 7.32 4.459, 7.554 6.919, 5.253 5.891, 3.058 5.234, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').triangularWaves(5,
                                                                                                             0.5).asWkt(
            3),
            'MultiPolygon (((1 1, 2.125 1.5, 4.375 0.5, 6.625 1.5, 8.875 0.5, 9.5 2.125, 10.5 4.375, 9.5 6.625, 10.5 8.875, 8.875 9.5, 6.625 10.5, 4.375 9.5, 2.125 10.5, 1.5 8.875, 0.5 6.625, 1.5 4.375, 0.5 2.125, 1 1)),((20 20, 19.5 21.219, 20.5 23.658, 19.5 26.097, 20.5 28.536, 21.042 29.665, 22.06 27.233, 24.491 26.216, 25.509 23.784, 27.94 22.767, 28.958 20.335, 28.536 19.5, 26.097 20.5, 23.658 19.5, 21.219 20.5, 20 20)))')

    def testTriangularRandomizedWaves(self):
        """Test randomized triangular waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').triangularWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').triangularWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         '')  # don't crash!
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
            'LineString (1 1, 1.499 3.933, 2.063 -1.999, 2.681 3.397, 3.375 -1.67, 4.343 3.846, 5 -1.525, 5.721 3.23, 6.489 -1.914, 7.217 3.431, 8.187 -1.778, 9.045 3.803, 9.591 -1.518, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
            'LineString (1 1, 2.499 3.933, 5.063 -1.999, 7.681 3.397, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWavesRandomized(8, 9, 2, 3, 1).asWkt(3),
            'LineString (1 1, 3.249 3.933, 7.313 -1.999, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWavesRandomized(10, 12, 1, 2, 1).asWkt(3),
            'LineString (1 1, 3.999 2.933, 9.127 -0.999, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1)').triangularWavesRandomized(20, 25, 2, 3, 1).asWkt(3),
            'LineString (1 1, 7.246 3.933, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').triangularWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
            'LineString (1 1, 2.499 3.933, 5.063 -1.999, 7.681 3.397, 12.67 1.375, 7.154 4.343, 12.525 7, 7.77 9.721, 10 10)')
        self.assertEqual(
            QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').triangularWavesRandomized(5, 6, 2, 3,
                                                                                                         1).asWkt(3),
            'MultiLineString ((1 1, 2.499 3.933, 5.063 -1.999, 7.681 3.397, 10 1),(10 10, 8.516 7.154, 5.859 12.525, 3.138 7.77, 0.371 12.914, 0 10))')
        self.assertEqual(QgsGeometry.fromWkt(
            'Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').triangularWavesRandomized(5, 6, 0.2,
                                                                                                           0.5,
                                                                                                           1).asWkt(3),
            'Polygon ((1 1, 2.499 1.48, 5.063 0.5, 7.681 1.319, 10.401 1.375, 9.546 4.343, 10.357 7, 9.731 9.721, 7.511 10.474, 4.783 9.671, 1.813 10.434, 1.441 7.955, 0.645 5.409, 1.449 2.476, 1 1),(3 4, 4.265 4.401, 7.061 3.599, 7.195 5.595, 5.738 6.835, 3.852 4.979, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').triangularWavesRandomized(
            5, 6, 0.2, 0.5, 1).asWkt(3),
            'MultiPolygon (((1 1, 2.499 1.48, 5.063 0.5, 7.681 1.319, 10.401 1.375, 9.546 4.343, 10.357 7, 9.731 9.721, 7.511 10.474, 4.783 9.671, 1.813 10.434, 1.441 7.955, 0.645 5.409, 1.449 2.476, 1 1)),((20 20, 19.599 21.265, 20.401 24.061, 19.741 26.767, 20.243 29.412, 21.858 28.6, 23.135 26.317, 25.615 24.795, 27.147 22.476, 29.37 21.112, 28.683 20.471, 26.123 19.643, 23.582 20.475, 20.626 19.71, 20 20)))')

    def testSquareWaves(self):
        """Test square waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').squareWaves(1, 2).asWkt(3), 'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').squareWaves(1, 2).asWkt(3), '')  # just don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(1, 2).asWkt(3),
                         'LineString (1 1, 1 3, 1.5 3, 1.5 -1, 2 -1, 2 3, 2.5 3, 2.5 -1, 3 -1, 3 3, 3.5 3, 3.5 -1, 4 -1, 4 3, 4.5 3, 4.5 -1, 5 -1, 5 3, 5.5 3, 5.5 -1, 6 -1, 6 3, 6.5 3, 6.5 -1, 7 -1, 7 3, 7.5 3, 7.5 -1, 8 -1, 8 3, 8.5 3, 8.5 -1, 9 -1, 9 3, 9.5 3, 9.5 -1, 10 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(5, 2).asWkt(3),
                         'LineString (1 1, 1 3, 3.25 3, 3.25 -1, 5.5 -1, 5.5 3, 7.75 3, 7.75 -1, 10 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(8, 2).asWkt(3),
                         'LineString (1 1, 1 3, 3.25 3, 3.25 -1, 5.5 -1, 5.5 3, 7.75 3, 7.75 -1, 10 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(8, 2, True).asWkt(3),
                         'LineString (1 1, 1 3, 5 3, 5 -1, 9 -1, 10 1)')  # this one could possibly be improved!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(10, 2).asWkt(3),
                         'LineString (1 1, 1 3, 5.5 3, 5.5 -1, 10 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWaves(20, 2).asWkt(3),
                         'LineString (1 1, 1 3, 5.5 3, 5.5 -1, 10 -1, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').squareWaves(5, 2).asWkt(3),
                         'LineString (1 1, 1 3, 3.25 3, 3.25 -1, 5.5 -1, 5.5 3, 7.75 3, 7.75 -1, 10 -1, 10 3, 8 3.25, 12 3.25, 12 5.5, 8 5.5, 8 7.75, 12 7.75, 12 10, 10 10)')
        self.assertEqual(QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').squareWaves(5, 2).asWkt(3),
                         'MultiLineString ((1 1, 1 3, 3.25 3, 3.25 -1, 5.5 -1, 5.5 3, 7.75 3, 7.75 -1, 10 -1, 10 1),(10 10, 10 8, 7.5 8, 7.5 12, 5 12, 5 8, 2.5 8, 2.5 12, 0 12, 0 10))')
        self.assertEqual(
            QgsGeometry.fromWkt('Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').squareWaves(5,
                                                                                                                 0.5).asWkt(
                3),
            'Polygon ((1 1, 1 1.5, 3.25 1.5, 3.25 0.5, 5.5 0.5, 5.5 1.5, 7.75 1.5, 7.75 0.5, 10 0.5, 10 1.5, 9.5 3.25, 10.5 3.25, 10.5 5.5, 9.5 5.5, 9.5 7.75, 10.5 7.75, 10.5 10, 9.5 10, 7.75 9.5, 7.75 10.5, 5.5 10.5, 5.5 9.5, 3.25 9.5, 3.25 10.5, 1 10.5, 1 9.5, 1.5 7.75, 0.5 7.75, 0.5 5.5, 1.5 5.5, 1.5 3.25, 0.5 3.25, 0.5 1, 1 1),(3 4, 3 4.5, 5.26 4.5, 5.26 3.5, 7.52 3.5, 7.52 4.5, 6.963 5.531, 7.911 5.847, 6.009 7.197, 6.325 6.248, 4.181 5.533, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').squareWaves(5, 0.5).asWkt(
            3),
            'MultiPolygon (((1 1, 1 1.5, 3.25 1.5, 3.25 0.5, 5.5 0.5, 5.5 1.5, 7.75 1.5, 7.75 0.5, 10 0.5, 10 1.5, 9.5 3.25, 10.5 3.25, 10.5 5.5, 9.5 5.5, 9.5 7.75, 10.5 7.75, 10.5 10, 9.5 10, 7.75 9.5, 7.75 10.5, 5.5 10.5, 5.5 9.5, 3.25 9.5, 3.25 10.5, 1 10.5, 1 9.5, 1.5 7.75, 0.5 7.75, 0.5 5.5, 1.5 5.5, 1.5 3.25, 0.5 3.25, 0.5 1, 1 1)),((20 20, 19.5 20, 19.5 22.439, 20.5 22.439, 20.5 24.877, 19.5 24.877, 19.5 27.316, 20.5 27.316, 20.5 29.755, 19.5 29.755, 21.905 28.802, 21.198 28.095, 22.922 26.371, 23.629 27.078, 25.354 25.354, 24.646 24.646, 26.371 22.922, 27.078 23.629, 28.802 21.905, 28.095 21.198, 29.755 20.5, 29.755 19.5, 27.316 19.5, 27.316 20.5, 24.877 20.5, 24.877 19.5, 22.439 19.5, 20 20)))')

    def testSquareRandomizedWaves(self):
        """Test randomized square waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').squareWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').squareWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         '')  # just don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1 3.997, 1.966 3.997, 1.966 -1.128, 2.966 -1.128, 2.966 3.236, 3.664 3.236, 3.664 -1.388, 4.499 -1.388, 4.499 3.936, 5.422 3.936, 5.422 -1.313, 6.184 -1.313, 6.184 3.443, 6.799 3.443, 6.799 -1.534, 7.756 -1.534, 7.756 3.457, 8.472 3.457, 8.472 -1.939, 9.361 -1.939, 9.361 3.716, 10 3.716, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1 3.997, 3.966 3.997, 3.966 -1.128, 6.966 -1.128, 6.966 3.236, 9.664 3.236, 9.664 -1.388, 10 -1.388, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWavesRandomized(8, 9, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1 3.997, 5.466 3.997, 5.466 -1.128, 9.966 -1.128, 9.966 3.236, 10 3.236, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWavesRandomized(10, 12, 1, 2, 1).asWkt(3),
                         'LineString (1 1, 1 2.997, 6.933 2.997, 6.933 -0.128, 10 -0.128, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').squareWavesRandomized(20, 25, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1 3.997, 10 3.997, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').squareWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
            'LineString (1 1, 1 3.997, 3.966 3.997, 3.966 -1.128, 6.966 -1.128, 6.966 3.236, 9.664 3.236, 9.664 -1.388, 12.388 3.499, 7.064 3.499, 7.064 6.422, 12.313 6.422, 12.313 9.184, 7.557 9.184, 7.557 10, 10 10)')
        self.assertEqual(
            QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').squareWavesRandomized(5, 6, 2, 3,
                                                                                                     1).asWkt(3),
            'MultiLineString ((1 1, 1 3.997, 3.966 3.997, 3.966 -1.128, 6.966 -1.128, 6.966 3.236, 9.664 3.236, 9.664 -1.388, 10 -1.388, 10 1),(10 10, 10 7.064, 7.077 7.064, 7.077 12.313, 4.315 12.313, 4.315 7.557, 1.7 7.557, 1.7 12.534, 0 12.534, 0 10))')
        self.assertEqual(QgsGeometry.fromWkt(
            'Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').squareWavesRandomized(3, 5, 0.2, 0.5,
                                                                                                       1).asWkt(3),
            'Polygon ((1 1, 1 1.499, 3.433 1.499, 3.433 0.762, 5.932 0.762, 5.932 1.271, 7.828 1.271, 7.828 0.684, 9.998 0.684, 9.998 1.481, 9.519 3.344, 10.294 3.344, 10.294 5.369, 9.667 5.369, 9.667 7.098, 10.36 7.098, 10.36 9.512, 9.663 9.512, 8.557 9.663, 8.557 10.482, 6.279 10.482, 6.279 9.585, 3.976 9.585, 3.976 10.228, 1.958 10.228, 1.958 9.54, 1.46 8.629, 0.551 8.629, 0.551 6.855, 1.218 6.855, 1.218 4.685, 0.622 4.685, 0.622 2.513, 1.324 2.513, 1.324 1, 1 1),(3 4, 3 4.287, 4.642 4.287, 4.642 3.565, 6.555 3.565, 6.555 4.21, 7.586 4.577, 8.163 4.77, 7.594 6.476, 6.9 6.244, 6.122 6.355, 5.946 6.883, 4.078 6.26, 4.22 5.832, 3.205 3.898, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').squareWavesRandomized(3, 5,
                                                                                                                   0.2,
                                                                                                                   0.5,
                                                                                                                   1).asWkt(
            3),
            'MultiPolygon (((1 1, 1 1.499, 3.433 1.499, 3.433 0.762, 5.932 0.762, 5.932 1.271, 7.828 1.271, 7.828 0.684, 9.998 0.684, 9.998 1.481, 9.519 3.344, 10.294 3.344, 10.294 5.369, 9.667 5.369, 9.667 7.098, 10.36 7.098, 10.36 9.512, 9.663 9.512, 8.557 9.663, 8.557 10.482, 6.279 10.482, 6.279 9.585, 3.976 9.585, 3.976 10.228, 1.958 10.228, 1.958 9.54, 1.46 8.629, 0.551 8.629, 0.551 6.855, 1.218 6.855, 1.218 4.685, 0.622 4.685, 0.622 2.513, 1.324 2.513, 1.324 1, 1 1)),((20 20, 19.713 20, 19.713 21.642, 20.435 21.642, 20.435 23.555, 19.79 23.555, 19.79 25.679, 20.398 25.679, 20.398 27.477, 19.666 27.477, 19.666 29.199, 20.222 29.199, 20.669 29.017, 20.988 29.336, 22.688 27.636, 22.359 27.308, 23.791 25.876, 24.117 26.202, 25.826 24.493, 25.332 23.999, 26.604 22.727, 27.204 23.327, 28.665 21.866, 28.128 21.329, 29.807 20.384, 29.807 19.722, 28.076 19.722, 28.076 20.36, 25.626 20.36, 25.626 19.652, 23.586 19.652, 23.586 20.43, 22.04 20.43, 22.04 19.758, 20 19.758, 20 20)))')

    def testRoundWaves(self):
        """Test round waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').roundWaves(1, 2).asWkt(3), 'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').roundWaves(1, 2).asWkt(3), '')  # just don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(1, 2).asWkt(3),
                         'LineString (1 1, 1.021 0.701, 1.044 0.408, 1.07 0.127, 1.097 -0.136, 1.125 -0.375, 1.153 -0.584, 1.18 -0.757, 1.206 -0.888, 1.23 -0.971, 1.25 -1, 1.318 -0.888, 1.374 -0.584, 1.421 -0.136, 1.462 0.408, 1.5 1, 1.538 1.592, 1.579 2.136, 1.626 2.584, 1.682 2.888, 1.75 3, 1.818 2.888, 1.874 2.584, 1.921 2.136, 1.962 1.592, 2 1, 2.038 0.408, 2.079 -0.136, 2.126 -0.584, 2.182 -0.888, 2.25 -1, 2.318 -0.888, 2.374 -0.584, 2.421 -0.136, 2.462 0.408, 2.5 1, 2.538 1.592, 2.579 2.136, 2.626 2.584, 2.682 2.888, 2.75 3, 2.818 2.888, 2.874 2.584, 2.921 2.136, 2.962 1.592, 3 1, 3.038 0.408, 3.079 -0.136, 3.126 -0.584, 3.182 -0.888, 3.25 -1, 3.318 -0.888, 3.374 -0.584, 3.421 -0.136, 3.462 0.408, 3.5 1, 3.538 1.592, 3.579 2.136, 3.626 2.584, 3.682 2.888, 3.75 3, 3.818 2.888, 3.874 2.584, 3.921 2.136, 3.962 1.592, 4 1, 4.038 0.408, 4.079 -0.136, 4.126 -0.584, 4.182 -0.888, 4.25 -1, 4.318 -0.888, 4.374 -0.584, 4.421 -0.136, 4.462 0.408, 4.5 1, 4.538 1.592, 4.579 2.136, 4.626 2.584, 4.682 2.888, 4.75 3, 4.818 2.888, 4.874 2.584, 4.921 2.136, 4.962 1.592, 5 1, 5.038 0.408, 5.079 -0.136, 5.126 -0.584, 5.182 -0.888, 5.25 -1, 5.318 -0.888, 5.374 -0.584, 5.421 -0.136, 5.462 0.408, 5.5 1, 5.538 1.592, 5.579 2.136, 5.626 2.584, 5.682 2.888, 5.75 3, 5.818 2.888, 5.874 2.584, 5.921 2.136, 5.962 1.592, 6 1, 6.038 0.408, 6.079 -0.136, 6.126 -0.584, 6.182 -0.888, 6.25 -1, 6.318 -0.888, 6.374 -0.584, 6.421 -0.136, 6.462 0.408, 6.5 1, 6.538 1.592, 6.579 2.136, 6.626 2.584, 6.682 2.888, 6.75 3, 6.818 2.888, 6.874 2.584, 6.921 2.136, 6.962 1.592, 7 1, 7.038 0.408, 7.079 -0.136, 7.126 -0.584, 7.182 -0.888, 7.25 -1, 7.318 -0.888, 7.374 -0.584, 7.421 -0.136, 7.462 0.408, 7.5 1, 7.538 1.592, 7.579 2.136, 7.626 2.584, 7.682 2.888, 7.75 3, 7.818 2.888, 7.874 2.584, 7.921 2.136, 7.962 1.592, 8 1, 8.038 0.408, 8.079 -0.136, 8.126 -0.584, 8.182 -0.888, 8.25 -1, 8.318 -0.888, 8.374 -0.584, 8.421 -0.136, 8.462 0.408, 8.5 1, 8.538 1.592, 8.579 2.136, 8.626 2.584, 8.682 2.888, 8.75 3, 8.818 2.888, 8.874 2.584, 8.921 2.136, 8.962 1.592, 9 1, 9.038 0.408, 9.079 -0.136, 9.126 -0.584, 9.182 -0.888, 9.25 -1, 9.318 -0.888, 9.374 -0.584, 9.421 -0.136, 9.462 0.408, 9.5 1, 9.538 1.592, 9.579 2.136, 9.626 2.584, 9.682 2.888, 9.75 3, 9.824 2.888, 9.892 2.584, 9.948 2.136, 9.986 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(5, 2).asWkt(3),
                         'LineString (1 1, 1.092 0.701, 1.198 0.408, 1.314 0.127, 1.437 -0.136, 1.563 -0.375, 1.689 -0.584, 1.811 -0.757, 1.927 -0.888, 2.033 -0.971, 2.125 -1, 2.431 -0.888, 2.683 -0.584, 2.895 -0.136, 3.079 0.408, 3.25 1, 3.421 1.592, 3.606 2.136, 3.817 2.584, 4.069 2.888, 4.375 3, 4.681 2.888, 4.933 2.584, 5.145 2.136, 5.329 1.592, 5.5 1, 5.671 0.408, 5.856 -0.136, 6.067 -0.584, 6.319 -0.888, 6.625 -1, 6.931 -0.888, 7.183 -0.584, 7.395 -0.136, 7.579 0.408, 7.75 1, 7.921 1.592, 8.106 2.136, 8.317 2.584, 8.569 2.888, 8.875 3, 9.208 2.888, 9.514 2.584, 9.766 2.136, 9.937 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(8, 2).asWkt(3),
                         'LineString (1 1, 1.092 0.701, 1.198 0.408, 1.314 0.127, 1.437 -0.136, 1.563 -0.375, 1.689 -0.584, 1.811 -0.757, 1.927 -0.888, 2.033 -0.971, 2.125 -1, 2.431 -0.888, 2.683 -0.584, 2.895 -0.136, 3.079 0.408, 3.25 1, 3.421 1.592, 3.606 2.136, 3.817 2.584, 4.069 2.888, 4.375 3, 4.681 2.888, 4.933 2.584, 5.145 2.136, 5.329 1.592, 5.5 1, 5.671 0.408, 5.856 -0.136, 6.067 -0.584, 6.319 -0.888, 6.625 -1, 6.931 -0.888, 7.183 -0.584, 7.395 -0.136, 7.579 0.408, 7.75 1, 7.921 1.592, 8.106 2.136, 8.317 2.584, 8.569 2.888, 8.875 3, 9.208 2.888, 9.514 2.584, 9.766 2.136, 9.937 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(8, 2, True).asWkt(3),
                         'LineString (1 1, 1.164 0.701, 1.352 0.408, 1.558 0.127, 1.776 -0.136, 2 -0.375, 2.224 -0.584, 2.442 -0.757, 2.648 -0.888, 2.836 -0.971, 3 -1, 3.544 -0.888, 3.992 -0.584, 4.368 -0.136, 4.696 0.408, 5 1, 5.304 1.592, 5.632 2.136, 6.008 2.584, 6.456 2.888, 7 3, 7.768 2.888, 8.524 2.584, 9.196 2.136, 9.712 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(10, 2).asWkt(3),
                         'LineString (1 1, 1.185 0.701, 1.396 0.408, 1.628 0.127, 1.873 -0.136, 2.125 -0.375, 2.377 -0.584, 2.622 -0.757, 2.854 -0.888, 3.066 -0.971, 3.25 -1, 3.862 -0.888, 4.366 -0.584, 4.789 -0.136, 5.158 0.408, 5.5 1, 5.842 1.592, 6.211 2.136, 6.634 2.584, 7.138 2.888, 7.75 3, 8.416 2.888, 9.028 2.584, 9.532 2.136, 9.874 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWaves(20, 2).asWkt(3),
                         'LineString (1 1, 1.185 0.701, 1.396 0.408, 1.628 0.127, 1.873 -0.136, 2.125 -0.375, 2.377 -0.584, 2.622 -0.757, 2.854 -0.888, 3.066 -0.971, 3.25 -1, 3.862 -0.888, 4.366 -0.584, 4.789 -0.136, 5.158 0.408, 5.5 1, 5.842 1.592, 6.211 2.136, 6.634 2.584, 7.138 2.888, 7.75 3, 8.416 2.888, 9.028 2.584, 9.532 2.136, 9.874 1.592, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').roundWaves(5, 2).asWkt(3),
                         'LineString (1 1, 1.092 0.701, 1.198 0.408, 1.314 0.127, 1.437 -0.136, 1.563 -0.375, 1.689 -0.584, 1.811 -0.757, 1.927 -0.888, 2.033 -0.971, 2.125 -1, 2.431 -0.888, 2.683 -0.584, 2.895 -0.136, 3.079 0.408, 3.25 1, 3.421 1.592, 3.606 2.136, 3.817 2.584, 4.069 2.888, 4.375 3, 4.681 2.888, 4.933 2.584, 5.145 2.136, 5.329 1.592, 5.5 1, 5.671 0.408, 5.856 -0.136, 6.067 -0.584, 6.319 -0.888, 6.625 -1, 6.931 -0.888, 7.183 -0.584, 7.395 -0.136, 7.579 0.408, 7.75 1, 7.921 1.592, 8.106 2.136, 8.317 2.584, 8.569 2.888, 8.875 3, 9.182 2.891, 9.44 2.609, 9.668 2.22, 9.885 1.792, 10.109 1.391, 10.36 1.083, 10.656 0.936, 11.015 1.016, 11.457 1.39, 12 2.125, 11.888 2.431, 11.584 2.683, 11.136 2.895, 10.592 3.079, 10 3.25, 9.408 3.421, 8.864 3.606, 8.416 3.817, 8.112 4.069, 8 4.375, 8.112 4.681, 8.416 4.933, 8.864 5.145, 9.408 5.329, 10 5.5, 10.592 5.671, 11.136 5.856, 11.584 6.067, 11.888 6.319, 12 6.625, 11.888 6.931, 11.584 7.183, 11.136 7.395, 10.592 7.579, 10 7.75, 9.408 7.921, 8.864 8.106, 8.416 8.317, 8.112 8.569, 8 8.875, 8.112 9.208, 8.416 9.514, 8.864 9.766, 9.408 9.937, 10 10)')
        self.assertEqual(QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').roundWaves(5, 2).asWkt(3),
                         'MultiLineString ((1 1, 1.092 0.701, 1.198 0.408, 1.314 0.127, 1.437 -0.136, 1.563 -0.375, 1.689 -0.584, 1.811 -0.757, 1.927 -0.888, 2.033 -0.971, 2.125 -1, 2.431 -0.888, 2.683 -0.584, 2.895 -0.136, 3.079 0.408, 3.25 1, 3.421 1.592, 3.606 2.136, 3.817 2.584, 4.069 2.888, 4.375 3, 4.681 2.888, 4.933 2.584, 5.145 2.136, 5.329 1.592, 5.5 1, 5.671 0.408, 5.856 -0.136, 6.067 -0.584, 6.319 -0.888, 6.625 -1, 6.931 -0.888, 7.183 -0.584, 7.395 -0.136, 7.579 0.408, 7.75 1, 7.921 1.592, 8.106 2.136, 8.317 2.584, 8.569 2.888, 8.875 3, 9.208 2.888, 9.514 2.584, 9.766 2.136, 9.937 1.592, 10 1),(10 10, 9.897 10.299, 9.78 10.592, 9.651 10.873, 9.515 11.136, 9.375 11.375, 9.235 11.584, 9.099 11.757, 8.97 11.888, 8.853 11.971, 8.75 12, 8.41 11.888, 8.13 11.584, 7.895 11.136, 7.69 10.592, 7.5 10, 7.31 9.408, 7.105 8.864, 6.87 8.416, 6.59 8.112, 6.25 8, 5.91 8.112, 5.63 8.416, 5.395 8.864, 5.19 9.408, 5 10, 4.81 10.592, 4.605 11.136, 4.37 11.584, 4.09 11.888, 3.75 12, 3.41 11.888, 3.13 11.584, 2.895 11.136, 2.69 10.592, 2.5 10, 2.31 9.408, 2.105 8.864, 1.87 8.416, 1.59 8.112, 1.25 8, 0.88 8.112, 0.54 8.416, 0.26 8.864, 0.07 9.408, 0 10))')
        self.assertEqual(
            QgsGeometry.fromWkt('Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').roundWaves(5,
                                                                                                                0.2).asWkt(
                3),
            'Polygon ((1 1, 1.092 0.97, 1.198 0.941, 1.314 0.913, 1.437 0.886, 1.563 0.863, 1.689 0.842, 1.811 0.824, 1.927 0.811, 2.033 0.803, 2.125 0.8, 2.431 0.811, 2.683 0.842, 2.895 0.886, 3.079 0.941, 3.25 1, 3.421 1.059, 3.606 1.114, 3.817 1.158, 4.069 1.189, 4.375 1.2, 4.681 1.189, 4.933 1.158, 5.145 1.114, 5.329 1.059, 5.5 1, 5.671 0.941, 5.856 0.886, 6.067 0.842, 6.319 0.811, 6.625 0.8, 6.931 0.811, 7.183 0.842, 7.395 0.886, 7.579 0.941, 7.75 1, 7.921 1.059, 8.106 1.114, 8.317 1.158, 8.569 1.189, 8.875 1.2, 9.18 1.19, 9.426 1.169, 9.62 1.149, 9.77 1.144, 9.884 1.166, 9.971 1.227, 10.038 1.341, 10.093 1.52, 10.145 1.777, 10.2 2.125, 10.189 2.431, 10.158 2.683, 10.114 2.895, 10.059 3.079, 10 3.25, 9.941 3.421, 9.886 3.606, 9.842 3.817, 9.811 4.069, 9.8 4.375, 9.811 4.681, 9.842 4.933, 9.886 5.145, 9.941 5.329, 10 5.5, 10.059 5.671, 10.114 5.856, 10.158 6.067, 10.189 6.319, 10.2 6.625, 10.189 6.931, 10.158 7.183, 10.114 7.395, 10.059 7.579, 10 7.75, 9.941 7.921, 9.886 8.106, 9.842 8.317, 9.811 8.569, 9.8 8.875, 9.81 9.18, 9.831 9.426, 9.851 9.62, 9.856 9.77, 9.834 9.884, 9.773 9.971, 9.659 10.038, 9.48 10.093, 9.223 10.145, 8.875 10.2, 8.569 10.189, 8.317 10.158, 8.106 10.114, 7.921 10.059, 7.75 10, 7.579 9.941, 7.395 9.886, 7.183 9.842, 6.931 9.811, 6.625 9.8, 6.319 9.811, 6.067 9.842, 5.856 9.886, 5.671 9.941, 5.5 10, 5.329 10.059, 5.145 10.114, 4.933 10.158, 4.681 10.189, 4.375 10.2, 4.069 10.189, 3.817 10.158, 3.606 10.114, 3.421 10.059, 3.25 10, 3.079 9.941, 2.895 9.886, 2.683 9.842, 2.431 9.811, 2.125 9.8, 1.82 9.81, 1.574 9.831, 1.38 9.851, 1.23 9.856, 1.116 9.834, 1.029 9.773, 0.962 9.659, 0.907 9.48, 0.855 9.223, 0.8 8.875, 0.811 8.569, 0.842 8.317, 0.886 8.106, 0.941 7.921, 1 7.75, 1.059 7.579, 1.114 7.395, 1.158 7.183, 1.189 6.931, 1.2 6.625, 1.189 6.319, 1.158 6.067, 1.114 5.856, 1.059 5.671, 1 5.5, 0.941 5.329, 0.886 5.145, 0.842 4.933, 0.811 4.681, 0.8 4.375, 0.811 4.069, 0.842 3.817, 0.886 3.606, 0.941 3.421, 1 3.25, 1.059 3.079, 1.114 2.895, 1.158 2.683, 1.189 2.431, 1.2 2.125, 1.189 1.792, 1.158 1.486, 1.114 1.234, 1.059 1.063, 1 1),(3 4, 3.093 3.97, 3.199 3.941, 3.315 3.913, 3.438 3.886, 3.565 3.863, 3.692 3.842, 3.815 3.824, 3.931 3.811, 4.037 3.803, 4.13 3.8, 4.437 3.811, 4.691 3.842, 4.903 3.886, 5.088 3.941, 5.26 4, 5.432 4.059, 5.617 4.114, 5.83 4.158, 6.083 4.189, 6.39 4.2, 6.697 4.19, 6.945 4.165, 7.145 4.137, 7.306 4.116, 7.437 4.11, 7.548 4.131, 7.649 4.188, 7.749 4.292, 7.857 4.453, 7.984 4.68, 7.876 4.968, 7.767 5.199, 7.658 5.386, 7.547 5.545, 7.437 5.689, 7.327 5.833, 7.216 5.992, 7.107 6.179, 6.998 6.41, 6.89 6.698, 6.707 6.663, 6.546 6.654, 6.4 6.662, 6.26 6.679, 6.115 6.698, 5.959 6.712, 5.781 6.712, 5.573 6.691, 5.326 6.641, 5.032 6.555, 4.744 6.446, 4.518 6.334, 4.344 6.214, 4.21 6.084, 4.107 5.94, 4.023 5.781, 3.95 5.602, 3.876 5.401, 3.791 5.175, 3.684 4.921, 3.585 4.748, 3.451 4.548, 3.298 4.341, 3.142 4.151, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').roundWaves(5, 0.2).asWkt(
            3),
            'MultiPolygon (((1 1, 1.092 0.97, 1.198 0.941, 1.314 0.913, 1.437 0.886, 1.563 0.863, 1.689 0.842, 1.811 0.824, 1.927 0.811, 2.033 0.803, 2.125 0.8, 2.431 0.811, 2.683 0.842, 2.895 0.886, 3.079 0.941, 3.25 1, 3.421 1.059, 3.606 1.114, 3.817 1.158, 4.069 1.189, 4.375 1.2, 4.681 1.189, 4.933 1.158, 5.145 1.114, 5.329 1.059, 5.5 1, 5.671 0.941, 5.856 0.886, 6.067 0.842, 6.319 0.811, 6.625 0.8, 6.931 0.811, 7.183 0.842, 7.395 0.886, 7.579 0.941, 7.75 1, 7.921 1.059, 8.106 1.114, 8.317 1.158, 8.569 1.189, 8.875 1.2, 9.18 1.19, 9.426 1.169, 9.62 1.149, 9.77 1.144, 9.884 1.166, 9.971 1.227, 10.038 1.341, 10.093 1.52, 10.145 1.777, 10.2 2.125, 10.189 2.431, 10.158 2.683, 10.114 2.895, 10.059 3.079, 10 3.25, 9.941 3.421, 9.886 3.606, 9.842 3.817, 9.811 4.069, 9.8 4.375, 9.811 4.681, 9.842 4.933, 9.886 5.145, 9.941 5.329, 10 5.5, 10.059 5.671, 10.114 5.856, 10.158 6.067, 10.189 6.319, 10.2 6.625, 10.189 6.931, 10.158 7.183, 10.114 7.395, 10.059 7.579, 10 7.75, 9.941 7.921, 9.886 8.106, 9.842 8.317, 9.811 8.569, 9.8 8.875, 9.81 9.18, 9.831 9.426, 9.851 9.62, 9.856 9.77, 9.834 9.884, 9.773 9.971, 9.659 10.038, 9.48 10.093, 9.223 10.145, 8.875 10.2, 8.569 10.189, 8.317 10.158, 8.106 10.114, 7.921 10.059, 7.75 10, 7.579 9.941, 7.395 9.886, 7.183 9.842, 6.931 9.811, 6.625 9.8, 6.319 9.811, 6.067 9.842, 5.856 9.886, 5.671 9.941, 5.5 10, 5.329 10.059, 5.145 10.114, 4.933 10.158, 4.681 10.189, 4.375 10.2, 4.069 10.189, 3.817 10.158, 3.606 10.114, 3.421 10.059, 3.25 10, 3.079 9.941, 2.895 9.886, 2.683 9.842, 2.431 9.811, 2.125 9.8, 1.82 9.81, 1.574 9.831, 1.38 9.851, 1.23 9.856, 1.116 9.834, 1.029 9.773, 0.962 9.659, 0.907 9.48, 0.855 9.223, 0.8 8.875, 0.811 8.569, 0.842 8.317, 0.886 8.106, 0.941 7.921, 1 7.75, 1.059 7.579, 1.114 7.395, 1.158 7.183, 1.189 6.931, 1.2 6.625, 1.189 6.319, 1.158 6.067, 1.114 5.856, 1.059 5.671, 1 5.5, 0.941 5.329, 0.886 5.145, 0.842 4.933, 0.811 4.681, 0.8 4.375, 0.811 4.069, 0.842 3.817, 0.886 3.606, 0.941 3.421, 1 3.25, 1.059 3.079, 1.114 2.895, 1.158 2.683, 1.189 2.431, 1.2 2.125, 1.189 1.792, 1.158 1.486, 1.114 1.234, 1.059 1.063, 1 1)),((20 20, 20.03 20.1, 20.059 20.215, 20.087 20.34, 20.114 20.473, 20.138 20.61, 20.158 20.746, 20.176 20.879, 20.189 21.005, 20.197 21.119, 20.2 21.219, 20.189 21.551, 20.158 21.824, 20.114 22.053, 20.059 22.253, 20 22.439, 19.941 22.624, 19.886 22.824, 19.842 23.053, 19.811 23.326, 19.8 23.658, 19.811 23.99, 19.842 24.263, 19.886 24.492, 19.941 24.692, 20 24.877, 20.059 25.063, 20.114 25.263, 20.158 25.492, 20.189 25.765, 20.2 26.097, 20.189 26.428, 20.158 26.702, 20.114 26.931, 20.059 27.131, 20 27.316, 19.941 27.502, 19.886 27.701, 19.842 27.931, 19.811 28.204, 19.8 28.536, 19.812 28.865, 19.844 29.126, 19.896 29.321, 19.963 29.454, 20.043 29.529, 20.134 29.55, 20.233 29.521, 20.336 29.446, 20.442 29.327, 20.547 29.17, 20.79 28.943, 21.005 28.771, 21.198 28.641, 21.378 28.538, 21.551 28.449, 21.724 28.36, 21.904 28.257, 22.098 28.126, 22.312 27.955, 22.555 27.728, 22.781 27.486, 22.953 27.271, 23.083 27.077, 23.186 26.897, 23.276 26.724, 23.365 26.552, 23.468 26.372, 23.598 26.178, 23.77 25.963, 23.996 25.721, 24.239 25.494, 24.453 25.323, 24.647 25.192, 24.827 25.089, 25 25, 25.173 24.911, 25.353 24.808, 25.547 24.677, 25.761 24.506, 26.004 24.279, 26.23 24.037, 26.402 23.822, 26.532 23.628, 26.635 23.448, 26.724 23.276, 26.814 23.103, 26.917 22.923, 27.047 22.729, 27.219 22.514, 27.445 22.272, 27.688 22.045, 27.902 21.874, 28.096 21.743, 28.276 21.64, 28.449 21.551, 28.622 21.462, 28.802 21.359, 28.995 21.229, 29.21 21.057, 29.453 20.83, 29.533 20.562, 29.59 20.369, 29.618 20.24, 29.612 20.163, 29.565 20.129, 29.472 20.125, 29.328 20.141, 29.128 20.167, 28.866 20.19, 28.536 20.2, 28.204 20.189, 27.931 20.158, 27.701 20.114, 27.502 20.059, 27.316 20, 27.131 19.941, 26.931 19.886, 26.702 19.842, 26.428 19.811, 26.097 19.8, 25.765 19.811, 25.492 19.842, 25.263 19.886, 25.063 19.941, 24.877 20, 24.692 20.059, 24.492 20.114, 24.263 20.158, 23.99 20.189, 23.658 20.2, 23.326 20.189, 23.053 20.158, 22.824 20.114, 22.624 20.059, 22.439 20, 22.253 19.941, 22.053 19.886, 21.824 19.842, 21.551 19.811, 21.219 19.8, 21.005 19.811, 20.746 19.842, 20.473 19.886, 20.215 19.941, 20 20)))')

    def testRoundRandomizedWaves(self):
        """Test randomized round waves"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').roundWavesRandomized(1, 2, 2, 3, 1).asWkt(3), 'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').roundWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         '')  # just don't crash!

        # very short line compared to minimum wavelength
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 2 1)').roundWavesRandomized(5, 6, 3, 5, 1).asWkt(3),
                         'LineString (1 1, 1.132 1.674, 1.265 2.199, 1.396 2.573, 1.521 2.798, 1.639 2.873, 1.745 2.798, 1.838 2.573, 1.913 2.199, 1.968 1.674, 2 1)')

        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWavesRandomized(1, 2, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1.04 0.552, 1.085 0.113, 1.135 -0.308, 1.187 -0.702, 1.242 -1.061, 1.296 -1.374, 1.348 -1.633, 1.398 -1.829, 1.444 -1.954, 1.483 -1.997, 1.56 -1.829, 1.623 -1.374, 1.676 -0.702, 1.722 0.113, 1.765 1.001, 1.808 1.888, 1.854 2.704, 1.907 3.375, 1.97 3.831, 2.047 3.999, 2.131 3.848, 2.2 3.438, 2.259 2.834, 2.309 2.1, 2.356 1.301, 2.403 0.503, 2.454 -0.231, 2.512 -0.835, 2.581 -1.246, 2.665 -1.397, 2.76 -1.255, 2.837 -0.87, 2.903 -0.302, 2.959 0.387, 3.012 1.137, 3.065 1.886, 3.122 2.575, 3.187 3.143, 3.265 3.528, 3.359 3.67, 3.491 3.515, 3.599 3.096, 3.69 2.478, 3.77 1.728, 3.843 0.912, 3.917 0.095, 3.996 -0.655, 4.087 -1.273, 4.195 -1.692, 4.327 -1.846, 4.416 -1.696, 4.49 -1.288, 4.552 -0.686, 4.605 0.044, 4.655 0.839, 4.705 1.634, 4.759 2.364, 4.821 2.966, 4.894 3.374, 4.984 3.525, 5.082 3.391, 5.163 3.03, 5.23 2.498, 5.29 1.851, 5.344 1.147, 5.399 0.444, 5.459 -0.203, 5.526 -0.735, 5.607 -1.096, 5.705 -1.23, 5.81 -1.086, 5.896 -0.695, 5.968 -0.119, 6.031 0.581, 6.089 1.342, 6.147 2.103, 6.21 2.803, 6.282 3.379, 6.368 3.77, 6.473 3.914, 6.572 3.764, 6.653 3.358, 6.722 2.76, 6.781 2.033, 6.837 1.242, 6.892 0.451, 6.952 -0.276, 7.02 -0.875, 7.102 -1.281, 7.201 -1.431, 7.333 -1.285, 7.442 -0.889, 7.533 -0.306, 7.612 0.403, 7.686 1.174, 7.76 1.945, 7.839 2.653, 7.93 3.237, 8.039 3.633, 8.171 3.778, 8.287 3.622, 8.383 3.198, 8.464 2.573, 8.534 1.814, 8.6 0.988, 8.665 0.162, 8.735 -0.597, 8.816 -1.222, 8.912 -1.646, 9.029 -1.803, 9.103 -1.654, 9.164 -1.249, 9.216 -0.653, 9.26 0.07, 9.302 0.858, 9.343 1.645, 9.388 2.369, 9.44 2.965, 9.501 3.369, 9.575 3.518, 9.61 3.482, 9.65 3.377, 9.694 3.212, 9.74 2.994, 9.788 2.731, 9.835 2.43, 9.881 2.099, 9.925 1.745, 9.965 1.376, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1.122 0.552, 1.261 0.113, 1.414 -0.308, 1.575 -0.702, 1.742 -1.061, 1.908 -1.374, 2.069 -1.633, 2.222 -1.829, 2.362 -1.954, 2.483 -1.997, 2.832 -1.829, 3.119 -1.374, 3.36 -0.702, 3.57 0.113, 3.765 1.001, 3.96 1.888, 4.17 2.704, 4.411 3.375, 4.698 3.831, 5.047 3.999, 5.403 3.848, 5.696 3.438, 5.943 2.834, 6.157 2.1, 6.356 1.301, 6.555 0.503, 6.77 -0.231, 7.016 -0.835, 7.309 -1.246, 7.665 -1.397, 7.948 -1.399, 8.238 -1.328, 8.529 -1.191, 8.814 -0.996, 9.085 -0.75, 9.337 -0.46, 9.561 -0.132, 9.751 0.225, 9.899 0.605, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWavesRandomized(8, 9, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1.183 0.552, 1.393 0.113, 1.623 -0.308, 1.866 -0.702, 2.117 -1.061, 2.367 -1.374, 2.61 -1.633, 2.84 -1.829, 3.05 -1.954, 3.233 -1.997, 3.786 -1.829, 4.241 -1.374, 4.623 -0.702, 4.956 0.113, 5.265 1.001, 5.574 1.888, 5.907 2.704, 6.289 3.375, 6.744 3.831, 7.297 3.999, 7.658 3.874, 8.02 3.687, 8.376 3.445, 8.717 3.158, 9.035 2.836, 9.322 2.487, 9.57 2.119, 9.771 1.743, 9.917 1.367, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWavesRandomized(10, 12, 1, 2, 1).asWkt(3),
                         'LineString (1 1, 1.243 0.701, 1.522 0.409, 1.828 0.128, 2.151 -0.134, 2.483 -0.373, 2.815 -0.582, 3.139 -0.755, 3.444 -0.885, 3.723 -0.968, 3.966 -0.997, 4.664 -0.885, 5.238 -0.582, 5.72 -0.134, 6.141 0.409, 6.53 1.001, 6.92 1.592, 7.341 2.136, 7.823 2.583, 8.397 2.887, 9.094 2.999, 9.169 2.97, 9.254 2.887, 9.347 2.756, 9.446 2.583, 9.547 2.374, 9.649 2.135, 9.747 1.873, 9.841 1.592, 9.926 1.299, 10 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').roundWavesRandomized(20, 25, 2, 3, 1).asWkt(3),
                         'LineString (1 1, 1.506 0.552, 2.085 0.113, 2.72 -0.308, 3.392 -0.702, 4.083 -1.061, 4.773 -1.374, 5.445 -1.633, 6.081 -1.829, 6.66 -1.954, 7.166 -1.997, 7.398 -1.954, 7.665 -1.829, 7.956 -1.633, 8.265 -1.374, 8.583 -1.061, 8.9 -0.702, 9.209 -0.308, 9.501 0.113, 9.768 0.552, 10 1)')
        self.assertEqual(
            QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').roundWavesRandomized(5, 6, 2, 3, 1).asWkt(3),
            'LineString (1 1, 1.122 0.552, 1.261 0.113, 1.414 -0.308, 1.575 -0.702, 1.742 -1.061, 1.908 -1.374, 2.069 -1.633, 2.222 -1.829, 2.362 -1.954, 2.483 -1.997, 2.832 -1.829, 3.119 -1.374, 3.36 -0.702, 3.57 0.113, 3.765 1.001, 3.96 1.888, 4.17 2.704, 4.411 3.375, 4.698 3.831, 5.047 3.999, 5.403 3.848, 5.696 3.438, 5.943 2.834, 6.157 2.1, 6.356 1.301, 6.555 0.503, 6.77 -0.231, 7.016 -0.835, 7.309 -1.246, 7.665 -1.397, 8.029 -1.257, 8.309 -0.888, 8.505 -0.365, 8.614 0.239, 8.634 0.848, 8.563 1.387, 8.399 1.783, 8.14 1.96, 7.785 1.844, 7.33 1.359, 7.485 1.763, 7.904 2.095, 8.522 2.374, 9.272 2.618, 10.088 2.843, 10.905 3.069, 11.655 3.312, 12.273 3.591, 12.692 3.923, 12.846 4.327, 12.696 4.688, 12.288 4.986, 11.686 5.236, 10.956 5.453, 10.161 5.655, 9.366 5.857, 8.636 6.075, 8.034 6.325, 7.626 6.622, 7.475 6.984, 7.609 7.354, 7.97 7.659, 8.502 7.914, 9.149 8.138, 9.853 8.344, 10.556 8.551, 11.203 8.775, 11.735 9.03, 12.096 9.335, 12.23 9.705, 12.197 9.729, 12.105 9.757, 11.959 9.788, 11.766 9.82, 11.533 9.853, 11.266 9.886, 10.973 9.918, 10.66 9.948, 10.333 9.976, 10 10)')
        self.assertEqual(
            QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').roundWavesRandomized(5, 6, 2, 3,
                                                                                                    1).asWkt(3),
            'MultiLineString ((1 1, 1.122 0.552, 1.261 0.113, 1.414 -0.308, 1.575 -0.702, 1.742 -1.061, 1.908 -1.374, 2.069 -1.633, 2.222 -1.829, 2.362 -1.954, 2.483 -1.997, 2.832 -1.829, 3.119 -1.374, 3.36 -0.702, 3.57 0.113, 3.765 1.001, 3.96 1.888, 4.17 2.704, 4.411 3.375, 4.698 3.831, 5.047 3.999, 5.403 3.848, 5.696 3.438, 5.943 2.834, 6.157 2.1, 6.356 1.301, 6.555 0.503, 6.77 -0.231, 7.016 -0.835, 7.309 -1.246, 7.665 -1.397, 7.948 -1.399, 8.238 -1.328, 8.529 -1.191, 8.814 -0.996, 9.085 -0.75, 9.337 -0.46, 9.561 -0.132, 9.751 0.225, 9.899 0.605, 10 1),(10 10, 9.88 10.439, 9.743 10.869, 9.592 11.281, 9.433 11.667, 9.269 12.018, 9.106 12.325, 8.946 12.579, 8.796 12.771, 8.658 12.893, 8.538 12.936, 8.177 12.783, 7.88 12.368, 7.63 11.756, 7.412 11.014, 7.21 10.205, 7.008 9.397, 6.79 8.655, 6.541 8.043, 6.243 7.628, 5.882 7.475, 5.512 7.609, 5.207 7.97, 4.951 8.502, 4.728 9.149, 4.521 9.853, 4.314 10.556, 4.091 11.203, 3.835 11.735, 3.53 12.096, 3.16 12.23, 2.784 12.086, 2.474 11.695, 2.214 11.119, 1.987 10.419, 1.776 9.658, 1.566 8.897, 1.339 8.197, 1.079 7.621, 0.769 7.23, 0.393 7.086, 0.361 7.128, 0.324 7.249, 0.283 7.44, 0.24 7.692, 0.196 7.997, 0.152 8.345, 0.11 8.728, 0.069 9.137, 0.032 9.564, 0 10))')
        self.assertEqual(QgsGeometry.fromWkt(
            'Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').roundWavesRandomized(5, 6, 0.2, 0.3,
                                                                                                      1).asWkt(3),
            'Polygon ((1 1, 1.122 0.955, 1.261 0.911, 1.414 0.869, 1.575 0.83, 1.742 0.794, 1.908 0.763, 2.069 0.737, 2.222 0.717, 2.362 0.705, 2.483 0.7, 2.832 0.717, 3.119 0.763, 3.36 0.83, 3.57 0.911, 3.765 1, 3.96 1.089, 4.17 1.17, 4.411 1.238, 4.698 1.283, 5.047 1.3, 5.403 1.285, 5.696 1.244, 5.943 1.183, 6.157 1.11, 6.356 1.03, 6.555 0.95, 6.77 0.877, 7.016 0.816, 7.309 0.775, 7.665 0.76, 8.031 0.775, 8.328 0.814, 8.57 0.872, 8.767 0.945, 8.934 1.025, 9.082 1.109, 9.223 1.189, 9.37 1.262, 9.536 1.32, 9.733 1.359, 9.748 1.763, 9.79 2.095, 9.852 2.374, 9.927 2.618, 10.009 2.843, 10.09 3.069, 10.165 3.312, 10.227 3.591, 10.269 3.923, 10.285 4.327, 10.27 4.688, 10.229 4.986, 10.169 5.236, 10.096 5.453, 10.016 5.655, 9.937 5.857, 9.864 6.075, 9.803 6.325, 9.763 6.622, 9.748 6.984, 9.761 7.354, 9.797 7.659, 9.85 7.914, 9.915 8.138, 9.985 8.344, 10.056 8.551, 10.12 8.775, 10.174 9.03, 10.21 9.335, 10.223 9.705, 9.866 9.831, 9.572 9.904, 9.324 9.934, 9.106 9.93, 8.902 9.901, 8.696 9.857, 8.472 9.806, 8.213 9.758, 7.904 9.722, 7.527 9.709, 7.156 9.724, 6.851 9.764, 6.594 9.824, 6.371 9.897, 6.163 9.976, 5.956 10.055, 5.732 10.128, 5.476 10.187, 5.17 10.228, 4.799 10.243, 4.395 10.228, 4.062 10.189, 3.783 10.131, 3.54 10.06, 3.314 9.983, 3.088 9.906, 2.845 9.835, 2.566 9.776, 2.233 9.737, 1.829 9.722, 1.664 9.634, 1.502 9.554, 1.346 9.472, 1.2 9.378, 1.068 9.262, 0.951 9.112, 0.855 8.919, 0.782 8.671, 0.736 8.359, 0.72 7.971, 0.735 7.625, 0.775 7.34, 0.835 7.1, 0.907 6.892, 0.986 6.698, 1.065 6.505, 1.137 6.296, 1.196 6.056, 1.237 5.771, 1.252 5.425, 1.237 5.026, 1.196 4.698, 1.136 4.422, 1.064 4.182, 0.984 3.959, 0.905 3.736, 0.833 3.495, 0.773 3.22, 0.732 2.891, 0.717 2.492, 0.729 2.272, 0.746 2.055, 0.769 1.846, 0.796 1.651, 0.826 1.473, 0.859 1.317, 0.894 1.187, 0.93 1.088, 0.965 1.024, 1 1),(3 4, 3.116 3.969, 3.25 3.939, 3.396 3.91, 3.55 3.883, 3.709 3.858, 3.868 3.837, 4.022 3.819, 4.168 3.806, 4.301 3.797, 4.418 3.794, 4.798 3.807, 5.111 3.843, 5.374 3.896, 5.603 3.961, 5.816 4.031, 6.028 4.101, 6.258 4.165, 6.521 4.218, 6.834 4.254, 7.214 4.267, 7.322 4.323, 7.431 4.384, 7.534 4.458, 7.626 4.55, 7.701 4.668, 7.752 4.819, 7.773 5.009, 7.759 5.247, 7.704 5.539, 7.601 5.891, 7.401 6.229, 7.241 6.475, 7.106 6.64, 6.984 6.732, 6.86 6.763, 6.72 6.743, 6.552 6.682, 6.341 6.59, 6.074 6.477, 5.737 6.353, 5.36 6.24, 5.045 6.165, 4.779 6.113, 4.551 6.068, 4.347 6.015, 4.156 5.939, 3.966 5.824, 3.764 5.656, 3.539 5.418, 3.278 5.095, 3.241 5.013, 3.203 4.916, 3.166 4.807, 3.131 4.69, 3.099 4.568, 3.069 4.444, 3.044 4.323, 3.023 4.206, 3.008 4.097, 3 4))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').roundWavesRandomized(5, 6,
                                                                                                                  0.2,
                                                                                                                  0.3,
                                                                                                                  1).asWkt(
            3),
            'MultiPolygon (((1 1, 1.122 0.955, 1.261 0.911, 1.414 0.869, 1.575 0.83, 1.742 0.794, 1.908 0.763, 2.069 0.737, 2.222 0.717, 2.362 0.705, 2.483 0.7, 2.832 0.717, 3.119 0.763, 3.36 0.83, 3.57 0.911, 3.765 1, 3.96 1.089, 4.17 1.17, 4.411 1.238, 4.698 1.283, 5.047 1.3, 5.403 1.285, 5.696 1.244, 5.943 1.183, 6.157 1.11, 6.356 1.03, 6.555 0.95, 6.77 0.877, 7.016 0.816, 7.309 0.775, 7.665 0.76, 8.031 0.775, 8.328 0.814, 8.57 0.872, 8.767 0.945, 8.934 1.025, 9.082 1.109, 9.223 1.189, 9.37 1.262, 9.536 1.32, 9.733 1.359, 9.748 1.763, 9.79 2.095, 9.852 2.374, 9.927 2.618, 10.009 2.843, 10.09 3.069, 10.165 3.312, 10.227 3.591, 10.269 3.923, 10.285 4.327, 10.27 4.688, 10.229 4.986, 10.169 5.236, 10.096 5.453, 10.016 5.655, 9.937 5.857, 9.864 6.075, 9.803 6.325, 9.763 6.622, 9.748 6.984, 9.761 7.354, 9.797 7.659, 9.85 7.914, 9.915 8.138, 9.985 8.344, 10.056 8.551, 10.12 8.775, 10.174 9.03, 10.21 9.335, 10.223 9.705, 9.866 9.831, 9.572 9.904, 9.324 9.934, 9.106 9.93, 8.902 9.901, 8.696 9.857, 8.472 9.806, 8.213 9.758, 7.904 9.722, 7.527 9.709, 7.156 9.724, 6.851 9.764, 6.594 9.824, 6.371 9.897, 6.163 9.976, 5.956 10.055, 5.732 10.128, 5.476 10.187, 5.17 10.228, 4.799 10.243, 4.395 10.228, 4.062 10.189, 3.783 10.131, 3.54 10.06, 3.314 9.983, 3.088 9.906, 2.845 9.835, 2.566 9.776, 2.233 9.737, 1.829 9.722, 1.664 9.634, 1.502 9.554, 1.346 9.472, 1.2 9.378, 1.068 9.262, 0.951 9.112, 0.855 8.919, 0.782 8.671, 0.736 8.359, 0.72 7.971, 0.735 7.625, 0.775 7.34, 0.835 7.1, 0.907 6.892, 0.986 6.698, 1.065 6.505, 1.137 6.296, 1.196 6.056, 1.237 5.771, 1.252 5.425, 1.237 5.026, 1.196 4.698, 1.136 4.422, 1.064 4.182, 0.984 3.959, 0.905 3.736, 0.833 3.495, 0.773 3.22, 0.732 2.891, 0.717 2.492, 0.729 2.272, 0.746 2.055, 0.769 1.846, 0.796 1.651, 0.826 1.473, 0.859 1.317, 0.894 1.187, 0.93 1.088, 0.965 1.024, 1 1)),((20 20, 20.031 20.116, 20.061 20.25, 20.09 20.396, 20.117 20.55, 20.142 20.709, 20.163 20.868, 20.181 21.022, 20.194 21.168, 20.203 21.301, 20.206 21.418, 20.193 21.798, 20.157 22.111, 20.104 22.374, 20.039 22.603, 19.969 22.816, 19.899 23.028, 19.835 23.258, 19.782 23.521, 19.746 23.834, 19.733 24.214, 19.746 24.582, 19.783 24.885, 19.838 25.14, 19.904 25.361, 19.976 25.567, 20.048 25.773, 20.115 25.995, 20.169 26.249, 20.206 26.552, 20.22 26.92, 20.208 27.28, 20.175 27.576, 20.126 27.825, 20.067 28.041, 20.003 28.242, 19.939 28.443, 19.88 28.66, 19.831 28.909, 19.798 29.205, 19.786 29.565, 20.071 29.52, 20.288 29.461, 20.454 29.384, 20.585 29.286, 20.698 29.164, 20.809 29.015, 20.935 28.837, 21.092 28.626, 21.297 28.379, 21.566 28.092, 21.818 27.86, 22.045 27.688, 22.252 27.561, 22.446 27.463, 22.634 27.381, 22.822 27.298, 23.017 27.2, 23.224 27.073, 23.45 26.901, 23.702 26.669, 23.965 26.387, 24.163 26.136, 24.312 25.909, 24.428 25.698, 24.529 25.494, 24.63 25.29, 24.746 25.079, 24.896 24.852, 25.093 24.601, 25.356 24.319, 25.627 24.066, 25.866 23.875, 26.083 23.73, 26.285 23.615, 26.478 23.516, 26.672 23.417, 26.873 23.303, 27.09 23.158, 27.33 22.966, 27.601 22.713, 27.835 22.46, 28.011 22.234, 28.143 22.028, 28.245 21.836, 28.332 21.651, 28.419 21.465, 28.521 21.273, 28.652 21.067, 28.828 20.841, 29.063 20.588, 29.263 20.481, 29.415 20.366, 29.516 20.248, 29.563 20.131, 29.552 20.021, 29.482 19.921, 29.347 19.835, 29.146 19.768, 28.875 19.725, 28.53 19.71, 28.182 19.725, 27.896 19.766, 27.655 19.827, 27.445 19.901, 27.25 19.981, 27.056 20.061, 26.846 20.135, 26.605 20.196, 26.319 20.237, 25.971 20.252, 25.625 20.237, 25.34 20.196, 25.101 20.135, 24.893 20.061, 24.7 19.98, 24.507 19.9, 24.298 19.826, 24.059 19.765, 23.774 19.724, 23.429 19.708, 23.027 19.723, 22.696 19.763, 22.418 19.821, 22.176 19.892, 21.951 19.969, 21.727 20.046, 21.484 20.117, 21.206 20.176, 20.875 20.215, 20.474 20.23, 20.435 20.227, 20.39 20.217, 20.341 20.202, 20.29 20.182, 20.237 20.158, 20.184 20.131, 20.132 20.1, 20.083 20.068, 20.039 20.034, 20 20)))')

    def testApplyDashPattern(self):
        """Test apply dash pattern"""
        self.assertEqual(QgsGeometry.fromWkt('Point (1 1)').applyDashPattern([1, 2]).asWkt(3), 'Point (1 1)')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1)').applyDashPattern([1, 2]).asWkt(3), '')  # don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString EMPTY').applyDashPattern([1, 2]).asWkt(3), '')  # don't crash!
        self.assertEqual(QgsGeometry.fromWkt('Polygon EMPTY').applyDashPattern([1, 2]).asWkt(3), '')  # don't crash!

        # bad pattern length
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10)').applyDashPattern([1, 2, 3]).asWkt(3), '')  # don't crash!
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2]).asWkt(3),
                         'MultiLineString ((1 1, 2 1),(4 1, 5 1),(7 1, 8 1),(10 1, 10 1, 10 1))')

        # pattern ends on gap
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1]).asWkt(3),
                         'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1))')

        # pattern ends on dash
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1.1, 1, 0.5, 0.1]).asWkt(3),
                         'MultiLineString ((1 1, 2.1 1),(3.1 1, 3.6 1),(3.7 1, 4.8 1),(5.8 1, 6.3 1),(6.4 1, 7.5 1),(8.5 1, 9 1),(9.1 1, 10 1, 10 1))')

        # pattern rules

        # start rule only
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.HalfDash).asWkt(3),
                         'MultiLineString ((1 1, 1.5 1),(3.5 1, 4 1),(4.1 1, 5.1 1),(7.1 1, 7.6 1),(7.7 1, 8.7 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.FullGap).asWkt(3),
                         'MultiLineString ((1.1 1, 2.1 1),(4.1 1, 4.6 1),(4.7 1, 5.7 1),(7.7 1, 8.2 1),(8.3 1, 9.3 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.HalfGap).asWkt(3),
                         'MultiLineString ((1.05 1, 2.05 1),(4.05 1, 4.55 1),(4.65 1, 5.65 1),(7.65 1, 8.15 1),(8.25 1, 9.25 1))')

        # end rule only
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], endRule=Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1 1, 1.841 1),(3.523 1, 3.944 1),(4.028 1, 4.869 1),(6.551 1, 6.972 1),(7.056 1, 7.897 1),(9.579 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], endRule=Qgis.DashPatternLineEndingRule.HalfDash).asWkt(3),
                         'MultiLineString ((1 1, 1.861 1),(3.584 1, 4.014 1),(4.1 1, 4.962 1),(6.684 1, 7.115 1),(7.201 1, 8.062 1),(9.785 1, 10 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], endRule=Qgis.DashPatternLineEndingRule.FullGap).asWkt(3),
                         'MultiLineString ((1 1, 1.833 1),(3.5 1, 3.917 1),(4 1, 4.833 1),(6.5 1, 6.917 1),(7 1, 7.833 1),(9.5 1, 9.917 1),(10 1, 10 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], endRule=Qgis.DashPatternLineEndingRule.HalfGap).asWkt(3),
                         'MultiLineString ((1 1, 1.837 1),(3.512 1, 3.93 1),(4.014 1, 4.851 1),(6.526 1, 6.944 1),(7.028 1, 7.865 1),(9.54 1, 9.958 1))')

        # start and end rules
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1 1, 1.841 1),(3.523 1, 3.944 1),(4.028 1, 4.869 1),(6.551 1, 6.972 1),(7.056 1, 7.897 1),(9.579 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.HalfDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1 1, 1.441 1),(3.206 1, 3.647 1),(3.735 1, 4.618 1),(6.382 1, 6.824 1),(6.912 1, 7.794 1),(9.559 1, 10 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.FullGap,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1.083 1, 1.917 1),(3.583 1, 4 1),(4.083 1, 4.917 1),(6.583 1, 7 1),(7.083 1, 7.917 1),(9.583 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1], Qgis.DashPatternLineEndingRule.HalfGap,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1.042 1, 1.879 1),(3.553 1, 3.972 1),(4.056 1, 4.893 1),(6.567 1, 6.986 1),(7.07 1, 7.907 1),(9.581 1, 10 1))')

        # adjustment rule
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternSizeAdjustment.ScaleDashOnly).asWkt(3),
                         'MultiLineString ((1 1, 1.622 1),(3.622 1, 3.933 1),(4.033 1, 4.656 1),(6.656 1, 6.967 1),(7.067 1, 7.689 1),(9.689 1, 10 1, 10 1))')

        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternSizeAdjustment.ScaleGapOnly).asWkt(3),
                         'MultiLineString ((1 1, 2 1),(3.452 1, 3.952 1),(4.024 1, 5.024 1),(6.476 1, 6.976 1),(7.048 1, 8.048 1),(9.5 1, 10 1, 10 1))')

        # pattern offset
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        patternOffset=15).asWkt(3),
                         'MultiLineString ((1 1, 1.336 1),(3.019 1, 3.439 1),(3.523 1, 4.364 1),(6.047 1, 6.467 1),(6.551 1, 7.393 1),(9.075 1, 9.495 1),(9.579 1, 10 1, 10 1))')
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        Qgis.DashPatternLineEndingRule.FullDash,
                                                                                        patternOffset=-15).asWkt(3),
                         'MultiLineString ((1 1, 1.421 1),(1.505 1, 2.346 1),(4.028 1, 4.449 1),(4.533 1, 5.374 1),(7.056 1, 7.477 1),(7.561 1, 8.402 1))')

        # short line compared to pattern length
        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 4 1)').applyDashPattern([1, 2, 0.5, 0.1],
                                                                                       Qgis.DashPatternLineEndingRule.FullDash,
                                                                                       Qgis.DashPatternLineEndingRule.FullDash,
                                                                                       Qgis.DashPatternSizeAdjustment.ScaleDashOnly).asWkt(3),
                         'MultiLineString ((1 1, 1.667 1),(3.667 1, 4 1))')

        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 2 1)').applyDashPattern([1, 2],
                                                                                       Qgis.DashPatternLineEndingRule.FullDash,
                                                                                       Qgis.DashPatternLineEndingRule.FullDash).asWkt(3),
                         'MultiLineString ((1 1, 2 1))')

        self.assertEqual(QgsGeometry.fromWkt('LineString (1 1, 10 1, 10 10)').applyDashPattern([1, 2, 0.5, 0.1]).asWkt(3),
                         'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1),(10 2.2, 10 2.7),(10 2.8, 10 3.8),(10 5.8, 10 6.3),(10 6.4, 10 7.4),(10 9.4, 10 9.9),(10 10, 10 10, 10 10))')

        self.assertEqual(
            QgsGeometry.fromWkt('MultiLineString ((1 1, 10 1),(10 10, 0 10))').applyDashPattern([1, 2, 0.5, 0.1]).asWkt(3),
            'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1),(10 10, 9 10),(7 10, 6.5 10),(6.4 10, 5.4 10),(3.4 10, 2.9 10),(2.8 10, 1.8 10))')

        self.assertEqual(
            QgsGeometry.fromWkt('Polygon ((1 1, 10 1, 10 10, 1 10, 1 1),(3 4, 8 4, 7 7, 4 6, 3 4))').applyDashPattern([1, 2, 0.5, 0.1]).asWkt(
                3),
            'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1),(10 2.2, 10 2.7),(10 2.8, 10 3.8),(10 5.8, 10 6.3),(10 6.4, 10 7.4),(10 9.4, 10 9.9),(10 10, 10 10, 9 10),(7 10, 6.5 10),(6.4 10, 5.4 10),(3.4 10, 2.9 10),(2.8 10, 1.8 10),(1 8.8, 1 8.3),(1 8.2, 1 7.2),(1 5.2, 1 4.7),(1 4.6, 1 3.6),(1 1.6, 1 1.1),(1 1, 1 1, 1 1),(3 4, 4 4),(6 4, 6.5 4),(6.6 4, 7.6 4),(7.494 5.518, 7.336 5.992),(7.304 6.087, 7 7, 6.964 6.988),(5.067 6.356, 4.593 6.198),(4.498 6.166, 4 6, 3.787 5.575))')
        self.assertEqual(QgsGeometry.fromWkt(
            'MultiPolygon (((1 1, 10 1, 10 10, 1 10, 1 1)),((20 20, 20 30, 30 20, 20 20)))').applyDashPattern([1, 2, 0.5, 0.1]).asWkt(
            3),
            'MultiLineString ((1 1, 2 1),(4 1, 4.5 1),(4.6 1, 5.6 1),(7.6 1, 8.1 1),(8.2 1, 9.2 1),(10 2.2, 10 2.7),(10 2.8, 10 3.8),(10 5.8, 10 6.3),(10 6.4, 10 7.4),(10 9.4, 10 9.9),(10 10, 10 10, 9 10),(7 10, 6.5 10),(6.4 10, 5.4 10),(3.4 10, 2.9 10),(2.8 10, 1.8 10),(1 8.8, 1 8.3),(1 8.2, 1 7.2),(1 5.2, 1 4.7),(1 4.6, 1 3.6),(1 1.6, 1 1.1),(1 1, 1 1, 1 1),(20 20, 20 21),(20 23, 20 23.5),(20 23.6, 20 24.6),(20 26.6, 20 27.1),(20 27.2, 20 28.2),(20.141 29.859, 20.495 29.505),(20.566 29.434, 21.273 28.727),(22.687 27.313, 23.041 26.959),(23.111 26.889, 23.818 26.182),(25.233 24.767, 25.586 24.414),(25.657 24.343, 26.364 23.636),(27.778 22.222, 28.132 21.868),(28.202 21.798, 28.91 21.09),(29.542 20, 29.042 20),(28.942 20, 27.942 20),(25.942 20, 25.442 20),(25.342 20, 24.342 20),(22.342 20, 21.842 20),(21.742 20, 20.742 20))')

    def testGeosCrash(self):
        # test we don't crash when geos returns a point geometry with no points
        QgsGeometry.fromWkt('Polygon ((0 0, 1 1, 1 0, 0 0))').intersection(QgsGeometry.fromWkt('Point (42 0)')).isNull()

    def testIsRectangle(self):
        """
        Test checking if geometries are rectangles
        """
        # non polygons
        self.assertFalse(QgsGeometry().isAxisParallelRectangle(0))
        self.assertFalse(QgsGeometry.fromWkt('Point(0 1)').isAxisParallelRectangle(0))
        self.assertFalse(QgsGeometry.fromWkt('LineString(0 1, 1 2)').isAxisParallelRectangle(0))

        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 0 0))').isAxisParallelRectangle(0))
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 0 0))').isAxisParallelRectangle(0, True))
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 0 1))').isAxisParallelRectangle(0))
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 0 1))').isAxisParallelRectangle(0, True))
        self.assertFalse(QgsGeometry.fromWkt('Polygon(())').isAxisParallelRectangle(0))

        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))').isAxisParallelRectangle(0))
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))').isAxisParallelRectangle(0, True))
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 1 0, 0 0))').isAxisParallelRectangle(0))
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 0 1, 1 1, 1 0, 0 0))').isAxisParallelRectangle(0, True))
        # with rings
        self.assertFalse(QgsGeometry.fromWkt(
            'Polygon((0 0, 1 0, 1 1, 0 1, 0 0), (0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.2, 0.1 0.1))').isAxisParallelRectangle(
            0))
        # densified
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 0.5 0.0, 1 0, 1 1, 0 1, 0 0))').densifyByCount(
            5).isAxisParallelRectangle(0))
        # not a simple rectangle
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 0.5 0.0, 1 0, 1 1, 0 1, 0 0))').densifyByCount(
            5).isAxisParallelRectangle(0, True))

        # starting mid way through a side
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0.5 0, 1 0, 1 1, 0 1, 0 0, 0.5 0))').densifyByCount(
            5).isAxisParallelRectangle(0))
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0.5 0, 1 0, 1 1, 0 1, 0 0, 0.5 0))').densifyByCount(
            5).isAxisParallelRectangle(0, True))

        # with tolerance
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 1 0.001, 1 1, 0 1, 0 0))').isAxisParallelRectangle(0))
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 1 0.001, 1 1, 0 1, 0 0))').isAxisParallelRectangle(1))
        self.assertFalse(QgsGeometry.fromWkt('Polygon((0 0, 1 0.1, 1 1, 0 1, 0 0))').isAxisParallelRectangle(1))
        self.assertTrue(QgsGeometry.fromWkt('Polygon((0 0, 1 0.1, 1 1, 0 1, 0 0))').isAxisParallelRectangle(10))
        self.assertTrue(
            QgsGeometry.fromWkt('Polygon((0 0, 1 0.1, 1 1, 0 1, 0 0))').densifyByCount(5).isAxisParallelRectangle(10))

    def testTransformWithClass(self):
        """
        Test transforming using a python based class (most of the tests for this are in c++, this is just
        checking the sip bindings!)
        """

        class Transformer(QgsAbstractGeometryTransformer):

            def transformPoint(self, x, y, z, m):
                return True, x * 2, y + 1, z, m

        transformer = Transformer()
        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10)')
        self.assertTrue(g.get().transform(transformer))
        self.assertEqual(g.asWkt(0), 'LineString (0 1, 20 1, 20 11)')

    @unittest.skipIf(Qgis.geosVersionInt() < 30900, "GEOS 3.9 required")
    def testFrechetDistance(self):
        """
        Test QgsGeometry.frechetDistance
        """
        l1 = QgsGeometry.fromWkt('LINESTRING (0 0, 100 0)')
        l2 = QgsGeometry.fromWkt('LINESTRING (0 0, 50 50, 100 0)')
        self.assertAlmostEqual(l1.frechetDistance(l2), 70.711, 3)
        self.assertAlmostEqual(l2.frechetDistance(l1), 70.711, 3)

    @unittest.skipIf(Qgis.geosVersionInt() < 30900, "GEOS 3.9 required")
    def testFrechetDistanceDensify(self):
        """
        Test QgsGeometry.frechetDistanceDensify
        """
        l1 = QgsGeometry.fromWkt('LINESTRING (0 0, 100 0)')
        l2 = QgsGeometry.fromWkt('LINESTRING (0 0, 50 50, 100 0)')
        self.assertAlmostEqual(l1.frechetDistanceDensify(l2, 0.5), 50.000, 3)
        self.assertAlmostEqual(l2.frechetDistanceDensify(l1, 0.5), 50.000, 3)

    @unittest.skipIf(Qgis.geosVersionInt() < 30900, "GEOS 3.9 required")
    def testLargestEmptyCircle(self):
        """
        Test QgsGeometry.largestEmptyCircle
        """
        g1 = QgsGeometry.fromWkt('POLYGON ((50 50, 150 50, 150 150, 50 150, 50 50))')
        self.assertEqual(g1.largestEmptyCircle(1).asWkt(), 'LineString (100 100, 100 50)')
        self.assertEqual(g1.largestEmptyCircle(0.1).asWkt(), 'LineString (100 100, 100 50)')
        g2 = QgsGeometry.fromWkt(
            'MultiPolygon (((95.03667481662591854 163.45354523227382515, 95.03667481662591854 122.0354523227383936, 34.10757946210270575 122.0354523227383936, 34.10757946210270575 163.45354523227382515, 95.03667481662591854 163.45354523227382515)),((35.64792176039119198 76.3386308068459698, 94.52322738386308743 76.3386308068459698, 94.52322738386308743 41.25305623471882654, 35.64792176039119198 41.25305623471882654, 35.64792176039119198 76.3386308068459698)),((185.23227383863081741 108.34352078239608375, 185.23227383863081741 78.56356968215158076, 118.99755501222495013 78.56356968215158076, 118.99755501222495013 108.34352078239608375, 185.23227383863081741 108.34352078239608375)))')
        self.assertEqual(g2.largestEmptyCircle(0.1).asWkt(1), 'LineString (129.3 142.5, 129.3 108.3)')

    @unittest.skipIf(Qgis.geosVersionInt() < 30600, "GEOS 3.6 required")
    def testMinimumClearance(self):
        """
        Test QgsGeometry.minimumClearance
        """
        l1 = QgsGeometry.fromWkt('POLYGON ((0 0, 1 0, 1 1, 0.5 3.2e-4, 0 0))')
        self.assertAlmostEqual(l1.minimumClearance(), 0.00032, 5)

    @unittest.skipIf(Qgis.geosVersionInt() < 30600, "GEOS 3.6 required")
    def testMinimumClearanceLine(self):
        """
        Test QgsGeometry.minimumClearanceLine
        """
        l1 = QgsGeometry.fromWkt('POLYGON ((0 0, 1 0, 1 1, 0.5 3.2e-4, 0 0))')
        self.assertEqual(l1.minimumClearanceLine().asWkt(6), 'LineString (0.5 0.00032, 0.5 0)')

    @unittest.skipIf(Qgis.geosVersionInt() < 30600, "GEOS 3.6 required")
    def testMinimumWidth(self):
        """
        Test QgsGeometry.minimumWidth
        """
        l1 = QgsGeometry.fromWkt('POLYGON ((0 0, 1 0, 1 1, 0.5 3.2e-4, 0 0))')
        self.assertEqual(l1.minimumWidth().asWkt(6), 'LineString (0.5 0.5, 1 0)')

    def testNode(self):
        """
        Test QgsGeometry.node
        """
        l1 = QgsGeometry.fromWkt('LINESTRINGZ(0 0 0, 10 10 10, 0 10 5, 10 0 3)')
        self.assertEqual(l1.node().asWkt(6),
                         'MultiLineStringZ ((0 0 0, 5 5 4.5),(5 5 4.5, 10 10 10, 0 10 5, 5 5 4.5),(5 5 4.5, 10 0 3))')
        l1 = QgsGeometry.fromWkt('MULTILINESTRING ((2 5, 2 1, 7 1), (6 1, 4 1, 2 3, 2 5))')
        self.assertEqual(l1.node().asWkt(6),
                         'MultiLineString ((2 5, 2 3),(2 3, 2 1, 4 1),(4 1, 6 1),(6 1, 7 1),(4 1, 2 3))')

    def testSharedPaths(self):
        """
        Test QgsGeometry.sharedPaths
        """
        l1 = QgsGeometry.fromWkt(
            'MULTILINESTRING((26 125,26 200,126 200,126 125,26 125),(51 150,101 150,76 175,51 150))')
        l2 = QgsGeometry.fromWkt('LINESTRING(151 100,126 156.25,126 125,90 161, 76 175)')
        self.assertEqual(l1.sharedPaths(l2).asWkt(6),
                         'GeometryCollection (MultiLineString ((126 156.25, 126 125),(101 150, 90 161),(90 161, 76 175)),MultiLineString EMPTY)')
        l1 = QgsGeometry.fromWkt('LINESTRING(76 175,90 161,126 125,126 156.25,151 100)')
        l2 = QgsGeometry.fromWkt(
            'MULTILINESTRING((26 125,26 200,126 200,126 125,26 125),(51 150,101 150,76 175,51 150))')
        self.assertEqual(l1.sharedPaths(l2).asWkt(6),
                         'GeometryCollection (MultiLineString EMPTY,MultiLineString ((76 175, 90 161),(90 161, 101 150),(126 125, 126 156.25)))')

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
            self.assertTrue(self.imageCheck(test['name'], test['reference_image'], rendered_image), test['name'])

            if hasattr(geom.constGet(), 'addToPainterPath'):
                # also check using painter path
                rendered_image = self.renderGeometry(geom, test['use_pen'], as_painter_path=True)
                assert self.imageCheck(test['name'], test['reference_image'], rendered_image)

            if 'as_polygon_reference_image' in test:
                rendered_image = self.renderGeometry(geom, False, True)
                self.assertTrue(self.imageCheck(test['name'] + '_aspolygon', test['as_polygon_reference_image'], rendered_image), test['name'] + '_aspolygon')

    def testGeometryAsQPainterPath(self):
        '''Tests conversion of different geometries to QPainterPath, including bad/odd geometries.'''
        empty_multipolygon = QgsMultiPolygon()
        empty_multipolygon.addGeometry(QgsPolygon())
        empty_polygon = QgsPolygon()
        empty_linestring = QgsLineString()

        tests = [{'name': 'LineString',
                  'wkt': 'LineString (0 0,3 4,4 3)',
                  'reference_image': 'linestring'},
                 {'name': 'Empty LineString',
                  'geom': QgsGeometry(empty_linestring),
                  'reference_image': 'empty'},
                 {'name': 'MultiLineString',
                  'wkt': 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))',
                  'reference_image': 'multilinestring'},
                 {'name': 'Polygon',
                  'wkt': 'Polygon ((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 7 5, 7 7 , 5 7, 5 5))',
                  'reference_image': 'polygon'},
                 {'name': 'Empty Polygon',
                  'geom': QgsGeometry(empty_polygon),
                  'reference_image': 'empty'},
                 {'name': 'MultiPolygon',
                  'wkt': 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))',
                  'reference_image': 'multipolygon'},
                 {'name': 'Empty MultiPolygon',
                  'geom': QgsGeometry(empty_multipolygon),
                  'reference_image': 'empty'},
                 {'name': 'CircularString',
                  'wkt': 'CIRCULARSTRING(268 415,227 505,227 406)',
                  'reference_image': 'circular_string'},
                 {'name': 'CompoundCurve',
                  'wkt': 'COMPOUNDCURVE((5 3, 5 13), CIRCULARSTRING(5 13, 7 15, 9 13), (9 13, 9 3), CIRCULARSTRING(9 3, 7 1, 5 3))',
                  'reference_image': 'compound_curve'},
                 {'name': 'CurvePolygon',
                  'wkt': 'CURVEPOLYGON(CIRCULARSTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon'},
                 {'name': 'MultiCurve',
                  'wkt': 'MultiCurve((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0, 2 1,2 2))',
                  'reference_image': 'multicurve'},
                 {'name': 'CurvePolygon_no_arc',  # refs #14028
                  'wkt': 'CURVEPOLYGON(LINESTRING(1 3, 3 5, 4 7, 7 3, 1 3))',
                  'reference_image': 'curve_polygon_no_arc'},
                 {'name': 'CurvePolygonInteriorRings',
                  'wkt': 'CurvePolygon(CircularString (20 30, 50 30, 50 90, 10 50, 20 30),LineString(30 45, 55 45, 30 75, 30 45))',
                  'reference_image': 'curvepolygon_circularstring_interiorrings'},
                 {'name': 'CompoundCurve With Line',
                  'wkt': 'CompoundCurve(CircularString (20 30, 50 30, 50 90),LineString(50 90, 10 90))',
                  'reference_image': 'compoundcurve_with_line'},
                 {'name': 'Collection LineString',
                  'wkt': 'GeometryCollection( LineString (0 0,3 4,4 3) )',
                  'reference_image': 'collection_linestring'},
                 {'name': 'Collection MultiLineString',
                  'wkt': 'GeometryCollection (LineString(0 0, 1 0, 1 1, 2 1, 2 0), LineString(3 1, 5 1, 5 0, 6 0))',
                  'reference_image': 'collection_multilinestring'},
                 {'name': 'Collection Polygon',
                  'wkt': 'GeometryCollection(Polygon ((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 7 5, 7 7 , 5 7, 5 5)))',
                  'reference_image': 'collection_polygon'},
                 {'name': 'Collection MultiPolygon',
                  'wkt': 'GeometryCollection( Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),Polygon((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))',
                  'reference_image': 'collection_multipolygon'},
                 {'name': 'Collection CircularString',
                  'wkt': 'GeometryCollection(CIRCULARSTRING(268 415,227 505,227 406))',
                  'reference_image': 'collection_circular_string'},
                 {'name': 'Collection CompoundCurve',
                  'wkt': 'GeometryCollection(COMPOUNDCURVE((5 3, 5 13), CIRCULARSTRING(5 13, 7 15, 9 13), (9 13, 9 3), CIRCULARSTRING(9 3, 7 1, 5 3)))',
                  'reference_image': 'collection_compound_curve'},
                 {'name': 'Collection CurvePolygon',
                  'wkt': 'GeometryCollection(CURVEPOLYGON(CIRCULARSTRING(1 3, 3 5, 4 7, 7 3, 1 3)))',
                  'reference_image': 'collection_curve_polygon'},
                 {'name': 'Collection CurvePolygon_no_arc',  # refs #14028
                  'wkt': 'GeometryCollection(CURVEPOLYGON(LINESTRING(1 3, 3 5, 4 7, 7 3, 1 3)))',
                  'reference_image': 'collection_curve_polygon_no_arc'},
                 {'name': 'Collection Mixed',
                  'wkt': 'GeometryCollection(Point(1 2), MultiPoint(3 3, 2 3), LineString (0 0,3 4,4 3), MultiLineString((3 1, 3 2, 4 2)), Polygon((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)), MultiPolygon(((4 0, 5 0, 5 1, 6 1, 6 2, 4 2, 4 0)),(( 1 4, 2 4, 1 5, 1 4))))',
                  'reference_image': 'collection_mixed'},
                 {'name': 'MultiCurvePolygon',
                  'wkt': 'MultiSurface (CurvePolygon (CompoundCurve (CircularString (-12942312 4593500, -11871048 5481118, -11363838 5065730, -11551856 4038191, -12133399 4130014),(-12133399 4130014, -12942312 4593500)),(-12120281 5175043, -12456964 4694067, -11752991 4256817, -11569346 4943300, -12120281 5175043)),Polygon ((-10856627 5625411, -11083997 4995770, -10887235 4357384, -9684796 4851477, -10069576 5428648, -10856627 5625411)))',
                  'reference_image': 'multicurvepolygon_with_rings'}
                 ]

        for test in tests:

            def get_geom():
                if 'geom' not in test:
                    geom = QgsGeometry.fromWkt(test['wkt'])
                    assert geom and not geom.isNull(), 'Could not create geometry {}'.format(test['wkt'])
                else:
                    geom = test['geom']
                return geom

            geom = get_geom()
            rendered_image = self.renderGeometryUsingPath(geom)
            self.assertTrue(
                self.imageCheck(test['name'], test['reference_image'], rendered_image, control_path="geometry_path"),
                test['name'])

            # Note - each test is repeated with the same geometry and reference image, but with added
            # z and m dimensions. This tests that presence of the dimensions does not affect rendering

            # test with Z
            geom_z = get_geom()
            geom_z.get().addZValue(5)
            rendered_image = self.renderGeometryUsingPath(geom_z)
            assert self.imageCheck(test['name'] + 'Z', test['reference_image'], rendered_image,
                                   control_path="geometry_path")

            # test with ZM
            geom_z.get().addMValue(15)
            rendered_image = self.renderGeometryUsingPath(geom_z)
            assert self.imageCheck(test['name'] + 'ZM', test['reference_image'], rendered_image,
                                   control_path="geometry_path")

            # test with M
            geom_m = get_geom()
            geom_m.get().addMValue(15)
            rendered_image = self.renderGeometryUsingPath(geom_m)
            assert self.imageCheck(test['name'] + 'M', test['reference_image'], rendered_image,
                                   control_path="geometry_path")

    def renderGeometryUsingPath(self, geom):
        image = QImage(200, 200, QImage.Format_RGB32)
        dest_bounds = image.rect()

        geom = QgsGeometry(geom)

        src_bounds = geom.buffer(geom.boundingBox().width() / 10, 5).boundingBox()
        if src_bounds.width() and src_bounds.height():
            scale = min(dest_bounds.width() / src_bounds.width(), dest_bounds.height() / src_bounds.height())
            t = QTransform.fromScale(scale, -scale)
            geom.transform(t)

        src_bounds = geom.buffer(geom.boundingBox().width() / 10, 5).boundingBox()
        t = QTransform.fromTranslate(-src_bounds.xMinimum(), -src_bounds.yMinimum())
        geom.transform(t)

        path = geom.constGet().asQPainterPath()

        painter = QPainter()
        painter.begin(image)
        pen = QPen(QColor(0, 255, 255))
        pen.setWidth(6)
        painter.setPen(pen)
        painter.setBrush(QBrush(QColor(255, 255, 0)))
        try:
            image.fill(QColor(0, 0, 0))

            painter.drawPath(path)

        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image, control_path="geometry"):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'geometry_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix(control_path)
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
