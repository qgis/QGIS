import unittest

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QGis)

# Convenience instances in case you may need them
# not used in this test
#from utilities import getQgisTestApp
#QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

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


if __name__ == '__main__':
    unittest.main()

