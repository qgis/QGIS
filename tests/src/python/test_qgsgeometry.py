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

import qgis

from qgis.core import (QgsGeometry,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QgsRectangle,
                       QGis)

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest,
                       compareWkt,
                       expectedFailure,
                       unitTestDataPath,
                       writeShape)

from PyQt4.QtCore import qDebug

# Convenience instances in case you may need them not used in this test

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsGeometry(TestCase):
    wkbPtr = 1

    def testWktPointLoading(self):
        myWKT='POINT(10 10)'
        myGeometry = QgsGeometry.fromWkt(myWKT)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBPoint, myGeometry.type()))
        assert myGeometry.wkbType() == QGis.WKBPoint, myMessage

    def testFromPoint(self):
        myPoint = QgsGeometry.fromPoint(QgsPoint(10, 10))
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBPoint, myPoint.type()))
        assert myPoint.wkbType() == QGis.WKBPoint, myMessage

    def testFromMultiPoint(self):
        myMultiPoint = QgsGeometry.fromMultiPoint([
            (QgsPoint(0, 0)),(QgsPoint(1, 1))])
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
            [[QgsPoint(0, 0),QgsPoint(1, 1)],[QgsPoint(0, 1), QgsPoint(2, 1)]])
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
        assert containsGeom == True, myMessage

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
        assert touchesGeom == True, myMessage

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
        assert overlapsGeom == True, myMessage

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
        assert withinGeom == True, myMessage

    def testEquals(self):
        myPointA = QgsGeometry.fromPoint(QgsPoint(1, 1))
        myPointB = QgsGeometry.fromPoint(QgsPoint(1, 1))
        equalsGeom = QgsGeometry.equals(myPointA, myPointB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ("True", equalsGeom))
        assert equalsGeom == True, myMessage

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
        assert crossesGeom == True, myMessage

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
        myMinimumLength = len('POLYGON(())')
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
            QgsPoint(10,10),
            QgsPoint(20,10),
            QgsPoint(30,10),
            QgsPoint(40,10),
            ]
        ))
        myFeature1.setAttributes(['Johny'])

        myFeature2 = QgsFeature()
        myFeature2.setGeometry(QgsGeometry.fromPolyline([
            QgsPoint(10,10),
            QgsPoint(20,20),
            QgsPoint(30,30),
            QgsPoint(40,40),
            ]
        ))
        myFeature2.setAttributes(['Be'])

        myFeature3 = QgsFeature()
        myFeature3.setGeometry(QgsGeometry.fromPolyline([
            QgsPoint(10,10),
            QgsPoint(10,20),
            QgsPoint(10,30),
            QgsPoint(10,40),
            ]
        ))

        myFeature3.setAttributes(['Good'])

        myResult, myFeatures = myProvider.addFeatures(
            [myFeature1, myFeature2, myFeature3])
        assert myResult == True
        assert len(myFeatures) == 3

        myClipPolygon = QgsGeometry.fromPolygon([[
            QgsPoint(20,20),
            QgsPoint(20,30),
            QgsPoint(30,30),
            QgsPoint(30,20),
            QgsPoint(20,20),
            ]]
        )
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

                myExpectedWkt = 'LINESTRING(20 20, 30 30)'
                # There should only be one feature that intersects this clip
                # poly so this assertion should work.
                assert compareWkt( myExpectedWkt,
                                 mySymmetricalGeometry.exportToWkt() )

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
          [ QgsPoint(5,0), QgsPoint(0,0), QgsPoint(0,4), QgsPoint(5,4), QgsPoint(5,1), QgsPoint(1,1), QgsPoint(1,3), QgsPoint(4,3), QgsPoint(4,2), QgsPoint(2,2) ]
        )

        (point, atVertex, beforeVertex, afterVertex, dist ) = polyline.closestVertex( QgsPoint(6,1) )
        self.assertEqual( point, QgsPoint(5,1) )
        self.assertEqual( beforeVertex, 3 )
        self.assertEqual( atVertex, 4 )
        self.assertEqual( afterVertex, 5 )
        self.assertEqual( dist, 1 )

        (dist,minDistPoint,afterVertex) = polyline.closestSegmentWithContext( QgsPoint(6,2) )
        self.assertEqual( dist, 1 )
        self.assertEqual( minDistPoint, QgsPoint(5,2) )
        self.assertEqual( afterVertex, 4)

        (point, atVertex, beforeVertex, afterVertex, dist ) = polyline.closestVertex( QgsPoint(6,0) )
        self.assertEqual( point, QgsPoint(5,0) )
        self.assertEqual( beforeVertex, -1 )
        self.assertEqual( atVertex, 0 )
        self.assertEqual( afterVertex, 1 )
        self.assertEqual( dist, 1 )

        (dist,minDistPoint,afterVertex) = polyline.closestSegmentWithContext( QgsPoint(6,0) )
        self.assertEqual( dist, 1 )
        self.assertEqual( minDistPoint, QgsPoint(5,0) )
        self.assertEqual( afterVertex, 1)

        (point, atVertex, beforeVertex, afterVertex, dist ) = polyline.closestVertex( QgsPoint(0,1) )
        self.assertEqual( point, QgsPoint(0,0) )
        self.assertEqual( beforeVertex, 0 )
        self.assertEqual( atVertex, 1 )
        self.assertEqual( afterVertex, 2 )
        self.assertEqual( dist, 1 )

        (dist,minDistPoint,afterVertex) = polyline.closestSegmentWithContext( QgsPoint(0,1) )
        self.assertEqual( dist, 0 )
        self.assertEqual( minDistPoint, QgsPoint(0,1) )
        self.assertEqual( afterVertex, 2)

        #   2-3 6-+-7 !
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromMultiPolyline(
          [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist ) = polyline.closestVertex( QgsPoint(6,1) )
        self.assertEqual( point, QgsPoint(5,1) )
        self.assertEqual( beforeVertex, 6 )
        self.assertEqual( atVertex, 7 )
        self.assertEqual( afterVertex, 8 )
        self.assertEqual( dist, 1 )

        (dist,minDistPoint,afterVertex) = polyline.closestSegmentWithContext( QgsPoint(7,0) )
        self.assertEqual( dist, 1 )
        self.assertEqual( minDistPoint, QgsPoint(6,0) )
        self.assertEqual( afterVertex, 9)

        # 5---4
        # |!  |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygon(
          [[
            QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0),
          ]]
        )
        (point, atVertex, beforeVertex, afterVertex, dist ) = polygon.closestVertex( QgsPoint(0.7,1.1) )
        self.assertEqual( point, QgsPoint(1,1) )
        self.assertEqual( beforeVertex, 1 )
        self.assertEqual( atVertex, 2 )
        self.assertEqual( afterVertex, 3 )
        assert abs( dist - 0.1 ) < 0.00001, "Expected: %f; Got:%f" % (dist,0.1)

        (dist,minDistPoint,afterVertex) = polygon.closestSegmentWithContext( QgsPoint(0.7,1.1) )
        self.assertEqual( afterVertex, 2)
        self.assertEqual( minDistPoint, QgsPoint(1,1) )
        exp = 0.3**2 + 0.1**2
        assert abs( dist - exp ) < 0.00001, "Expected: %f; Got:%f" % (exp,dist)

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | |!| |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        polygon = QgsGeometry.fromPolygon(
          [
            [ QgsPoint(0,0), QgsPoint(3,0), QgsPoint(3,3), QgsPoint(0,3), QgsPoint(0,0) ],
            [ QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(1,2), QgsPoint(1,1) ],
          ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist ) = polygon.closestVertex( QgsPoint(1.1,1.9) )
        self.assertEqual( point, QgsPoint(1,2) )
        self.assertEqual( beforeVertex, 7 )
        self.assertEqual( atVertex, 8 )
        self.assertEqual( afterVertex, 9 )
        assert abs( dist - 0.02 ) < 0.00001, "Expected: %f; Got:%f" % (dist,0.02)

        (dist,minDistPoint,afterVertex) = polygon.closestSegmentWithContext( QgsPoint(1.2,1.9) )
        self.assertEqual( afterVertex, 8)
        self.assertEqual( minDistPoint, QgsPoint(1.2,2) )
        exp = 0.01
        assert abs( dist - exp ) < 0.00001, "Expected: %f; Got:%f" % (exp,dist)

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2!+
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygon(
          [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
          ]
        )
        (point, atVertex, beforeVertex, afterVertex, dist ) = polygon.closestVertex( QgsPoint(4.1,1.1) )
        self.assertEqual( point, QgsPoint(4,1) )
        self.assertEqual( beforeVertex, 11 )
        self.assertEqual( atVertex, 12 )
        self.assertEqual( afterVertex, 13 )
        assert abs( dist - 0.02 ) < 0.00001, "Expected: %f; Got:%f" % (dist,0.02)

        (dist,minDistPoint,afterVertex) = polygon.closestSegmentWithContext( QgsPoint(4.1,1.1) )
        self.assertEqual( afterVertex, 12)
        self.assertEqual( minDistPoint, QgsPoint(4,1) )
        exp = 0.02
        assert abs( dist - exp ) < 0.00001, "Expected: %f; Got:%f" % (exp,dist)

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
          [ QgsPoint(5,0), QgsPoint(0,0), QgsPoint(0,4), QgsPoint(5,4), QgsPoint(5,1), QgsPoint(1,1), QgsPoint(1,3), QgsPoint(4,3), QgsPoint(4,2), QgsPoint(2,2) ]
        )

        # don't crash
        (before,after) = polyline.adjacentVertices(-100)
        if TestQgsGeometry.wkbPtr:
          # CHANGE previous implementation returned (-101,-99) here
          assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        for i in range(0, 10):
                (before,after) = polyline.adjacentVertices(i)
                if i==0:
                        assert before==-1 and after==1, "Expected (0,1), Got:(%d,%d)" % (before,after)
                elif i==9:
                        assert before==i-1 and after==-1, "Expected (0,1), Got:(%d,%d)" % (before,after)
                else:
                        assert before==i-1 and after==i+1, "Expected (0,1), Got:(%d,%d)" % (before,after)

        (before,after) = polyline.adjacentVertices(100)
        if TestQgsGeometry.wkbPtr:
          # CHANGE previous implementation returned (99,101) here
          assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromMultiPolyline(
          [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]
        )

        (before,after) = polyline.adjacentVertices(-100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        for i in range(0,10):
                (before,after) = polyline.adjacentVertices(i)

                if i==0 or i==5:
                        assert before==-1 and after==i+1, "Expected (-1,%d), Got:(%d,%d)" % (i+1,before,after)
                elif i==4 or i==9:
                        assert before==i-1 and after==-1, "Expected (%d,-1), Got:(%d,%d)" % (i-1,before,after)
                else:
                        assert before==i-1 and after==i+1, "Expected (%d,%d), Got:(%d,%d)" % (i-1,i+1,before,after)

        (before,after) = polyline.adjacentVertices(100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromPolygon(
          [[
            QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0),
          ]]
        )

        (before,after) = polygon.adjacentVertices(-100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        for i in range(0,7):
                (before,after) = polygon.adjacentVertices(i)

                if i==0 or i==6:
                        assert before==5 and after==1, "Expected (5,1), Got:(%d,%d)" % (before,after)
                else:
                        assert before==i-1 and after==i+1, "Expected (%d,%d), Got:(%d,%d)" % (i-1,i+1,before,after)

        (before,after) = polygon.adjacentVertices(100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        polygon = QgsGeometry.fromPolygon(
          [
            [ QgsPoint(0,0), QgsPoint(3,0), QgsPoint(3,3), QgsPoint(0,3), QgsPoint(0,0) ],
            [ QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(1,2), QgsPoint(1,1) ],
          ]
        )

        (before,after) = polygon.adjacentVertices(-100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        for i in range(0,8):
                (before,after) = polygon.adjacentVertices(i)

                if i==0 or i==4:
                        assert before==3 and after==1, "Expected (3,1), Got:(%d,%d)" % (before,after)
                elif i==5:
                        assert before==8 and after==6, "Expected (2,0), Got:(%d,%d)" % (before,after)
                else:
                        assert before==i-1 and after==i+1, "Expected (%d,%d), Got:(%d,%d)" % (i-1,i+1,before,after)

        (before,after) = polygon.adjacentVertices(100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromMultiPolygon(
          [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
          ]
        )

        (before,after) = polygon.adjacentVertices(-100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

        for i in range(0,14):
                (before,after) = polygon.adjacentVertices(i)

                if i==0 or i==6:
                        assert before==5 and after==1, "Expected (5,1), Got:(%d,%d)" % (before,after)
                elif i==7 or i==13:
                        assert before==12 and after==8, "Expected (12,8), Got:(%d,%d)" % (before,after)
                else:
                        assert before==i-1 and after==i+1, "Expected (%d,%d), Got:(%d,%d)" % (i-1,i+1,before,after)

        (before,after) = polygon.adjacentVertices(100)
        assert before==-1 and after==-1, "Expected (-1,-1), Got:(%d,%d)" % (before,after)

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
        points = [ QgsPoint(5,0), QgsPoint(0,0), QgsPoint(0,4), QgsPoint(5,4), QgsPoint(5,1), QgsPoint(1,1), QgsPoint(1,3), QgsPoint(4,3), QgsPoint(4,2), QgsPoint(2,2) ]
        polyline = QgsGeometry.fromPolyline(points)

        for i in range(0, len(points)):
                assert points[i] == polyline.vertexAt(i), "Mismatch at %d" % i

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]
        polyline = QgsGeometry.fromMultiPolyline(points)

        p = polyline.vertexAt(-100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        p = polyline.vertexAt(100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
                for k in range(0, len(points[j])):
                        assert points[j][k] == polyline.vertexAt(i), "Mismatch at %d / %d,%d" % (i,j,k)
                        i+=1

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0),
          ]]
        polygon = QgsGeometry.fromPolygon(points)

        p = polygon.vertexAt(-100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
                for k in range(0, len(points[j])):
                        assert points[j][k] == polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i,j,k)
                        i+=1

        # 3-+-+-2
        # |     |
        # + 8-7 +
        # | | | |
        # + 5-6 +
        # |     |
        # 0-+-+-1
        points = [
            [ QgsPoint(0,0), QgsPoint(3,0), QgsPoint(3,3), QgsPoint(0,3), QgsPoint(0,0) ],
            [ QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(1,2), QgsPoint(1,1) ],
          ]
        polygon = QgsGeometry.fromPolygon(points)

        p = polygon.vertexAt(-100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
                for k in range(0, len(points[j])):
                        assert points[j][k] == polygon.vertexAt(i), "Mismatch at %d / %d,%d" % (i,j,k)
                        i+=1

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
          ]

        polygon = QgsGeometry.fromMultiPolygon(points)

        p = polygon.vertexAt(-100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        p = polygon.vertexAt(100)
        assert p==QgsPoint(0,0), "Expected 0,0, Got %s" % p.toString()

        i = 0
        for j in range(0, len(points)):
                for k in range(0, len(points[j])):
                        for l in range(0, len(points[j][k])):
                                p = polygon.vertexAt(i)
                                assert points[j][k][l] == p, "Got %s, Expected %s at %d / %d,%d,%d" % (p.toString(),points[j][k][l].toString(),i,j,k,l)
                                i+=1

    def testMultipoint(self):
        # CHANGE previous implementation didn't support multipoint too much
        if not TestQgsGeometry.wkbPtr:
          return

        # #9423
        points = [ QgsPoint(10, 30), QgsPoint(40, 20), QgsPoint(30,10), QgsPoint(20,10) ]
        wkt = "MULTIPOINT (10 30, 40 20, 30 10, 20 10)"
        multipoint = QgsGeometry.fromWkt(wkt)
        assert multipoint.isMultipart(), "Expected MULTIPOINT to be multipart"
        assert multipoint.wkbType() == QGis.WKBMultiPoint, "Expected wkbType to be WKBMultipoint"
        i = 0
        for p in multipoint.asMultiPoint():
                assert p == points[i], "Expected %s at %d, got %s" % (points[i].toString(), i, p.toString())
                i+=1

        multipoint = QgsGeometry.fromWkt( "MULTIPOINT(5 5)" )
        assert multipoint.vertexAt( 0 ) == QgsPoint(5,5), "MULTIPOINT fromWkt failed"

        assert multipoint.insertVertex(4, 4, 0), "MULTIPOINT insert 4,4 at 0 failed"
        expwkt = "MULTIPOINT(4 4, 5 5)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert multipoint.insertVertex(7, 7, 2), "MULTIPOINT append 7,7 at 2 failed"
        expwkt = "MULTIPOINT(4 4, 5 5, 7 7)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert multipoint.insertVertex(6, 6, 2), "MULTIPOINT append 6,6 at 2 failed"
        expwkt = "MULTIPOINT(4 4, 5 5, 6 6, 7 7)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert not multipoint.deleteVertex(4), "MULTIPOINT delete at 4 unexpectedly succeeded"
        assert not multipoint.deleteVertex(-1), "MULTIPOINT delete at -1 unexpectedly succeeded"

        assert multipoint.deleteVertex(1), "MULTIPOINT delete at 1 failed"
        expwkt = "MULTIPOINT(4 4, 6 6, 7 7)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert multipoint.deleteVertex(2), "MULTIPOINT delete at 2 failed"
        expwkt = "MULTIPOINT(4 4, 6 6)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert multipoint.deleteVertex(0), "MULTIPOINT delete at 2 failed"
        expwkt = "MULTIPOINT(6 6)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

    def testMoveVertex(self):
        multipoint = QgsGeometry.fromWkt( "MULTIPOINT(5 0,0 0,0 4,5 4,5 1,1 1,1 3,4 3,4 2,2 2)" )
        for i in range(0,10):
          assert multipoint.moveVertex( i+1, -1-i, i ), "move vertex %d failed" % i
        expwkt = "MULTIPOINT(1 -1, 2 -2, 3 -3, 4 -4, 5 -5, 6 -6, 7 -7, 8 -8, 9 -9, 10 -10)"
        wkt = multipoint.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        # 2-+-+-+-+-3
        # |         |
        # + 6-+-+-7 +
        # | |     | |
        # + + 9-+-8 +
        # | |       |
        # ! 5-+-+-+-4 !
        # |
        # 1-+-+-+-+-0 !
        polyline = QgsGeometry.fromWkt( "LINESTRING(5 0,0 0,0 4,5 4,5 1,1 1,1 3,4 3,4 2,2 2)" )
        assert polyline.moveVertex( 5.5, 4.5, 3 ), "move vertex failed"
        expwkt = "LINESTRING(5 0, 0 0, 0 4, 5.5 4.5, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        # 5-+-4
        # |   |
        # 6 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromWkt( "POLYGON((0 0,1 0,1 1,2 1,2 2,0 2,0 0))" )

        assert not polygon.moveVertex( 3, 4, -10 ), "move vertex unexpectedly succeeded"
        assert not polygon.moveVertex( 3, 4, 7 ), "move vertex unexpectedly succeeded"

        assert polygon.moveVertex( 1, 2, 0 ), "move vertex failed"
        expwkt = "POLYGON((1 2,1 0,1 1,2 1,2 2,0 2,1 2))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.moveVertex( 3, 4, 3 ), "move vertex failed"
        expwkt = "POLYGON((1 2,1 0,1 1,3 4,2 2,0 2,1 2))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.moveVertex( 2, 3, 6 ), "move vertex failed"
        expwkt = "POLYGON((2 3,1 0,1 1,3 4,2 2,0 2,2 3))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2!+
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.moveVertex( 6, 2, 9 ), "move vertex failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,6 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.moveVertex( 1, 2, 0 ), "move vertex failed"
        expwkt = "MULTIPOLYGON(((1 2,1 0,1 1,2 1,2 2,0 2,1 2)),((4 0,5 0,6 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.moveVertex( 2, 1, 7 ), "move vertex failed"
        expwkt = "MULTIPOLYGON(((1 2,1 0,1 1,2 1,2 2,0 2,1 2)),((2 1,5 0,6 2,3 2,3 1,4 1,2 1)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

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
        polyline = QgsGeometry.fromWkt( "LINESTRING(5 0,0 0,0 4,5 4,5 1,1 1,1 3,4 3,4 2,2 2)" )
        assert polyline.deleteVertex( 3 ), "Delete vertex 5 4 failed"
        expwkt = "LINESTRING(5 0, 0 0, 0 4, 5 1, 1 1, 1 3, 4 3, 4 2, 2 2)"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert not polyline.deleteVertex( -5 ), "Delete vertex -5 unexpectedly succeeded"
        assert not polyline.deleteVertex( 100 ), "Delete vertex 100 unexpectedly succeeded"

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        polyline = QgsGeometry.fromWkt("MULTILINESTRING((0 0, 1 0, 1 1, 2 1,2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert not polyline.deleteVertex(-100), "Delete vertex -100 unexpectedly succeeded"
        assert not polyline.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        assert polyline.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "MULTILINESTRING((1 0, 1 1, 2 1, 2 0), (3 1, 5 1, 5 0, 6 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        polyline = QgsGeometry.fromWkt("MULTILINESTRING((0 0, 1 0, 1 1, 2 1,2 0),(3 0, 3 1, 5 1, 5 0, 6 0))")
        for i in range(5):
                assert polyline.deleteVertex(5), "Delete vertex 5 failed"
        expwkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 0))"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        polygon = QgsGeometry.fromWkt("POLYGON((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))")

        assert polygon.deleteVertex(2), "Delete vertex 2 failed"
        print "FIXME: exportToWkt doesn't put a blank behind the comma"
        expwkt = "POLYGON((0 0,1 0,2 1,2 2,0 2,0 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.deleteVertex(0), "Delete vertex 0 failed"
        expwkt = "POLYGON((1 0,2 1,2 2,0 2,1 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.deleteVertex(4), "Delete vertex 4 failed"
        expwkt = "POLYGON((2 1,2 2,0 2,2 1))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert not polygon.deleteVertex(-100), "Delete vertex -100 unexpectedly succeeded"
        assert not polygon.deleteVertex(100), "Delete vertex 100 unexpectedly succeeded"

        # 5-+-4 0-+-9
        # |   | |   |
        # 6 2-3 1-2 +
        # | |     | |
        # 0-1     7-8
        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.deleteVertex( 9 ), "Delete vertex 5 2 failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.deleteVertex( 0 ), "Delete vertex 0 failed"
        expwkt = "MULTIPOLYGON(((1 0,1 1,2 1,2 2,0 2,1 0)),((4 0,5 0,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert polygon.deleteVertex( 6 ), "Delete vertex 6 failed"
        expwkt = "MULTIPOLYGON(((1 0,1 1,2 1,2 2,0 2,1 0)),((5 0,3 2,3 1,4 1,5 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        for i in range(6):
             assert polygon.deleteVertex( 0 ), "Delete vertex 0 failed"

        expwkt = "MULTIPOLYGON(((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        # 3-+-+-+-+-+-+-+-+-2
        # |                 |
        # + 8-7 3-2 8-7 3-2 +
        # | | | | | | | | | |
        # + 5-6 0-1 5-6 0-1 +
        # |                 |
        # 0-+-+-+-+---+-+-+-1
        polygon = QgsGeometry.fromWkt( "POLYGON((0 0,9 0,9 3,0 3,0 0),(1 1,2 1,2 2,1 2,1 1),(3 1,4 1,4 2,3 2,3 1),(5 1,6 1,6 2,5 2,5 1),(7 1,8 1,8 2,7 2,7 1))" )
        #                                          0   1   2   3   4     5   6   7   8   9    10  11  12  13  14    15  16  17  18  19    20  21  22  23  24

        for i in range(4):
            assert polygon.deleteVertex(16), "Delete vertex 16 failed" % i

        expwkt = "POLYGON((0 0,9 0,9 3,0 3,0 0),(1 1,2 1,2 2,1 2,1 1),(3 1,4 1,4 2,3 2,3 1),(7 1,8 1,8 2,7 2,7 1))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        for i in range(3):
            for j in range(4):
               assert polygon.deleteVertex(5), "Delete vertex 5 failed" % i

        expwkt = "POLYGON((0 0,9 0,9 3,0 3,0 0))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

    def testInsertVertex(self):
        linestring = QgsGeometry.fromWkt( "LINESTRING(1 0,2 0)" )

        if TestQgsGeometry.wkbPtr:
          # CHANGE old implementation fails to insert vertex
          assert linestring.insertVertex( 0, 0, 0 ), "Insert vertex 0 0 at 0 failed"
          expwkt = "LINESTRING(0 0, 1 0, 2 0)"
          wkt = linestring.exportToWkt()
          assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert linestring.insertVertex( 1.5, 0, 2 if TestQgsGeometry.wkbPtr else 1 ), "Insert vertex 1.5 0 at 2 failed"
        expwkt = "LINESTRING(0 0, 1 0, 1.5 0, 2 0)" if TestQgsGeometry.wkbPtr else "LINESTRING(1 0, 1.5 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        assert not linestring.insertVertex( 3, 0, 5 ), "Insert vertex 3 0 at 5 should have failed"

        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.insertVertex( 0, 0, 8 ), "Insert vertex 0 0 at 8 failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,0 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()

        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )
        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.insertVertex( 0, 0, 7 ), "Insert vertex 0 0 at 7 failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((0 0,4 0,5 0,5 2,3 2,3 1,4 1,0 0)))"
        wkt = polygon.exportToWkt()

        if TestQgsGeometry.wkbPtr:
          # CHANGE old implementation produces: MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),())
          assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

    def testTranslate(self):
        point = QgsGeometry.fromWkt( "POINT(1 1)" )
        assert point.translate( 1, 2 )==0, "Translate failed"
        expwkt = "POINT(2 3)"
        wkt = point.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        point = QgsGeometry.fromWkt( "MULTIPOINT(1 1,2 2,3 3)" )
        assert point.translate( 1, 2 )==0, "Translate failed"
        expwkt = "MULTIPOINT(2 3, 3 4, 4 5)"
        wkt = point.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        linestring = QgsGeometry.fromWkt( "LINESTRING(1 0,2 0)" )
        assert linestring.translate( 1, 2 )==0, "Translate failed"
        expwkt = "LINESTRING(2 2, 3 2)"
        wkt = linestring.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.translate( 1, 2 )==0, "Translate failed"
        expwkt = "MULTIPOLYGON(((1 2,2 2,2 3,3 3,3 4,1 4,1 2)),((5 2,6 2,6 2,4 4,4 3,5 3,5 2)))"
        wkt = polygon.exportToWkt()

        ct = QgsCoordinateTransform()

        point = QgsGeometry.fromWkt( "POINT(1 1)" )
        assert point.transform( ct )==0, "Translate failed"
        expwkt = "POINT(1 1)"
        wkt = point.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        point = QgsGeometry.fromWkt( "MULTIPOINT(1 1,2 2,3 3)" )
        assert point.transform( ct )==0, "Translate failed"
        expwkt = "MULTIPOINT(1 1, 2 2, 3 3)"
        wkt = point.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        linestring = QgsGeometry.fromWkt( "LINESTRING(1 0,2 0)" )
        assert linestring.transform( ct )==0, "Translate failed"
        expwkt = "LINESTRING(1 0, 2 0)"
        wkt = linestring.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        polygon = QgsGeometry.fromWkt( "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))" )
        assert polygon.transform( ct )==0, "Translate failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
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
        points = [ QgsPoint(5,0), QgsPoint(0,0), QgsPoint(0,4), QgsPoint(5,4), QgsPoint(5,1), QgsPoint(1,1), QgsPoint(1,3), QgsPoint(4,3), QgsPoint(4,2), QgsPoint(2,2) ]
        polyline = QgsGeometry.fromPolyline(points)
        expbb = QgsRectangle(0,0,5,4)
        bb = polyline.boundingBox()
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]
        polyline = QgsGeometry.fromMultiPolyline(points)
        expbb = QgsRectangle(0,0,6,1)
        bb = polyline.boundingBox()
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

        # 5---4
        # |   |
        # | 2-3
        # | |
        # 0-1
        points = [[
            QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0),
          ]]
        polygon = QgsGeometry.fromPolygon(points)
        expbb = QgsRectangle(0,0,2,2)
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
            [ QgsPoint(0,0), QgsPoint(3,0), QgsPoint(3,3), QgsPoint(0,3), QgsPoint(0,0) ],
            [ QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(1,2), QgsPoint(1,1) ],
          ]
        polygon = QgsGeometry.fromPolygon(points)
        expbb = QgsRectangle(0,0,3,3)
        bb = polygon.boundingBox()
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
          ]

        polygon = QgsGeometry.fromMultiPolygon(points)
        expbb = QgsRectangle(0,0,5,2)
        bb = polygon.boundingBox()
        assert expbb == bb, "Expected:\n%s\nGot:\n%s\n" % (expbb.toString(), bb.toString())

    def testAddPart(self):
        #   2-3 6-+-7
        #   | | |   |
        # 0-1 4 5   8-9
        points = [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]

        polyline = QgsGeometry.fromPolyline( points[0] )
        assert polyline.addPart( points[1][0:1] ) == 2, "addPart with one point line unexpectedly succeeded."
        assert polyline.addPart( points[1][0:2] ) == 0, "addPart with two point line failed."
        expwkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1))"
        wkt = polyline.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

        polyline = QgsGeometry.fromPolyline( points[0] )
        assert polyline.addPart( points[1] ) == 0, "addPart with %d point line failed." % len(points[1])
        expwkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 0), (3 0, 3 1, 5 1, 5 0, 6 0))"

        # 5-+-4 0-+-9
        # |   | |   |
        # | 2-3 1-2 |
        # | |     | |
        # 0-1     7-8
        points = [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
          ]

        polygon = QgsGeometry.fromPolygon( points[0] )

        assert polygon.addPart( points[1][0][0:1] ) == 2, "addPart with one point ring unexpectedly succeeded."
        assert polygon.addPart( points[1][0][0:2] ) == 2, "addPart with two point ring unexpectedly succeeded."
        assert polygon.addPart( points[1][0][0:3] ) == 2, "addPart with unclosed three point ring unexpectedly succeeded."
        assert polygon.addPart( [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(4,0) ] ) == 2, "addPart with 'closed' three point ring unexpectedly succeeded."

        assert polygon.addPart( points[1][0] ) == 0, "addPart failed"
        expwkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
        wkt = polygon.exportToWkt()
        assert compareWkt( expwkt, wkt ), "Expected:\n%s\nGot:\n%s\n" % (expwkt, wkt )

    def testConvertToType(self):
        # 5-+-4 0-+-9  13-+-+-12
        # |   | |   |  |       |
        # | 2-3 1-2 |  + 18-17 +
        # | |     | |  | |   | |
        # 0-1     7-8  + 15-16 +
        #              |       |
        #              10-+-+-11
        points = [
            [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0) ], ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0) ], ],
            [ [ QgsPoint(10,0), QgsPoint(13,0), QgsPoint(13,3), QgsPoint(10,3), QgsPoint(10,0) ], [ QgsPoint(11,1), QgsPoint(12,1), QgsPoint(12,2), QgsPoint(11,2), QgsPoint(11,1) ] ]
          ]
        ######## TO POINT ########
        # POINT TO POINT
        point = QgsGeometry.fromPoint(QgsPoint(1,1))
        wkt = point.convertToType(QGis.Point, False).exportToWkt()
        expWkt = "POINT(1 1)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from point to point. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POINT TO MULTIPOINT
        point = QgsGeometry.fromPoint(QgsPoint(1,1))
        wkt = point.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MULTIPOINT(1 1)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from point to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # LINE TO MULTIPOINT
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MULTIPOINT(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from line to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTILINE TO MULTIPOINT
        multiLine = QgsGeometry.fromMultiPolyline(points[2])
        wkt = multiLine.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MULTIPOINT(10 0, 13 0, 13 3, 10 3, 10 0, 11 1, 12 1, 12 2, 11 2, 11 1)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multiline to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POLYGON TO MULTIPOINT
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MULTIPOINT(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from poylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOLYGON TO MULTIPOINT
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QGis.Point, True).exportToWkt()
        expWkt = "MULTIPOINT(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0, 4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0, 10 0, 13 0, 13 3, 10 3, 10 0, 11 1, 12 1, 12 2, 11 2, 11 1)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipoylgon to multipoint. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )


        ######## TO LINE ########
        # POINT TO LINE
        point = QgsGeometry.fromPoint(QgsPoint(1,1))
        assert point.convertToType(QGis.Line, False) is None , "convertToType with a point should return a null geometry"
        # MULTIPOINT TO LINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LINESTRING(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipoint to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOINT TO MULTILINE
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipoint to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTILINE (which has a single part) TO LINE
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LINESTRING(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multiline to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # LINE TO MULTILINE
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from line to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POLYGON TO LINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LINESTRING(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from polygon to line. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POLYGON TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from polygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POLYGON with ring TO MULTILINE
        polygon = QgsGeometry.fromPolygon(points[2])
        wkt = polygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MULTILINESTRING((10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from polygon with ring to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOLYGON (which has a single part) TO LINE
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QGis.Line, False).exportToWkt()
        expWkt = "LINESTRING(0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0)"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOLYGON TO MULTILINE
        multiPolygon = QgsGeometry.fromMultiPolygon(points)
        wkt = multiPolygon.convertToType(QGis.Line, True).exportToWkt()
        expWkt = "MULTILINESTRING((0 0, 1 0, 1 1, 2 1, 2 2, 0 2, 0 0), (4 0, 5 0, 5 2, 3 2, 3 1, 4 1, 4 0), (10 0, 13 0, 13 3, 10 3, 10 0), (11 1, 12 1, 12 2, 11 2, 11 1))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipolygon to multiline. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )


        ######## TO POLYGON ########
         # MULTIPOINT TO POLYGON
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "POLYGON((0 0,1 0,1 1,2 1,2 2,0 2,0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipoint to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOINT TO MULTIPOLYGON
        multipoint = QgsGeometry.fromMultiPoint(points[0][0])
        wkt = multipoint.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multipoint to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # LINE TO POLYGON
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "POLYGON((0 0,1 0,1 1,2 1,2 2,0 2,0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from line to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # LINE ( 3 vertices, with first = last ) TO POLYGON
        line = QgsGeometry.fromPolyline([QgsPoint(1,1),QgsPoint(0,0),QgsPoint(1,1)])
        assert line.convertToType(QGis.Polygon, False) is None , "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry"
        # MULTILINE ( with a part of 3 vertices, with first = last ) TO MULTIPOLYGON
        multiline = QgsGeometry.fromMultiPolyline([points[0][0],[QgsPoint(1,1),QgsPoint(0,0),QgsPoint(1,1)]])
        assert multiline.convertToType(QGis.Polygon, True) is None , "convertToType to polygon of a 3 vertices lines with first and last vertex identical should return a null geometry"
        # LINE TO MULTIPOLYGON
        line = QgsGeometry.fromPolyline(points[0][0])
        wkt = line.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from line to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTILINE (which has a single part) TO POLYGON
        multiLine = QgsGeometry.fromMultiPolyline(points[0])
        wkt = multiLine.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "POLYGON((0 0,1 0,1 1,2 1,2 2,0 2,0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTILINE TO MULTIPOLYGON
        multiLine = QgsGeometry.fromMultiPolyline([points[0][0],points[1][0]])
        wkt = multiLine.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)),((4 0,5 0,5 2,3 2,3 1,4 1,4 0)))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multiline to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # POLYGON TO MULTIPOLYGON
        polygon = QgsGeometry.fromPolygon(points[0])
        wkt = polygon.convertToType(QGis.Polygon, True).exportToWkt()
        expWkt = "MULTIPOLYGON(((0 0,1 0,1 1,2 1,2 2,0 2,0 0)))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from polygon to multipolygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )
        # MULTIPOLYGON (which has a single part) TO POLYGON
        multiPolygon = QgsGeometry.fromMultiPolygon([points[0]])
        wkt = multiPolygon.convertToType(QGis.Polygon, False).exportToWkt()
        expWkt = "POLYGON((0 0,1 0,1 1,2 1,2 2,0 2,0 0))"
        assert compareWkt( expWkt, wkt ), "convertToType failed: from multiline to polygon. Expected:\n%s\nGot:\n%s\n" % (expWkt, wkt )

if __name__ == '__main__':
    unittest.main()
