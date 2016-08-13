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

from qgis.core import (
    QgsGeometry,
    QgsVectorLayer,
    QgsFeature,
    QgsPoint,
    QgsPointV2,
    QgsCircularString,
    QgsCompoundCurve,
    QgsCurvePolygon,
    QgsGeometryCollection,
    QgsLineString,
    QgsMultiCurve,
    QgsMultiLineString,
    QgsMultiPointV2,
    QgsMultiPolygonV2,
    QgsMultiSurface,
    QgsPolygonV2,
    QgsCoordinateTransform,
    QgsRectangle,
    QgsWkbTypes,
    Qgis
)

from qgis.testing import (
    start_app,
    unittest,
)

from utilities import(
    compareWkt,
    doubleNear,
    unitTestDataPath,
    writeShape
)

# Convenience instances in case you may need them not used in this test

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometry(unittest.TestCase):

    def testBool(self):
        """ Test boolean evaluation of QgsGeometry """
        g = QgsGeometry()
        self.assertFalse(g)
        myWKT = 'Point (10 10)'
        g = QgsGeometry.fromWkt(myWKT)
        self.assertTrue(g)
        g.setGeometry(None)
        self.assertFalse(g)

    def testWktPointLoading(self):
        myWKT = 'Point (10 10)'
        myGeometry = QgsGeometry.fromWkt(myWKT)
        self.assertEqual(myGeometry.wkbType(), QgsWkbTypes.Point)

    def testWktMultiPointLoading(self):
        # Standard format
        wkt = 'MultiPoint ((10 15),(20 30))'
        geom = QgsGeometry.fromWkt(wkt)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
        self.assertEqual(geom.geometry().numGeometries(), 2)
        self.assertEqual(geom.geometry().geometryN(0).x(), 10)
        self.assertEqual(geom.geometry().geometryN(0).y(), 15)
        self.assertEqual(geom.geometry().geometryN(1).x(), 20)
        self.assertEqual(geom.geometry().geometryN(1).y(), 30)

        # Check MS SQL format
        wkt = 'MultiPoint (11 16, 21 31)'
        geom = QgsGeometry.fromWkt(wkt)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QgsWkbTypes.Point, geom.type())))
        self.assertEqual(geom.geometry().numGeometries(), 2)
        self.assertEqual(geom.geometry().geometryN(0).x(), 11)
        self.assertEqual(geom.geometry().geometryN(0).y(), 16)
        self.assertEqual(geom.geometry().geometryN(1).x(), 21)
        self.assertEqual(geom.geometry().geometryN(1).y(), 31)

    def testFromPoint(self):
        myPoint = QgsGeometry.fromPoint(QgsPoint(10, 10))
        self.assertEqual(myPoint.wkbType(), QgsWkbTypes.Point)

    def testFromMultiPoint(self):
        myMultiPoint = QgsGeometry.fromMultiPoint([
            (QgsPoint(0, 0)), (QgsPoint(1, 1))])
        self.assertEqual(myMultiPoint.wkbType(), QgsWkbTypes.MultiPoint)

    def testFromLine(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(1, 1), QgsPoint(2, 2)])
        self.assertEqual(myLine.wkbType(), QgsWkbTypes.LineString)

    def testFromMultiLine(self):
        myMultiPolyline = QgsGeometry.fromMultiPolyline(
            [[QgsPoint(0, 0), QgsPoint(1, 1)], [QgsPoint(0, 1), QgsPoint(2, 1)]])
        self.assertEqual(myMultiPolyline.wkbType(), QgsWkbTypes.MultiLineString)

    def testFromPolygon(self):
        myPolygon = QgsGeometry.fromPolygon(
            [[QgsPoint(1, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)]])
        self.assertEqual(myPolygon.wkbType(), QgsWkbTypes.Polygon)

    def testFromMultiPolygon(self):
        myMultiPolygon = QgsGeometry.fromMultiPolygon([
            [[QgsPoint(1, 1),
                QgsPoint(2, 2),
                QgsPoint(1, 2),
                QgsPoint(1, 1)]],
            [[QgsPoint(2, 2),
                QgsPoint(3, 3),
                QgsPoint(3, 1),
                QgsPoint(2, 2)]]
        ])
        self.assertEqual(myMultiPolygon.wkbType(), QgsWkbTypes.MultiPolygon)

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
                result = geom.exportToWkt()
                exp = row['valid_wkt']
                assert compareWkt(result, exp, 0.000001), "WKT conversion {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test num points in geometry
                exp_nodes = int(row['num_points'])
                self.assertEqual(geom.geometry().nCoordinates(), exp_nodes, "Node count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_nodes, geom.geometry().nCoordinates()))

                # test num geometries in collections
                exp_geometries = int(row['num_geometries'])
                try:
                    self.assertEqual(geom.geometry().numGeometries(), exp_geometries, "Geometry count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_geometries, geom.geometry().numGeometries()))
                except:
                    # some geometry types don't have numGeometries()
                    assert exp_geometries <= 1, "Geometry count {}:  Expected:\n{} geometries but could not call numGeometries()\n".format(i + 1, exp_geometries)

                # test count of rings
                exp_rings = int(row['num_rings'])
                try:
                    self.assertEqual(geom.geometry().numInteriorRings(), exp_rings, "Ring count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_rings, geom.geometry().numInteriorRings()))
                except:
                    # some geometry types don't have numInteriorRings()
                    assert exp_rings <= 1, "Ring count {}:  Expected:\n{} rings but could not call numInteriorRings()\n{}".format(i + 1, exp_rings, geom.geometry())

                # test isClosed
                exp = (row['is_closed'] == '1')
                try:
                    self.assertEqual(geom.geometry().isClosed(), exp, "isClosed {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, True, geom.geometry().isClosed()))
                except:
                    # some geometry types don't have isClosed()
                    assert not exp, "isClosed {}:  Expected:\n isClosed() but could not call isClosed()\n".format(i + 1)

                # test geometry centroid
                exp = row['centroid']
                result = geom.centroid().exportToWkt()
                assert compareWkt(result, exp, 0.00001), "Centroid {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test bounding box limits
                bbox = geom.geometry().boundingBox()
                exp = float(row['x_min'])
                result = bbox.xMinimum()
                assert doubleNear(result, exp), "Min X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)
                exp = float(row['y_min'])
                result = bbox.yMinimum()
                assert doubleNear(result, exp), "Min Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)
                exp = float(row['x_max'])
                result = bbox.xMaximum()
                assert doubleNear(result, exp), "Max X {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)
                exp = float(row['y_max'])
                result = bbox.yMaximum()
                assert doubleNear(result, exp), "Max Y {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test area calculation
                exp = float(row['area'])
                result = geom.geometry().area()
                assert doubleNear(result, exp), "Area {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test length calculation
                exp = float(row['length'])
                result = geom.geometry().length()
                assert doubleNear(result, exp, 0.00001), "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                # test perimeter calculation
                exp = float(row['perimeter'])
                result = geom.geometry().perimeter()
                assert doubleNear(result, exp, 0.00001), "Perimeter {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

    def testIntersection(self):
        myLine = QgsGeometry.fromPolyline([
            QgsPoint(0, 0),
            QgsPoint(1, 1),
            QgsPoint(2, 2)])
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
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
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
        bufferGeom = myPoint.buffer(10, 5)
        self.assertEqual(bufferGeom.wkbType(), QgsWkbTypes.Polygon)
        myTestPoint = QgsGeometry.fromPoint(QgsPoint(3, 3))
        self.assertTrue(bufferGeom.intersects(myTestPoint))

    def testContains(self):
        myPoly = QgsGeometry.fromPolygon(
            [[QgsPoint(0, 0),
              QgsPoint(2, 0),
              QgsPoint(2, 2),
              QgsPoint(0, 2),
              QgsPoint(0, 0)]])
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
        self.assertTrue(QgsGeometry.contains(myPoly, myPoint))

    def testTouches(self):
        myLine = QgsGeometry.fromPolyline([
            QgsPoint(0, 0),
            QgsPoint(1, 1),
            QgsPoint(2, 2)])
        myPoly = QgsGeometry.fromPolygon([[
            QgsPoint(0, 0),
            QgsPoint(1, 1),
            QgsPoint(2, 0),
            QgsPoint(0, 0)]])
        touchesGeom = QgsGeometry.touches(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", touchesGeom))
        assert touchesGeom, myMessage

    def testOverlaps(self):
        myPolyA = QgsGeometry.fromPolygon([[
            QgsPoint(0, 0),
            QgsPoint(1, 3),
            QgsPoint(2, 0),
            QgsPoint(0, 0)]])
        myPolyB = QgsGeometry.fromPolygon([[
            QgsPoint(0, 0),
            QgsPoint(2, 0),
            QgsPoint(2, 2),
            QgsPoint(0, 2),
            QgsPoint(0, 0)]])
        overlapsGeom = QgsGeometry.overlaps(myPolyA, myPolyB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", overlapsGeom))
        assert overlapsGeom, myMessage

    def testWithin(self):
        myLine = QgsGeometry.fromPolyline([
            QgsPoint(0.5, 0.5),
            QgsPoint(1, 1),
            QgsPoint(1.5, 1.5)
        ])
        myPoly = QgsGeometry.fromPolygon([[
            QgsPoint(0, 0),
            QgsPoint(2, 0),
            QgsPoint(2, 2),
            QgsPoint(0, 2),
            QgsPoint(0, 0)]])
        withinGeom = QgsGeometry.within(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", withinGeom))
        assert withinGeom, myMessage

    def testEquals(self):
        myPointA = QgsGeometry.fromPoint(QgsPoint(1, 1))
        myPointB = QgsGeometry.fromPoint(QgsPoint(1, 1))
        equalsGeom = QgsGeometry.equals(myPointA, myPointB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", equalsGeom))
        assert equalsGeom, myMessage

    def testCrosses(self):
        myLine = QgsGeometry.fromPolyline([
            QgsPoint(0, 0),
            QgsPoint(1, 1),
            QgsPoint(3, 3)])
        myPoly = QgsGeometry.fromPolygon([[
            QgsPoint(1, 0),
            QgsPoint(2, 0),
            QgsPoint(2, 2),
            QgsPoint(1, 2),
            QgsPoint(1, 0)]])
        crossesGeom = QgsGeometry.crosses(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", crossesGeom))
        assert crossesGeom, myMessage

    def testSimplifyIssue4189(self):
        """Test we can simplify a complex geometry.

        Note: there is a ticket related to this issue here:
        http://hub.qgis.org/issues/4189

        Backstory: Ole Nielson pointed out an issue to me
        (Tim Sutton) where simplify ftools was dropping
        features. This test replicates that issues.

        Interestingly we could replicate the issue in PostGIS too:
         - doing straight simplify returned no feature
         - transforming to UTM49, then simplify with e.g. 200 threshold is ok
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
        myEndLength = len(mySimpleGeometry.exportToWkt())
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
        myFeature1.setGeometry(QgsGeometry.fromPolyline([
            QgsPoint(10, 10),
            QgsPoint(20, 10),
            QgsPoint(30, 10),
            QgsPoint(40, 10),
        ]))
        myFeature1.setAttributes(['Johny'])

        myFeature2 = QgsFeature()
        myFeature2.setGeometry(QgsGeometry.fromPolyline([
            QgsPoint(10, 10),
            QgsPoint(20, 20),
            QgsPoint(30, 30),
            QgsPoint(40, 40),
        ]))
        myFeature2.setAttributes(['Be'])

        myFeature3 = QgsFeature()
        myFeature3.setGeometry(QgsGeometry.fromPolyline([
            QgsPoint(10, 10),
            QgsPoint(10, 20),
            QgsPoint(10, 30),
            QgsPoint(10, 40),
        ]))

        myFeature3.setAttributes(['Good'])

        myResult, myFeatures = myProvider.addFeatures(
            [myFeature1, myFeature2, myFeature3])
        assert myResult
        self.assertEqual(len(myFeatures), 3)

        myClipPolygon = QgsGeometry.fromPolygon([[
            QgsPoint(20, 20),
            QgsPoint(20, 30),
            QgsPoint(30, 30),
            QgsPoint(30, 20),
            QgsPoint(20, 20),
        ]])
        print('Clip: %s' % myClipPolygon.exportToWkt())
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
                # print 'Original: %s' % myGeometry.exportToWkt()
                # print 'Combined: %s' % myCombinedGeometry.exportToWkt()
                # print 'Difference: %s' % myDifferenceGeometry.exportToWkt()
                print('Symmetrical: %s' % mySymmetricalGeometry.exportToWkt())

                myExpectedWkt = 'Polygon ((20 20, 20 30, 30 30, 30 20, 20 20))'

                # There should only be one feature that intersects this clip
                # poly so this assertion should work.
                assert compareWkt(myExpectedWkt,
                                  mySymmetricalGeometry.exportToWkt())

                myNewFeature = QgsFeature()
                myNewFeature.setAttributes(myFeature.attributes())
                myNewFeature.setGeometry(mySymmetricalGeometry)
                myFeatures.append(myNewFeature)

        myNewMemoryLayer = QgsVectorLayer(
            ('LineString?crs=epsg:4326&field=name:string(20)&index=yes'),
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
        polyline = QgsGeometry.fromPolyline(
            [QgsPoint(5, 0), QgsPoint(0, 0), QgsPoint(0, 4), QgsPoint(5, 4), QgsPoint(5, 1), QgsPoint(1, 1), QgsPoint(1, 3), QgsPoint(4, 3), QgsPoint(4, 2), QgsPoint(2, 2)]
        )

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPoint(6, 1))
        self.assertEqual(point, QgsPoint(5, 1))
        self.assertEqual(beforeVertex, 3)
        self.assertEqual(atVertex, 4)
        self.assertEqual(afterVertex, 5)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex) = polyline.closestSegmentWithContext(QgsPoint(6, 2))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPoint(5, 2))
        self.assertEqual(afterVertex, 4)

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPoint(6, 0))
        self.assertEqual(point, QgsPoint(5, 0))
        self.assertEqual(beforeVertex, -1)
        self.assertEqual(atVertex, 0)
        self.assertEqual(afterVertex, 1)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex) = polyline.closestSegmentWithContext(QgsPoint(6, 0))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPoint(5, 0))
        self.assertEqual(afterVertex, 1)

        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPoint(0, -1))
        self.assertEqual(point, QgsPoint(0, 0))
        self.assertEqual(beforeVertex, 0)
        self.assertEqual(atVertex, 1)
        self.assertEqual(afterVertex, 2)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex) = polyline.closestSegmentWithContext(QgsPoint(0, 1))
        self.assertEqual(dist, 0)
        self.assertEqual(minDistPoint, QgsPoint(0, 1))
        self.assertEqual(afterVertex, 2)

        #   2-3 6-+-7 !
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromMultiPolyline(
            [
                [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
                [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polyline.closestVertex(QgsPoint(5, 2))
        self.assertEqual(point, QgsPoint(5, 1))
        self.assertEqual(beforeVertex, 6)
        self.assertEqual(atVertex, 7)
        self.assertEqual(afterVertex, 8)
        self.assertEqual(dist, 1)

        (dist, minDistPoint, afterVertex) = polyline.closestSegmentWithContext(QgsPoint(7, 0))
        self.assertEqual(dist, 1)
        self.assertEqual(minDistPoint, QgsPoint(6, 0))
        self.assertEqual(afterVertex, 9)

        # 5---4
        # |!  |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygon(
            [[
                QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
            ]]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPoint(0.7, 1.1))
        self.assertEqual(point, QgsPoint(1, 1))
        self.assertEqual(beforeVertex, 1)
        self.assertEqual(atVertex, 2)
        self.assertEqual(afterVertex, 3)
        assert abs(dist - 0.1) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.1)

        (dist, minDistPoint, afterVertex) = polygon.closestSegmentWithContext(QgsPoint(0.7, 1.1))
        self.assertEqual(afterVertex, 2)
        self.assertEqual(minDistPoint, QgsPoint(1, 1))
        exp = 0.3 ** 2 + 0.1 ** 2
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | |!| |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        polygon = QgsGeometry.fromPolygon(
            [
                [QgsPoint(0, 0), QgsPoint(3, 0), QgsPoint(3, 3), QgsPoint(0, 3), QgsPoint(0, 0)],
                [QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)],
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPoint(1.1, 1.9))
        self.assertEqual(point, QgsPoint(1, 2))
        self.assertEqual(beforeVertex, 7)
        self.assertEqual(atVertex, 8)
        self.assertEqual(afterVertex, 9)
        assert abs(dist - 0.02) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.02)

        (dist, minDistPoint, afterVertex) = polygon.closestSegmentWithContext(QgsPoint(1.2, 1.9))
        self.assertEqual(afterVertex, 8)
        self.assertEqual(minDistPoint, QgsPoint(1.2, 2))
        exp = 0.01
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2!+
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygon(
            [
                [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
                [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
            ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist) = polygon.closestVertex(QgsPoint(4.1, 1.1))
        self.assertEqual(point, QgsPoint(4, 1))
        self.assertEqual(beforeVertex, 11)
        self.assertEqual(atVertex, 12)
        self.assertEqual(afterVertex, 13)
        assert abs(dist - 0.02) < 0.00001, "Expected: %f; Got:%f" % (dist, 0.02)

        (dist, minDistPoint, afterVertex) = polygon.closestSegmentWithContext(QgsPoint(4.1, 1.1))
        self.assertEqual(afterVertex, 12)
        self.assertEqual(minDistPoint, QgsPoint(4, 1))
        exp = 0.02
        assert abs(dist - exp) < 0.00001, "Expected: %f; Got:%f" % (exp, dist)

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
        polyline = QgsGeometry.fromPolyline(
            [QgsPoint(5, 0), QgsPoint(0, 0), QgsPoint(0, 4), QgsPoint(5, 4), QgsPoint(5, 1), QgsPoint(1, 1), QgsPoint(1, 3), QgsPoint(4, 3), QgsPoint(4, 2), QgsPoint(2, 2)]
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
        polyline = QgsGeometry.fromMultiPolyline(
            [
                [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
                [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
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
        polygon = QgsGeometry.fromPolygon(
            [[
                QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
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
        polygon = QgsGeometry.fromPolygon(
            [
                [QgsPoint(0, 0), QgsPoint(3, 0), QgsPoint(3, 3), QgsPoint(0, 3), QgsPoint(0, 0)],
                [QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)],
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
        polygon = QgsGeometry.fromMultiPolygon(
            [
                [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
                [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
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
        points = [QgsPoint(5, 0), QgsPoint(0, 0), QgsPoint(0, 4), QgsPoint(5, 4), QgsPoint(5, 1), QgsPoint(1, 1), QgsPoint(1, 3), QgsPoint(4, 3), QgsPoint(4, 2), QgsPoint(2, 2)]
        polyline = QgsGeometry.fromPolyline(points)

        for i in range(0, len(points)):
            self.assertEqual(points[i], polyline.vertexAt(i), "Mismatch at %d" % i)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
            [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolyline(points)

        p = polyline.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        p = polyline.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(points[j][k], polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
        ]]
        polygon = QgsGeometry.fromPolygon(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(points[j][k], polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        points = [
            [QgsPoint(0, 0), QgsPoint(3, 0), QgsPoint(3, 3), QgsPoint(0, 3), QgsPoint(0, 0)],
            [QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)],
        ]
        polygon = QgsGeometry.fromPolygon(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                self.assertEqual(points[j][k], polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k))
                i += 1

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
            [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
        ]

        polygon = QgsGeometry.fromMultiPolygon(points)

        p = polygon.vertexAt(-100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        p = polygon.vertexAt(100)
        self.assertEqual(p, QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString())

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                for l in range(0, len(points[j][k])):
                    p = polygon.vertexAt(i)
                    self.assertEqual(points[j][k][l], p, "Got %s, Expected %s at %d / %d,%d,%d" % (p.toString(), points[j][k][l].toString(), i, j, k, l))
                    i += 1

    def testMultipoint(self):
        # #9423
        points = [QgsPoint(10, 30), QgsPoint(40, 20), QgsPoint(30, 10), QgsPoint(20, 10)]
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
        wkt = multipoint.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.insertVertex(7, 7, 2), "MULTIPOINT append 7,7 at 2 failed"
        expwkt = "MultiPoint ((4 4),(5 5),(7 7))"
        wkt = multipoint.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.insertVertex(6, 6, 2), "MULTIPOINT append 6,6 at 2 failed"
        expwkt = "MultiPoint ((4 4),(5 5),(6 6),(7 7))"
        wkt = multipoint.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not multipoint.deleteVertex(4), "MULTIPOINT delete at 4 unexpectedly succeeded"
        assert not multipoint.deleteVertex(-1), "MULTIPOINT delete at -1 unexpectedly succeeded"

        assert multipoint.deleteVertex(1), "MULTIPOINT delete at 1 failed"
        expwkt = "MultiPoint ((4 4),(6 6),(7 7))"
        wkt = multipoint.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.deleteVertex(2), "MULTIPOINT delete at 2 failed"
        expwkt = "MultiPoint ((4 4),(6 6))"
        wkt = multipoint.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert multipoint.deleteVertex(0), "MULTIPOINT delete at 2 failed"
        expwkt = "MultiPoint ((6 6))"
        wkt = multipoint.exportToWkt()
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
        wkt = multipoint.exportToWkt()
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
        wkt = polyline.exportToWkt()
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
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(3, 4, 3), "move vertex failed"
        expwkt = "Polygon ((1 2, 1 0, 1 1, 3 4, 2 2, 0 2, 1 2))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(2, 3, 6), "move vertex failed"
        expwkt = "Polygon ((2 3, 1 0, 1 1, 3 4, 2 2, 0 2, 2 3))"
        wkt = polygon.exportToWkt()
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
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(1, 2, 0), "move vertex failed"
        expwkt = "MultiPolygon (((1 2, 1 0, 1 1, 2 1, 2 2, 0 2, 1 2)),((4 0, 5 0, 6 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.moveVertex(2, 1, 7), "move vertex failed"
        expwkt = "MultiPolygon (((1 2, 1 0, 1 1, 2 1, 2 2, 0 2, 1 2)),((2 1, 5 0, 6 2, 3 2, 3 1, 4 1, 2 1)))"
        wkt = polygon.exportToWkt()
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
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not polyline.deleteVertex(-5), "Delete vertex -5 unexpectedly succeeded"
        assert not polyline.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromWkt("MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not polyline.deleteVertex(-100), "Delete vertex -100 unexpectedly succeeded"
        assert not polyline.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        assert polyline.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "MultiLineString ((1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polyline = QgsGeometry.fromWkt("MultiLineString ((0 0, 1 0, 1 1, 2 1,2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        for i in range(4):
            assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))")

        assert polygon.deleteVertex(2), "Delete vertex 2 failed"
        expwkt = "Polygon ((0 0, 1 0, 2 1, 2 2, 0 2, 0 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "Polygon ((1 0, 2 1, 2 2, 0 2, 1 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(4), "Delete vertex 4 failed"
        #"Polygon ((2 1, 2 2, 0 2, 2 1))" #several possibilities are correct here
        expwkt = "Polygon ((0 2, 2 1, 2 2, 0 2))"
        wkt = polygon.exportToWkt()
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
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "MultiPolygon (((1 0, 1 1, 2 1, 2 2, 0 2, 1 0)),((4 0, 5 0, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert polygon.deleteVertex(6), "Delete vertex 6 failed"
        expwkt = "MultiPolygon (((1 0, 1 1, 2 1, 2 2, 0 2, 1 0)),((5 0, 3 2, 3 1, 4 1, 5 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        for i in range(4):
            assert polygon.deleteVertex(0), "Delete vertex 0 failed"

        expwkt = "MultiPolygon (((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
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
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        for i in range(3):
            for j in range(2):
                assert polygon.deleteVertex(5), "Delete vertex 5 failed" % i

        expwkt = "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # Remove whole outer ring, inner ring should become outer
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1))")
        for i in range(2):
            assert polygon.deleteVertex(0), "Delete vertex 16 failed" % i

        expwkt = "Polygon ((1 1, 2 1, 2 2, 1 2, 1 1))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testInsertVertex(self):
        linestring = QgsGeometry.fromWkt("LineString(1 0, 2 0)")

        assert linestring.insertVertex(0, 0, 0), "Insert vertex 0 0 at 0 failed"
        expwkt = "LineString (0 0, 1 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert linestring.insertVertex(1.5, 0, 2), "Insert vertex 1.5 0 at 2 failed"
        expwkt = "LineString (0 0, 1 0, 1.5 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        assert not linestring.insertVertex(3, 0, 5), "Insert vertex 3 0 at 5 should have failed"

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.insertVertex(0, 0, 8), "Insert vertex 0 0 at 8 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 0 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.insertVertex(0, 0, 7), "Insert vertex 0 0 at 7 failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((0 0, 4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 0 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

    def testTranslate(self):
        point = QgsGeometry.fromWkt("Point (1 1)")
        self.assertEqual(point.translate(1, 2), 0, "Translate failed")
        expwkt = "Point (2 3)"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        self.assertEqual(point.translate(1, 2), 0, "Translate failed")
        expwkt = "MultiPoint ((2 3),(3 4),(4 5))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        self.assertEqual(linestring.translate(1, 2), 0, "Translate failed")
        expwkt = "LineString (2 2, 3 2)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        self.assertEqual(polygon.translate(1, 2), 0, "Translate failed")
        expwkt = "MultiPolygon (((1 2, 2 2, 2 3, 3 3, 3 4, 1 4, 1 2)),((5 2, 6 2, 6 2, 4 4, 4 3, 5 3, 5 2)))"
        wkt = polygon.exportToWkt()

        ct = QgsCoordinateTransform()

        point = QgsGeometry.fromWkt("Point (1 1)")
        self.assertEqual(point.transform(ct), 0, "Translate failed")
        expwkt = "Point (1 1)"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        self.assertEqual(point.transform(ct), 0, "Translate failed")
        expwkt = "MultiPoint ((1 1),(2 2),(3 3))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        self.assertEqual(linestring.transform(ct), 0, "Translate failed")
        expwkt = "LineString (1 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))")
        self.assertEqual(polygon.transform(ct), 0, "Translate failed")
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()

    def testExtrude(self):
        # test with empty geometry
        g = QgsGeometry()
        self.assertTrue(g.extrude(1, 2).isEmpty())

        points = [QgsPoint(1, 2), QgsPoint(3, 2), QgsPoint(4, 3)]
        line = QgsGeometry.fromPolyline(points)
        expected = QgsGeometry.fromWkt('Polygon ((1 2, 3 2, 4 3, 5 5, 4 4, 2 4, 1 2))')
        self.assertEqual(line.extrude(1, 2).exportToWkt(), expected.exportToWkt())

        points2 = [[QgsPoint(1, 2), QgsPoint(3, 2)], [QgsPoint(4, 3), QgsPoint(8, 3)]]
        multiline = QgsGeometry.fromMultiPolyline(points2)
        expected = QgsGeometry.fromWkt('MultiPolygon (((1 2, 3 2, 4 4, 2 4, 1 2)),((4 3, 8 3, 9 5, 5 5, 4 3)))')
        self.assertEqual(multiline.extrude(1, 2).exportToWkt(), expected.exportToWkt())

    def testNearestPoint(self):
        # test with empty geometries
        g1 = QgsGeometry()
        g2 = QgsGeometry()
        self.assertTrue(g1.nearestPoint(g2).isEmpty())
        g1 = QgsGeometry.fromWkt('LineString( 1 1, 5 1, 5 5 )')
        self.assertTrue(g1.nearestPoint(g2).isEmpty())
        self.assertTrue(g2.nearestPoint(g1).isEmpty())

        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'Point( 5 3 )'
        wkt = g1.nearestPoint(g2).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        expWkt = 'Point( 6 3 )'
        wkt = g2.nearestPoint(g1).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g1 = QgsGeometry.fromWkt('Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))')
        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'Point( 5 3 )'
        wkt = g1.nearestPoint(g2).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        expWkt = 'Point( 6 3 )'
        wkt = g2.nearestPoint(g1).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        g2 = QgsGeometry.fromWkt('Point( 2 3 )')
        expWkt = 'Point( 2 3 )'
        wkt = g1.nearestPoint(g2).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

    def testShortestLine(self):
        # test with empty geometries
        g1 = QgsGeometry()
        g2 = QgsGeometry()
        self.assertTrue(g1.shortestLine(g2).isEmpty())
        g1 = QgsGeometry.fromWkt('LineString( 1 1, 5 1, 5 5 )')
        self.assertTrue(g1.shortestLine(g2).isEmpty())
        self.assertTrue(g2.shortestLine(g1).isEmpty())

        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'LineString( 5 3, 6 3 )'
        wkt = g1.shortestLine(g2).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        expWkt = 'LineString( 6 3, 5 3 )'
        wkt = g2.shortestLine(g1).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        g1 = QgsGeometry.fromWkt('Polygon ((1 1, 5 1, 5 5, 1 5, 1 1))')
        g2 = QgsGeometry.fromWkt('Point( 6 3 )')
        expWkt = 'LineString( 5 3, 6 3 )'
        wkt = g1.shortestLine(g2).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))

        expWkt = 'LineString( 6 3, 5 3 )'
        wkt = g2.shortestLine(g1).exportToWkt()
        self.assertTrue(compareWkt(expWkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt))
        g2 = QgsGeometry.fromWkt('Point( 2 3 )')
        expWkt = 'LineString( 2 3, 2 3 )'
        wkt = g1.shortestLine(g2).exportToWkt()
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
        points = [QgsPoint(5, 0), QgsPoint(0, 0), QgsPoint(0, 4), QgsPoint(5, 4), QgsPoint(5, 1), QgsPoint(1, 1), QgsPoint(1, 3), QgsPoint(4, 3), QgsPoint(4, 2), QgsPoint(2, 2)]
        polyline = QgsGeometry.fromPolyline(points)
        expbb = QgsRectangle(0, 0, 5, 4)
        bb = polyline.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
            [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolyline(points)
        expbb = QgsRectangle(0, 0, 6, 1)
        bb = polyline.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
        ]]
        polygon = QgsGeometry.fromPolygon(points)
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
            [QgsPoint(0, 0), QgsPoint(3, 0), QgsPoint(3, 3), QgsPoint(0, 3), QgsPoint(0, 0)],
            [QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)],
        ]
        polygon = QgsGeometry.fromPolygon(points)
        expbb = QgsRectangle(0, 0, 3, 3)
        bb = polygon.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
            [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
        ]

        polygon = QgsGeometry.fromMultiPolygon(points)
        expbb = QgsRectangle(0, 0, 5, 2)
        bb = polygon.boundingBox()
        self.assertEqual(expbb, bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString()))

        # NULL
        points = []
        line = QgsGeometry.fromPolyline(points)
        assert line.boundingBox().isNull()

    def testAddPart(self):
        # add a part to a multipoint
        points = [QgsPoint(0, 0), QgsPoint(1, 0)]

        point = QgsGeometry.fromPoint(points[0])
        self.assertEqual(point.addPoints([points[1]]), 0)
        expwkt = "MultiPoint ((0 0), (1 0))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        point = QgsGeometry.fromPoint(points[0])
        point.geometry().addZValue(4.0)
        self.assertEqual(point.addPointsV2([QgsPointV2(QgsWkbTypes.PointZ, points[1][0], points[1][1], 3.0)]), 0)
        expwkt = "MultiPointZ ((0 0 4), (1 0 3))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
            [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
        ]

        polyline = QgsGeometry.fromPolyline(points[0])
        self.assertEqual(polyline.addPoints(points[1][0:1]), 2, "addPoints with one point line unexpectedly succeeded.")
        self.assertEqual(polyline.addPoints(points[1][0:2]), 0, "addPoints with two point line failed.")
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polyline = QgsGeometry.fromPolyline(points[0])
        self.assertEqual(polyline.addPoints(points[1]), 0, "addPoints with %d point line failed." % len(points[1]))
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        polyline = QgsGeometry.fromPolyline(points[0])
        polyline.geometry().addZValue(4.0)
        points2 = [QgsPointV2(QgsWkbTypes.PointZ, p[0], p[1], 3.0) for p in points[1]]
        self.assertEqual(polyline.addPointsV2(points2), 0)
        expwkt = "MultiLineStringZ ((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 0 4),(3 0 3, 3 1 3, 5 1 3, 5 0 3, 6 0 3))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
            [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
        ]

        polygon = QgsGeometry.fromPolygon(points[0])

        self.assertEqual(polygon.addPoints(points[1][0][0:1]), 2, "addPoints with one point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPoints(points[1][0][0:2]), 2, "addPoints with two point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPoints(points[1][0][0:3]), 2, "addPoints with unclosed three point ring unexpectedly succeeded.")
        self.assertEqual(polygon.addPoints([QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(4, 0)]), 2, "addPoints with 'closed' three point ring unexpectedly succeeded.")

        self.assertEqual(polygon.addPoints(points[1][0]), 0, "addPoints failed")
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygon(points[:1])
        p = QgsGeometry.fromPolygon(points[1])

        self.assertEqual(mp.addPartGeometry(p), 0)
        wkt = mp.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygon(points[:1])
        mp2 = QgsGeometry.fromMultiPolygon(points[1:])
        self.assertEqual(mp.addPartGeometry(mp2), 0)
        wkt = mp.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # test adding a part with Z values
        polygon = QgsGeometry.fromPolygon(points[0])
        polygon.geometry().addZValue(4.0)
        points2 = [QgsPointV2(QgsWkbTypes.PointZ, pi[0], pi[1], 3.0) for pi in points[1][0]]
        self.assertEqual(polygon.addPointsV2(points2), 0)
        expwkt = "MultiPolygonZ (((0 0 4, 1 0 4, 1 1 4, 2 1 4, 2 2 4, 0 2 4, 0 0 4)),((4 0 3, 5 0 3, 5 2 3, 3 2 3, 3 1 3, 4 1 3, 4 0 3)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        # Test adding parts to empty geometry, should become first part
        empty = QgsGeometry()
        # if not default type specified, addPart should fail
        result = empty.addPoints([QgsPoint(4, 0)])
        assert result != 0, 'Got return code {}'.format(result)
        result = empty.addPoints([QgsPoint(4, 0)], QgsWkbTypes.PointGeometry)
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
        expwkt = 'MultiPoint ((4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPoints([QgsPoint(5, 1)])
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
        expwkt = 'MultiPoint ((4 0),(5 1))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        # next try with lines
        empty = QgsGeometry()
        result = empty.addPoints(points[0][0], QgsWkbTypes.LineGeometry)
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPoints(points[1][0])
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0),(4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        # finally try with polygons
        empty = QgsGeometry()
        result = empty.addPoints(points[0][0], QgsWkbTypes.PolygonGeometry)
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
        expwkt = 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPoints(points[1][0])
        self.assertEqual(result, 0, 'Got return code {}'.format(result))
        wkt = empty.exportToWkt()
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
            [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0)], ],
            [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0)], ],
            [[QgsPoint(10, 0), QgsPoint(13, 0), QgsPoint(13, 3), QgsPoint(10, 3), QgsPoint(10, 0)], [QgsPoint(11, 1), QgsPoint(12, 1), QgsPoint(12, 2), QgsPoint(11, 2), QgsPoint(11, 1)]]
        ]
        ######## TO POINT ########
        # POINT TO POINT
        point = QgsGeometry.fromPoint(QgsPoint(1, 1))
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, False).exportToWkt()
        expWkt = "Point (1 1)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to point. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # POINT TO MultiPoint
        point = QgsGeometry.fromPoint(QgsPoint(1, 1))
        wkt = point.convertToType(QgsWkbTypes.PointGeometry, True).exportToWkt()
        expWkt = "MultiPoint ((1 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MultiPoint
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PointGeometry, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPoint
        multiLine = QgsGeometry.fromMultiPolyline(points[2])
        wkt = multiLine.convertToType(QgsWkbTypes.PointGeometry, True).exportToWkt()
        expWkt = "MultiPoint ((10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPoint
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PointGeometry, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from poylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MultiPoint
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.PointGeometry, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0),(4 0),(5 0),(5 2),(3 2),(3 1),(4 1),(4 0),(10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        ######## TO LINE ########
        # POINT TO LINE
        point = QgsGeometry.fromPoint(QgsPoint(1, 1))
        self.assertFalse(point.convertToType(QgsWkbTypes.LineGeometry, False)), "convertToType with a point should return a null geometry"
        # MultiPoint TO LINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MULTILINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.LineGeometry, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO LINE
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.LineGeometry, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MULTILINE
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.LineGeometry, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO LINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon with ring TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[2])
        wkt = polygon.convertToType(QgsWkbTypes.LineGeometry, True).exportToWkt()
        expWkt = "MultiLineString ((10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon with ring to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO LINE
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MULTILINE
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QgsWkbTypes.LineGeometry, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0), (4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0), (10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        ######## TO Polygon ########
        # MultiPoint TO Polygon
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MultiPolygon
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QgsWkbTypes.PolygonGeometry, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO Polygon
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE ( 3 vertices, with first = last ) TO Polygon
        line = QgsGeometry.fromPolyline([QgsPoint(1, 1), QgsPoint(0, 0), QgsPoint(1, 1)])
        self.assertFalse(line.convertToType(QgsWkbTypes.PolygonGeometry, False), "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # MULTILINE ( with a part of 3 vertices, with first = last ) TO MultiPolygon
        multiline = QgsGeometry.fromMultiPolyline([points[0][0], [QgsPoint(1, 1), QgsPoint(0, 0), QgsPoint(1, 1)]])
        self.assertFalse(multiline.convertToType(QgsWkbTypes.PolygonGeometry, True), "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry")
        # LINE TO MultiPolygon
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QgsWkbTypes.PolygonGeometry, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO Polygon
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPolygon
        multiLine = QgsGeometry.fromMultiPolyline([points[0][0], points[1][0]])
        wkt = multiLine.convertToType(QgsWkbTypes.PolygonGeometry, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPolygon
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QgsWkbTypes.PolygonGeometry, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO Polygon
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QgsWkbTypes.PolygonGeometry, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13053(self):
        """ See http://hub.qgis.org/issues/13053 """
        p = QgsGeometry.fromWkt('MULTIPOLYGON(((62.0 18.0, 62.0 19.0, 63.0 19.0, 63.0 18.0, 62.0 18.0)), ((63.0 19.0, 63.0 20.0, 64.0 20.0, 64.0 19.0, 63.0 19.0)))')
        assert p is not None

        expWkt = 'MultiPolygon (((62 18, 62 19, 63 19, 63 18, 62 18)),((63 19, 63 20, 64 20, 64 19, 63 19)))'
        wkt = p.exportToWkt()
        assert compareWkt(expWkt, wkt), "testRegression13053 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13055(self):
        """ See http://hub.qgis.org/issues/13055
            Testing that invalid WKT with z values but not using PolygonZ is still parsed
            by QGIS.
        """
        p = QgsGeometry.fromWkt('Polygon((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))')
        assert p is not None

        expWkt = 'PolygonZ ((0 0 0, 0 1 0, 1 1 0, 0 0 0 ))'
        wkt = p.exportToWkt()
        assert compareWkt(expWkt, wkt), "testRegression13055 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testRegression13274(self):
        """ See http://hub.qgis.org/issues/13274
            Testing that two combined linestrings produce another line string if possible
        """
        a = QgsGeometry.fromWkt('LineString (0 0, 1 0)')
        b = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        c = a.combine(b)

        expWkt = 'LineString (0 0, 1 0, 2 0)'
        wkt = c.exportToWkt()
        assert compareWkt(expWkt, wkt), "testRegression13274 failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testReshape(self):
        """ Test geometry reshaping """
        g = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        g.reshapeGeometry([QgsPoint(0, 1.5), QgsPoint(1.5, 0)])
        expWkt = 'Polygon ((0.5 1, 0 1, 0 0, 1 0, 1 0.5, 0.5 1))'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a geometry involving the first/last vertex (http://hub.qgis.org/issues/14443)
        g.reshapeGeometry([QgsPoint(0.5, 1), QgsPoint(0, 0.5)])

        expWkt = 'Polygon ((0 0.5, 0 0, 1 0, 1 0.5, 0.5 1, 0 0.5))'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Test reshape a line from first/last vertex
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1)')
        # extend start
        self.assertEqual(g.reshapeGeometry([QgsPoint(0, 0), QgsPoint(-1, 0)]), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1)'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # extend end
        self.assertEqual(g.reshapeGeometry([QgsPoint(5, 1), QgsPoint(10, 1), QgsPoint(10, 2)]), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1, 10 1, 10 2)'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test with reversed lines
        g = QgsGeometry.fromWkt('LineString (0 0, 5 0, 5 1)')
        # extend start
        self.assertEqual(g.reshapeGeometry([QgsPoint(-1, 0), QgsPoint(0, 0)]), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1)'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # extend end
        self.assertEqual(g.reshapeGeometry([QgsPoint(10, 2), QgsPoint(10, 1), QgsPoint(5, 1)]), 0)
        expWkt = 'LineString (-1 0, 0 0, 5 0, 5 1, 10 1, 10 2)'
        wkt = g.exportToWkt()
        assert compareWkt(expWkt, wkt), "testReshape failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testConvertToMultiType(self):
        """ Test converting geometries to multi type """
        point = QgsGeometry.fromWkt('Point (1 2)')
        assert point.convertToMultiType()
        expWkt = 'MultiPoint ((1 2))'
        wkt = point.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiPoint
        assert point.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        assert line.convertToMultiType()
        expWkt = 'MultiLineString ((1 0, 2 0))'
        wkt = line.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiLineString
        assert line.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))')
        assert poly.convertToMultiType()
        expWkt = 'MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)))'
        wkt = poly.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of MultiPolygon
        assert poly.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testConvertToSingleType(self):
        """ Test converting geometries to single type """
        point = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert point.convertToSingleType()
        expWkt = 'Point (1 2)'
        wkt = point.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of Point
        assert point.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('MultiLineString ((1 0, 2 0),(2 3, 4 5))')
        assert line.convertToSingleType()
        expWkt = 'LineString (1 0, 2 0)'
        wkt = line.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of LineString
        assert line.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)),((2 3,2 4, 3 4, 3 3, 2 3)))')
        assert poly.convertToSingleType()
        expWkt = 'Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))'
        wkt = poly.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # test conversion of Polygon
        assert poly.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddZValue(self):
        """ Test adding z dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.geometry().addZValue(2)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.CircularStringZ)
        expWkt = 'CircularStringZ (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.geometry().addZValue(2)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.CompoundCurveZ)
        expWkt = 'CompoundCurveZ ((5 3 2, 5 13 2),CircularStringZ (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringZ (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.geometry().addZValue(3)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.PolygonZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PolygonZ)
        expWkt = 'PolygonZ ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.geometry().addZValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.MultiPointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.MultiPointZ)
        expWkt = 'MultiPointZ ((1 2 4),(2 3 4))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.geometry().addZValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.LineStringZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.LineStringZ)
        expWkt = 'LineStringZ (1 2 4, 2 3 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.geometry().addZValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.PointZ)
        self.assertEqual(geom.wkbType(), QgsWkbTypes.PointZ)
        expWkt = 'PointZ (1 2 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddMValue(self):
        """ Test adding m dimension to geometries """

        # circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.geometry().addMValue(2)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.CircularStringM)
        expWkt = 'CircularStringM (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.geometry().addMValue(2)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.CompoundCurveM)
        expWkt = 'CompoundCurveM ((5 3 2, 5 13 2),CircularStringM (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringM (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.geometry().addMValue(3)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.PolygonM)
        expWkt = 'PolygonM ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.geometry().addMValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.MultiPointM)
        expWkt = 'MultiPointM ((1 2 4),(2 3 4))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.geometry().addMValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.LineStringM)
        expWkt = 'LineStringM (1 2 4, 2 3 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        # Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.geometry().addMValue(4)
        self.assertEqual(geom.geometry().wkbType(), QgsWkbTypes.PointM)
        expWkt = 'PointM (1 2 4)'
        wkt = geom.exportToWkt()
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
            (QgsMultiPointV2, "MultiPoint", QgsWkbTypes.MultiPoint),
            (QgsMultiPolygonV2, "MultiPolygon", QgsWkbTypes.MultiPolygon),
            (QgsMultiSurface, "MultiSurface", QgsWkbTypes.MultiSurface),
            (QgsPointV2, "Point", QgsWkbTypes.Point),
            (QgsPolygonV2, "Polygon", QgsWkbTypes.Polygon),
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
                result = QgsGeometry.createGeometryEngine(geom1.geometry()).relate(geom2.geometry())
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

    def testDeleteVertexCircularString(self):

        wkt = "CircularString ((0 0,1 1,2 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.exportToWkt(), QgsCircularString().asWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CircularString (2 0, 3 -1, 4 0)"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CircularString (0 0, 3 -1, 4 0)"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        expected_wkt = "CircularString (0 0, 1 1, 4 0)"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CircularString ((0 0,1 1,2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CircularString (0 0,1 1,2 0)"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

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
        self.assertEqual(geom.exportToWkt(), QgsCompoundCurve().asWkt())

        wkt = "CompoundCurve ((0 0,1 1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        self.assertEqual(geom.exportToWkt(), QgsCompoundCurve().asWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CompoundCurve ((1 1,2 2))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((0 0,2 2))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve ((0 0,1 1),(1 1,2 2))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CompoundCurve ((0 0,1 1))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve ((0 0,1 1),CircularString(1 1,2 0,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((0 0,1 -1))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve (CircularString(0 0,1 1,2 0),CircularString(2 0,3 -1,4 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CompoundCurve ((0 0, 4 0))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CompoundCurve (CircularString (0 0, 1 1, 2 0),(2 0, 0 0))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve ((-1 0,0 0),CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CompoundCurve ((-1 0, 2 0),CircularString (2 0, 1.5 -0.5, 1 -1))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CompoundCurve (CircularString(-1 -1,-1.5 -0.5,-2 0,-1 1,0 0),CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CompoundCurve (CircularString (-1 -1, -1.5 -0.5, -2 0),(-2 0, 2 0),CircularString (2 0, 1.5 -0.5, 1 -1))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

    def testDeleteVertexCurvePolygon(self):

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert not geom.deleteVertex(-1)
        assert not geom.deleteVertex(4)
        assert geom.deleteVertex(0)
        self.assertEqual(geom.exportToWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        self.assertEqual(geom.exportToWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        self.assertEqual(geom.exportToWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0),(2 0,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        self.assertEqual(geom.exportToWkt(), QgsCurvePolygon().asWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(0)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (2 0, 1.5 -0.5, 1 -1),(1 -1, 2 0)))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(1)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1.5 -0.5, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(2)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(3)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 1 -1),(1 -1, 0 0)))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

        wkt = "CurvePolygon (CompoundCurve (CircularString(0 0,1 1,2 0,1.5 -0.5,1 -1),(1 -1,0 0)))"
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.deleteVertex(4)
        expected_wkt = "CurvePolygon (CompoundCurve (CircularString (0 0, 1 1, 2 0),(2 0, 0 0)))"
        self.assertEqual(geom.exportToWkt(), QgsGeometry.fromWkt(expected_wkt).exportToWkt())

    def testSingleSidedBuffer(self):

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideLeft)
        result = out.exportToWkt()
        expected_wkt = "Polygon ((10 0, 0 0, 0 1, 10 1, 10 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight)
        result = out.exportToWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleMitre)
        result = out.exportToWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

        wkt = "LineString( 0 0, 10 0, 10 10)"
        geom = QgsGeometry.fromWkt(wkt)
        out = geom.singleSidedBuffer(1, 8, QgsGeometry.SideRight, QgsGeometry.JoinStyleBevel)
        result = out.exportToWkt()
        expected_wkt = "Polygon ((0 0, 10 0, 10 10, 11 10, 11 0, 10 -1, 0 -1, 0 0))"
        self.assertTrue(compareWkt(result, expected_wkt, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(expected_wkt, result))

    def testMisc(self):

        # Test that we cannot add a CurvePolygon in a MultiPolygon
        multipolygon = QgsMultiPolygonV2()
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
        self.assertEqual(geom.exportToWkt(), QgsMultiPolygonV2().asWkt())

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
        self.assertTrue(result.isEmpty())

        # linestring should be returned intact
        geom = QgsGeometry.fromWkt('LineString(0 0, 10 10)')
        result = geom.mergeLines().exportToWkt()
        exp = 'LineString(0 0, 10 10)'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        # multilinestring
        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(10 10, 20 20))')
        result = geom.mergeLines().exportToWkt()
        exp = 'LineString(0 0, 10 10, 20 20)'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4),(10 10, 20 20))')
        result = geom.mergeLines().exportToWkt()
        exp = 'MultiLineString((0 0, 10 10, 20 20),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

        geom = QgsGeometry.fromWkt('MultiLineString((0 0, 10 10),(12 2, 14 4))')
        result = geom.mergeLines().exportToWkt()
        exp = 'MultiLineString((0 0, 10 10),(12 2, 14 4))'
        self.assertTrue(compareWkt(result, exp, 0.00001), "Merge lines: mismatch Expected:\n{}\nGot:\n{}\n".format(exp, result))

if __name__ == '__main__':
    unittest.main()
