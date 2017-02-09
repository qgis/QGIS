# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsInterval.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/05/2016'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsInterval
from qgis.testing import unittest
from qgis.PyQt.QtCore import QDateTime, QDate, QTime


class TestQgsInterval(unittest.TestCase):

    def testIntervalConstructor(self):
        """ Test QgsInterval constructor """

        # invalid interval
        i = QgsInterval()
        self.assertFalse(i.isValid())
        i.setValid(True)
        self.assertTrue(i.isValid())
        i.setValid(False)
        self.assertFalse(i.isValid())
        # setting a duration should make interval valid
        i.setSeconds(5)
        self.assertTrue(i.isValid())

        # constructor with duration
        i = QgsInterval(56)
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 56)

    def testSettersGetters(self):
        # setters and getters
        i = QgsInterval()
        i.setSeconds(60)
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 60.0)

        i = QgsInterval()
        i.setMinutes(10)
        self.assertTrue(i.isValid())
        self.assertEqual(i.minutes(), 10.0)

        i = QgsInterval()
        i.setHours(5)
        self.assertTrue(i.isValid())
        self.assertEqual(i.hours(), 5.0)

        i = QgsInterval()
        i.setDays(70)
        self.assertTrue(i.isValid())
        self.assertEqual(i.days(), 70.0)

        i = QgsInterval()
        i.setWeeks(9)
        self.assertTrue(i.isValid())
        self.assertEqual(i.weeks(), 9.0)

        i = QgsInterval()
        i.setMonths(4)
        self.assertTrue(i.isValid())
        self.assertEqual(i.months(), 4.0)

        i = QgsInterval()
        i.setYears(8)
        self.assertTrue(i.isValid())
        self.assertEqual(i.years(), 8.0)

    def testConversions(self):
        i = QgsInterval()
        # conversions
        i.setYears(1)
        self.assertEqual(round(i.months()), 12)
        self.assertEqual(round(i.weeks()), 52)
        self.assertEqual(round(i.days()), 365)
        i.setDays(5)
        self.assertEqual(i.hours(), 5 * 24)
        self.assertEqual(i.minutes(), 5 * 24 * 60)
        self.assertEqual(i.seconds(), 5 * 24 * 60 * 60)

    def testEquality(self):
        i1 = QgsInterval()
        i2 = QgsInterval()
        self.assertEqual(i1, i2)
        i1 = QgsInterval(5)
        self.assertNotEqual(i1, i2)
        i1.setValid(False)
        i2 = QgsInterval(5)
        self.assertNotEqual(i1, i2)
        i1 = QgsInterval(5)
        self.assertEqual(i1, i2)
        i1.setSeconds(6)
        self.assertNotEqual(i1, i2)

    def testFromString(self):
        i = QgsInterval.fromString('1 Year 1 Month 1 Week 1 Hour 1 Minute')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('1 Year, 1 Month, 1 Week, 1 Hour, 1 Minute')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('1 Year; 1 Month; 1 Week; 1 Hour; 1 Minute')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('1 Year. 1 Month. 1 Week. 1 Hour. 1 Minute.')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('2 Years')
        self.assertTrue(i.isValid())
        self.assertEqual(i.years(), 2)
        i = QgsInterval.fromString('30 month')
        self.assertTrue(i.isValid())
        self.assertEqual(i.months(), 30)
        i = QgsInterval.fromString(' 40 MONTHS ')
        self.assertTrue(i.isValid())
        self.assertEqual(i.months(), 40)
        i = QgsInterval.fromString('2.5 weeks')
        self.assertTrue(i.isValid())
        self.assertEqual(i.weeks(), 2.5)
        i = QgsInterval.fromString('2.5 WEEK')
        self.assertTrue(i.isValid())
        self.assertEqual(i.weeks(), 2.5)
        i = QgsInterval.fromString('1 Day')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 24 * 60 * 60)
        i = QgsInterval.fromString('2  dAys')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 48 * 60 * 60)
        i = QgsInterval.fromString('1 hours')
        self.assertTrue(i.isValid())
        self.assertEqual(i.hours(), 1)
        i = QgsInterval.fromString('1.7 HoURS ')
        self.assertTrue(i.isValid())
        self.assertEqual(i.hours(), 1.7)
        i = QgsInterval.fromString('2 minutes')
        self.assertTrue(i.isValid())
        self.assertEqual(i.minutes(), 2)
        i = QgsInterval.fromString('123 MiNuTe ')
        self.assertTrue(i.isValid())
        self.assertEqual(i.minutes(), 123)
        i = QgsInterval.fromString('5 Seconds')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 5)
        i = QgsInterval.fromString('5 second')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 5)
        i = QgsInterval.fromString('bad')
        self.assertFalse(i.isValid())

if __name__ == '__main__':
    unittest.main()
