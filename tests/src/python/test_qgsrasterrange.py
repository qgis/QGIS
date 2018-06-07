# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterRange.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '07/06/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA switch sip api

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


if __name__ == '__main__':
    unittest.main()
