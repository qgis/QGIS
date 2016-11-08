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
                       QgsFieldConstraints,
                       QgsFields,
                       QgsFeature,
                       NULL
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

    def test_validate_attribute(self):
        """ test validating attributes against constraints """
        layer = createLayerWithOnePoint()

        # field expression check
        layer.setConstraintExpression(1, 'fldint>5')

        f = QgsFeature(2)
        f.setAttributes(["test123", 6])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        f.setAttributes(["test123", 2])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)
        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # bad field expression check
        layer.setConstraintExpression(1, 'fldint>')
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        layer.setConstraintExpression(1, None)

        # not null constraint
        f.setAttributes(["test123", NULL])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # unique constraint
        f.setAttributes(["test123", 123])
        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)
        print(errors)

        # checking only for provider constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, origin=QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # checking only for soft constraints
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintUnique, QgsFieldConstraints.ConstraintStrengthHard)
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1, strength=QgsFieldConstraints.ConstraintStrengthSoft)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)
        # checking for hard constraints
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1,
                                                            strength=QgsFieldConstraints.ConstraintStrengthHard)
        self.assertFalse(res)
        self.assertEqual(len(errors), 1)

        # check - same id should be ignored when testing for uniqueness
        f1 = QgsFeature(1)
        f1.setAttributes(["test123", 123])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f1, 1)
        self.assertTrue(res)
        self.assertEqual(len(errors), 0)

        # test double constraint failure
        layer.setConstraintExpression(1, 'fldint>5')
        layer.removeFieldConstraint(1, QgsFieldConstraints.ConstraintUnique)
        layer.setFieldConstraint(1, QgsFieldConstraints.ConstraintNotNull)
        f.setAttributes(["test123", NULL])
        res, errors = QgsVectorLayerUtils.validateAttribute(layer, f, 1)
        self.assertFalse(res)
        self.assertEqual(len(errors), 2)
        print(errors)

    def testCreateUniqueValue(self):
        """ test creating a unique value """
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double",
                               "addfeat", "memory")
        # add a bunch of features
        f = QgsFeature()
        f.setAttributes(["test", 123, 1.0])
        f1 = QgsFeature(2)
        f1.setAttributes(["test_1", 124, 1.1])
        f2 = QgsFeature(3)
        f2.setAttributes(["test_2", 125, 2.4])
        f3 = QgsFeature(4)
        f3.setAttributes(["test_3", 126, 1.7])
        f4 = QgsFeature(5)
        f4.setAttributes(["superpig", 127, 0.8])
        self.assertTrue(layer.dataProvider().addFeatures([f, f1, f2, f3, f4]))

        # bad field indices
        self.assertFalse(QgsVectorLayerUtils.createUniqueValue(layer, -10))
        self.assertFalse(QgsVectorLayerUtils.createUniqueValue(layer, 10))

        # integer field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 1), 128)

        # double field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 2), 3.0)

        # string field
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0), 'test_4')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'test_1'), 'test_4')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'seed'), 'seed')
        self.assertEqual(QgsVectorLayerUtils.createUniqueValue(layer, 0, 'superpig'), 'superpig_1')

if __name__ == '__main__':
    unittest.main()
