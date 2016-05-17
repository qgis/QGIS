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

import qgis  # NOQA

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QgsDistanceArea,
                       QgsCoordinateReferenceSystem,
                       QGis,
                       QgsUnitTypes
                       )

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QLocale

# Convenience instances in case you may need them
# not used in this test

start_app()


class TestQgsDistanceArea(unittest.TestCase):

    def testCrs(self):
        # test setting/getting the source CRS
        da = QgsDistanceArea()

        # try setting using a crs id
        da.setSourceCrs(3452)
        self.assertEqual(da.sourceCrsId(), 3452)

        # try setting using a CRS object
        crs = QgsCoordinateReferenceSystem(3111, QgsCoordinateReferenceSystem.EpsgCrsId)
        da.setSourceCrs(crs)
        self.assertEqual(da.sourceCrsId(), crs.srsid())

    def testMeasureLine(self):
        #   +-+
        #   | |
        # +-+ +
        linestring = QgsGeometry.fromPolyline(
            [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ]
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
                [QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 0), ],
                [QgsPoint(3, 0), QgsPoint(3, 1), QgsPoint(5, 1), QgsPoint(5, 0), QgsPoint(6, 0), ]
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
                QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
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
                [QgsPoint(0, 0), QgsPoint(3, 0), QgsPoint(3, 3), QgsPoint(0, 3), QgsPoint(0, 0)],
                [QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(1, 2), QgsPoint(1, 1)],
            ]
        )
        da = QgsDistanceArea()
        area = da.measure(polygon)
        assert area == 8, "Expected:\n%f\nGot:\n%f\n" % (8, area)

