# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeatureSource.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '26/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
import qgis  # NOQA

import os

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
start_app()


def createLayerWithFivePoints():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 1])
    f.setGeometry(QgsGeometry.fromPoint(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 3])
    f2.setGeometry(QgsGeometry.fromPoint(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 3])
    f3.setGeometry(QgsGeometry.fromPoint(QgsPointXY(300, 200)))
    f4 = QgsFeature()
    f4.setAttributes(["test3", 3])
    f4.setGeometry(QgsGeometry.fromPoint(QgsPointXY(400, 300)))
    f5 = QgsFeature()
    f5.setAttributes(["test4", 4])
    f5.setGeometry(QgsGeometry.fromPoint(QgsPointXY(0, 0)))
    assert pr.addFeatures([f, f2, f3, f4, f5])
    assert layer.featureCount() == 5
    return layer


class TestQgsFeatureSource(unittest.TestCase):

    def testUniqueValues(self):
        """
        Test retrieving unique values using base class method
        """

        # memory provider uses base class method
        layer = createLayerWithFivePoints()
        self.assertFalse(layer.dataProvider().uniqueValues(-1))
        self.assertFalse(layer.dataProvider().uniqueValues(100))
        self.assertEqual(layer.dataProvider().uniqueValues(0), {'test', 'test2', 'test3', 'test4'})
        self.assertEqual(layer.dataProvider().uniqueValues(1), {1, 3, 3, 4})

    def testMinValues(self):
        """
        Test retrieving min values using base class method
        """

        # memory provider uses base class method
        layer = createLayerWithFivePoints()
        self.assertFalse(layer.dataProvider().minimumValue(-1))
        self.assertFalse(layer.dataProvider().minimumValue(100))
        self.assertEqual(layer.dataProvider().minimumValue(0), 'test')
        self.assertEqual(layer.dataProvider().minimumValue(1), 1)

    def testMaxValues(self):
        """
        Test retrieving min values using base class method
        """

        # memory provider uses base class method
        layer = createLayerWithFivePoints()
        self.assertFalse(layer.dataProvider().maximumValue(-1))
        self.assertFalse(layer.dataProvider().maximumValue(100))
        self.assertEqual(layer.dataProvider().maximumValue(0), 'test4')
        self.assertEqual(layer.dataProvider().maximumValue(1), 4)


if __name__ == '__main__':
    unittest.main()
