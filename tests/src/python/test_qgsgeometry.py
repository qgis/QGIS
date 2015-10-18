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

import qgis
import os
import csv

from qgis.core import (QgsGeometry,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsPoint,
                       QgsCoordinateTransform,
                       QgsRectangle,
                       QgsWKBTypes,
                       QGis)

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest,
                       compareWkt,
                       doubleNear,
                       unitTestDataPath,
                       writeShape)


# Convenience instances in case you may need them not used in this test

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometry(TestCase):

    def testWktPointLoading(self):
        myWKT = 'Point (10 10)'
        myGeometry = QgsGeometry.fromWkt(myWKT)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBPoint, myGeometry.type()))
        assert myGeometry.wkbType() == QGis.WKBPoint, myMessage

    def testWktMultiPointLoading(self):
        #Standard format
        wkt = 'MultiPoint ((10 15),(20 30))'
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.wkbType() == QGis.WKBMultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QGis.WKBPoint, geom.type()))
        assert geom.geometry().numGeometries() == 2
        assert geom.geometry().geometryN(0).x() == 10
        assert geom.geometry().geometryN(0).y() == 15
        assert geom.geometry().geometryN(1).x() == 20
        assert geom.geometry().geometryN(1).y() == 30

        #Check MS SQL format
        wkt = 'MultiPoint (11 16, 21 31)'
        geom = QgsGeometry.fromWkt(wkt)
        assert geom.wkbType() == QGis.WKBMultiPoint, ('Expected:\n%s\nGot:\n%s\n' % (QGis.WKBPoint, geom.type()))
        assert geom.geometry().numGeometries() == 2
        assert geom.geometry().geometryN(0).x() == 11
        assert geom.geometry().geometryN(0).y() == 16
        assert geom.geometry().geometryN(1).x() == 21
        assert geom.geometry().geometryN(1).y() == 31

    def testFromPoint(self):
        myPoint = QgsGeometry.fromPoint(QgsPoint(10, 10))
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBPoint, myPoint.type()))
        assert myPoint.wkbType() == QGis.WKBPoint, myMessage

    def testFromMultiPoint(self):
        myMultiPoint = QgsGeometry.fromMultiPoint([
            (QgsPoint(0, 0)), (QgsPoint(1, 1))])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBMultiPoint, myMultiPoint.type()))
        assert myMultiPoint.wkbType() == QGis.WKBMultiPoint, myMessage

    def testFromLine(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(1, 1), QgsPoint(2, 2)])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBLineString, myLine.type()))
        assert myLine.wkbType() == QGis.WKBLineString, myMessage

    def testFromMultiLine(self):
        myMultiPolyline = QgsGeometry.fromMultiPolyline(
            [[QgsPoint(0, 0), QgsPoint(1, 1)], [QgsPoint(0, 1), QgsPoint(2, 1)]])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBMultiLineString, myMultiPolyline.type()))
        assert myMultiPolyline.wkbType() == QGis.WKBMultiLineString, myMessage

    def testFromPolygon(self):
        myPolygon = QgsGeometry.fromPolygon(
            [[QgsPoint(1, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)]])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBPolygon, myPolygon.type()))
        assert myPolygon.wkbType() == QGis.WKBPolygon, myMessage

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
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.WKBMultiPolygon, myMultiPolygon.type()))
        assert myMultiPolygon.wkbType() == QGis.WKBMultiPolygon, myMessage

    def testReferenceGeometry(self):
        """ Test parsing a whole range of valid reference wkt formats and variants, and checking
        expected values such as length, area, centroids, bounding boxes, etc of the resultant geometry.
        Note the bulk of this test data was taken from the PostGIS WKT test data """

        with open(os.path.join(TEST_DATA_DIR, 'geom_data.csv'), 'rb') as f:
            reader = csv.DictReader(f)
            for i, row in enumerate(reader):

                #test that geometry can be created from WKT
                geom = QgsGeometry.fromWkt(row['wkt'])
                assert geom, "WKT conversion {} failed: could not create geom:\n{}\n".format(i + 1, row['wkt'])

                #test exporting to WKT results in expected string
                result = geom.exportToWkt()
                exp = row['valid_wkt']
                assert compareWkt(result, exp, 0.000001), "WKT conversion {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                #test num points in geometry
                exp_nodes = int(row['num_points'])
                assert geom.geometry().nCoordinates() == exp_nodes, "Node count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_nodes, geom.geometry().nCoordinates())

                #test num geometries in collections
                exp_geometries = int(row['num_geometries'])
                try:
                    assert geom.geometry().numGeometries() == exp_geometries, "Geometry count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_geometries, geom.geometry().numGeometries())
                except:
                    #some geometry types don't have numGeometries()
                    assert exp_geometries <= 1, "Geometry count {}:  Expected:\n{} geometries but could not call numGeometries()\n".format(i + 1, exp_geometries)

                #test count of rings
                exp_rings = int(row['num_rings'])
                try:
                    assert geom.geometry().numInteriorRings() == exp_rings, "Ring count {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp_rings, geom.geometry().numInteriorRings())
                except:
                    #some geometry types don't have numInteriorRings()
                    assert exp_rings <= 1, "Ring count {}:  Expected:\n{} rings but could not call numInteriorRings()\n{}".format(i + 1, exp_rings, geom.geometry())

                #test isClosed
                exp = (row['is_closed'] == '1')
                try:
                    assert geom.geometry().isClosed() == exp, "isClosed {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, True, geom.geometry().isClosed())
                except:
                    #some geometry types don't have isClosed()
                    assert not exp, "isClosed {}:  Expected:\n isClosed() but could not call isClosed()\n".format(i + 1)

                #test geometry centroid
                exp = row['centroid']
                result = geom.centroid().exportToWkt()
                assert compareWkt(result, exp), "Centroid {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                #test bounding box limits
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

                #test area calculation
                exp = float(row['area'])
                result = geom.geometry().area()
                assert doubleNear(result, exp), "Area {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                #test length calculation
                exp = float(row['length'])
                result = geom.geometry().length()
                assert doubleNear(result, exp, 0.00001), "Length {}: mismatch Expected:\n{}\nGot:\n{}\n".format(i + 1, exp, result)

                #test perimeter calculation
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
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.Point, intersectionGeom.type()))
        assert intersectionGeom.wkbType() == QGis.WKBPoint, myMessage

        layer = QgsVectorLayer("Point", "intersection", "memory")
        assert layer.isValid(), "Failed to create valid point memory layer"

        provider = layer.dataProvider()

        ft = QgsFeature()
        ft.setGeometry(intersectionGeom)
        provider.addFeatures([ft])

        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (1, layer.featureCount()))
        assert layer.featureCount() == 1, myMessage

    def testBuffer(self):
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
        bufferGeom = myPoint.buffer(10, 5)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     (QGis.Polygon, bufferGeom.type()))
        assert bufferGeom.wkbType() == QGis.WKBPolygon, myMessage
        myTestPoint = QgsGeometry.fromPoint(QgsPoint(3, 3))
        assert bufferGeom.intersects(myTestPoint)

    def testContains(self):
        myPoly = QgsGeometry.fromPolygon(
            [[QgsPoint(0, 0),
              QgsPoint(2, 0),
              QgsPoint(2, 2),
              QgsPoint(0, 2),
              QgsPoint(0, 0)]])
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
        containsGeom = QgsGeometry.contains(myPoly, myPoint)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                     ("True", containsGeom))
        assert containsGeom, myMessage

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
        myWKTFile = file(os.path.join(unitTestDataPath('wkt'),
                                      'simplify_error.wkt'), 'rt')
        myWKT = myWKTFile.readline()
        myWKTFile.close()
        print myWKT
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

        assert myMemoryLayer is not None, 'Provider not initialised'
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
        assert len(myFeatures) == 3

        myClipPolygon = QgsGeometry.fromPolygon([[
            QgsPoint(20, 20),
            QgsPoint(20, 30),
            QgsPoint(30, 30),
            QgsPoint(30, 20),
            QgsPoint(20, 20),
        ]])
        print 'Clip: %s' % myClipPolygon.exportToWkt()
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
                #print 'Original: %s' % myGeometry.exportToWkt()
                #print 'Combined: %s' % myCombinedGeometry.exportToWkt()
                #print 'Difference: %s' % myDifferenceGeometry.exportToWkt()
                print 'Symmetrical: %s' % mySymmetricalGeometry.exportToWkt()

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
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

        for i in range(0, 10):
            (before, after) = polyline.adjacentVertices(i)
            if i == 0:
                assert before == -1 and after == 1, "Expected (0,1), Got:(%d,%d)" % (before, after)
            elif i == 9:
                assert before == i - 1 and after == -1, "Expected (0,1), Got:(%d,%d)" % (before, after)
            else:
                assert before == i - 1 and after == i + 1, "Expected (0,1), Got:(%d,%d)" % (before, after)

        (before, after) = polyline.adjacentVertices(100)
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

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
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

        for i in range(0, 10):
            (before, after) = polyline.adjacentVertices(i)

            if i == 0 or i == 5:
                assert before == -1 and after == i + 1, "Expected (-1,%d), Got:(%d,%d)" % (i + 1, before, after)
            elif i == 4 or i == 9:
                assert before == i - 1 and after == -1, "Expected (%d,-1), Got:(%d,%d)" % (i - 1, before, after)
            else:
                assert before == i - 1 and after == i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after)

        (before, after) = polyline.adjacentVertices(100)
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

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
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

        for i in range(0, 7):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 6:
                assert before == 5 and after == 1, "Expected (5,1), Got:(%d,%d)" % (before, after)
            else:
                assert before == i - 1 and after == i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after)

        (before, after) = polygon.adjacentVertices(100)
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

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
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

        for i in range(0, 8):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 4:
                assert before == 3 and after == 1, "Expected (3,1), Got:(%d,%d)" % (before, after)
            elif i == 5:
                assert before == 8 and after == 6, "Expected (2,0), Got:(%d,%d)" % (before, after)
            else:
                assert before == i - 1 and after == i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after)

        (before, after) = polygon.adjacentVertices(100)
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

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
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

        for i in range(0, 14):
            (before, after) = polygon.adjacentVertices(i)

            if i == 0 or i == 6:
                assert before == 5 and after == 1, "Expected (5,1), Got:(%d,%d)" % (before, after)
            elif i == 7 or i == 13:
                assert before == 12 and after == 8, "Expected (12,8), Got:(%d,%d)" % (before, after)
            else:
                assert before == i - 1 and after == i + 1, "Expected (%d,%d), Got:(%d,%d)" % (i - 1, i + 1, before, after)

        (before, after) = polygon.adjacentVertices(100)
        assert before == -1 and after == -1, "Expected (-1,-1), Got:(%d,%d)" % (before, after)

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
            assert points[i] == polyline.vertexAt(i), "Mismatch at %d" % i

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
            [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
        ]
        polyline = QgsGeometry.fromMultiPolyline(points)

        p = polyline.vertexAt(-100)
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        p = polyline.vertexAt(100)
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                assert points[j][k] == polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k)
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
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                assert points[j][k] == polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k)
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
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                assert points[j][k] == polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i, j, k)
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
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p == QgsPoint(0, 0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
            for k in range(0, len(points[j])):
                for l in range(0, len(points[j][k])):
                    p = polygon.vertexAt(i)
                    assert points[j][k][l] == p, "Got %s, Expected %s at %d / %d,%d,%d" % (p.toString(), points[j][k][l].toString(), i, j, k, l)
                    i += 1

    def testMultipoint(self):
        # #9423
        points = [QgsPoint(10, 30), QgsPoint(40, 20), QgsPoint(30, 10), QgsPoint(20, 10)]
        wkt = "MultiPoint ((10 30),(40 20),(30 10),(20 10))"
        multipoint = QgsGeometry.fromWkt(wkt)
        assert multipoint.isMultipart(), "Expected MultiPoint to be multipart"
        assert multipoint.wkbType() == QGis.WKBMultiPoint, "Expected wkbType to be WKBMultipoint"
        i = 0
        for p in multipoint.asMultiPoint():
            assert p == points[i], "Expected %s at %d, got %s" % (points[i].toString(), i, p.toString())
            i += 1

        multipoint = QgsGeometry.fromWkt("MultiPoint ((5 5))")
        assert multipoint.vertexAt(0) == QgsPoint(5, 5), "MULTIPOINT fromWkt failed"

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
        assert multipoint.vertexAt(0) == QgsPoint(5, 5), "MultiPoint fromWkt failed"

    def testMoveVertex(self):
        multipoint = QgsGeometry.fromWkt("MultiPoint ((5 0),(0 0),(0 4),(5 4),(5 1),(1 1),(1 3),(4 3),(4 2),(2 2))")

        #try moving invalid vertices
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

        #try moving invalid vertices
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
        for i in range(5):
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
        for i in range(6):
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

        for i in range(4):
            assert polygon.deleteVertex(16), "Delete vertex 16 failed" % i

        expwkt = "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1),(3 1, 4 1, 4 2, 3 2, 3 1),(7 1, 8 1, 8 2, 7 2, 7 1))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        for i in range(3):
            for j in range(4):
                assert polygon.deleteVertex(5), "Delete vertex 5 failed" % i

        expwkt = "Polygon ((0 0, 9 0, 9 3, 0 3, 0 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        #Remove whole outer ring, inner ring should become outer
        polygon = QgsGeometry.fromWkt("Polygon ((0 0, 9 0, 9 3, 0 3, 0 0),(1 1, 2 1, 2 2, 1 2, 1 1))")
        for i in range(4):
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
        assert point.translate(1, 2) == 0, "Translate failed"
        expwkt = "Point (2 3)"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        assert point.translate(1, 2) == 0, "Translate failed"
        expwkt = "MultiPoint ((2 3),(3 4),(4 5))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        assert linestring.translate(1, 2) == 0, "Translate failed"
        expwkt = "LineString (2 2, 3 2)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))")
        assert polygon.translate(1, 2) == 0, "Translate failed"
        expwkt = "MultiPolygon (((1 2, 2 2, 2 3, 3 3, 3 4, 1 4, 1 2)),((5 2, 6 2, 6 2, 4 4, 4 3, 5 3, 5 2)))"
        wkt = polygon.exportToWkt()

        ct = QgsCoordinateTransform()

        point = QgsGeometry.fromWkt("Point (1 1)")
        assert point.transform(ct) == 0, "Translate failed"
        expwkt = "Point (1 1)"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        point = QgsGeometry.fromWkt("MultiPoint ((1 1),(2 2),(3 3))")
        assert point.transform(ct) == 0, "Translate failed"
        expwkt = "MultiPoint ((1 1),(2 2),(3 3))"
        wkt = point.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        linestring = QgsGeometry.fromWkt("LineString (1 0, 2 0)")
        assert linestring.transform(ct) == 0, "Translate failed"
        expwkt = "LineString (1 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polygon = QgsGeometry.fromWkt("MultiPolygon(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))")
        assert polygon.transform(ct) == 0, "Translate failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()

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
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

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
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

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
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

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
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

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
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

        # NULL
        points = []
        line = QgsGeometry.fromPolyline(points)
        assert line.boundingBox().isNull()

    def testAddPart(self):
        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
            [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
        ]

        polyline = QgsGeometry.fromPolyline(points[0])
        assert polyline.addPart(points[1][0:1]) == 2, "addPart with one point line unexpectedly succeeded."
        assert polyline.addPart(points[1][0:2]) == 0, "addPart with two point line failed."
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1))"
        wkt = polyline.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        polyline = QgsGeometry.fromPolyline(points[0])
        assert polyline.addPart(points[1]) == 0, "addPart with %d point line failed." % len(points[1])
        expwkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1, 5 1, 5 0, 6 0))"

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

        assert polygon.addPart(points[1][0][0:1]) == 2, "addPart with one point ring unexpectedly succeeded."
        assert polygon.addPart(points[1][0][0:2]) == 2, "addPart with two point ring unexpectedly succeeded."
        assert polygon.addPart(points[1][0][0:3]) == 2, "addPart with unclosed three point ring unexpectedly succeeded."
        assert polygon.addPart([QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(4, 0)]) == 2, "addPart with 'closed' three point ring unexpectedly succeeded."

        assert polygon.addPart(points[1][0]) == 0, "addPart failed"
        expwkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygon(points[:1])
        p = QgsGeometry.fromPolygon(points[1])

        assert mp.addPartGeometry(p) == 0
        wkt = mp.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        mp = QgsGeometry.fromMultiPolygon(points[:1])
        mp2 = QgsGeometry.fromMultiPolygon(points[1:])
        assert mp.addPartGeometry(mp2) == 0
        wkt = mp.exportToWkt()
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)

        #Test adding parts to empty geometry, should become first part
        empty = QgsGeometry()
        #if not default type specified, addPart should fail
        result = empty.addPart([QgsPoint(4, 0)])
        assert result != 0, 'Got return code {}'.format(result)
        result = empty.addPart([QgsPoint(4, 0)], QGis.Point)
        assert result == 0, 'Got return code {}'.format(result)
        wkt = empty.exportToWkt()
        expwkt = 'MultiPoint ((4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPart([QgsPoint(5, 1)])
        assert result == 0, 'Got return code {}'.format(result)
        wkt = empty.exportToWkt()
        expwkt = 'MultiPoint ((4 0),(5 1))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        #next try with lines
        empty = QgsGeometry()
        result = empty.addPart(points[0][0], QGis.Line)
        assert result == 0, 'Got return code {}'.format(result)
        wkt = empty.exportToWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPart(points[1][0])
        assert result == 0, 'Got return code {}'.format(result)
        wkt = empty.exportToWkt()
        expwkt = 'MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0),(4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        #finally try with polygons
        empty = QgsGeometry()
        result = empty.addPart(points[0][0], QGis.Polygon)
        assert result == 0, 'Got return code {}'.format(result)
        wkt = empty.exportToWkt()
        expwkt = 'MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))'
        assert compareWkt(expwkt, wkt), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt)
        result = empty.addPart(points[1][0])
        assert result == 0, 'Got return code {}'.format(result)
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
        wkt = point.convertToType(QGis.Point, False).exportToWkt()
        expWkt = "Point (1 1)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to point. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # POINT TO MultiPoint
        point = QgsGeometry.fromPoint(QgsPoint(1, 1))
        wkt = point.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MultiPoint ((1 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from point to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MultiPoint
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPoint
        multiLine = QgsGeometry.fromMultiPolyline(points[2])
        wkt = multiLine.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MultiPoint ((10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPoint
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from poylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MultiPoint
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MultiPoint ((0 0),(1 0),(1 1),(2 1),(2 2),(0 2),(0 0),(4 0),(5 0),(5 2),(3 2),(3 1),(4 1),(4 0),(10 0),(13 0),(13 3),(10 3),(10 0),(11 1),(12 1),(12 2),(11 2),(11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        ######## TO LINE ########
        # POINT TO LINE
        point = QgsGeometry.fromPoint(QgsPoint(1, 1))
        assert point.convertToType(QGis.Line, False) is None, "convertToType with a point should return a null geometry"
        # MultiPoint TO LINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MULTILINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO LINE
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO MULTILINE
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO LINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon with ring TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[2])
        wkt = polygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MultiLineString ((10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon with ring to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO LINE
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LineString (0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon TO MULTILINE
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MultiLineString ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0), (4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0), (10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        ######## TO Polygon ########
        # MultiPoint TO Polygon
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPoint TO MultiPolygon
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multipoint to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE TO Polygon
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # LINE ( 3 vertices, with first = last ) TO Polygon
        line = QgsGeometry.fromPolyline([QgsPoint(1, 1), QgsPoint(0, 0), QgsPoint(1, 1)])
        assert line.convertToType(QGis.Polygon, False) is None, "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry"
        # MULTILINE ( with a part of 3 vertices, with first = last ) TO MultiPolygon
        multiline = QgsGeometry.fromMultiPolyline([points[0][0], [QgsPoint(1, 1), QgsPoint(0, 0), QgsPoint(1, 1)]])
        assert multiline.convertToType(QGis.Polygon, True) is None, "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry"
        # LINE TO MultiPolygon
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from line to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE (which has a single part) TO Polygon
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MULTILINE TO MultiPolygon
        multiLine = QgsGeometry.fromMultiPolyline([points[0][0], points[1][0]])
        wkt = multiLine.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)),((4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from multiline to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # Polygon TO MultiPolygon
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MultiPolygon (((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)))"
        assert compareWkt(expWkt, wkt), "convertToType failed: from polygon to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        # MultiPolygon (which has a single part) TO Polygon
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QGis.Polygon, False).exportToWkt()
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

    def testConvertToMultiType(self):
        """ Test converting geometries to multi type """
        point = QgsGeometry.fromWkt('Point (1 2)')
        assert point.convertToMultiType()
        expWkt = 'MultiPoint ((1 2))'
        wkt = point.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of MultiPoint
        assert point.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('LineString (1 0, 2 0)')
        assert line.convertToMultiType()
        expWkt = 'MultiLineString ((1 0, 2 0))'
        wkt = line.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of MultiLineString
        assert line.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))')
        assert poly.convertToMultiType()
        expWkt = 'MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)))'
        wkt = poly.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of MultiPolygon
        assert poly.convertToMultiType()
        assert compareWkt(expWkt, wkt), "testConvertToMultiType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testConvertToSingleType(self):
        """ Test converting geometries to single type """
        point = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert point.convertToSingleType()
        expWkt = 'Point (1 2)'
        wkt = point.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of Point
        assert point.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        line = QgsGeometry.fromWkt('MultiLineString ((1 0, 2 0),(2 3, 4 5))')
        assert line.convertToSingleType()
        expWkt = 'LineString (1 0, 2 0)'
        wkt = line.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of LineString
        assert line.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        poly = QgsGeometry.fromWkt('MultiPolygon (((1 0, 2 0, 2 1, 1 1, 1 0)),((2 3,2 4, 3 4, 3 3, 2 3)))')
        assert poly.convertToSingleType()
        expWkt = 'Polygon ((1 0, 2 0, 2 1, 1 1, 1 0))'
        wkt = poly.exportToWkt()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)
        #test conversion of Polygon
        assert poly.convertToSingleType()
        assert compareWkt(expWkt, wkt), "testConvertToSingleType failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddZValue(self):
        """ Test adding z dimension to geometries """

        #circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.geometry().addZValue(2)
        assert geom.geometry().wkbType() == QgsWKBTypes.CircularStringZ
        expWkt = 'CircularStringZ (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.geometry().addZValue(2)
        assert geom.geometry().wkbType() == QgsWKBTypes.CompoundCurveZ
        expWkt = 'CompoundCurveZ ((5 3 2, 5 13 2),CircularStringZ (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringZ (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.geometry().addZValue(3)
        assert geom.geometry().wkbType() == QgsWKBTypes.PolygonZ
        expWkt = 'PolygonZ ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.geometry().addZValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.MultiPointZ
        expWkt = 'MultiPointZ ((1 2 4),(2 3 4))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.geometry().addZValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.LineStringZ
        expWkt = 'LineStringZ (1 2 4, 2 3 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.geometry().addZValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.PointZ
        expWkt = 'PointZ (1 2 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addZValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

    def testAddMValue(self):
        """ Test adding m dimension to geometries """

        #circular string
        geom = QgsGeometry.fromWkt('CircularString (1 5, 6 2, 7 3)')
        assert geom.geometry().addMValue(2)
        assert geom.geometry().wkbType() == QgsWKBTypes.CircularStringM
        expWkt = 'CircularStringM (1 5 2, 6 2 2, 7 3 2)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CircularString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #compound curve
        geom = QgsGeometry.fromWkt('CompoundCurve ((5 3, 5 13),CircularString (5 13, 7 15, 9 13),(9 13, 9 3),CircularString (9 3, 7 1, 5 3))')
        assert geom.geometry().addMValue(2)
        assert geom.geometry().wkbType() == QgsWKBTypes.CompoundCurveM
        expWkt = 'CompoundCurveM ((5 3 2, 5 13 2),CircularStringM (5 13 2, 7 15 2, 9 13 2),(9 13 2, 9 3 2),CircularStringM (9 3 2, 7 1 2, 5 3 2))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CompoundCurve failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #curve polygon
        geom = QgsGeometry.fromWkt('Polygon ((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))')
        assert geom.geometry().addMValue(3)
        assert geom.geometry().wkbType() == QgsWKBTypes.PolygonM
        expWkt = 'PolygonM ((0 0 3, 1 0 3, 1 1 3, 2 1 3, 2 2 3, 0 2 3, 0 0 3))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to CurvePolygon failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #geometry collection
        geom = QgsGeometry.fromWkt('MultiPoint ((1 2),(2 3))')
        assert geom.geometry().addMValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.MultiPointM
        expWkt = 'MultiPointM ((1 2 4),(2 3 4))'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to GeometryCollection failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #LineString
        geom = QgsGeometry.fromWkt('LineString (1 2, 2 3)')
        assert geom.geometry().addMValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.LineStringM
        expWkt = 'LineStringM (1 2 4, 2 3 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to LineString failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

        #Point
        geom = QgsGeometry.fromWkt('Point (1 2)')
        assert geom.geometry().addMValue(4)
        assert geom.geometry().wkbType() == QgsWKBTypes.PointM
        expWkt = 'PointM (1 2 4)'
        wkt = geom.exportToWkt()
        assert compareWkt(expWkt, wkt), "addMValue to Point failed: mismatch Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt)

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
                assert result == exp, "Relates {} failed: mismatch Expected:\n{}\nGot:\n{}\nGeom1:\n{}\nGeom2:\n{}\n".format(i + 1, exp, result, test_data[0], test_data[1])

    def testWkbTypes(self):
        """ Test QgsWKBTypes methods """

        # test hasZ methods
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.Unknown)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.Point)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.LineString)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.Polygon)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiPoint)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiLineString)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiPolygon)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.GeometryCollection)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CircularString)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CompoundCurve)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CurvePolygon)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiCurve)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiSurface)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.NoGeometry)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.PointZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.LineStringZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.PolygonZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPointZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiLineStringZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPolygonZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.GeometryCollectionZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CircularStringZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CompoundCurveZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CurvePolygonZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiCurveZ)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiSurfaceZ)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.PointM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.LineStringM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.PolygonM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiPointM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiLineStringM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiPolygonM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.GeometryCollectionM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CircularStringM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CompoundCurveM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.CurvePolygonM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiCurveM)
        assert not QgsWKBTypes.hasZ(QgsWKBTypes.MultiSurfaceM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.PointZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.LineStringZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.PolygonZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPointZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiLineStringZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPolygonZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.GeometryCollectionZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CircularStringZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CompoundCurveZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.CurvePolygonZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiCurveZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiSurfaceZM)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.Point25D)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.LineString25D)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.Polygon25D)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPoint25D)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiLineString25D)
        assert QgsWKBTypes.hasZ(QgsWKBTypes.MultiPolygon25D)

        # test hasM methods
        assert not QgsWKBTypes.hasM(QgsWKBTypes.Unknown)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.Point)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.LineString)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.Polygon)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPoint)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiLineString)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPolygon)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.GeometryCollection)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CircularString)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CompoundCurve)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CurvePolygon)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiCurve)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiSurface)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.NoGeometry)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.PointZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.LineStringZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.PolygonZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPointZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiLineStringZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPolygonZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.GeometryCollectionZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CircularStringZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CompoundCurveZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.CurvePolygonZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiCurveZ)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiSurfaceZ)
        assert QgsWKBTypes.hasM(QgsWKBTypes.PointM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.LineStringM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.PolygonM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiPointM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiLineStringM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiPolygonM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.GeometryCollectionM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CircularStringM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CompoundCurveM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CurvePolygonM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiCurveM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiSurfaceM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.PointZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.LineStringZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.PolygonZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiPointZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiLineStringZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiPolygonZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.GeometryCollectionZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CircularStringZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CompoundCurveZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.CurvePolygonZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiCurveZM)
        assert QgsWKBTypes.hasM(QgsWKBTypes.MultiSurfaceZM)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.Point25D)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.LineString25D)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.Polygon25D)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPoint25D)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiLineString25D)
        assert not QgsWKBTypes.hasM(QgsWKBTypes.MultiPolygon25D)

        # test adding z dimension to types
        assert QgsWKBTypes.addZ(QgsWKBTypes.Unknown) == QgsWKBTypes.Unknown
        assert QgsWKBTypes.addZ(QgsWKBTypes.Point) == QgsWKBTypes.PointZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.PointZ) == QgsWKBTypes.PointZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.PointM) == QgsWKBTypes.PointZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.PointZM) == QgsWKBTypes.PointZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPoint) == QgsWKBTypes.MultiPointZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPointZ) == QgsWKBTypes.MultiPointZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPointM) == QgsWKBTypes.MultiPointZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPointZM) == QgsWKBTypes.MultiPointZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.LineString) == QgsWKBTypes.LineStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.LineStringZ) == QgsWKBTypes.LineStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.LineStringM) == QgsWKBTypes.LineStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.LineStringZM) == QgsWKBTypes.LineStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiLineString) == QgsWKBTypes.MultiLineStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiLineStringZ) == QgsWKBTypes.MultiLineStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiLineStringM) == QgsWKBTypes.MultiLineStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiLineStringZM) == QgsWKBTypes.MultiLineStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.Polygon) == QgsWKBTypes.PolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.PolygonZ) == QgsWKBTypes.PolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.PolygonM) == QgsWKBTypes.PolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.PolygonZM) == QgsWKBTypes.PolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPolygon) == QgsWKBTypes.MultiPolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPolygonZ) == QgsWKBTypes.MultiPolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPolygonM) == QgsWKBTypes.MultiPolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPolygonZM) == QgsWKBTypes.MultiPolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.GeometryCollection) == QgsWKBTypes.GeometryCollectionZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.GeometryCollectionZ) == QgsWKBTypes.GeometryCollectionZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.GeometryCollectionM) == QgsWKBTypes.GeometryCollectionZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.GeometryCollectionZM) == QgsWKBTypes.GeometryCollectionZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CircularString) == QgsWKBTypes.CircularStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CircularStringZ) == QgsWKBTypes.CircularStringZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CircularStringM) == QgsWKBTypes.CircularStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CircularStringZM) == QgsWKBTypes.CircularStringZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CompoundCurve) == QgsWKBTypes.CompoundCurveZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CompoundCurveZ) == QgsWKBTypes.CompoundCurveZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CompoundCurveM) == QgsWKBTypes.CompoundCurveZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CompoundCurveZM) == QgsWKBTypes.CompoundCurveZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CurvePolygon) == QgsWKBTypes.CurvePolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CurvePolygonZ) == QgsWKBTypes.CurvePolygonZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.CurvePolygonM) == QgsWKBTypes.CurvePolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.CurvePolygonZM) == QgsWKBTypes.CurvePolygonZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiCurve) == QgsWKBTypes.MultiCurveZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiCurveZ) == QgsWKBTypes.MultiCurveZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiCurveM) == QgsWKBTypes.MultiCurveZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiCurveZM) == QgsWKBTypes.MultiCurveZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiSurface) == QgsWKBTypes.MultiSurfaceZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiSurfaceZ) == QgsWKBTypes.MultiSurfaceZ
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiSurfaceM) == QgsWKBTypes.MultiSurfaceZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiSurfaceZM) == QgsWKBTypes.MultiSurfaceZM
        assert QgsWKBTypes.addZ(QgsWKBTypes.NoGeometry) == QgsWKBTypes.NoGeometry
        assert QgsWKBTypes.addZ(QgsWKBTypes.Point25D) == QgsWKBTypes.Point25D
        assert QgsWKBTypes.addZ(QgsWKBTypes.LineString25D) == QgsWKBTypes.LineString25D
        assert QgsWKBTypes.addZ(QgsWKBTypes.Polygon25D) == QgsWKBTypes.Polygon25D
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiLineString25D) == QgsWKBTypes.MultiLineString25D
        assert QgsWKBTypes.addZ(QgsWKBTypes.MultiPolygon25D) == QgsWKBTypes.MultiPolygon25D

        # test adding m dimension to types
        assert QgsWKBTypes.addM(QgsWKBTypes.Unknown) == QgsWKBTypes.Unknown
        assert QgsWKBTypes.addM(QgsWKBTypes.Point) == QgsWKBTypes.PointM
        assert QgsWKBTypes.addM(QgsWKBTypes.PointZ) == QgsWKBTypes.PointZM
        assert QgsWKBTypes.addM(QgsWKBTypes.PointM) == QgsWKBTypes.PointM
        assert QgsWKBTypes.addM(QgsWKBTypes.PointZM) == QgsWKBTypes.PointZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPoint) == QgsWKBTypes.MultiPointM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPointZ) == QgsWKBTypes.MultiPointZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPointM) == QgsWKBTypes.MultiPointM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPointZM) == QgsWKBTypes.MultiPointZM
        assert QgsWKBTypes.addM(QgsWKBTypes.LineString) == QgsWKBTypes.LineStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.LineStringZ) == QgsWKBTypes.LineStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.LineStringM) == QgsWKBTypes.LineStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.LineStringZM) == QgsWKBTypes.LineStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiLineString) == QgsWKBTypes.MultiLineStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiLineStringZ) == QgsWKBTypes.MultiLineStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiLineStringM) == QgsWKBTypes.MultiLineStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiLineStringZM) == QgsWKBTypes.MultiLineStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.Polygon) == QgsWKBTypes.PolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.PolygonZ) == QgsWKBTypes.PolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.PolygonM) == QgsWKBTypes.PolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.PolygonZM) == QgsWKBTypes.PolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPolygon) == QgsWKBTypes.MultiPolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPolygonZ) == QgsWKBTypes.MultiPolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPolygonM) == QgsWKBTypes.MultiPolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPolygonZM) == QgsWKBTypes.MultiPolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.GeometryCollection) == QgsWKBTypes.GeometryCollectionM
        assert QgsWKBTypes.addM(QgsWKBTypes.GeometryCollectionZ) == QgsWKBTypes.GeometryCollectionZM
        assert QgsWKBTypes.addM(QgsWKBTypes.GeometryCollectionM) == QgsWKBTypes.GeometryCollectionM
        assert QgsWKBTypes.addM(QgsWKBTypes.GeometryCollectionZM) == QgsWKBTypes.GeometryCollectionZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CircularString) == QgsWKBTypes.CircularStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.CircularStringZ) == QgsWKBTypes.CircularStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CircularStringM) == QgsWKBTypes.CircularStringM
        assert QgsWKBTypes.addM(QgsWKBTypes.CircularStringZM) == QgsWKBTypes.CircularStringZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CompoundCurve) == QgsWKBTypes.CompoundCurveM
        assert QgsWKBTypes.addM(QgsWKBTypes.CompoundCurveZ) == QgsWKBTypes.CompoundCurveZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CompoundCurveM) == QgsWKBTypes.CompoundCurveM
        assert QgsWKBTypes.addM(QgsWKBTypes.CompoundCurveZM) == QgsWKBTypes.CompoundCurveZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CurvePolygon) == QgsWKBTypes.CurvePolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.CurvePolygonZ) == QgsWKBTypes.CurvePolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.CurvePolygonM) == QgsWKBTypes.CurvePolygonM
        assert QgsWKBTypes.addM(QgsWKBTypes.CurvePolygonZM) == QgsWKBTypes.CurvePolygonZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiCurve) == QgsWKBTypes.MultiCurveM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiCurveZ) == QgsWKBTypes.MultiCurveZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiCurveM) == QgsWKBTypes.MultiCurveM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiCurveZM) == QgsWKBTypes.MultiCurveZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiSurface) == QgsWKBTypes.MultiSurfaceM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiSurfaceZ) == QgsWKBTypes.MultiSurfaceZM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiSurfaceM) == QgsWKBTypes.MultiSurfaceM
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiSurfaceZM) == QgsWKBTypes.MultiSurfaceZM
        assert QgsWKBTypes.addM(QgsWKBTypes.NoGeometry) == QgsWKBTypes.NoGeometry
        # can't be added to these types
        assert QgsWKBTypes.addM(QgsWKBTypes.Point25D) == QgsWKBTypes.Point25D
        assert QgsWKBTypes.addM(QgsWKBTypes.LineString25D) == QgsWKBTypes.LineString25D
        assert QgsWKBTypes.addM(QgsWKBTypes.Polygon25D) == QgsWKBTypes.Polygon25D
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiLineString25D) == QgsWKBTypes.MultiLineString25D
        assert QgsWKBTypes.addM(QgsWKBTypes.MultiPolygon25D) == QgsWKBTypes.MultiPolygon25D


if __name__ == '__main__':
    unittest.main()
