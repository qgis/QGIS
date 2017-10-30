# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFeatureSink.

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

from qgis.core import (QgsFeatureStore,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsField,
                       QgsFields,
                       QgsCoordinateReferenceSystem,
                       QgsProxyFeatureSink)
from qgis.PyQt.QtCore import QVariant
from qgis.testing import start_app, unittest
start_app()


def createLayerWithFivePoints():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    f2 = QgsFeature()
    f2.setAttributes(["test2", 457])
    f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
    f3 = QgsFeature()
    f3.setAttributes(["test2", 888])
    f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
    f4 = QgsFeature()
    f4.setAttributes(["test3", -1])
    f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(400, 300)))
    f5 = QgsFeature()
    f5.setAttributes(["test4", 0])
    f5.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(0, 0)))
    assert pr.addFeatures([f, f2, f3, f4, f5])
    assert layer.featureCount() == 5
    return layer


class TestQgsFeatureSink(unittest.TestCase):

    def testFromIterator(self):
        """
        Test adding features from an iterator
        :return:
        """
        layer = createLayerWithFivePoints()
        store = QgsFeatureStore(layer.fields(), layer.crs())

        self.assertTrue(store.addFeatures(layer.getFeatures()))
        vals = [f['fldint'] for f in store.features()]
        self.assertEqual(vals, [123, 457, 888, -1, 0])

    def testProxyFeatureSink(self):
        fields = QgsFields()
        fields.append(QgsField('fldtxt', QVariant.String))
        fields.append(QgsField('fldint', QVariant.Int))

        store = QgsFeatureStore(fields, QgsCoordinateReferenceSystem())
        proxy = QgsProxyFeatureSink(store)
        self.assertEqual(proxy.destinationSink(), store)

        self.assertEqual(len(store), 0)

        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        proxy.addFeature(f)
        self.assertEqual(len(store), 1)
        self.assertEqual(store.features()[0]['fldtxt'], 'test')

        f2 = QgsFeature()
        f2.setAttributes(["test2", 457])
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(200, 200)))
        f3 = QgsFeature()
        f3.setAttributes(["test3", 888])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(300, 200)))
        proxy.addFeatures([f2, f3])
        self.assertEqual(len(store), 3)
        self.assertEqual(store.features()[1]['fldtxt'], 'test2')
        self.assertEqual(store.features()[2]['fldtxt'], 'test3')


if __name__ == '__main__':
    unittest.main()
