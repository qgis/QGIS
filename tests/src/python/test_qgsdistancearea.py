# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDistanceArea.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Jürgen E. Fischer'
__date__ = '19/01/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

import qgis

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QgsDistanceArea,
                       QGis)

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest)

from PyQt4.QtCore import qDebug

# Convenience instances in case you may need them
# not used in this test

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsDistanceArea(TestCase):

    def testMeasureLine(self):
        #   +-+
        #   | |
        # +-+ +
	linestring = QgsGeometry.fromPolyline(
          [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ]
	)
	da = QgsDistanceArea()
        length = da.measure(linestring)
	myMessage = ('Expected:\n%f\nGot:\n%f\n' %
		      (4, length))
        assert length == 4, myMessage

    def testMeasureMultiLine(self):
        #   +-+ +-+-+
        #   | | |   |
        # +-+ + +   +-+
	linestring = QgsGeometry.fromMultiPolyline(
	  [
            [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,0), ],
            [ QgsPoint(3,0), QgsPoint(3,1), QgsPoint(5,1), QgsPoint(5,0), QgsPoint(6,0), ]
          ]
	)
	da = QgsDistanceArea()
        length = da.measure(linestring)
	myMessage = ('Expected:\n%f\nGot:\n%f\n' %
		      (9, length))
        assert length == 9, myMessage

    def testMeasurePolygon(self):
	# +-+-+
        # |   |
        # + +-+
	# | |
        # +-+
        polygon = QgsGeometry.fromPolygon(
          [[
            QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0),
          ]]
        )

	da = QgsDistanceArea()
        area = da.measure(polygon)
        assert area == 3, 'Expected:\n%f\nGot:\n%f\n' % (3, area)

	perimeter = da.measurePerimeter(polygon)
        assert perimeter == 8, 'Expected:\n%f\nGot:\n%f\n' % (8, perimeter)

    def testMeasurePolygonWithHole(self):
	# +-+-+-+
        # |     |
        # + +-+ +
        # | | | |
        # + +-+ +
        # |     |
	# +-+-+-+
        polygon = QgsGeometry.fromPolygon(
          [
	    [ QgsPoint(0,0), QgsPoint(3,0), QgsPoint(3,3), QgsPoint(0,3), QgsPoint(0,0) ],
	    [ QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(1,2), QgsPoint(1,1) ],
          ]
        )
	da = QgsDistanceArea()
        area = da.measure(polygon)
        assert area == 8, "Expected:\n%f\nGot:\n%f\n" % (8, area)

	perimeter = da.measurePerimeter(polygon)
        assert perimeter == 12, "Expected:\n%f\nGot:\n%f\n" % (12, perimeter)

    def testMeasureMultiPolygon(self):
	# +-+-+ +-+-+
        # |   | |   |
        # + +-+ +-+ +
	# | |     | |
        # +-+     +-+
        polygon = QgsGeometry.fromMultiPolygon(
	  [
	    [ [ QgsPoint(0,0), QgsPoint(1,0), QgsPoint(1,1), QgsPoint(2,1), QgsPoint(2,2), QgsPoint(0,2), QgsPoint(0,0), ] ],
            [ [ QgsPoint(4,0), QgsPoint(5,0), QgsPoint(5,2), QgsPoint(3,2), QgsPoint(3,1), QgsPoint(4,1), QgsPoint(4,0), ] ]
	  ]
        )

	da = QgsDistanceArea()
        area = da.measure(polygon)
        assert area == 6, 'Expected:\n%f\nGot:\n%f\n' % (6, area)

	perimeter = da.measurePerimeter(polygon)
        assert perimeter == 16, "Expected:\n%f\nGot:\n%f\n" % (16, perimeter)

if __name__ == '__main__':
    unittest.main()

