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

import qgis  # NOQA

from qgis.core import QgsInterval, QgsUnitTypes
from qgis.testing import unittest


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

        # constructor with unit type
        i = QgsInterval(56, QgsUnitTypes.TemporalMilliseconds)
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 0.056)

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

        # using original units

        i1 = QgsInterval(1, QgsUnitTypes.TemporalMonths)
        self.assertEqual(i1, QgsInterval(1, QgsUnitTypes.TemporalMonths))
        i2 = QgsInterval(720, QgsUnitTypes.TemporalHours)
        self.assertEqual(i2, QgsInterval(720, QgsUnitTypes.TemporalHours))
        # these QgsInterval would be equal if we test on the approximated seconds value alone, but should be treated as not equal
        # as their original units differ and we don't want the odd situation that the QgsInterval objects report equality
        # but i1.months() != i2.months()!!
        self.assertEqual(i1.seconds(), i2.seconds())
        self.assertNotEqual(i1, i2)

        # these should be treated as equal - they have unknown original units, so we are just comparing
        # their computed seconds values
        i1 = QgsInterval(0, 0, 0, 1, 0, 0, 1)
        i2 = QgsInterval(0, 0, 0, 0, 24, 0, 1)
        self.assertEqual(i1, i2)

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
        i = QgsInterval.fromString('1 Year. 1 Month. 1 Week. 01:01:00 ')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('1 Year. 1 Mon. 1 Week. 01:01:00 ')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 34758060)
        i = QgsInterval.fromString('2 Years')
        self.assertTrue(i.isValid())
        self.assertEqual(i.years(), 2)
        i = QgsInterval.fromString('20000 Years')
        self.assertTrue(i.isValid())
        self.assertEqual(i.years(), 20000)
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
        i = QgsInterval.fromString('101.5 Days')
        self.assertTrue(i.isValid())
        self.assertEqual(i.seconds(), 101.5 * 24 * 60 * 60)
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

    def testFromUnits(self):
        i = QgsInterval(2, 0, 0, 0, 0, 0, 0)
        self.assertEqual(i.seconds(), 63115200.0)
        i = QgsInterval(0, 2, 0, 0, 0, 0, 0)
        self.assertEqual(i.seconds(), 5184000.0)
        i = QgsInterval(0, 0, 2, 0, 0, 0, 0)
        self.assertEqual(i.seconds(), 1209600.0)
        i = QgsInterval(0, 0, 0, 2, 0, 0, 0)
        self.assertEqual(i.seconds(), 172800.0)
        i = QgsInterval(0, 0, 0, 0, 2, 0, 0)
        self.assertEqual(i.seconds(), 7200.0)
        i = QgsInterval(0, 0, 0, 0, 0, 2, 0)
        self.assertEqual(i.seconds(), 120.0)
        i = QgsInterval(0, 0, 0, 0, 0, 0, 2)
        self.assertEqual(i.seconds(), 2)
        i = QgsInterval(1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 2)
        self.assertEqual(i.seconds(), 56342192.0)

    def testIntervalDurationUnitSetting(self):
        i = QgsInterval()
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalUnknownUnit)
        i = QgsInterval(2, QgsUnitTypes.TemporalMilliseconds)
        self.assertEqual(i.originalDuration(), 2.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMilliseconds)
        i = QgsInterval(34.56, QgsUnitTypes.TemporalSeconds)
        self.assertEqual(i.originalDuration(), 34.56)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)
        i = QgsInterval(10, QgsUnitTypes.TemporalMinutes)
        self.assertEqual(i.originalDuration(), 10.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMinutes)
        i = QgsInterval(10.012, QgsUnitTypes.TemporalHours)
        self.assertEqual(i.originalDuration(), 10.012)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalHours)
        i = QgsInterval(10.32, QgsUnitTypes.TemporalDays)
        self.assertEqual(i.originalDuration(), 10.32)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalDays)
        i = QgsInterval(100, QgsUnitTypes.TemporalWeeks)
        self.assertEqual(i.originalDuration(), 100.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalWeeks)
        i = QgsInterval(1000, QgsUnitTypes.TemporalMonths)
        self.assertEqual(i.originalDuration(), 1000.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMonths)
        i = QgsInterval(500.005, QgsUnitTypes.TemporalYears)
        self.assertEqual(i.originalDuration(), 500.005)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalYears)
        i = QgsInterval(0.02, QgsUnitTypes.TemporalDecades)
        self.assertEqual(i.originalDuration(), 0.02)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalDecades)
        i = QgsInterval(0.2, QgsUnitTypes.TemporalCenturies)
        self.assertEqual(i.originalDuration(), 0.2)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalCenturies)

        i = QgsInterval(10)
        self.assertEqual(i.originalDuration(), 10.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)
        i = QgsInterval(0)
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)

        i = QgsInterval(1, 0, 0, 0, 0, 0, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalYears)
        i = QgsInterval(0, 1, 0, 0, 0, 0, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMonths)
        i = QgsInterval(0, 0, 1, 0, 0, 0, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalWeeks)
        i = QgsInterval(0, 0, 0, 1, 0, 0, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalDays)
        i = QgsInterval(0, 0, 0, 0, 1, 0, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalHours)
        i = QgsInterval(0, 0, 0, 0, 0, 1, 0)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMinutes)
        i = QgsInterval(0, 0, 0, 0, 0, 0, 1)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)
        i = QgsInterval(0, 0, 0, 0, 0, 0, 0)

        # we may as well treat this the same as if 0 seconds was explicitly specified!
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)

        i = QgsInterval(0, 0, 0, 0, 0, 1, 1)
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalUnknownUnit)

    def testSettersDurationUnitChange(self):
        i = QgsInterval()
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalUnknownUnit)
        i.setYears(1)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalYears)
        i.setMonths(3)
        self.assertEqual(i.originalDuration(), 3.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMonths)
        i.setWeeks(1)
        self.assertEqual(i.originalDuration(), 1.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalWeeks)
        i.setDays(4)
        self.assertEqual(i.originalDuration(), 4.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalDays)
        i.setHours(22.3)
        self.assertEqual(i.originalDuration(), 22.3)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalHours)
        i.setMinutes(11)
        self.assertEqual(i.originalDuration(), 11.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalMinutes)
        i.setSeconds(100)
        self.assertEqual(i.originalDuration(), 100.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalSeconds)

    def testGettersDurationUnitChange(self):
        i = QgsInterval()
        self.assertEqual(i.originalDuration(), 0.0)
        self.assertEqual(i.originalUnit(), QgsUnitTypes.TemporalUnknownUnit)
        i.setYears(1)
        self.assertEqual(i.years(), 1.0)
        i.setMonths(3)
        self.assertEqual(i.months(), 3.0)
        i.setWeeks(1)
        self.assertEqual(i.weeks(), 1.0)
        i.setDays(4)
        self.assertEqual(i.days(), 4.0)
        i.setHours(22.3)
        self.assertEqual(i.hours(), 22.3)
        i.setMinutes(11)
        self.assertEqual(i.minutes(), 11.0)
        i.setSeconds(100)
        self.assertEqual(i.seconds(), 100.0)

        i = QgsInterval(0, 0, 0, 0, 0, 1, 30)
        self.assertEqual(i.minutes(), 1.5)
        i.setDays(45)
        self.assertEqual(i.months(), 1.5)


if __name__ == '__main__':
    unittest.main()