# MH150729: Changed behaviour to consider inner rings for perimeter calculation. Therefore, expected result is 16.
        perimeter = da.measurePerimeter(polygon)
        assert perimeter == 16, "Expected:\n%f\nGot:\n%f\n" % (16, perimeter)

    def testMeasureMultiPolygon(self):
        # +-+-+ +-+-+
        # |   | |   |
        # + +-+ +-+ +
        # | |     | |
        # +-+     +-+
        polygon = QgsGeometry.fromMultiPolygon(
            [
                [[QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0), ]],
                [[QgsPoint(4, 0), QgsPoint(5, 0), QgsPoint(5, 2), QgsPoint(3, 2), QgsPoint(3, 1), QgsPoint(4, 1), QgsPoint(4, 0), ]]
            ]
        )

        da = QgsDistanceArea()
        area = da.measure(polygon)
        assert area == 6, 'Expected:\n%f\nGot:\n%f\n' % (6, area)

        perimeter = da.measurePerimeter(polygon)
        assert perimeter == 16, "Expected:\n%f\nGot:\n%f\n" % (16, perimeter)

    def testWillUseEllipsoid(self):
        """test QgsDistanceArea::willUseEllipsoid """

        da = QgsDistanceArea()
        da.setEllipsoidalMode(False)
        da.setEllipsoid("NONE")
        self.assertFalse(da.willUseEllipsoid())

        da.setEllipsoidalMode(True)
        self.assertFalse(da.willUseEllipsoid())

        da.setEllipsoid("WGS84")
        assert da.willUseEllipsoid()

        da.setEllipsoidalMode(False)
        self.assertFalse(da.willUseEllipsoid())

    def testLengthMeasureAndUnits(self):
        """Test a variety of length measurements in different CRS and ellipsoid modes, to check that the
           calculated lengths and units are always consistent
        """

        da = QgsDistanceArea()
        da.setSourceCrs(3452)
        da.setEllipsoidalMode(False)
        da.setEllipsoid("NONE")
        daCRS = QgsCoordinateReferenceSystem()
        daCRS.createFromSrsId(da.sourceCrs())

        # We check both the measured length AND the units, in case the logic regarding
        # ellipsoids and units changes in future
        distance = da.measureLine(QgsPoint(1, 1), QgsPoint(2, 3))
        units = da.lengthUnits()

        print("measured {} in {}".format(distance, QgsUnitTypes.toString(units)))
        assert ((abs(distance - 2.23606797) < 0.00000001 and units == QGis.Degrees) or
                (abs(distance - 248.52) < 0.01 and units == QGis.Meters))

        da.setEllipsoid("WGS84")
        distance = da.measureLine(QgsPoint(1, 1), QgsPoint(2, 3))
        units = da.lengthUnits()

        print("measured {} in {}".format(distance, QgsUnitTypes.toString(units)))
        assert ((abs(distance - 2.23606797) < 0.00000001 and units == QGis.Degrees) or
                (abs(distance - 248.52) < 0.01 and units == QGis.Meters))

        da.setEllipsoidalMode(True)
        distance = da.measureLine(QgsPoint(1, 1), QgsPoint(2, 3))
        units = da.lengthUnits()

        print("measured {} in {}".format(distance, QgsUnitTypes.toString(units)))
        # should always be in Meters
        self.assertAlmostEqual(distance, 247555.57, delta=0.01)
        self.assertEqual(units, QGis.Meters)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QGis.NauticalMiles)
        self.assertAlmostEqual(distance, 133.669, delta=0.01)

        # now try with a source CRS which is in feet
        da.setSourceCrs(27469)
        da.setEllipsoidalMode(False)
        # measurement should be in feet
        distance = da.measureLine(QgsPoint(1, 1), QgsPoint(2, 3))
        units = da.lengthUnits()
        print("measured {} in {}".format(distance, QgsUnitTypes.toString(units)))
        self.assertAlmostEqual(distance, 2.23606797, delta=0.000001)
        self.assertEqual(units, QGis.Feet)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QGis.Meters)
        self.assertAlmostEqual(distance, 0.6815, delta=0.001)

        da.setEllipsoidalMode(True)
        # now should be in Meters again
        distance = da.measureLine(QgsPoint(1, 1), QgsPoint(2, 3))
        units = da.lengthUnits()
        print("measured {} in {}".format(distance, QgsUnitTypes.toString(units)))
        self.assertAlmostEqual(distance, 0.67953772, delta=0.000001)
        self.assertEqual(units, QGis.Meters)

        # test converting the resultant length
        distance = da.convertLengthMeasurement(distance, QGis.Feet)
        self.assertAlmostEqual(distance, 2.2294, delta=0.001)

    def testAreaMeasureAndUnits(self):
        """Test a variety of area measurements in different CRS and ellipsoid modes, to check that the
           calculated areas and units are always consistent
        """

        da = QgsDistanceArea()
        da.setSourceCrs(3452)
        da.setEllipsoidalMode(False)
        da.setEllipsoid("NONE")
        daCRS = QgsCoordinateReferenceSystem()
        daCRS.createFromSrsId(da.sourceCrs())

        polygon = QgsGeometry.fromPolygon(
            [[
                QgsPoint(0, 0), QgsPoint(1, 0), QgsPoint(1, 1), QgsPoint(2, 1), QgsPoint(2, 2), QgsPoint(0, 2), QgsPoint(0, 0),
            ]]
        )

        # We check both the measured area AND the units, in case the logic regarding
        # ellipsoids and units changes in future
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print("measured {} in {}".format(area, QgsUnitTypes.toString(units)))
        assert ((abs(area - 3.0) < 0.00000001 and units == QgsUnitTypes.SquareDegrees) or
                (abs(area - 37176087091.5) < 0.1 and units == QgsUnitTypes.SquareMeters))

        da.setEllipsoid("WGS84")
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print("measured {} in {}".format(area, QgsUnitTypes.toString(units)))
        assert ((abs(area - 3.0) < 0.00000001 and units == QgsUnitTypes.SquareDegrees) or
                (abs(area - 37176087091.5) < 0.1 and units == QgsUnitTypes.SquareMeters))

        da.setEllipsoidalMode(True)
        area = da.measureArea(polygon)
        units = da.areaUnits()

        print("measured {} in {}".format(area, QgsUnitTypes.toString(units)))
        # should always be in Meters Squared
        self.assertAlmostEqual(area, 37416879192.9, delta=0.1)
        self.assertEqual(units, QgsUnitTypes.SquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.SquareMiles)
        self.assertAlmostEqual(area, 14446.7378, delta=0.001)

        # now try with a source CRS which is in feet
        polygon = QgsGeometry.fromPolygon(
            [[
                QgsPoint(1850000, 4423000), QgsPoint(1851000, 4423000), QgsPoint(1851000, 4424000), QgsPoint(1852000, 4424000), QgsPoint(1852000, 4425000), QgsPoint(1851000, 4425000), QgsPoint(1850000, 4423000)
            ]]
        )
        da.setSourceCrs(27469)
        da.setEllipsoidalMode(False)
        # measurement should be in square feet
        area = da.measureArea(polygon)
        units = da.areaUnits()
        print("measured {} in {}".format(area, QgsUnitTypes.toString(units)))
        self.assertAlmostEqual(area, 2000000, delta=0.001)
        self.assertEqual(units, QgsUnitTypes.SquareFeet)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.SquareYards)
        self.assertAlmostEqual(area, 222222.2222, delta=0.001)

        da.setEllipsoidalMode(True)
        # now should be in Square Meters again
        area = da.measureArea(polygon)
        units = da.areaUnits()
        print("measured {} in {}".format(area, QgsUnitTypes.toString(units)))
        self.assertAlmostEqual(area, 184149.37, delta=1.0)
        self.assertEqual(units, QgsUnitTypes.SquareMeters)

        # test converting the resultant area
        area = da.convertAreaMeasurement(area, QgsUnitTypes.SquareYards)
        self.assertAlmostEqual(area, 220240.8172549, delta=1.0)

    def testFormatDistance(self):
        """Test formatting distances"""
        QLocale.setDefault(QLocale.c())
        self.assertEqual(QgsDistanceArea.formatDistance(45, 3, QGis.Meters), u'45.000 m')
        self.assertEqual(QgsDistanceArea.formatDistance(1300, 1, QGis.Meters, False), u'1.3 km')
        self.assertEqual(QgsDistanceArea.formatDistance(.005, 1, QGis.Meters, False), u'5.0 mm')
        self.assertEqual(QgsDistanceArea.formatDistance(.05, 1, QGis.Meters, False), u'5.0 cm')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QGis.Kilometers, True), u'1.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QGis.Kilometers, False), u'1.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QGis.Kilometers, True), u'0.500 km')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QGis.Kilometers, False), u'500.000 m')
        self.assertEqual(QgsDistanceArea.formatDistance(6000, 0, QGis.Feet, True), u'6,000 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(6000, 3, QGis.Feet, False), u'1.136 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QGis.Feet, True), u'300 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QGis.Feet, False), u'300 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(3000, 0, QGis.Yards, True), u'3,000 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(3000, 3, QGis.Yards, False), u'1.705 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QGis.Yards, True), u'300 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(300, 0, QGis.Yards, False), u'300 yd')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QGis.Miles, True), u'1.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 3, QGis.Miles, False), u'1.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 3, QGis.Miles, True), u'0.500 mi')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 0, QGis.Miles, False), u'2,640 ft')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 1, QGis.NauticalMiles, True), u'0.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(0.5, 1, QGis.NauticalMiles, False), u'0.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QGis.NauticalMiles, True), u'1.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QGis.NauticalMiles, False), u'1.5 NM')
        self.assertEqual(QgsDistanceArea.formatDistance(1.5, 1, QGis.Degrees, True), u'1.5 degrees')
        self.assertEqual(QgsDistanceArea.formatDistance(1.0, 1, QGis.Degrees, False), u'1.0 degree')
        self.assertEqual(QgsDistanceArea.formatDistance(1.0, 1, QGis.UnknownUnit, False), u'1.0')
        QLocale.setDefault(QLocale.system())

if __name__ == '__main__':
    unittest.main()
