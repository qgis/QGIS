"""QGIS Unit tests for QgsStringStatisticalSummary.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "07/05/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.core import NULL, QgsStringStatisticalSummary
from qgis.testing import unittest


class PyQgsStringStatisticalSummary(unittest.TestCase):

    def testStats(self):
        # we test twice, once with values added as a list and once using values
        # added one-at-a-time
        s = QgsStringStatisticalSummary()
        self.assertEqual(s.statistics(), QgsStringStatisticalSummary.Statistic.All)
        strings = [
            "cc",
            "aaaa",
            "bbbbbbbb",
            "aaaa",
            "eeee",
            "",
            "eeee",
            "aaaa",
            None,
            "dddd",
        ]
        s.calculate(strings)
        s2 = QgsStringStatisticalSummary()
        for string in strings:
            s2.addString(string)
        s2.finalize()
        self.assertEqual(s.count(), 10)
        self.assertEqual(s2.count(), 10)
        self.assertEqual(s.countDistinct(), 6)
        self.assertEqual(s2.countDistinct(), 6)
        self.assertEqual(
            set(s.distinctValues()), {"cc", "aaaa", "bbbbbbbb", "eeee", "dddd", ""}
        )
        self.assertEqual(s2.distinctValues(), s.distinctValues())
        self.assertEqual(s.countMissing(), 2)
        self.assertEqual(s2.countMissing(), 2)
        self.assertEqual(s.min(), "aaaa")
        self.assertEqual(s2.min(), "aaaa")
        self.assertEqual(s.max(), "eeee")
        self.assertEqual(s2.max(), "eeee")
        self.assertEqual(s.minLength(), 0)
        self.assertEqual(s2.minLength(), 0)
        self.assertEqual(s.maxLength(), 8)
        self.assertEqual(s2.maxLength(), 8)
        self.assertEqual(s.meanLength(), 3.4)
        self.assertEqual(s2.meanLength(), 3.4)
        self.assertEqual(s.minority(), "bbbbbbbb")
        self.assertEqual(s2.minority(), "bbbbbbbb")
        self.assertEqual(s.majority(), "aaaa")
        self.assertEqual(s2.majority(), "aaaa")

        # extra check for minLength without empty strings
        s.calculate(["1111111", "111", "11111"])
        self.assertEqual(s.minLength(), 3)

    def testIndividualStats(self):
        # tests calculation of statistics one at a time, to make sure statistic calculations are not
        # dependent on each other
        tests = [
            {"stat": QgsStringStatisticalSummary.Statistic.Count, "expected": 10},
            {
                "stat": QgsStringStatisticalSummary.Statistic.CountDistinct,
                "expected": 6,
            },
            {"stat": QgsStringStatisticalSummary.Statistic.CountMissing, "expected": 2},
            {"stat": QgsStringStatisticalSummary.Statistic.Min, "expected": "aaaa"},
            {"stat": QgsStringStatisticalSummary.Statistic.Max, "expected": "eeee"},
            {
                "stat": QgsStringStatisticalSummary.Statistic.MinimumLength,
                "expected": 0,
            },
            {
                "stat": QgsStringStatisticalSummary.Statistic.MaximumLength,
                "expected": 8,
            },
            {"stat": QgsStringStatisticalSummary.Statistic.MeanLength, "expected": 3.4},
            {
                "stat": QgsStringStatisticalSummary.Statistic.Minority,
                "expected": "bbbbbbbb",
            },
            {
                "stat": QgsStringStatisticalSummary.Statistic.Majority,
                "expected": "aaaa",
            },
        ]

        s = QgsStringStatisticalSummary()
        s3 = QgsStringStatisticalSummary()
        for t in tests:
            # test constructor
            s2 = QgsStringStatisticalSummary(t["stat"])
            self.assertEqual(s2.statistics(), t["stat"])

            s.setStatistics(t["stat"])
            s3.setStatistics(t["stat"])
            self.assertEqual(s.statistics(), t["stat"])

            strings = [
                "cc",
                "aaaa",
                "bbbbbbbb",
                "aaaa",
                "eeee",
                "",
                "eeee",
                "aaaa",
                "",
                "dddd",
            ]
            s.calculate(strings)
            s3.reset()
            for string in strings:
                s3.addString(string)
            s3.finalize()

            self.assertEqual(s.statistic(t["stat"]), t["expected"])
            self.assertEqual(s3.statistic(t["stat"]), t["expected"])

            # display name
            self.assertGreater(
                len(QgsStringStatisticalSummary.displayName(t["stat"])), 0
            )

    def testVariantStats(self):
        s = QgsStringStatisticalSummary()
        self.assertEqual(s.statistics(), QgsStringStatisticalSummary.Statistic.All)
        s.calculateFromVariants(["cc", 5, "bbbb", "aaaa", "eeee", 6, 9, "9", None])
        self.assertEqual(s.count(), 6)
        self.assertEqual(
            set(s.distinctValues()), {"cc", "aaaa", "bbbb", "eeee", "", "9"}
        )
        self.assertEqual(s.countMissing(), 1)
        self.assertEqual(s.min(), "9")
        self.assertEqual(s.max(), "eeee")

    def testAddVariantStats(self):
        s = QgsStringStatisticalSummary()
        self.assertEqual(s.statistics(), QgsStringStatisticalSummary.Statistic.All)
        for v in ["cc", 5, "bbbb", "aaaa", "eeee", 6, 9, "9", None]:
            s.addValue(v)
        self.assertEqual(s.count(), 6)
        self.assertEqual(
            set(s.distinctValues()), {"cc", "aaaa", "bbbb", "eeee", "", "9"}
        )
        self.assertEqual(s.countMissing(), 1)
        self.assertEqual(s.min(), "9")
        self.assertEqual(s.max(), "eeee")


if __name__ == "__main__":
    unittest.main()
