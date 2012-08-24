import unittest

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QGis)

#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsGeoprocessing(unittest.TestCase):

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

