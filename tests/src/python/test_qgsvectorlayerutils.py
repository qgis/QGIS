# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsVectorLayer,
                       QgsVectorLayerUtils,
                       QgsField,
                       QgsFields,
                       QgsFeature
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
start_app()


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    assert pr.addFeatures([f])
    assert layer.pendingFeatureCount() == 1
    return layer


class TestQgsVectorLayerUtils(unittest.TestCase):

    def test_value_exists(self):
        layer = createLayerWithOnePoint()
        # add some more features
        f1 = QgsFeature(2)
        f1.setAttributes(["test1", 124])
        f2 = QgsFeature(3)
        f2.setAttributes(["test2", 125])
        f3 = QgsFeature(4)
        f3.setAttributes(["test3", 126])
        f4 = QgsFeature(5)
        f4.setAttributes(["test4", 127])
        layer.dataProvider().addFeatures([f1, f2, f3, f4])

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test4'))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'not present!'))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 123))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 124))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 127))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 99))

        # no layer
        self.assertFalse(QgsVectorLayerUtils.valueExists(None, 1, 123))
        # bad field indexes
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, -1, 'test'))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 100, 'test'))

        # with ignore list
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [3, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [2]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [99999, 2]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 0, 'test1', [3, 4, 5, 2]))

        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5]))
        self.assertTrue(QgsVectorLayerUtils.valueExists(layer, 1, 125, [999999]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [99999, 3]))
        self.assertFalse(QgsVectorLayerUtils.valueExists(layer, 1, 125, [2, 4, 5, 3]))


if __name__ == '__main__':
    unittest.main()
