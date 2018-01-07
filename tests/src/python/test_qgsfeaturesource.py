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
                       QgsPointXY,
                       QgsFeatureRequest,
                       QgsWkbTypes,
                       QgsCoordinateReferenceSystem)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
start_app()


def createLayerWithFivePoints():
    layer = QgsVectorLayer("Point?field=id:integer&field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes([1, "test", 1])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
    f2 = QgsFeature()
    f2.setAttributes([2, "test2", 3])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 2)))
    f3 = QgsFeature()
    f3.setAttributes([3, "test2", 3])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 2)))
    f4 = QgsFeature()
    f4.setAttributes([4, "test3", 3])
    f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(4, 3)))
    f5 = QgsFeature()
    f5.setAttributes([5, "test4", 4])
    f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
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
        self.assertEqual(layer.dataProvider().uniqueValues(1), {'test', 'test2', 'test3', 'test4'})
        self.assertEqual(layer.dataProvider().uniqueValues(2), {1, 3, 3, 4})

    def testMinValues(self):
        """
        Test retrieving min values using base class method
        """

        # memory provider uses base class method
        layer = createLayerWithFivePoints()
        self.assertFalse(layer.dataProvider().minimumValue(-1))
        self.assertFalse(layer.dataProvider().minimumValue(100))
        self.assertEqual(layer.dataProvider().minimumValue(1), 'test')
        self.assertEqual(layer.dataProvider().minimumValue(2), 1)

    def testMaxValues(self):
        """
        Test retrieving min values using base class method
        """

        # memory provider uses base class method
        layer = createLayerWithFivePoints()
        self.assertFalse(layer.dataProvider().maximumValue(-1))
        self.assertFalse(layer.dataProvider().maximumValue(100))
        self.assertEqual(layer.dataProvider().maximumValue(1), 'test4')
        self.assertEqual(layer.dataProvider().maximumValue(2), 4)

    def testMaterialize(self):
        """
        Test materializing layers
        """

        layer = createLayerWithFivePoints()
        original_features = {f[0]: f for f in layer.getFeatures()}

        # materialize all features, unchanged
        request = QgsFeatureRequest()
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields(), layer.fields())
        self.assertEqual(new_layer.crs(), layer.crs())
        self.assertEqual(new_layer.featureCount(), 5)
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.Point)
        new_features = {f[0]: f for f in new_layer.getFeatures()}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes(), f.attributes())
            self.assertEqual(new_features[id].geometry().asWkt(), f.geometry().asWkt())

        # materialize with no geometry
        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields(), layer.fields())
        self.assertEqual(new_layer.crs(), layer.crs())
        self.assertEqual(new_layer.featureCount(), 5)
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.NoGeometry)
        new_features = {f[0]: f for f in new_layer.getFeatures()}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes(), f.attributes())

        # materialize with reprojection
        request = QgsFeatureRequest().setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3785'))
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields(), layer.fields())
        self.assertEqual(new_layer.crs().authid(), 'EPSG:3785')
        self.assertEqual(new_layer.featureCount(), 5)
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.Point)
        new_features = {f[0]: f for f in new_layer.getFeatures()}

        expected_geometry = {1: 'Point (111319 222684)',
                             2: 'Point (222639 222684)',
                             3: 'Point (333958 222684)',
                             4: 'Point (445278 334111)',
                             5: 'Point (0 -0)'}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes(), f.attributes())
            self.assertEqual(new_features[id].geometry().asWkt(0), expected_geometry[id])

        # materialize with attribute subset
        request = QgsFeatureRequest().setSubsetOfAttributes([0, 2])
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields().count(), 2)
        self.assertEqual(new_layer.fields().at(0), layer.fields().at(0))
        self.assertEqual(new_layer.fields().at(1), layer.fields().at(2))
        self.assertEqual(new_layer.crs(), layer.crs())
        self.assertEqual(new_layer.featureCount(), 5)
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.Point)
        new_features = {f.attributes()[0]: f for f in new_layer.getFeatures()}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes()[0], f.attributes()[0])
            self.assertEqual(new_features[id].attributes()[1], f.attributes()[2])

        request = QgsFeatureRequest().setSubsetOfAttributes([0, 1])
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields().count(), 2)
        self.assertEqual(new_layer.fields().at(0), layer.fields().at(0))
        self.assertEqual(new_layer.fields().at(1), layer.fields().at(1))
        new_features = {f.attributes()[0]: f for f in new_layer.getFeatures()}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes()[0], f.attributes()[0])
            self.assertEqual(new_features[id].attributes()[1], f.attributes()[1])

        request = QgsFeatureRequest().setSubsetOfAttributes([0])
        new_layer = layer.materialize(request)
        self.assertEqual(new_layer.fields().count(), 1)
        self.assertEqual(new_layer.fields().at(0), layer.fields().at(0))
        new_features = {f.attributes()[0]: f for f in new_layer.getFeatures()}
        for id, f in original_features.items():
            self.assertEqual(new_features[id].attributes()[0], f.attributes()[0])


if __name__ == '__main__':
    unittest.main()
