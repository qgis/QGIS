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

from qgis.testing import unittest, start_app
from qgis.core import QgsClassificationMethod, QgsClassificationLogarithmic, QgsFeature, QgsVectorLayer, QgsPointXY, \
    QgsGeometry


start_app()

#===========================================================
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
        r = m.classes(vl, 'value', 9)

        self.assertEqual(r.count(), 6)
        self.assertEqual(r[0].label(), '2746.71 - 10^4')
        self.assertEqual(QgsClassificationMethod.listToValues(r),
                         [10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0])

        self.assertEqual(m.classes(vl, 'value', 4).count(), 4)


if __name__ == "__main__":
    unittest.main()
