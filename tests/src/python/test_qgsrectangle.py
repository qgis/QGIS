import unittest

from qgis.core import (QGis,
                       QgsRectangle,
                       QgsPoint)

from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsRectangle(unittest.TestCase):

    def testCtor(self):
        rect = QgsRectangle( 5.0, 5.0, 10.0, 10.0)

        assert rect.isEmpty(), "Empty rectangle constructed"

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (5.0, rect.xMinimum()))
        assert rect.xMinimum() == 5.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (5.0, rect.yMinimum()))
        assert rect.yMinimum() == 5.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (10.0, rect.xMaximum()))
        assert rect.xMaximum() == 10.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (10.0, rect.yMaximum()))
        assert rect.yMaximum() == 10.0, myMessage


    def testDimensions(self):
        rect = QgsRectangle( 0.0, 0.0, 10.0, 10.0)

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (10.0, rect.width()))
        assert rect.width() == 10.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (10.0, rect.height()))
        assert rect.height() == 10.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      ("5.0, 5.0", rect.center().toString()))
        assert rect.center() == QgsPoint(5.0, 5.0), myMessage

        rect.scale(2.0)

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (20.0, rect.width()))
        assert rect.width() == 20.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (20.0, rect.height()))
        assert rect.height() == 20.0, myMessage

    def testIntersection(self):
        rect1 = QgsRectangle( 0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle( 2.0, 2.0, 7.0, 7.0)

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.intersects(rect2)))
        assert rect1.intersects(rect2), myMessage

        rect3 = rect1.intersect(rect2)
        assert rect3.isEmpty(), "Empty rectangle returned"

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (3.0, rect.width()))
        assert rect.width() == 3.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (3.0, rect.height()))
        assert rect.height() == 3.0, myMessage

    def testContains(self):
        rect1 = QgsRectangle( 0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle( 2.0, 2.0, 7.0, 7.0)
        pnt1 = QgsPoint(4.0, 4.0)
        pnt2 = QgsPoint(6.0, 2.0)

        rect3 = rect1.intersect(rect2)

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.contains(rect3)))
        assert rect1.contains(rect3), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect2.contains(rect3)))
        assert rect2.contains(rect3), myMessage

        # test for point
        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.contains(pnt1)))
        assert rect1.contains(pnt1), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect2.contains(pnt1)))
        assert rect2.contains(pnt1), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect3.contains(pnt1)))
        assert rect3.contains(pnt1), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (False, rect1.contains(pnt2)))
        assert rect1.contains(pnt2), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect2.contains(pnt2)))
        assert rect2.contains(pnt2), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect3.contains(pnt2)))
        assert rect3.contains(pnt2), myMessage

    def testUnion(self):
        rect1 = QgsRectangle( 0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle( 2.0, 2.0, 7.0, 7.0)
        pnt1 = QgsPoint(6.0, 2.0)

        rect1.combineExtentWith(rect2)
        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.contains(rect2)))
        assert rect1.contains(rect2), myMessage

        print rect1.toString()
        assert rect1 == QgsRectangle(0.0, 0.0, 7.0, 7.0), "Wrong combine with rectangle result"

        rect1 = QgsRectangle( 0.0, 0.0, 5.0, 5.0)
        rect1.combineExtentWith(6.0, 2.0)
        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.contains(pnt1)))
        assert rect1.contains(pnt1), myMessage

        print rect1.toString()
        assert rect1 == QgsRectangle(0.0, 0.0, 6.0, 6.0), "Wrong combine with point result"

        rect1 = QgsRectangle( 0.0, 0.0, 5.0, 5.0)
        rect1.unionRect(rect2)
        myMessage = ('Expected: %s\nGot: %s\n' %
                      (True, rect1.contains(rect2)))
        assert rect1.contains(rect2), myMessage

        assert rect1 == QgsRectangle(0.0, 0.0, 7.0, 7.0), "Wrong union result"
