"""QGIS Unit tests for QgsLineString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '12/09/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsLineString, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLineString(QgisTestCase):

    def testConstruct(self):
        # With points
        line = QgsLineString(QgsPoint(1, 2), QgsPoint(3, 4))
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

        # With list
        line = QgsLineString([[1, 2], [3, 4]])
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

    def testMeasureLine(self):
        line = QgsLineString()
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line.asWkt(0), "LineStringM EMPTY")

        line = QgsLineString([[0, 0], [2, 0], [4, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(2, 0, m=15), QgsPoint(4, 0, m=20)]))

        line = QgsLineString([[0, 0], [9, 0], [10, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(9, 0, m=19), QgsPoint(10, 0, m=20)]))

        line = QgsLineString([[0, 0], [0, 0], [0, 0]])
        m_line = line.measuredLine(10, 20)
        self.assertEqual(m_line, QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(0, 0, m=15), QgsPoint(0, 0, m=20)]))


if __name__ == '__main__':
    unittest.main()
