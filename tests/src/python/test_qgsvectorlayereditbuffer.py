# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerEditBuffer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '15/07/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsField)
from qgis.testing import start_app, unittest
start_app()


def createEmptyLayer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    assert layer.isValid()
    return layer


def createLayerWithOnePoint():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    pr = layer.dataProvider()
    f = QgsFeature()
    f.setAttributes(["test", 123])
    f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
    assert pr.addFeatures([f])
    assert layer.pendingFeatureCount() == 1
    return layer


def createEmptyLinestringLayer():
    layer = QgsVectorLayer("Linestring?field=fldtxt:string&field=fldint:integer",
                           "addfeat", "memory")
    assert layer.isValid()
    return layer


class TestQgsVectorLayerEditBuffer(unittest.TestCase):

    def testAddFeatures(self):
        # test adding features to an edit buffer
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(1 in layer.editBuffer().addedFeatures())
        self.assertFalse(3 in layer.editBuffer().addedFeatures())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])

        self.assertTrue(layer.addFeature(f2))

        # test contents of buffer
        added = layer.editBuffer().addedFeatures()
        new_feature_ids = list(added.keys())
        self.assertEqual(added[new_feature_ids[0]]['fldtxt'], 'test2')
        self.assertEqual(added[new_feature_ids[0]]['fldint'], 246)
        self.assertEqual(added[new_feature_ids[1]]['fldtxt'], 'test')
        self.assertEqual(added[new_feature_ids[1]]['fldint'], 123)

        self.assertTrue(new_feature_ids[0] in layer.editBuffer().addedFeatures())
        self.assertTrue(new_feature_ids[1] in layer.editBuffer().addedFeatures())

        # check if error in case adding not adaptable geometry
        # eg. a Multiline in a Line
        layer = createEmptyLinestringLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(1 in layer.editBuffer().addedFeatures())
        self.assertFalse(3 in layer.editBuffer().addedFeatures())

        # add a features with a multi line geometry of not touched lines =>
        # cannot be forced to be linestring
        multiline = [
            [QgsPoint(1, 1), QgsPoint(2, 2)],
            [QgsPoint(3, 3), QgsPoint(4, 4)],
        ]
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromMultiPolyline(multiline))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeatures([f1]))
        self.assertFalse(layer.commitChanges())

    def testAddMultipleFeatures(self):
        # test adding multiple features to an edit buffer
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(1 in layer.editBuffer().addedFeatures())
        self.assertFalse(3 in layer.editBuffer().addedFeatures())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])

        self.assertTrue(layer.addFeatures([f1, f2]))

        # test contents of buffer
        added = layer.editBuffer().addedFeatures()
        new_feature_ids = list(added.keys())
        self.assertEqual(added[new_feature_ids[0]]['fldtxt'], 'test2')
        self.assertEqual(added[new_feature_ids[0]]['fldint'], 246)
        self.assertEqual(added[new_feature_ids[1]]['fldtxt'], 'test')
        self.assertEqual(added[new_feature_ids[1]]['fldint'], 123)

        self.assertTrue(new_feature_ids[0] in layer.editBuffer().addedFeatures())
        self.assertTrue(new_feature_ids[1] in layer.editBuffer().addedFeatures())

    def testDeleteFeatures(self):
        # test deleting features from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [])
        self.assertFalse(1 in layer.editBuffer().deletedFeatureIds())
        self.assertFalse(2 in layer.editBuffer().deletedFeatureIds())

        # delete a feature
        layer.deleteFeature(1)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [1])
        self.assertTrue(1 in layer.editBuffer().deletedFeatureIds())
        self.assertFalse(2 in layer.editBuffer().deletedFeatureIds())

        # delete a feature
        layer.deleteFeature(2)

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().deletedFeatureIds()), set([1, 2]))
        self.assertTrue(1 in layer.editBuffer().deletedFeatureIds())
        self.assertTrue(2 in layer.editBuffer().deletedFeatureIds())

    def testDeleteMultipleFeatures(self):
        # test deleting multiple features from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [])
        self.assertFalse(1 in layer.editBuffer().addedFeatures())
        self.assertFalse(2 in layer.editBuffer().addedFeatures())

        # delete features
        layer.deleteFeatures([1, 2])

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().deletedFeatureIds()), set([1, 2]))
        self.assertTrue(1 in layer.editBuffer().deletedFeatureIds())
        self.assertTrue(2 in layer.editBuffer().deletedFeatureIds())

    def testChangeAttributeValues(self):
        # test changing attributes values from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().changedAttributeValues(), {})
        self.assertFalse(1 in layer.editBuffer().changedAttributeValues())
        self.assertFalse(2 in layer.editBuffer().changedAttributeValues())

        # change attribute values
        layer.changeAttributeValue(1, 0, 'a')

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedAttributeValues().keys()), [1])
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertTrue(1 in layer.editBuffer().changedAttributeValues())
        self.assertFalse(2 in layer.editBuffer().changedAttributeValues())

        layer.changeAttributeValue(2, 1, 5)

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().changedAttributeValues().keys()), set([1, 2]))
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertEqual(layer.editBuffer().changedAttributeValues()[2], {1: 5})
        self.assertTrue(1 in layer.editBuffer().changedAttributeValues())
        self.assertTrue(2 in layer.editBuffer().changedAttributeValues())

    def testChangeGeometry(self):
        # test changing geometries values from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPoint(QgsPoint(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPoint(QgsPoint(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().changedGeometries(), {})
        self.assertFalse(1 in layer.editBuffer().changedGeometries())
        self.assertFalse(2 in layer.editBuffer().changedGeometries())

        # change attribute values
        layer.changeGeometry(1, QgsGeometry.fromPoint(QgsPoint(10, 20)))

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].geometry().x(), 10)
        self.assertTrue(1 in layer.editBuffer().changedGeometries())
        self.assertFalse(2 in layer.editBuffer().changedGeometries())

        layer.changeGeometry(2, QgsGeometry.fromPoint(QgsPoint(20, 40)))

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().changedGeometries().keys()), set([1, 2]))
        self.assertEqual(layer.editBuffer().changedGeometries()[1].geometry().x(), 10)
        self.assertEqual(layer.editBuffer().changedGeometries()[2].geometry().x(), 20)
        self.assertTrue(1 in layer.editBuffer().changedGeometries())
        self.assertTrue(2 in layer.editBuffer().changedGeometries())

    def testDeleteAttribute(self):
        # test deleting attributes from an edit buffer

        layer = createEmptyLayer()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [])
        self.assertFalse(0 in layer.editBuffer().deletedAttributeIds())
        self.assertFalse(1 in layer.editBuffer().deletedAttributeIds())

        # delete attribute
        layer.deleteAttribute(0)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [0])
        self.assertTrue(0 in layer.editBuffer().deletedAttributeIds())
        self.assertFalse(1 in layer.editBuffer().deletedAttributeIds())

        # delete remaining attribute (now at position 0)
        layer.deleteAttribute(0)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [0, 1])
        self.assertTrue(0 in layer.editBuffer().deletedAttributeIds())
        self.assertTrue(1 in layer.editBuffer().deletedAttributeIds())

    def testAddAttribute(self):
        # test adding attributes to an edit buffer

        layer = createEmptyLayer()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().addedAttributes(), [])

        # add attribute
        layer.addAttribute(QgsField('new1', QVariant.String))

        # test contents of buffer
        self.assertEqual(layer.editBuffer().addedAttributes()[0].name(), 'new1')

        # add another attribute
        layer.addAttribute(QgsField('new2', QVariant.String))

        # test contents of buffer
        self.assertEqual(layer.editBuffer().addedAttributes()[0].name(), 'new1')
        self.assertEqual(layer.editBuffer().addedAttributes()[1].name(), 'new2')


if __name__ == '__main__':
    unittest.main()
