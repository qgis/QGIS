import unittest

from qgis.core import (QgsGeometry,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsPoint,
                       QGis)

from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

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
        
        layer = QgsVectorLayer("Polygon", "buffer", "memory")
        assert layer.isValid(), "Failed to create valid polygon memory layer"
        
        provider = layer.dataProvider()
                
        ft = QgsFeature()
        ft.setGeometry(bufferGeom)
        provider.addFeatures([ft])
        
        myMessage = ('Expected:\n%s\nGot:\n%s\n' %
                      (1, layer.featureCount()))
        assert layer.featureCount() == 1, myMessage
                
if __name__ == '__main__':
    unittest.main()

