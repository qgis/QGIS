# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMargins.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2017-01'
__copyright__ = 'Copyright 2017, The QGIS Project'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import QgsMargins


class TestQgsOptional(unittest.TestCase):

    def testGetSet(self):
        margins = QgsMargins()
        margins.setLeft(1.1)
        self.assertEqual(margins.left(), 1.1)
        margins.setTop(2.2)
        self.assertEqual(margins.top(), 2.2)
        margins.setBottom(3.3)
        self.assertEqual(margins.bottom(), 3.3)
        margins.setRight(4.4)
        self.assertEqual(margins.right(), 4.4)

        margins = QgsMargins()
        self.assertTrue(margins.isNull())
        margins.setLeft(5.5)
        margins.setRight(5.5)
        self.assertFalse(margins.isNull())
        self.assertEqual(margins, QgsMargins(5.5, 0.0, 5.5, 0.0))

    def testOperators(self):
        m1 = QgsMargins(12.1, 14.1, 16.1, 18.1)
        m2 = QgsMargins(2.1, 3.1, 4.1, 5.1)

        added = m1 + m2
        self.assertAlmostEqual(added.left(), 14.2)
        self.assertAlmostEqual(added.top(), 17.2)
        self.assertAlmostEqual(added.right(), 20.2)
        self.assertAlmostEqual(added.bottom(), 23.2)
        a = QgsMargins(m1)
        a += m2
        self.assertEqual(a, added)

        subtracted = m1 - m2
        self.assertAlmostEqual(subtracted.left(), 10.0)
        self.assertAlmostEqual(subtracted.top(), 11.0)
        self.assertAlmostEqual(subtracted.right(), 12.0)
        self.assertAlmostEqual(subtracted.bottom(), 13.0)
        a = QgsMargins(m1)
        a -= m2
        self.assertEqual(a, subtracted)

        h = QgsMargins(m1)
        h += 2.1
        self.assertAlmostEqual(h.left(), 14.2)
        self.assertAlmostEqual(h.top(), 16.2)
        self.assertAlmostEqual(h.right(), 18.2)
        self.assertAlmostEqual(h.bottom(), 20.2)
        h -= 2.1
        self.assertEqual(h, m1)

        doubled = m1 * 2.0
        self.assertEqual(doubled, QgsMargins(24.2, 28.2, 32.2, 36.2))
        self.assertEqual(2.0 * m1, doubled)
        self.assertEqual(m1 * 2.0, doubled)

        a = QgsMargins(m1)
        a *= 2.0
        self.assertEqual(a, doubled)

        halved = m1 / 2.0
        self.assertAlmostEqual(halved.left(), 6.05)
        self.assertAlmostEqual(halved.top(), 7.05)
        self.assertAlmostEqual(halved.right(), 8.05)
        self.assertAlmostEqual(halved.bottom(), 9.05)

        a = QgsMargins(m1)
        a /= 2.0
        self.assertEqual(a, halved)

        self.assertEqual(m1 + (-m1), QgsMargins())

        m3 = QgsMargins(10.3, 11.4, 12.5, 13.6)
        self.assertEqual(m3 + 1.1, QgsMargins(11.4, 12.5, 13.6, 14.7))
        self.assertEqual(1.1 + m3, QgsMargins(11.4, 12.5, 13.6, 14.7))
        m4 = m3 - 1.1
        self.assertAlmostEqual(m4.left(), 9.2)
        self.assertAlmostEqual(m4.top(), 10.3)
        self.assertAlmostEqual(m4.right(), 11.4)
        self.assertAlmostEqual(m4.bottom(), 12.5)
        self.assertEqual(+m3, QgsMargins(10.3, 11.4, 12.5, 13.6))
        self.assertEqual(-m3, QgsMargins(-10.3, -11.4, -12.5, -13.6))

    def testToString(self):
        # null margin
        self.assertFalse(QgsMargins().toString())

        self.assertEqual(QgsMargins(1, 2, 3, 4).toString(), '1,2,3,4')
        self.assertEqual(QgsMargins(1, -2, 3, -4).toString(), '1,-2,3,-4')

    def testFromString(self):

        self.assertTrue(QgsMargins.fromString('').isNull())
        self.assertTrue(QgsMargins.fromString('not good').isNull())
        self.assertTrue(QgsMargins.fromString('1,2,3').isNull())
        self.assertTrue(QgsMargins.fromString('1,2,3,4,5').isNull())

        self.assertEqual(QgsMargins.fromString('1,2,3,4'), QgsMargins(1, 2, 3, 4))
        self.assertEqual(QgsMargins.fromString('1,-2,3,-4'), QgsMargins(1, -2, 3, -4))


if __name__ == '__main__':
    unittest.main()
