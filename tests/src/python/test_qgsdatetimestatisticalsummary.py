# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDateTimeStatisticalSummary.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '07/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsDateTimeStatisticalSummary,
                       QgsInterval,
                       NULL
                       )
from qgis.PyQt.QtCore import QDateTime, QDate, QTime, Qt
from qgis.testing import unittest


class PyQgsDateTimeStatisticalSummary(unittest.TestCase):
    def testStats(self):
        # we test twice, once with values added as a list and once using values
        # added one-at-a-time

        dates = [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                 QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
                 QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                 QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                 QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                 QDateTime(),
                 QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                 QDateTime(),
                 QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]
        s = QgsDateTimeStatisticalSummary()
        self.assertEqual(s.statistics(), QgsDateTimeStatisticalSummary.All)
        s.calculate(dates)
        s2 = QgsDateTimeStatisticalSummary()
        for d in dates:
            s2.addValue(d)
        s2.finalize()
        self.assertEqual(s.count(), 7)
        self.assertEqual(s2.count(), 7)
        self.assertEqual(s.countDistinct(), 5)
        self.assertEqual(s2.countDistinct(), 5)
        self.assertEqual(set(s.distinctValues()),
                         set([QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                              QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
                              QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                              QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                              QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]))
        self.assertEqual(s2.distinctValues(), s.distinctValues())
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s2.countMissing(), 2)
        self.assertEqual(s.mean(), QDateTime(QDate(2012, 3, 21), QTime(5, 9, 38, 858)))
        self.assertEqual(s2.mean(), QDateTime(QDate(2012, 3, 21), QTime(5, 9, 38, 858)))
        self.assertEqual(s.median(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s2.median(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.stDev().seconds(), 203658409.3682543)
        self.assertEqual(s2.stDev().seconds(), 203658409.3682543)
        self.assertEqual(s.sampleStDev().seconds(), 219976223.69430903)
        self.assertEqual(s2.sampleStDev().seconds(), 219976223.69430903)
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s2.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s2.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s.range(), QgsInterval(693871147))
        self.assertEqual(s2.range(), QgsInterval(693871147))
        self.assertEqual(s.minority(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s2.minority(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.majority(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s2.majority(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.firstQuartile(), QDateTime(QDate(2011, 1, 5), QTime(13, 6, 57, 500)))
        self.assertEqual(s2.firstQuartile(), QDateTime(QDate(2011, 1, 5), QTime(13, 6, 57, 500)))
        self.assertEqual(s.thirdQuartile(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s2.thirdQuartile(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.interQuartileRange().seconds(), 131234636.5)
        self.assertEqual(s2.interQuartileRange().seconds(), 131234636.5)
        self.assertEqual(s.first(), dates[0])
        self.assertEqual(s2.first(), dates[0])
        self.assertEqual(s.last(), dates[-1])
        self.assertEqual(s2.last(), dates[-1])
        self.assertEqual(s.mode(), [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))])
        self.assertEqual(s2.mode(), [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))])

    def testIndividualStats(self):
        # tests calculation of statistics one at a time, to make sure statistic calculations are not
        # dependent on each other
        tests = [{'stat': QgsDateTimeStatisticalSummary.Count, 'expected': 7},
                 {'stat': QgsDateTimeStatisticalSummary.CountDistinct, 'expected': 5},
                 {'stat': QgsDateTimeStatisticalSummary.CountMissing, 'expected': 2},
                 {'stat': QgsDateTimeStatisticalSummary.Mean, 'expected': QDateTime(QDate(2012, 3, 21), QTime(5, 9, 38, 858))},
                 {'stat': QgsDateTimeStatisticalSummary.Median, 'expected': QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.StDev, 'expected': QgsInterval(203658409.3682543)},
                 {'stat': QgsDateTimeStatisticalSummary.StDevSample, 'expected': QgsInterval(219976223.69430903)},
                 {'stat': QgsDateTimeStatisticalSummary.Min, 'expected': QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.Max, 'expected': QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1))},
                 {'stat': QgsDateTimeStatisticalSummary.Range, 'expected': QgsInterval(693871147)},
                 {'stat': QgsDateTimeStatisticalSummary.Minority, 'expected': QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.Majority, 'expected': QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.FirstQuartile, 'expected': QDateTime(QDate(2011, 1, 5), QTime(13, 6, 57, 500))},
                 {'stat': QgsDateTimeStatisticalSummary.ThirdQuartile, 'expected': QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.InterQuartileRange, 'expected': QgsInterval(131234636.5)},
                 {'stat': QgsDateTimeStatisticalSummary.First, 'expected': QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.Last, 'expected': QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.Mode, 'expected': [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))]},
                 ]

        # we test twice, once with values added as a list and once using values
        # added one-at-a-time
        s = QgsDateTimeStatisticalSummary()
        s3 = QgsDateTimeStatisticalSummary()
        for t in tests:
            # test constructor
            s2 = QgsDateTimeStatisticalSummary(t['stat'])

            self.assertEqual(s2.statistics(), t['stat'])

            s.setStatistics(t['stat'])
            self.assertEqual(s.statistics(), t['stat'])
            s3.setStatistics(t['stat'])
            dates = [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                     QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
                     QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                     QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                     QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                     QDateTime(),
                     QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                     QDateTime(),
                     QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]
            s.calculate(dates)
            s3.reset()
            for d in dates:
                s3.addValue(d)
            s3.finalize()

            self.assertEqual(s.statistic(t['stat']), t['expected'])
            self.assertEqual(s3.statistic(t['stat']), t['expected'])

            # display name
            self.assertTrue(len(QgsDateTimeStatisticalSummary.displayName(t['stat'])) > 0)

    def testVariantStats(self):
        """ test with non-datetime values """
        s = QgsDateTimeStatisticalSummary()
        self.assertEqual(s.statistics(), QgsDateTimeStatisticalSummary.All)
        s.calculate([QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                     'asdasd',
                     QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                     34,
                     QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                     QDateTime(),
                     QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                     QDateTime(),
                     QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))])

        self.assertEqual(s.count(), 5)
        self.assertEqual(set(s.distinctValues()), set([QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                                                       QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                                                       QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                                                       QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]))
        self.assertEqual(s.countMissing(), 4)
        self.assertEqual(s.mean(), QDateTime(QDate(2011, 11, 14), QTime(16, 22, 43, 400)))
        self.assertEqual(s.median(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.stDev().seconds(), 236475001.99310222)
        self.assertEqual(s.sampleStDev().seconds(), 264387089.71798742)
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s.range().seconds(), 693871147)
        self.assertEqual(s.minority(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.majority(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.firstQuartile(), QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54)))
        self.assertEqual(s.thirdQuartile(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.interQuartileRange().seconds(), 131241600.0)
        self.assertEqual(s.first(), QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)))
        self.assertEqual(s.last(), QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54)))
        self.assertEqual(s.mode(), [QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54))])

    def testDates(self):
        """ test with date values """
        s = QgsDateTimeStatisticalSummary()
        self.assertEqual(s.statistics(), QgsDateTimeStatisticalSummary.All)
        s.calculate([QDate(2015, 3, 4),
                     QDate(2015, 3, 4),
                     QDate(2019, 12, 28),
                     QDate(),
                     QDate(1998, 1, 2),
                     QDate(),
                     QDate(2011, 1, 5)])
        self.assertEqual(s.count(), 5)
        self.assertEqual(set(s.distinctValues()), set([
            QDateTime(QDate(2015, 3, 4), QTime()),
            QDateTime(QDate(2019, 12, 28), QTime()),
            QDateTime(QDate(1998, 1, 2), QTime()),
            QDateTime(QDate(2011, 1, 5), QTime())]))
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s.mean(), QDateTime(QDate(2011, 11, 14), QTime(4, 48)))
        self.assertEqual(s.median(), QDateTime(QDate(2015, 3, 4), QTime(0, 0)))
        self.assertEqual(s.stDev(), QgsInterval(236452327.04639977))
        self.assertEqual(s.sampleStDev(), QgsInterval(264361738.356881))
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime()))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime()))
        self.assertEqual(s.range(), QgsInterval(693792000))
        self.assertEqual(s.minority(), QDateTime(QDate(1998, 1, 2), QTime()))
        self.assertEqual(s.majority(), QDateTime(QDate(2015, 3, 4), QTime()))
        self.assertEqual(s.firstQuartile(), QDateTime(QDate(2011, 1, 5), QTime()))
        self.assertEqual(s.thirdQuartile(), QDateTime(QDate(2015, 3, 4), QTime()))
        self.assertEqual(s.interQuartileRange(), QgsInterval(131241600.0))
        self.assertEqual(s.first(), QDateTime(QDate(2015, 3, 4), QTime()))
        self.assertEqual(s.last(), QDateTime(QDate(2011, 1, 5), QTime()))
        self.assertEqual(s.mode(), [QDateTime(QDate(2015, 3, 4), QTime())])

    def testTimes(self):
        """ test with time values """
        s = QgsDateTimeStatisticalSummary()
        self.assertEqual(s.statistics(), QgsDateTimeStatisticalSummary.All)
        s.calculate([QTime(11, 3, 4),
                     QTime(15, 3, 4),
                     QTime(19, 12, 28),
                     QTime(),
                     QTime(8, 1, 2),
                     QTime(),
                     QTime(19, 12, 28)])
        self.assertEqual(s.count(), 5)
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Count), 5)
        self.assertEqual(s.countDistinct(), 4)
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.CountDistinct), 4)
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.CountMissing), 2)
        self.assertEqual(s.mean(), QDateTime(QDate.fromJulianDay(0), QTime(14, 30, 25, 200)))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Mean), QTime(14, 30, 25, 200))
        self.assertEqual(s.median(), QDateTime(QDate.fromJulianDay(0), QTime(15, 3, 4, 0)))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Median), QTime(15, 3, 4, 0))
        self.assertEqual(s.stDev(), QgsInterval(15982.178700039616))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.StDev), QgsInterval(15982.178700039616))
        self.assertEqual(s.sampleStDev(), QgsInterval(17868.6190009189))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.StDevSample), QgsInterval(17868.6190009189))
        self.assertEqual(s.min().time(), QTime(8, 1, 2))
        self.assertEqual(s.max().time(), QTime(19, 12, 28))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Min), QTime(8, 1, 2))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Max), QTime(19, 12, 28))
        self.assertEqual(s.range(), QgsInterval(40286))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Range), QgsInterval(40286))
        self.assertEqual(s.minority().time(), QTime(8, 1, 2))
        self.assertEqual(s.majority().time(), QTime(19, 12, 28))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Minority), QTime(8, 1, 2))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Majority), QTime(19, 12, 28))
        self.assertEqual(s.firstQuartile().time(), QTime(11, 3, 4))
        self.assertEqual(s.thirdQuartile().time(), QTime(19, 12, 28))
        self.assertEqual(s.interQuartileRange(), QgsInterval(29364.0))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.FirstQuartile), QTime(11, 3, 4))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.ThirdQuartile), QTime(19, 12, 28))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.InterQuartileRange), QgsInterval(29364.0))
        self.assertEqual(s.first().time(), QTime(11, 3, 4))
        self.assertEqual(s.last().time(), QTime(19, 12, 28))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.First), QTime(11, 3, 4))
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Last), QTime(19, 12, 28))
        self.assertEqual(s.mode(), [QDateTime(QDate.fromJulianDay(0), QTime(19, 12, 28))])
        self.assertEqual(s.statistic(QgsDateTimeStatisticalSummary.Mode), [QTime(19, 12, 28)])

    def testMissing(self):
        s = QgsDateTimeStatisticalSummary()
        s.calculate([NULL, 'not a date'])
        self.assertEqual(s.count(), 0)
        self.assertEqual(s.countMissing(), 2)


if __name__ == '__main__':
    unittest.main()
