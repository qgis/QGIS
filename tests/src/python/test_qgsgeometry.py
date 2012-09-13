import unittest

from qgis.core import (QgsGeometry,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsPoint,
                       QGis)

# Convenience instances in case you may need them
# not used in this test
from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsGeometry(unittest.TestCase):

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
        myMultiPoint = QgsGeometry.fromMultiPoint([(QgsPoint(0, 0)),(QgsPoint(1, 1))])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBMultiPoint, myMultiPoint.type()))
        assert myMultiPoint.wkbType() == QGis.WKBMultiPoint, myMessage

    def testFromLine(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(1, 1), QgsPoint(2, 2)])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBLineString, myLine.type()))
        assert myLine.wkbType() == QGis.WKBLineString, myMessage

    def testFromMultiLine(self):
        myMultiPolyline = QgsGeometry.fromMultiPolyline([[QgsPoint(0, 0),QgsPoint(1, 1)],[QgsPoint(0, 1), QgsPoint(2, 1)]])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBMultiLineString, myMultiPolyline.type()))
        assert myMultiPolyline.wkbType() == QGis.WKBMultiLineString, myMessage

    def testFromPolygon(self):
        myPolygon = QgsGeometry.fromPolygon([[QgsPoint(1, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)]])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBPolygon, myPolygon.type()))
        assert myPolygon.wkbType() == QGis.WKBPolygon, myMessage

    def testFromMultiPolygon(self):
        myMultiPolygon = QgsGeometry.fromMultiPolygon([[[QgsPoint(1, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)]],
                                                      [[QgsPoint(2, 2), QgsPoint(3, 3), QgsPoint(3, 1), QgsPoint(2, 2)]]])
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (QGis.WKBMultiPolygon, myMultiPolygon.type()))
        assert myMultiPolygon.wkbType() == QGis.WKBMultiPolygon, myMessage

    def testIntersection(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(0, 0),QgsPoint(1, 1),QgsPoint(2, 2)])
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
        myPoly = QgsGeometry.fromPolygon([[QgsPoint(0, 0),QgsPoint(2, 0),QgsPoint(2, 2),QgsPoint(0, 2), QgsPoint(0, 0)]])
        myPoint = QgsGeometry.fromPoint(QgsPoint(1, 1))
        containsGeom = QgsGeometry.contains(myPoly, myPoint)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ("True", containsGeom))
        assert containsGeom == True, myMessage
               
    def testTouches(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(0, 0),QgsPoint(1, 1),QgsPoint(2, 2)])
        myPoly = QgsGeometry.fromPolygon([[QgsPoint(0, 0),QgsPoint(1, 1),QgsPoint(2, 0),QgsPoint(0, 0)]])
        touchesGeom = QgsGeometry.touches(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ("True", touchesGeom))
        assert touchesGeom == True, myMessage
        
    def testOverlaps(self):
        myPolyA = QgsGeometry.fromPolygon([[QgsPoint(0, 0),QgsPoint(1, 3),QgsPoint(2, 0),QgsPoint(0, 0)]])
        myPolyB = QgsGeometry.fromPolygon([[QgsPoint(0, 0),QgsPoint(2, 0),QgsPoint(2, 2),QgsPoint(0, 2), QgsPoint(0, 0)]])
        overlapsGeom = QgsGeometry.overlaps(myPolyA, myPolyB)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ("True", overlapsGeom))
        assert overlapsGeom == True, myMessage
    
    def testWithin(self):
        myLine = QgsGeometry.fromPolyline([QgsPoint(0.5, 0.5),QgsPoint(1, 1),QgsPoint(1.5, 1.5)])
        myPoly = QgsGeometry.fromPolygon([[QgsPoint(0, 0),QgsPoint(2, 0),QgsPoint(2, 2),QgsPoint(0, 2), QgsPoint(0, 0)]])
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
        myLine = QgsGeometry.fromPolyline([QgsPoint(0, 0),QgsPoint(1, 1),QgsPoint(3, 3)])
        myPoly = QgsGeometry.fromPolygon([[QgsPoint(1, 0),QgsPoint(2, 0),QgsPoint(2, 2),QgsPoint(1, 2), QgsPoint(1, 0)]])
        crossesGeom = QgsGeometry.crosses(myLine, myPoly)
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      ("True", crossesGeom))
        assert crossesGeom == True, myMessage


if __name__ == '__main__':
    unittest.main()

