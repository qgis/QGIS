"""QGIS Unit tests for QgsRasterRange.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "07/06/2018"
__copyright__ = "Copyright 2018, The QGIS Project"


from qgis.core import QgsRasterRange
from qgis.testing import unittest


class TestQgsRasterRange(unittest.TestCase):

    def testBasic(self):
        range = QgsRasterRange(1, 5)
        self.assertEqual(range.min(), 1)
        self.assertEqual(range.max(), 5)
        range.setMin(2.2)
        range.setMax(10.4)
        self.assertEqual(range.min(), 2.2)
        self.assertEqual(range.max(), 10.4)
        self.assertEqual(range.bounds(), QgsRasterRange.BoundsType.IncludeMinAndMax)
        range.setBounds(QgsRasterRange.BoundsType.IncludeMin)
        self.assertEqual(range.bounds(), QgsRasterRange.BoundsType.IncludeMin)

    def testEquality(self):
        range = QgsRasterRange(1, 5)
        range2 = QgsRasterRange(1, 5)
        self.assertEqual(range, range2)
        range2.setMin(2)
        self.assertNotEqual(range, range2)
        range2.setMin(1)
        range2.setMax(4)
        self.assertNotEqual(range, range2)
        range2.setMax(5)
        self.assertEqual(range, range2)
        range.setBounds(QgsRasterRange.BoundsType.IncludeMax)
        self.assertNotEqual(range, range2)
        range2.setBounds(QgsRasterRange.BoundsType.IncludeMax)
        self.assertEqual(range, range2)
        range = QgsRasterRange()
        range2 = QgsRasterRange()
        self.assertEqual(range, range2)
        range.setMin(1)
        self.assertNotEqual(range, range2)
        range2.setMin(1)
        self.assertEqual(range, range2)
        range = QgsRasterRange()
        range2 = QgsRasterRange()
        range.setMax(5)
        self.assertNotEqual(range, range2)
        range2.setMax(5)
        self.assertEqual(range, range2)

    def testContains(self):
        range = QgsRasterRange(1, 5)
        self.assertTrue(range.contains(1))
        self.assertTrue(range.contains(5))
        self.assertTrue(range.contains(4))
        self.assertTrue(range.contains(1.00001))
        self.assertTrue(range.contains(4.99999))
        self.assertFalse(range.contains(0.99999))
        self.assertFalse(range.contains(5.00001))

        # with nan min/maxs
        range = QgsRasterRange()
        self.assertTrue(range.contains(1))
        self.assertTrue(range.contains(-909999999))
        self.assertTrue(range.contains(999999999))
        range.setMin(5)
        self.assertFalse(range.contains(0))
        self.assertTrue(range.contains(5))
        self.assertTrue(range.contains(10000000))

        range = QgsRasterRange()
        range.setMax(5)
        self.assertFalse(range.contains(6))
        self.assertTrue(range.contains(5))
        self.assertTrue(range.contains(-99999))

        range = QgsRasterRange(1, 5, QgsRasterRange.BoundsType.IncludeMax)
        self.assertFalse(range.contains(0))
        self.assertFalse(range.contains(1))
        self.assertTrue(range.contains(2))
        self.assertTrue(range.contains(5))
        self.assertFalse(range.contains(6))

        range = QgsRasterRange(1, 5, QgsRasterRange.BoundsType.IncludeMin)
        self.assertFalse(range.contains(0))
        self.assertTrue(range.contains(1))
        self.assertTrue(range.contains(2))
        self.assertFalse(range.contains(5))
        self.assertFalse(range.contains(6))

        range = QgsRasterRange(1, 5, QgsRasterRange.BoundsType.Exclusive)
        self.assertFalse(range.contains(0))
        self.assertFalse(range.contains(1))
        self.assertTrue(range.contains(2))
        self.assertFalse(range.contains(5))
        self.assertFalse(range.contains(6))

    def testContainsList(self):
        self.assertFalse(QgsRasterRange.contains(1, []))
        ranges = [QgsRasterRange(1, 5)]
        self.assertTrue(QgsRasterRange.contains(3, ranges))
        self.assertFalse(QgsRasterRange.contains(13, ranges))
        ranges.append(QgsRasterRange(11, 15))
        self.assertTrue(QgsRasterRange.contains(3, ranges))
        self.assertTrue(QgsRasterRange.contains(13, ranges))
        self.assertFalse(QgsRasterRange.contains(16, ranges))

    def testOverlaps(self):
        # includes both ends
        range = QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMinAndMax)
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(10, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertFalse(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(0, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

        range = QgsRasterRange(
            float("NaN"), 10, QgsRasterRange.BoundsType.IncludeMinAndMax
        )
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(10, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(0, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

        range = QgsRasterRange(
            0, float("NaN"), QgsRasterRange.BoundsType.IncludeMinAndMax
        )
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(10, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertFalse(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertTrue(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(0, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

        # includes left end
        range = QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMin)
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertFalse(range.overlaps(QgsRasterRange(10, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertFalse(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(0, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

        # includes right end
        range = QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMax)
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(10, 11)))
        self.assertFalse(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertFalse(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 50)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertTrue(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

        # includes neither end
        range = QgsRasterRange(0, 10, QgsRasterRange.BoundsType.Exclusive)
        self.assertTrue(range.overlaps(QgsRasterRange(1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(0, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 9)))
        self.assertTrue(range.overlaps(QgsRasterRange(1, 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, 11)))
        self.assertFalse(range.overlaps(QgsRasterRange(10, 11)))
        self.assertFalse(range.overlaps(QgsRasterRange(-1, 0)))
        self.assertFalse(range.overlaps(QgsRasterRange(-10, -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(11, 12)))
        self.assertTrue(range.overlaps(QgsRasterRange(-1, float("NaN"))))
        self.assertTrue(range.overlaps(QgsRasterRange(1, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(10, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(11, float("NaN"))))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), -1)))
        self.assertFalse(range.overlaps(QgsRasterRange(float("NaN"), 0)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 1)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 10)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), 11)))
        self.assertTrue(range.overlaps(QgsRasterRange(float("NaN"), float("NaN"))))
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(-1, 0, QgsRasterRange.BoundsType.IncludeMax))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.Exclusive))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMin))
        )
        self.assertFalse(
            range.overlaps(QgsRasterRange(10, 11, QgsRasterRange.BoundsType.IncludeMax))
        )

    def testAsText(self):
        self.assertEqual(
            QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMinAndMax).asText(),
            "0 ≤ x ≤ 10",
        )
        self.assertEqual(QgsRasterRange(-1, float("NaN")).asText(), "-1 ≤ x ≤ ∞")
        self.assertEqual(QgsRasterRange(float("NaN"), 5).asText(), "-∞ ≤ x ≤ 5")
        self.assertEqual(
            QgsRasterRange(float("NaN"), float("NaN")).asText(), "-∞ ≤ x ≤ ∞"
        )
        self.assertEqual(
            QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMin).asText(),
            "0 ≤ x < 10",
        )
        self.assertEqual(
            QgsRasterRange(
                -1, float("NaN"), QgsRasterRange.BoundsType.IncludeMin
            ).asText(),
            "-1 ≤ x < ∞",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), 5, QgsRasterRange.BoundsType.IncludeMin
            ).asText(),
            "-∞ ≤ x < 5",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), float("NaN"), QgsRasterRange.BoundsType.IncludeMin
            ).asText(),
            "-∞ ≤ x < ∞",
        )
        self.assertEqual(
            QgsRasterRange(0, 10, QgsRasterRange.BoundsType.IncludeMax).asText(),
            "0 < x ≤ 10",
        )
        self.assertEqual(
            QgsRasterRange(
                -1, float("NaN"), QgsRasterRange.BoundsType.IncludeMax
            ).asText(),
            "-1 < x ≤ ∞",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), 5, QgsRasterRange.BoundsType.IncludeMax
            ).asText(),
            "-∞ < x ≤ 5",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), float("NaN"), QgsRasterRange.BoundsType.IncludeMax
            ).asText(),
            "-∞ < x ≤ ∞",
        )
        self.assertEqual(
            QgsRasterRange(0, 10, QgsRasterRange.BoundsType.Exclusive).asText(),
            "0 < x < 10",
        )
        self.assertEqual(
            QgsRasterRange(
                -1, float("NaN"), QgsRasterRange.BoundsType.Exclusive
            ).asText(),
            "-1 < x < ∞",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), 5, QgsRasterRange.BoundsType.Exclusive
            ).asText(),
            "-∞ < x < 5",
        )
        self.assertEqual(
            QgsRasterRange(
                float("NaN"), float("NaN"), QgsRasterRange.BoundsType.Exclusive
            ).asText(),
            "-∞ < x < ∞",
        )


if __name__ == "__main__":
    unittest.main()
