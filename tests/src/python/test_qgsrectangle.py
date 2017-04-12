# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRectangle.

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

import qgis  # NOQA

from qgis.core import QgsRectangle, QgsPoint

from qgis.testing import start_app, unittest
from utilities import compareWkt

start_app()


class TestQgsRectangle(unittest.TestCase):

    # Because isEmpty() is not returning expected result in 9b0fee3

    @unittest.expectedFailure
    def testCtor(self):
        rect = QgsRectangle(5.0, 5.0, 10.0, 10.0)

        myExpectedResult = True
        myResult = rect.isEmpty()
        myMessage = ('Expected: %s Got: %s' % (myExpectedResult, myResult))
        assert rect.isEmpty(), myMessage

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
        rect = QgsRectangle(0.0, 0.0, 10.0, 10.0)

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

    def testCalculations(self):
        rect = QgsRectangle(0.0, 1.0, 20.0, 10.0)
        self.assertEqual(rect.area(), 180.0)
        self.assertEqual(rect.perimeter(), 58.0)

    def testIntersection(self):
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect1.intersects(rect2)))
        assert rect1.intersects(rect2), myMessage

        rect3 = rect1.intersect(rect2)
        self.assertFalse(rect3.isEmpty(), "Empty rectangle returned")

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3.0, rect3.width()))
        assert rect3.width() == 3.0, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3.0, rect3.height()))
        assert rect3.height() == 3.0, myMessage

    def testContains(self):
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)
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
        self.assertFalse(rect1.contains(pnt2), myMessage)

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect2.contains(pnt2)))
        assert rect2.contains(pnt2), myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (False, rect3.contains(pnt2)))
        self.assertFalse(rect3.contains(pnt2), myMessage)

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect3.contains(pnt1)))
        self.assertTrue(rect3.contains(pnt1), myMessage)

    def testUnion(self):
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect2 = QgsRectangle(2.0, 2.0, 7.0, 7.0)
        pnt1 = QgsPoint(6.0, 2.0)

        rect1.combineExtentWith(rect2)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect1.contains(rect2)))
        assert rect1.contains(rect2), myMessage

        print((rect1.toString()))
        assert rect1 == QgsRectangle(
            0.0, 0.0, 7.0, 7.0), 'Wrong combine with rectangle result'

        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect1.combineExtentWith(6.0, 2.0)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect1.contains(pnt1)))
        assert rect1.contains(pnt1), myMessage

        myExpectedResult = QgsRectangle(0.0, 0.0, 6.0, 5.0).toString()
        myResult = rect1.toString()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedResult, myResult))
        self.assertEqual(myResult, myExpectedResult, myMessage)

        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        rect1.unionRect(rect2)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (True, rect1.contains(rect2)))
        assert rect1.contains(rect2), myMessage

        assert rect1 == QgsRectangle(0.0, 0.0, 7.0, 7.0), "Wrong union result"

    def testAsWktCoordinates(self):
        """Test that we can get a proper wkt representation fo the rect"""
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        myExpectedWkt = ('0 0, '
                         '5 5')
        myWkt = rect1.asWktCoordinates()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedWkt, myWkt))
        assert compareWkt(myWkt, myExpectedWkt), myMessage

    def testAsWktPolygon(self):
        """Test that we can get a proper rect wkt polygon representation for rect"""
        rect1 = QgsRectangle(0.0, 0.0, 5.0, 5.0)
        myExpectedWkt = ('POLYGON((0 0, '
                         '5 0, '
                         '5 5, '
                         '0 5, '
                         '0 0))')
        myWkt = rect1.asWktPolygon()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedWkt, myWkt))
        assert compareWkt(myWkt, myExpectedWkt), myMessage

    def testToString(self):
        """Test the different string representations"""
        rect = QgsRectangle(0, 0.1, 0.2, 0.3)
        myExpectedString = '0.0000000000000000,0.1000000000000000 : 0.2000000000000000,0.3000000000000000'
        myString = rect.toString()
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

        myExpectedString = '0,0 : 0,0'
        myString = rect.toString(0)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

        myExpectedString = '0.00,0.10 : 0.20,0.30'
        myString = rect.toString(2)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

        myExpectedString = '0.0,0.1 : 0.2,0.3'
        myString = rect.toString(1)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

        myExpectedString = '0.00,0.10 : 0.20,0.30'
        myString = rect.toString(-1)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

        rect = QgsRectangle(5000000.01111, -0.3, 5000000.44111, 99.8)
        myExpectedString = '5000000.01,-0.30 : 5000000.44,99.80'
        myString = rect.toString(-1)
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (myExpectedString, myString))
        assert myString == myExpectedString, myMessage

    def testToBox3d(self):
        rect = QgsRectangle(0, 0.1, 0.2, 0.3)
        box = rect.toBox3d(0.4, 0.5)
        self.assertEqual(box.xMinimum(), 0.0)
        self.assertEqual(box.yMinimum(), 0.1)
        self.assertEqual(box.zMinimum(), 0.4)
        self.assertEqual(box.xMaximum(), 0.2)
        self.assertEqual(box.yMaximum(), 0.3)
        self.assertEqual(box.zMaximum(), 0.5)


if __name__ == '__main__':
    unittest.main()
