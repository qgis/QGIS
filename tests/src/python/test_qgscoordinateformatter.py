# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCoordinateFormatter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/07/2014'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis
from qgis.testing import unittest
from qgis.core import (
    QgsCoordinateFormatter,
    QgsPointXY,
    Qgis
)
from qgis.PyQt.QtCore import QLocale


class TestQgsCoordinateFormatter(unittest.TestCase):

    def setUp(self):
        super().setUp()
        QLocale.setDefault(QLocale(QLocale.English))

    def testFormatXPair(self):
        """Test formatting x as pair"""

        self.assertEqual(QgsCoordinateFormatter.formatX(20, QgsCoordinateFormatter.FormatPair, 0), '20')
        self.assertEqual(QgsCoordinateFormatter.formatX(-20, QgsCoordinateFormatter.FormatPair, 0), '-20')
        self.assertEqual(QgsCoordinateFormatter.formatX(20.11111111111111111, QgsCoordinateFormatter.FormatPair, 3), '20.111')
        self.assertEqual(QgsCoordinateFormatter.formatX(20.11161111111111111, QgsCoordinateFormatter.FormatPair, 3), '20.112')
        self.assertEqual(QgsCoordinateFormatter.formatX(20, QgsCoordinateFormatter.FormatPair, 3), '20.000')
        self.assertEqual(QgsCoordinateFormatter.formatX(float('inf'), QgsCoordinateFormatter.FormatPair, 3), 'infinite')

    def testFormatYPair(self):
        """Test formatting y as pair"""

        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatPair, 0), '20')
        self.assertEqual(QgsCoordinateFormatter.formatY(-20, QgsCoordinateFormatter.FormatPair, 0), '-20')
        self.assertEqual(QgsCoordinateFormatter.formatY(20.11111111111111111, QgsCoordinateFormatter.FormatPair, 3), '20.111')
        self.assertEqual(QgsCoordinateFormatter.formatY(20.11161111111111111, QgsCoordinateFormatter.FormatPair, 3), '20.112')
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatPair, 3), '20.000')
        self.assertEqual(QgsCoordinateFormatter.formatY(float('inf'), QgsCoordinateFormatter.FormatPair, 3), 'infinite')

    def testAsPair(self):
        """Test formatting x/y as pair"""
        self.assertEqual(QgsCoordinateFormatter.asPair(20, 30, 0), '20,30')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, -30, 0), '20,-30')
        self.assertEqual(QgsCoordinateFormatter.asPair(20.111, 10.999, 0), '20,11')
        self.assertEqual(QgsCoordinateFormatter.asPair(20.111, 10.999, 2), '20.11,11.00')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, 10, 2), '20.00,10.00')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, -10, 2), '20.00,-10.00')

        self.assertEqual(QgsCoordinateFormatter.asPair(20, -10, 2, order=Qgis.CoordinateOrder.XY), '20.00,-10.00')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, -10, 2, order=Qgis.CoordinateOrder.YX), '-10.00,20.00')

    def testFormat(self):
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1, 30.2), QgsCoordinateFormatter.FormatPair, 0), '20,30')
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1, 30.2), QgsCoordinateFormatter.FormatPair, 1), '20.1,30.2')
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20, 30), QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 0), '20°0′0″E,30°0′0″N')

        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1, 30.2), QgsCoordinateFormatter.FormatPair, 1, order=Qgis.CoordinateOrder.XY), '20.1,30.2')
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1, 30.2), QgsCoordinateFormatter.FormatPair, 1,
                                                       order=Qgis.CoordinateOrder.YX), '30.2,20.1')
        self.assertEqual(
            QgsCoordinateFormatter.format(QgsPointXY(20, 30), QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 0, order=Qgis.CoordinateOrder.YX),
            '30°0′0″N,20°0′0″E')

    def testFormatXFormatDegreesMinutesSeconds(self):
        """Test formatting x as DMS"""

        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "80°0′0.00″E")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "80°0′0.0000″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "80°7′24.4444″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 0), "80°7′24″E")

        # check if longitudes > 180 or <-180 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatX(370, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "10°0′0.00″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(-370, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "10°0′0.00″W")
        self.assertEqual(QgsCoordinateFormatter.formatX(181, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "179°0′0.00″W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-181, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "179°0′0.00″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(359, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "1°0′0.00″W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-359, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "1°0′0.00″E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "0°0′0.00360″W")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "0°0′0.00360″E")

        # should be no directional suffixes for 180 degree longitudes
        self.assertEqual(QgsCoordinateFormatter.formatX(180, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "180°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "180°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "179°59′59.99640″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "180°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "179°59′59.99640″W")

        # test rounding does not create seconds >= 60
        self.assertEqual(QgsCoordinateFormatter.formatX(99.999999, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "100°0′0.00″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(89.999999, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "90°0′0.00″E")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "80°0′0.00″")

        # test 0 longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        # test near zero longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00360″")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, QgsCoordinateFormatter.FormatFlags()), "-0°0′0.00360″")

        # test with padding
        padding_and_suffix = QgsCoordinateFormatter.FormatFlags(QgsCoordinateFormatter.FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter.FlagDegreesUseStringSuffix)
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "80°00′00.00″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(85.44, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "85°26′24.00″E")
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, padding_and_suffix), "0°00′00.00360″W")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, padding_and_suffix), "0°00′00.00360″E")

    def testFormatYFormatDegreesMinutesSeconds(self):
        """Test formatting y as DMS"""

        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "20°0′0.00″N")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "20°0′0.0000″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "20°7′24.4444″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 0), "20°7′24″N")

        # check if latitudes > 90 or <-90 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatY(190, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "10°0′0.00″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(-190, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "10°0′0.00″S")
        self.assertEqual(QgsCoordinateFormatter.formatY(91, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "89°0′0.00″S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-91, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "89°0′0.00″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(179, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "1°0′0.00″S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-179, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "1°0′0.00″N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "0°0′0.00360″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5), "0°0′0.00360″S")

        # test rounding does not create seconds >= 60
        self.assertEqual(QgsCoordinateFormatter.formatY(89.999999, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), "90°0′0.00″N")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "20°0′0.00″")

        # test 0 latitude
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        # test near zero lat/long
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, QgsCoordinateFormatter.FormatFlags()), "0°0′0.00360″")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, QgsCoordinateFormatter.FormatFlags()), "-0°0′0.00360″")

        # test with padding
        padding_and_suffix = QgsCoordinateFormatter.FormatFlags(QgsCoordinateFormatter.FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter.FlagDegreesUseStringSuffix)
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "20°00′00.00″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(85.44, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "85°26′24.00″N")
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, padding_and_suffix), "0°00′00.00″")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, padding_and_suffix), "0°00′00.00360″S")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 5, padding_and_suffix), "0°00′00.00360″N")

    def testFormatXDegreesMinutes(self):
        """Test formatting x as DM"""

        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "80°0.00′E")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutes, 4), "80°0.0000′E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDegreesMinutes, 4), "80°7.4074′E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDegreesMinutes, 0), "80°7′E")

        # check if longitudes > 180 or <-180 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatX(370, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "10°0.00′E")
        self.assertEqual(QgsCoordinateFormatter.formatX(-370, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "10°0.00′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(181, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "179°0.00′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-181, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "179°0.00′E")
        self.assertEqual(QgsCoordinateFormatter.formatX(359, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "1°0.00′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-359, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "1°0.00′E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "0°0.00006′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "0°0.00006′E")

        # test rounding does not create minutes >= 60
        self.assertEqual(QgsCoordinateFormatter.formatX(99.999999, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "100°0.00′E")

        # should be no directional suffixes for 180 degree longitudes
        self.assertEqual(QgsCoordinateFormatter.formatX(180, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "180°0.00′")

        # should also be no directional suffix for 180 degree longitudes within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "180°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "180°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "179°59.99994′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "179°59.99994′E")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "80°0.00′")
        # test 0 longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        # test near zero longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, QgsCoordinateFormatter.FormatFlags()), "0°0.00006′")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, QgsCoordinateFormatter.FormatFlags()), "-0°0.00006′")

        # test with padding
        padding_and_suffix = QgsCoordinateFormatter.FormatFlags(QgsCoordinateFormatter.FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter.FlagDegreesUseStringSuffix)
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "80°00.00′E")
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, padding_and_suffix), "0°00.00006′W")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, padding_and_suffix), "0°00.00006′E")

    def testFormatYDegreesMinutes(self):
        """Test formatting y as DM"""

        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "20°0.00′N")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutes, 4), "20°0.0000′N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutes, 4), "20°7.4074′N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutes, 0), "20°7′N")

        # check if latitudes > 90 or <-90 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatY(190, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "10°0.00′N")
        self.assertEqual(QgsCoordinateFormatter.formatY(-190, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "10°0.00′S")
        self.assertEqual(QgsCoordinateFormatter.formatY(91, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "89°0.00′S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-91, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "89°0.00′N")
        self.assertEqual(QgsCoordinateFormatter.formatY(179, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "1°0.00′S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-179, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "1°0.00′N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "0°0.00006′S")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5), "0°0.00006′N")

        # test rounding does not create minutes >= 60
        self.assertEqual(QgsCoordinateFormatter.formatY(79.999999, QgsCoordinateFormatter.FormatDegreesMinutes, 2), "80°0.00′N")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "20°0.00′")
        # test 0 latitude
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        # test near zero latitude
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, QgsCoordinateFormatter.FormatFlags()), "0°0.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, QgsCoordinateFormatter.FormatFlags()), "0°0.00006′")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, QgsCoordinateFormatter.FormatFlags()), "-0°0.00006′")

        # test with padding
        padding_and_suffix = QgsCoordinateFormatter.FormatFlags(QgsCoordinateFormatter.FlagDegreesPadMinutesSeconds | QgsCoordinateFormatter.FlagDegreesUseStringSuffix)
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "20°00.00′N")
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 2, padding_and_suffix), "0°00.00′")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, padding_and_suffix), "0°00.00006′S")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDegreesMinutes, 5, padding_and_suffix), "0°00.00006′N")

    def testFormatXDegrees(self):
        """Test formatting x as decimal degrees"""

        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "80.00°E")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDecimalDegrees, 4), "80.0000°E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDecimalDegrees, 4), "80.1235°E")
        self.assertEqual(QgsCoordinateFormatter.formatX(80.12345678, QgsCoordinateFormatter.FormatDecimalDegrees, 0), "80°E")

        # check if longitudes > 180 or <-180 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatX(370, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "10.00°E")
        self.assertEqual(QgsCoordinateFormatter.formatX(-370, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "10.00°W")
        self.assertEqual(QgsCoordinateFormatter.formatX(181, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "179.00°W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-181, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "179.00°E")
        self.assertEqual(QgsCoordinateFormatter.formatX(359, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "1.00°W")
        self.assertEqual(QgsCoordinateFormatter.formatX(-359, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "1.00°E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 5), "0.00001°W")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 5), "0.00001°E")

        # should be no directional suffixes for 180 degree longitudes
        self.assertEqual(QgsCoordinateFormatter.formatX(180, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "180.00°")

        # should also be no directional suffix for 180 degree longitudes within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "180.00°")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "180.00°")
        self.assertEqual(QgsCoordinateFormatter.formatX(180.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 6), "179.999999°W")
        self.assertEqual(QgsCoordinateFormatter.formatX(179.999999, QgsCoordinateFormatter.FormatDecimalDegrees, 6), "179.999999°E")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatX(80, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "80.00°")
        # test 0 longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        # test near zero longitude
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatX(0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 6, QgsCoordinateFormatter.FormatFlags()), "0.000001°")
        self.assertEqual(QgsCoordinateFormatter.formatX(-0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 6, QgsCoordinateFormatter.FormatFlags()), "-0.000001°")

    def testFormatYDegrees(self):
        """Test formatting y as decimal degrees"""

        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "20.00°N")

        # check precision
        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDecimalDegrees, 4), "20.0000°N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDecimalDegrees, 4), "20.1235°N")
        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDecimalDegrees, 0), "20°N")

        # check if latitudes > 90 or <-90 wrap around
        self.assertEqual(QgsCoordinateFormatter.formatY(190, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "10.00°N")
        self.assertEqual(QgsCoordinateFormatter.formatY(-190, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "10.00°S")
        self.assertEqual(QgsCoordinateFormatter.formatY(91, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "89.00°S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-91, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "89.00°N")
        self.assertEqual(QgsCoordinateFormatter.formatY(179, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "1.00°S")
        self.assertEqual(QgsCoordinateFormatter.formatY(-179, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "1.00°N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 5), "0.00001°S")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.00001, QgsCoordinateFormatter.FormatDecimalDegrees, 5), "0.00001°N")

        # test without direction suffix
        self.assertEqual(QgsCoordinateFormatter.formatY(80, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "80.00°")
        # test 0 longitude
        self.assertEqual(QgsCoordinateFormatter.formatY(0, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        # test near zero latitude
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 2, QgsCoordinateFormatter.FormatFlags()), "0.00°")
        self.assertEqual(QgsCoordinateFormatter.formatY(0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 6, QgsCoordinateFormatter.FormatFlags()), "0.000001°")
        self.assertEqual(QgsCoordinateFormatter.formatY(-0.000001, QgsCoordinateFormatter.FormatDecimalDegrees, 6, QgsCoordinateFormatter.FormatFlags()), "-0.000001°")

    def testFormatLocale(self):
        """Test formatting with locales that use comma as decimal separator"""

        QLocale.setDefault(QLocale(QLocale.Italian))

        self.assertEqual(QgsCoordinateFormatter.formatY(20, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "20,00°N")
        self.assertEqual(QgsCoordinateFormatter.formatX(20, QgsCoordinateFormatter.FormatDecimalDegrees, 2), "20,00°E")

        self.assertEqual(QgsCoordinateFormatter.formatY(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "20°7′24,4444″N")
        self.assertEqual(QgsCoordinateFormatter.formatX(20.12345678, QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 4), "20°7′24,4444″E")

        # formatting x/y as pair
        self.assertEqual(QgsCoordinateFormatter.asPair(20, 30, 0), '20 30')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, -30, 0), '20 -30')
        self.assertEqual(QgsCoordinateFormatter.asPair(20.111, 10.999, 0), '20 11')
        self.assertEqual(QgsCoordinateFormatter.asPair(20.111, 10.999, 2), '20,11 11,00')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, 10, 2), '20,00 10,00')
        self.assertEqual(QgsCoordinateFormatter.asPair(20, -10, 2), '20,00 -10,00')

        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1111, 30.2111), QgsCoordinateFormatter.FormatPair, 2), '20,11 30,21')
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.111, 30.211), QgsCoordinateFormatter.FormatPair, 2), '20,11 30,21')
        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.111, 30.211), QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2), '20°6′39,60″E 30°12′39,60″N')

        self.assertEqual(QgsCoordinateFormatter.format(QgsPointXY(20.1111, 30.2111), QgsCoordinateFormatter.FormatDegreesMinutesSeconds, 2, QgsCoordinateFormatter.FlagDegreesUseStringSuffix | QgsCoordinateFormatter.FlagDegreesPadMinutesSeconds), '20°06′39,96″E 30°12′39,96″N')

    def testSeparator(self):
        """Test X/Y separator with different locales"""

        self.assertEqual(QgsCoordinateFormatter.separator(), ',')

        QLocale.setDefault(QLocale(QLocale.Italian))
        self.assertEqual(QgsCoordinateFormatter.separator(), ' ')


if __name__ == "__main__":
    unittest.main()
