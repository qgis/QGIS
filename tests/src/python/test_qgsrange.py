"""QGIS Unit tests for QgsRange

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "11.04.2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QDate
from qgis.core import QgsDateRange, QgsDoubleRange, QgsIntRange, Qgis
from qgis.testing import unittest


class TestQgsIntRange(unittest.TestCase):

    def testGetters(self):
        range = QgsIntRange(1, 11)
        self.assertEqual(range.lower(), 1)
        self.assertEqual(range.upper(), 11)
        self.assertTrue(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeBoth)

        range = QgsIntRange(-1, 3, False, False)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeBoth)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.IncludeBoth)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertTrue(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeBoth)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.IncludeLowerExcludeUpper)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertTrue(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeLowerExcludeUpper)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.ExcludeLowerIncludeUpper)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeLowerIncludeUpper)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.ExcludeBoth)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeBoth)

    def testIsInfinite(self):
        range = QgsIntRange()
        self.assertTrue(range.isInfinite())
        range2 = QgsIntRange(range.lower(), 5)
        self.assertFalse(range2.isInfinite())
        range2 = QgsIntRange(5, range.upper())
        self.assertFalse(range2.isInfinite())

    def testEquality(self):
        self.assertEqual(QgsIntRange(1, 10), QgsIntRange(1, 10))
        self.assertNotEqual(QgsIntRange(1, 10), QgsIntRange(1, 11))
        self.assertNotEqual(QgsIntRange(1, 10), QgsIntRange(2, 10))
        self.assertNotEqual(QgsIntRange(1, 10, False), QgsIntRange(1, 10))
        self.assertNotEqual(QgsIntRange(1, 10, True, False), QgsIntRange(1, 10))

    def testIsEmpty(self):
        range = QgsIntRange(1, 1)
        # should not be empty because 1 is included
        self.assertFalse(range.isEmpty())

        range = QgsIntRange(1, 1, False, False)
        # should be empty because 1 is NOT included
        self.assertTrue(range.isEmpty())

        # invalid range is empty
        range = QgsIntRange(1, -1)
        self.assertTrue(range.isEmpty())

    def testIsSingleton(self):
        range = QgsIntRange(1, 1)
        self.assertTrue(range.isSingleton())

        range = QgsIntRange(1, 10)
        self.assertFalse(range.isSingleton())

        range = QgsIntRange(1, 1, False, False)
        # should not be singleton because 1 is NOT included
        self.assertFalse(range.isSingleton())

        # invalid range is not singleton
        range = QgsIntRange(1, -1)
        self.assertFalse(range.isSingleton())

    def testContains(self):
        # includes both ends
        range = QgsIntRange(0, 10)
        self.assertTrue(range.contains(QgsIntRange(1, 9)))
        self.assertTrue(range.contains(QgsIntRange(1, 10)))
        self.assertTrue(range.contains(QgsIntRange(0, 9)))
        self.assertTrue(range.contains(QgsIntRange(0, 10)))
        self.assertFalse(range.contains(QgsIntRange(-1, 9)))
        self.assertFalse(range.contains(QgsIntRange(1, 11)))

        # does not include left end
        range = QgsIntRange(0, 10, False, True)
        self.assertTrue(range.contains(QgsIntRange(1, 9)))
        self.assertTrue(range.contains(QgsIntRange(1, 10)))
        self.assertFalse(range.contains(QgsIntRange(0, 9)))
        self.assertFalse(range.contains(QgsIntRange(0, 10)))
        self.assertFalse(range.contains(QgsIntRange(-1, 9)))
        self.assertFalse(range.contains(QgsIntRange(1, 11)))

        # does not include right end
        range = QgsIntRange(0, 10, True, False)
        self.assertTrue(range.contains(QgsIntRange(1, 9)))
        self.assertFalse(range.contains(QgsIntRange(1, 10)))
        self.assertTrue(range.contains(QgsIntRange(0, 9)))
        self.assertFalse(range.contains(QgsIntRange(0, 10)))
        self.assertFalse(range.contains(QgsIntRange(-1, 9)))
        self.assertFalse(range.contains(QgsIntRange(1, 11)))

    def testContainsElement(self):
        # includes both ends
        range = QgsIntRange(0, 10)
        self.assertTrue(range.contains(0))
        self.assertTrue(range.contains(5))
        self.assertTrue(range.contains(10))
        self.assertFalse(range.contains(-1))
        self.assertFalse(range.contains(11))

        # includes left end
        range = QgsIntRange(0, 10, True, False)
        self.assertTrue(range.contains(0))
        self.assertTrue(range.contains(5))
        self.assertFalse(range.contains(10))
        self.assertFalse(range.contains(-1))
        self.assertFalse(range.contains(11))

        # includes right end
        range = QgsIntRange(0, 10, False, True)
        self.assertFalse(range.contains(0))
        self.assertTrue(range.contains(5))
        self.assertTrue(range.contains(10))
        self.assertFalse(range.contains(-1))
        self.assertFalse(range.contains(11))

        # includes neither end
        range = QgsIntRange(0, 10, False, False)
        self.assertFalse(range.contains(0))
        self.assertTrue(range.contains(5))
        self.assertFalse(range.contains(10))
        self.assertFalse(range.contains(-1))
        self.assertFalse(range.contains(11))

    def testOverlaps(self):
        """
        Test overlaps with every combination of include/exclude bounds
        """

        # reference range
        refLower, refUpper = 0, 10

        # other range is completely before or after reference range
        for otherLower, otherUpper in [[-16, -2], [12, 32]]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsIntRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsIntRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertFalse(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range overlaps reference range
        for otherLower, otherUpper in [
            # parts of the ranges overlaps
            [-2, 3],
            [3, 14],
            # full overlap and a bound in common
            [-2, 10],
            [0, 14],
            # partial overlap and a bound in common
            [0, 3],
            [3, 10],
            # same bounds
            [0, 10],
            # reference range is completely inside other range
            [-2, 14],
            # other range is completely inside reference range
            [3, 8],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsIntRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsIntRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertTrue(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        for (
            otherLower,
            otherUpper,
            otherIncLower,
            otherIncUpper,
            refIncLower,
            refIncUpper,
            expected,
        ) in [
            # other range and reference range are contiguous
            [-3, 0, True, True, True, True, True],
            [-3, 0, True, True, True, False, True],
            [-3, 0, True, True, False, True, False],
            [-3, 0, True, True, False, False, False],
            [-3, 0, True, False, True, True, False],
            [-3, 0, True, False, True, False, False],
            [-3, 0, True, False, False, True, False],
            [-3, 0, True, False, False, False, False],
            [-3, 0, False, True, True, True, True],
            [-3, 0, False, True, True, False, True],
            [-3, 0, False, True, False, True, False],
            [-3, 0, False, True, False, False, False],
            [-3, 0, False, False, True, True, False],
            [-3, 0, False, False, True, False, False],
            [-3, 0, False, False, False, True, False],
            [-3, 0, False, False, False, False, False],
            # reference range and other range are contiguous
            [10, 14, True, True, True, True, True],
            [10, 14, True, True, True, False, False],
            [10, 14, True, True, False, True, True],
            [10, 14, True, True, False, False, False],
            [10, 14, True, False, True, True, True],
            [10, 14, True, False, True, False, False],
            [10, 14, True, False, False, True, True],
            [10, 14, True, False, False, False, False],
            [10, 14, False, True, True, True, False],
            [10, 14, False, True, True, False, False],
            [10, 14, False, True, False, True, False],
            [10, 14, False, True, False, False, False],
            [10, 14, False, False, True, True, False],
            [10, 14, False, False, True, False, False],
            [10, 14, False, False, False, True, False],
            [10, 14, False, False, False, False, False],
        ]:
            refRange = QgsIntRange(refLower, refUpper, refIncLower, refIncUpper)
            otherRange = QgsIntRange(
                otherLower, otherUpper, otherIncLower, otherIncUpper
            )
            self.assertEqual(
                refRange.overlaps(otherRange), expected, f"{refRange=} {otherRange=}"
            )


class TestQgsDoubleRange(unittest.TestCase):

    def testGetters(self):
        range = QgsDoubleRange(1.0, 11.0)
        self.assertEqual(range.lower(), 1)
        self.assertEqual(range.upper(), 11)
        self.assertTrue(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeBoth)

        range = QgsDoubleRange(-1.0, 3.0, False, False)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeBoth)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.IncludeBoth)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertTrue(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeBoth)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.IncludeLowerExcludeUpper)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertTrue(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.IncludeLowerExcludeUpper)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.ExcludeLowerIncludeUpper)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertTrue(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeLowerIncludeUpper)

        range = QgsIntRange(-1, 3, Qgis.RangeLimits.ExcludeBoth)
        self.assertEqual(range.lower(), -1)
        self.assertEqual(range.upper(), 3)
        self.assertFalse(range.includeLower())
        self.assertFalse(range.includeUpper())
        self.assertEqual(range.rangeLimits(), Qgis.RangeLimits.ExcludeBoth)

    def testEquality(self):
        self.assertEqual(QgsDoubleRange(1, 10), QgsDoubleRange(1, 10))
        self.assertNotEqual(QgsDoubleRange(1, 10), QgsDoubleRange(1, 11))
        self.assertNotEqual(QgsDoubleRange(1, 10), QgsDoubleRange(2, 10))
        self.assertNotEqual(QgsDoubleRange(1, 10, False), QgsDoubleRange(1, 10))
        self.assertNotEqual(QgsDoubleRange(1, 10, True, False), QgsDoubleRange(1, 10))

    def testIsInfinite(self):
        range = QgsDoubleRange()
        self.assertTrue(range.isInfinite())
        range2 = QgsDoubleRange(range.lower(), 5)
        self.assertFalse(range2.isInfinite())
        range2 = QgsDoubleRange(5, range.upper())
        self.assertFalse(range2.isInfinite())


class TestQgsDateRange(unittest.TestCase):

    def testGetters(self):
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2))
        self.assertEqual(range.begin(), QDate(2010, 3, 1))
        self.assertEqual(range.end(), QDate(2010, 6, 2))
        self.assertTrue(range.includeBeginning())
        self.assertTrue(range.includeEnd())

        range = QgsDateRange(QDate(), QDate(2010, 6, 2))
        self.assertFalse(range.begin().isValid())
        self.assertEqual(range.end(), QDate(2010, 6, 2))

        range = QgsDateRange(QDate(2010, 3, 1), QDate())
        self.assertEqual(range.begin(), QDate(2010, 3, 1))
        self.assertFalse(range.end().isValid())

    def testIsEmpty(self):
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2))
        self.assertFalse(range.isEmpty())

        range = QgsDateRange(QDate(), QDate(2010, 6, 2))
        self.assertFalse(range.isEmpty())

        range = QgsDateRange(QDate(2010, 3, 1), QDate())
        self.assertFalse(range.isEmpty())

        # check QgsDateRange docs - this is treated as an infinite range, so is NOT empty
        range = QgsDateRange(QDate(), QDate())
        self.assertFalse(range.isEmpty())

        range = QgsDateRange(QDate(2017, 3, 1), QDate(2010, 6, 2))
        self.assertTrue(range.isEmpty())

        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 3, 1))
        self.assertFalse(range.isEmpty())

        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 3, 1), False, False)
        self.assertTrue(range.isEmpty())

    def testContains(self):
        # includes both ends
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2))
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 6, 2)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2)))
        )
        self.assertFalse(
            range.contains(QgsDateRange(QDate(2009, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertFalse(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2017, 4, 5)))
        )
        self.assertFalse(range.contains(QgsDateRange(QDate(2010, 4, 1), QDate())))
        self.assertFalse(range.contains(QgsDateRange(QDate(), QDate(2010, 4, 1))))

        # infinite left end
        range = QgsDateRange(QDate(), QDate(2010, 6, 2))
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 6, 2)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2009, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertFalse(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2017, 4, 5)))
        )
        self.assertFalse(range.contains(QgsDateRange(QDate(2010, 4, 1), QDate())))
        self.assertTrue(range.contains(QgsDateRange(QDate(), QDate(2010, 4, 1))))

        # infinite right end
        range = QgsDateRange(QDate(2010, 3, 1), QDate())
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2010, 6, 2)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2)))
        )
        self.assertFalse(
            range.contains(QgsDateRange(QDate(2009, 4, 1), QDate(2010, 4, 5)))
        )
        self.assertTrue(
            range.contains(QgsDateRange(QDate(2010, 4, 1), QDate(2017, 4, 5)))
        )
        self.assertTrue(range.contains(QgsDateRange(QDate(2010, 4, 1), QDate())))
        self.assertFalse(range.contains(QgsDateRange(QDate(), QDate(2010, 4, 1))))

    def testContainsElement(self):
        # includes both ends
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2))
        self.assertTrue(range.contains(QDate(2010, 3, 1)))
        self.assertTrue(range.contains(QDate(2010, 5, 2)))
        self.assertTrue(range.contains(QDate(2010, 6, 2)))
        self.assertFalse(range.contains(QDate(2009, 6, 2)))
        self.assertFalse(range.contains(QDate(2017, 6, 2)))
        self.assertFalse(range.contains(QDate()))

        # infinite left end
        range = QgsDateRange(QDate(), QDate(2010, 6, 2))
        self.assertTrue(range.contains(QDate(2010, 3, 1)))
        self.assertTrue(range.contains(QDate(2010, 5, 2)))
        self.assertTrue(range.contains(QDate(2010, 6, 2)))
        self.assertTrue(range.contains(QDate(2009, 6, 2)))
        self.assertFalse(range.contains(QDate(2017, 6, 2)))
        self.assertFalse(range.contains(QDate()))

        # infinite right end
        range = QgsDateRange(QDate(2010, 3, 1), QDate())
        self.assertTrue(range.contains(QDate(2010, 3, 1)))
        self.assertTrue(range.contains(QDate(2010, 5, 2)))
        self.assertTrue(range.contains(QDate(2010, 6, 2)))
        self.assertFalse(range.contains(QDate(2009, 6, 2)))
        self.assertTrue(range.contains(QDate(2017, 6, 2)))
        self.assertFalse(range.contains(QDate()))

    def testOverlaps(self):
        """
        Test overlaps with every combination of include/exclude bounds
        and infinite bounds.
        """

        # -----------------------------------------------------------
        # reference range has no infinite bounds
        refLower, refUpper = QDate(2007, 4, 7), QDate(2013, 3, 2)

        # other range is completely before or after reference range
        for otherLower, otherUpper in [
            [QDate(), QDate(2000, 4, 6)],
            [QDate(2018, 8, 9), QDate(2020, 9, 9)],
            [QDate(2000, 1, 1), QDate(2004, 5, 7)],
            [QDate(2024, 4, 3), QDate()],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertFalse(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range overlaps reference range
        for otherLower, otherUpper in [
            # parts of the ranges overlaps
            [QDate(1999, 3, 2), QDate(2010, 8, 8)],
            [QDate(2010, 8, 8), QDate(2024, 5, 9)],
            [QDate(), QDate(2010, 8, 8)],
            [QDate(2010, 8, 8), QDate()],
            # full overlap and a bound in common
            [QDate(1999, 3, 2), QDate(2013, 3, 2)],
            [QDate(2007, 4, 7), QDate(2024, 5, 9)],
            [QDate(2007, 4, 7), QDate()],
            [QDate(), QDate(2013, 3, 2)],
            # partial overlap and a bound in common
            [QDate(2007, 4, 7), QDate(2010, 3, 1)],
            [QDate(2010, 3, 1), QDate(2013, 3, 2)],
            # same bounds
            [QDate(2007, 4, 7), QDate(2013, 3, 2)],
            # reference range is completely inside other range
            [QDate(2005, 1, 1), QDate(2021, 10, 20)],
            [QDate(), QDate(2021, 10, 20)],
            [QDate(2005, 1, 1), QDate()],
            [QDate(), QDate()],
            # other range is completely inside reference range
            [QDate(2010, 3, 1), QDate(2012, 8, 11)],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertTrue(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range and reference range are contiguous
        otherUpper = QDate(2007, 4, 7)
        for otherLower in [QDate(1999, 3, 4), QDate()]:
            for (
                otherIncLower,
                otherIncUpper,
                refIncLower,
                refIncUpper,
                expectedOverlaps,
            ) in [
                [True, True, True, True, True],
                [True, True, True, False, True],
                [True, True, False, True, False],
                [True, True, False, False, False],
                [True, False, True, True, False],
                [True, False, True, False, False],
                [True, False, False, True, False],
                [True, False, False, False, False],
                [False, True, True, True, True],
                [False, True, True, False, True],
                [False, True, False, True, False],
                [False, True, False, False, False],
                [False, False, True, True, False],
                [False, False, True, False, False],
                [False, False, False, True, False],
                [False, False, False, False, False],
            ]:
                refRange = QgsDateRange(refLower, refUpper, refIncLower, refIncUpper)
                otherRange = QgsDateRange(
                    otherLower, otherUpper, otherIncLower, otherIncUpper
                )
                self.assertEqual(
                    refRange.overlaps(otherRange),
                    expectedOverlaps,
                    f"{refRange=} {otherRange=}",
                )

        # reference range and other range are contiguous
        otherLower = QDate(2013, 3, 2)
        for otherUpper in [QDate(2025, 2, 13), QDate()]:
            for (
                otherIncLower,
                otherIncUpper,
                refIncLower,
                refIncUpper,
                expectedOverlaps,
            ) in [
                [True, True, True, True, True],
                [True, True, True, False, False],
                [True, True, False, True, True],
                [True, True, False, False, False],
                [True, False, True, True, True],
                [True, False, True, False, False],
                [True, False, False, True, True],
                [True, False, False, False, False],
                [False, True, True, True, False],
                [False, True, True, False, False],
                [False, True, False, True, False],
                [False, True, False, False, False],
                [False, False, True, True, False],
                [False, False, True, False, False],
                [False, False, False, True, False],
                [False, False, False, False, False],
            ]:
                refRange = QgsDateRange(refLower, refUpper, refIncLower, refIncUpper)
                otherRange = QgsDateRange(
                    otherLower, otherUpper, otherIncLower, otherIncUpper
                )
                self.assertEqual(
                    refRange.overlaps(otherRange),
                    expectedOverlaps,
                    f"{refRange=} {otherRange=}",
                )

        # -----------------------------------------------------------
        # reference range has a left infinite bound
        refLower, refUpper = QDate(), QDate(2013, 3, 2)

        # other range is completely after reference range
        for otherLower, otherUpper in [
            [QDate(2018, 8, 9), QDate(2020, 9, 9)],
            [QDate(2024, 4, 3), QDate()],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertFalse(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range overlaps reference range
        for otherLower, otherUpper in [
            # parts of the ranges overlaps
            [QDate(2010, 8, 8), QDate(2024, 5, 9)],
            [QDate(2010, 8, 8), QDate()],
            # a bound in common
            [QDate(1999, 8, 8), QDate(2013, 3, 2)],
            # same bounds
            [QDate(), QDate(2013, 3, 2)],
            # reference range is completely inside other range
            [QDate(), QDate(2020, 6, 6)],
            # other range is completely inside reference range
            [QDate(2010, 3, 1), QDate(2012, 8, 11)],
            [QDate(), QDate(2010, 8, 8)],
            # other range infinite
            [QDate(), QDate()],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertTrue(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # reference range and other range are contiguous
        otherLower = QDate(2013, 3, 2)
        for otherUpper in [QDate(2025, 2, 13), QDate()]:
            for (
                otherIncLower,
                otherIncUpper,
                refIncLower,
                refIncUpper,
                expectedOverlaps,
            ) in [
                [True, True, True, True, True],
                [True, True, True, False, False],
                [True, True, False, True, True],
                [True, True, False, False, False],
                [True, False, True, True, True],
                [True, False, True, False, False],
                [True, False, False, True, True],
                [True, False, False, False, False],
                [False, True, True, True, False],
                [False, True, True, False, False],
                [False, True, False, True, False],
                [False, True, False, False, False],
                [False, False, True, True, False],
                [False, False, True, False, False],
                [False, False, False, True, False],
                [False, False, False, False, False],
            ]:
                refRange = QgsDateRange(refLower, refUpper, refIncLower, refIncUpper)
                otherRange = QgsDateRange(
                    otherLower, otherUpper, otherIncLower, otherIncUpper
                )
                self.assertEqual(
                    refRange.overlaps(otherRange),
                    expectedOverlaps,
                    f"{refRange=} {otherRange=}",
                )

        # -----------------------------------------------------------
        # reference range has a right infinite bound
        refLower, refUpper = QDate(2013, 3, 2), QDate()

        # other range is completely before reference range
        for otherLower, otherUpper in [
            [QDate(1993, 5, 9), QDate(2005, 9, 9)],
            [QDate(), QDate(1999, 3, 9)],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertFalse(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range overlaps reference range
        for otherLower, otherUpper in [
            # parts of the ranges overlaps
            [QDate(2010, 5, 9), QDate(2025, 1, 8)],
            [QDate(), QDate(2025, 1, 8)],
            # a bound in common
            [QDate(2013, 3, 2), QDate(2025, 1, 2)],
            # same bounds
            [QDate(2013, 3, 2), QDate()],
            # reference range is completely inside other range
            [QDate(2013, 1, 1), QDate()],
            # other range is completely inside reference range
            [QDate(2016, 3, 1), QDate(2020, 8, 11)],
            [QDate(2016, 3, 1), QDate()],
            # other range infinite
            [QDate(), QDate()],
        ]:
            for refIncLower in [True, False]:
                for refIncUpper in [True, False]:
                    for otherIncLower in [True, False]:
                        for otherIncUpper in [True, False]:
                            refRange = QgsDateRange(
                                refLower, refUpper, refIncLower, refIncUpper
                            )
                            otherRange = QgsDateRange(
                                otherLower, otherUpper, otherIncLower, otherIncUpper
                            )
                            self.assertTrue(
                                refRange.overlaps(otherRange),
                                f"{refRange=} {otherRange=}",
                            )

        # other range and reference range are contiguous
        otherUpper = QDate(2013, 3, 2)
        for otherLower in [QDate(), QDate(1998, 4, 8)]:
            for (
                otherIncLower,
                otherIncUpper,
                refIncLower,
                refIncUpper,
                expectedOverlaps,
            ) in [
                [True, True, True, True, True],
                [True, True, True, False, True],
                [True, True, False, True, False],
                [True, True, False, False, False],
                [True, False, True, True, False],
                [True, False, True, False, False],
                [True, False, False, True, False],
                [True, False, False, False, False],
                [False, True, True, True, True],
                [False, True, True, False, True],
                [False, True, False, True, False],
                [False, True, False, False, False],
                [False, False, True, True, False],
                [False, False, True, False, False],
                [False, False, False, True, False],
                [False, False, False, False, False],
            ]:
                refRange = QgsDateRange(refLower, refUpper, refIncLower, refIncUpper)
                otherRange = QgsDateRange(
                    otherLower, otherUpper, otherIncLower, otherIncUpper
                )
                self.assertEqual(
                    refRange.overlaps(otherRange),
                    expectedOverlaps,
                    f"{refRange=} {otherRange=}",
                )

    def testIsInstant(self):
        self.assertFalse(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2)).isInstant())
        self.assertTrue(QgsDateRange(QDate(2010, 3, 1), QDate(2010, 3, 1)).isInstant())
        self.assertFalse(
            QgsDateRange(QDate(2010, 3, 1), QDate(2010, 3, 1), False, False).isInstant()
        )
        self.assertFalse(QgsDateRange(QDate(), QDate()).isInstant())

    def testIsInfinite(self):
        self.assertFalse(
            QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2)).isInfinite()
        )
        self.assertFalse(
            QgsDateRange(QDate(2010, 3, 1), QDate(2010, 3, 1)).isInfinite()
        )
        self.assertFalse(
            QgsDateRange(
                QDate(2010, 3, 1), QDate(2010, 3, 1), False, False
            ).isInfinite()
        )
        self.assertTrue(QgsDateRange(QDate(), QDate()).isInfinite())

    def testEquality(self):
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        )
        self.assertNotEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, True)
        )
        self.assertNotEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), True, False)
        )
        self.assertNotEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 3), False, False)
        )
        self.assertNotEqual(
            range, QgsDateRange(QDate(2010, 3, 2), QDate(2010, 6, 2), False, False)
        )

    def testExtend(self):
        range_empty = QgsDateRange(QDate(2010, 6, 2), QDate(2010, 3, 1))

        # Empty
        self.assertFalse(range_empty.extend(range_empty))
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertFalse(range.extend(range_empty))
        range = QgsDateRange(QDate(2010, 6, 2), QDate(2010, 3, 1))
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        )

        # Extend low
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 2, 1), QDate(2010, 6, 2), False, False)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 2, 1), QDate(2010, 6, 2), False, False)
        )
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 2, 1), QDate(2010, 5, 2), True, False)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 2, 1), QDate(2010, 6, 2), True, False)
        )

        # Extend high
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 3, 1), QDate(2010, 7, 2), False, False)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 7, 2), False, False)
        )
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, True)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, True)
        )

        # Extend both
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 2, 1), QDate(2010, 7, 2), False, False)
            )
        )
        self.assertEqual(
            range, QgsDateRange(QDate(2010, 2, 1), QDate(2010, 7, 2), False, False)
        )

        # Extend none
        range = QgsDateRange(QDate(2010, 3, 1), QDate(2010, 6, 2), False, False)
        self.assertFalse(
            range.extend(
                QgsDateRange(QDate(2010, 4, 6), QDate(2010, 5, 2), False, False)
            )
        )

        # Test infinity
        range = QgsDateRange(QDate(), QDate())
        self.assertFalse(
            range.extend(
                QgsDateRange(QDate(2010, 4, 6), QDate(2010, 5, 2), False, False)
            )
        )
        range = QgsDateRange(QDate(), QDate(2010, 5, 2))
        self.assertFalse(
            range.extend(
                QgsDateRange(QDate(2010, 4, 6), QDate(2010, 5, 2), False, False)
            )
        )
        self.assertEqual(range, QgsDateRange(QDate(), QDate(2010, 5, 2), True, True))
        range = QgsDateRange(QDate(2010, 4, 6), QDate())
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 3, 6), QDate(2010, 5, 2), False, False)
            )
        )
        self.assertEqual(range, QgsDateRange(QDate(2010, 3, 6), QDate(), False, True))
        range = QgsDateRange(QDate(), QDate(2010, 5, 2))
        self.assertTrue(
            range.extend(
                QgsDateRange(QDate(2010, 3, 6), QDate(2010, 6, 2), False, False)
            )
        )
        self.assertEqual(range, QgsDateRange(QDate(), QDate(2010, 6, 2), True, False))
        range = QgsDateRange(QDate(2010, 4, 6), QDate())
        self.assertTrue(
            range.extend(QgsDateRange(QDate(), QDate(2010, 5, 2), True, False))
        )
        self.assertEqual(range, QgsDateRange(QDate(), QDate(), True, True))
        range = QgsDateRange(QDate(), QDate(2010, 4, 6))
        self.assertTrue(range.extend(QgsDateRange(QDate(), QDate(), True, True)))
        self.assertEqual(range, QgsDateRange(QDate(), QDate(), True, True))


if __name__ == "__main__":
    unittest.main()
