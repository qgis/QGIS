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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsDateTimeStatisticalSummary,
                       QgsInterval
                       )
from qgis.PyQt.QtCore import QDateTime, QDate, QTime
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
        self.assertEqual(s.count(), 9)
        self.assertEqual(s2.count(), 9)
        self.assertEqual(s.countDistinct(), 6)
        self.assertEqual(s2.countDistinct(), 6)
        self.assertEqual(set(s.distinctValues()),
                         set([QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                              QDateTime(QDate(2011, 1, 5), QTime(15, 3, 1)),
                              QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                              QDateTime(),
                              QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                              QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54))]))
        self.assertEqual(s2.distinctValues(), s.distinctValues())
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s2.countMissing(), 2)
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s2.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s2.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s.range(), QgsInterval(693871147))
        self.assertEqual(s2.range(), QgsInterval(693871147))

    def testIndividualStats(self):
        # tests calculation of statistics one at a time, to make sure statistic calculations are not
        # dependent on each other
        tests = [{'stat': QgsDateTimeStatisticalSummary.Count, 'expected': 9},
                 {'stat': QgsDateTimeStatisticalSummary.CountDistinct, 'expected': 6},
                 {'stat': QgsDateTimeStatisticalSummary.CountMissing, 'expected': 2},
                 {'stat': QgsDateTimeStatisticalSummary.Min, 'expected': QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54))},
                 {'stat': QgsDateTimeStatisticalSummary.Max, 'expected': QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1))},
                 {'stat': QgsDateTimeStatisticalSummary.Range, 'expected': QgsInterval(693871147)},
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
        self.assertEqual(s.count(), 7)
        self.assertEqual(set(s.distinctValues()), set([QDateTime(QDate(2015, 3, 4), QTime(11, 10, 54)),
                                                       QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)),
                                                       QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)),
                                                       QDateTime(QDate(2011, 1, 5), QTime(11, 10, 54)),
                                                       QDateTime()]))
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime(1, 10, 54)))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime(23, 10, 1)))
        self.assertEqual(s.range(), QgsInterval(693871147))

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
        self.assertEqual(s.count(), 7)
        self.assertEqual(set(s.distinctValues()), set([
            QDateTime(QDate(2015, 3, 4), QTime()),
            QDateTime(QDate(2019, 12, 28), QTime()),
            QDateTime(QDate(1998, 1, 2), QTime()),
            QDateTime(),
            QDateTime(QDate(2011, 1, 5), QTime())]))
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s.min(), QDateTime(QDate(1998, 1, 2), QTime()))
        self.assertEqual(s.max(), QDateTime(QDate(2019, 12, 28), QTime()))
        self.assertEqual(s.range(), QgsInterval(693792000))


if __name__ == '__main__':
    unittest.main()
