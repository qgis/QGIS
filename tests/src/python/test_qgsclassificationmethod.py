# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsClassificationMethod implementations

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '3/09/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import random

from qgis.PyQt.QtCore import QLocale
from qgis.testing import unittest, start_app
from qgis.core import QgsClassificationMethod, QgsClassificationLogarithmic, QgsClassificationJenks, QgsFeature, QgsVectorLayer, QgsPointXY, \
    QgsGeometry

start_app()


# ===========================================================
# Utility functions


def createMemoryLayer(values):
    ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                        "test_data", "memory")
    # Data as list of x, y, id, value
    assert ml.isValid()
    pr = ml.dataProvider()
    fields = pr.fields()
    id = 0
    for value in values:
        id += 1
        feat = QgsFeature(fields)
        feat['id'] = id
        feat['value'] = value
        g = QgsGeometry.fromPointXY(QgsPointXY(id / 100, id / 100))
        feat.setGeometry(g)
        pr.addFeatures([feat])
    ml.updateExtents()
    return ml


class TestQgsClassificationMethods(unittest.TestCase):

    def testQgsClassificationLogarithmic(self):
        values = [2746.71,
                  66667.49,
                  77282.52,
                  986567.01,
                  1729508.41,
                  9957836.86,
                  35419826.29,
                  52584164.80,
                  296572842.00]

        vl = createMemoryLayer(values)

        m = QgsClassificationLogarithmic()
        r = m.classes(vl, 'value', 8)

        self.assertEqual(len(r), 6)
        self.assertEqual(r[0].label(), '{} - 10^4'.format(QLocale().toString(2746.71)))
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r),
                         [10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0])

        self.assertEqual(len(m.classes(vl, 'value', 4)), 4)

    def testQgsClassificationLogarithmicCloseMinimum(self):
        """See issue GH #45454: Incorrect scale range legend after applying
        logarithmic graduated symbology to a vector layer"""

        values = [0.009900019065438,
                  0.010851322017611,
                  0.01755707784994,
                  0.031925433036994,
                  0.046422733606398]

        vl = createMemoryLayer(values)

        m = QgsClassificationLogarithmic()
        r = m.classes(vl, 'value', 4)

        classes = [(c.lowerBound(), c.upperBound()) for c in r]

        for l, h in classes:
            self.assertLess(l, h)

    def testQgsClassificationLogarithmic_FilterZeroNeg(self):
        values = [-2, 0, 1, 7, 66, 555, 4444]
        vl = createMemoryLayer(values)
        m = QgsClassificationLogarithmic()

        m.setParameterValues({'ZERO_NEG_VALUES_HANDLE': QgsClassificationLogarithmic.Discard})
        r = m.classes(vl, 'value', 4)
        self.assertEqual(len(r), 4)
        self.assertEqual(r[0].label(), '1 - 10^1')
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r), [10.0, 100.0, 1000.0, 10000.0])

        m.setParameterValues({'ZERO_NEG_VALUES_HANDLE': QgsClassificationLogarithmic.PrependBreak})
        r = m.classes(vl, 'value', 4)
        self.assertEqual(r[0].label(), '-2 - 10^0')
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r), [1.0, 10.0, 100.0, 1000.0, 10000.0])

    def testQgsClassificationJenksSimple(self):
        # This is a simple Natural Breaks Jenks test checking if simple calculation can be done
        # when number of values is below 3000 (value hardcoded in qgsclassificationjenks.h)
        # And if returned values are consistent (the same as at the time of creation of this test)
        values = [-33, -41, -43, 16, 29, 9, -35, 57, 26, -30]
        vl = createMemoryLayer(values)
        m = QgsClassificationJenks()

        r = m.classes(vl, 'value', 4)
        self.assertEqual(len(r), 4)
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r),
                         [-30.0, 16.0, 29.0, 57.0])

    def testQgsClassificationJenksHighNumber(self):
        # This test checks if Jenkis classification does not crash when number of
        # values in a set is higher than 3000 (value hardcoded in qgsclassificationjenks.h)
        # And if returned values are consistent (the same as at the time of creation of this test)
        random.seed(42 * 42)
        values = [random.randint(-1000, 1000) for _ in range(5000)]
        vl = createMemoryLayer(values)
        m = QgsClassificationJenks()

        r = m.classes(vl, 'value', 4)
        self.assertEqual(len(r), 4)
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r),
                         [-506.0, -4.0, 499.0, 1000.0])


if __name__ == "__main__":
    unittest.main()
