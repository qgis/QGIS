"""QGIS Unit tests for QgsMultiLineString.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '12/12/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsMultiLineString, QgsLineString, QgsPoint
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLineString(QgisTestCase):

    def testConstruct(self):
        # With list
        line = QgsLineString([QgsPoint(1, 2), QgsPoint(3, 4)])
        multiline = QgsMultiLineString()
        multiline.addGeometry(line)
        self.assertEqual(multiline.numGeometries(), 1)
        line = multiline.geometryN(0)
        self.assertEqual(line.startPoint(), QgsPoint(1, 2))
        self.assertEqual(line.endPoint(), QgsPoint(3, 4))

    def testMeasureLine(self):
        multiline = QgsMultiLineString()
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.asWkt(0), "MultiLineStringM EMPTY")

        multiline.addGeometry(QgsLineString([[0, 0], [2, 0], [4, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.geometryN(0), QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(2, 0, m=15), QgsPoint(4, 0, m=20)]))

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[0, 0], [9, 0], [10, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.geometryN(0), QgsLineString([QgsPoint(0, 0, m=10), QgsPoint(9, 0, m=19), QgsPoint(10, 0, m=20)]))

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[1, 0], [3, 0], [4, 0]]))
        multiline.addGeometry(QgsLineString([[0, 0], [9, 0], [10, 0]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.numGeometries(), 2)
        self.assertEqual(m_line.asWkt(0), "MultiLineStringM ((1 0 10, 3 0 12, 4 0 12),(0 0 12, 9 0 19, 10 0 20))")

        multiline = QgsMultiLineString()
        multiline.addGeometry(QgsLineString([[1, 0], [1, 0], [1, 0]]))
        multiline.addGeometry(QgsLineString([[2, 2], [2, 2], [2, 2]]))
        m_line = multiline.measuredLine(10, 20)
        self.assertEqual(m_line.numGeometries(), 2)
        self.assertEqual(m_line.asWkt(0), "MultiLineStringM ((1 0 nan, 1 0 nan, 1 0 nan),(2 2 nan, 2 2 nan, 2 2 nan))")


if __name__ == '__main__':
    unittest.main()
