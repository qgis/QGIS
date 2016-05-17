# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsStringStatisticalSummary.

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

from qgis.core import (QgsStringStatisticalSummary
                       )
from qgis.testing import unittest


class PyQgsStringStatisticalSummary(unittest.TestCase):

    def testStats(self):
        # we test twice, once with values added as a list and once using values
        # added one-at-a-time
        s = QgsStringStatisticalSummary()
        self.assertEqual(s.statistics(), QgsStringStatisticalSummary.All)
        strings = ['cc', 'aaaa', 'bbbbbbbb', 'aaaa', 'eeee', '', 'eeee', '', 'dddd']
        s.calculate(strings)
        s2 = QgsStringStatisticalSummary()
        for string in strings:
            s2.addString(string)
        s2.finalize()
        self.assertEqual(s.count(), 9)
        self.assertEqual(s2.count(), 9)
        self.assertEqual(s.countDistinct(), 6)
        self.assertEqual(s2.countDistinct(), 6)
        self.assertEqual(set(s.distinctValues()), set(['cc', 'aaaa', 'bbbbbbbb', 'eeee', 'dddd', '']))
        self.assertEqual(s2.distinctValues(), s.distinctValues())
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s2.countMissing(), 2)
        self.assertEqual(s.min(), 'aaaa')
        self.assertEqual(s2.min(), 'aaaa')
        self.assertEqual(s.max(), 'eeee')
        self.assertEqual(s2.max(), 'eeee')
        self.assertEqual(s.minLength(), 0)
        self.assertEqual(s2.minLength(), 0)
        self.assertEqual(s.maxLength(), 8)
        self.assertEqual(s2.maxLength(), 8)

        #extra check for minLength without empty strings
        s.calculate(['1111111', '111', '11111'])
        self.assertEqual(s.minLength(), 3)

    def testIndividualStats(self):
        # tests calculation of statistics one at a time, to make sure statistic calculations are not
        # dependent on each other
        tests = [{'stat': QgsStringStatisticalSummary.Count, 'expected': 9},
                 {'stat': QgsStringStatisticalSummary.CountDistinct, 'expected': 6},
                 {'stat': QgsStringStatisticalSummary.CountMissing, 'expected': 2},
                 {'stat': QgsStringStatisticalSummary.Min, 'expected': 'aaaa'},
                 {'stat': QgsStringStatisticalSummary.Max, 'expected': 'eeee'},
                 {'stat': QgsStringStatisticalSummary.MinimumLength, 'expected': 0},
                 {'stat': QgsStringStatisticalSummary.MaximumLength, 'expected': 8},
                 ]

        s = QgsStringStatisticalSummary()
        s3 = QgsStringStatisticalSummary()
        for t in tests:
            # test constructor
            s2 = QgsStringStatisticalSummary(t['stat'])
            self.assertEqual(s2.statistics(), t['stat'])

            s.setStatistics(t['stat'])
            s3.setStatistics(t['stat'])
            self.assertEqual(s.statistics(), t['stat'])

            strings = ['cc', 'aaaa', 'bbbbbbbb', 'aaaa', 'eeee', '', 'eeee', '', 'dddd']
            s.calculate(strings)
            s3.reset()
            for string in strings:
                s3.addString(string)
            s3.finalize()

            self.assertEqual(s.statistic(t['stat']), t['expected'])
            self.assertEqual(s3.statistic(t['stat']), t['expected'])

            # display name
            self.assertTrue(len(QgsStringStatisticalSummary.displayName(t['stat'])) > 0)

    def testVariantStats(self):
        s = QgsStringStatisticalSummary()
        self.assertEqual(s.statistics(), QgsStringStatisticalSummary.All)
        s.calculateFromVariants(['cc', 5, 'bbbb', 'aaaa', 'eeee', 6, 9, '9', ''])
        self.assertEqual(s.count(), 6)
        self.assertEqual(set(s.distinctValues()), set(['cc', 'aaaa', 'bbbb', 'eeee', '', '9']))
        self.assertEqual(s.countMissing(), 1)
        self.assertEqual(s.min(), '9')
        self.assertEqual(s.max(), 'eeee')

if __name__ == '__main__':
    unittest.main()
