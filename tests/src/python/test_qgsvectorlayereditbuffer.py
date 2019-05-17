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

import qgis  # NOQA

from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
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
    f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
    assert pr.addFeatures([f])
    assert layer.featureCount() == 1
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
        self.assertFalse(layer.editBuffer().isFeatureAdded(1))
        self.assertFalse(layer.editBuffer().isFeatureAdded(3))

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])

        self.assertTrue(layer.addFeature(f2))

        # test contents of buffer
        added = layer.editBuffer().addedFeatures()
        new_feature_ids = list(added.keys())
        self.assertEqual(added[new_feature_ids[0]]['fldtxt'], 'test2')
        self.assertEqual(added[new_feature_ids[0]]['fldint'], 246)
        self.assertEqual(added[new_feature_ids[1]]['fldtxt'], 'test')
        self.assertEqual(added[new_feature_ids[1]]['fldint'], 123)

        self.assertTrue(layer.editBuffer().isFeatureAdded(new_feature_ids[0]))
        self.assertTrue(layer.editBuffer().isFeatureAdded(new_feature_ids[1]))

        # check if error in case adding not adaptable geometry
        # eg. a Multiline in a Line
        layer = createEmptyLinestringLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(layer.editBuffer().isFeatureAdded(1))
        self.assertFalse(layer.editBuffer().isFeatureAdded(3))

        # add a features with a multi line geometry of not touched lines =>
        # cannot be forced to be linestring
        multiline = [
            [QgsPointXY(1, 1), QgsPointXY(2, 2)],
            [QgsPointXY(3, 3), QgsPointXY(4, 4)],
        ]
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromMultiPolylineXY(multiline))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeatures([f1]))
        self.assertFalse(layer.commitChanges())

    def testAddMultipleFeatures(self):
        # test adding multiple features to an edit buffer
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(layer.editBuffer().isFeatureAdded(1))
        self.assertFalse(layer.editBuffer().isFeatureAdded(3))

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])

        self.assertTrue(layer.addFeatures([f1, f2]))

        # test contents of buffer
        added = layer.editBuffer().addedFeatures()
        new_feature_ids = list(added.keys())
        self.assertEqual(added[new_feature_ids[0]]['fldtxt'], 'test2')
        self.assertEqual(added[new_feature_ids[0]]['fldint'], 246)
        self.assertEqual(added[new_feature_ids[1]]['fldtxt'], 'test')
        self.assertEqual(added[new_feature_ids[1]]['fldint'], 123)

        self.assertTrue(layer.editBuffer().isFeatureAdded(new_feature_ids[0]))
        self.assertTrue(layer.editBuffer().isFeatureAdded(new_feature_ids[1]))

    def testDeleteFeatures(self):
        # test deleting features from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [])
        self.assertFalse(layer.editBuffer().isFeatureDeleted(1))
        self.assertFalse(layer.editBuffer().isFeatureDeleted(2))

        # delete a feature
        layer.deleteFeature(1)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [1])
        self.assertTrue(layer.editBuffer().isFeatureDeleted(1))
        self.assertFalse(layer.editBuffer().isFeatureDeleted(2))

        # delete a feature
        layer.deleteFeature(2)

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().deletedFeatureIds()), set([1, 2]))
        self.assertTrue(layer.editBuffer().isFeatureDeleted(1))
        self.assertTrue(layer.editBuffer().isFeatureDeleted(2))

    def testDeleteMultipleFeatures(self):
        # test deleting multiple features from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedFeatureIds(), [])
        self.assertFalse(layer.editBuffer().isFeatureDeleted(1))
        self.assertFalse(layer.editBuffer().isFeatureDeleted(2))

        # delete features
        layer.deleteFeatures([1, 2])

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().deletedFeatureIds()), set([1, 2]))
        self.assertTrue(layer.editBuffer().isFeatureDeleted(1))
        self.assertTrue(layer.editBuffer().isFeatureDeleted(2))

    def testChangeAttributeValues(self):
        # test changing attributes values from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().changedAttributeValues(), {})
        self.assertFalse(layer.editBuffer().isFeatureAttributesChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureAttributesChanged(2))

        # change attribute values
        layer.changeAttributeValue(1, 0, 'a')

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedAttributeValues().keys()), [1])
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureAttributesChanged(2))

        layer.changeAttributeValue(2, 1, 5)

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().changedAttributeValues().keys()), set([1, 2]))
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertEqual(layer.editBuffer().changedAttributeValues()[2], {1: 5})
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(1))
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(2))

    def testChangeGeometry(self):
        # test changing geometries values from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())

        # add two features
        f1 = QgsFeature(layer.fields(), 1)
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f1.setAttributes(["test", 123])
        self.assertTrue(layer.addFeature(f1))

        f2 = QgsFeature(layer.fields(), 2)
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2, 4)))
        f2.setAttributes(["test2", 246])
        self.assertTrue(layer.addFeature(f2))

        layer.commitChanges()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().changedGeometries(), {})
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))

        # change geometry
        layer.changeGeometry(1, QgsGeometry.fromPointXY(QgsPointXY(10, 20)))

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 10)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.undoStack().count(), 1)

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 10)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        # apply second change to same feature
        layer.beginEditCommand('second change') # need to use an edit command to avoid the two geometry changes being merged
        layer.changeGeometry(1, QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        layer.endEditCommand()

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 100)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.undoStack().count(), 2)

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 100)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        layer.changeGeometry(2, QgsGeometry.fromPointXY(QgsPointXY(20, 40)))

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().changedGeometries().keys()), set([1, 2]))
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 100)
        self.assertEqual(layer.editBuffer().changedGeometries()[2].constGet().x(), 20)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.undoStack().count(), 3)

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 100)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 20)

        layer.undoStack().undo()

        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 100)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 100)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        layer.undoStack().undo()
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 10)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        layer.undoStack().undo()
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [])
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 1)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

    def testDeleteAttribute(self):
        # test deleting attributes from an edit buffer

        layer = createEmptyLayer()
        layer.startEditing()

        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [])
        self.assertFalse(layer.editBuffer().isAttributeDeleted(0))
        self.assertFalse(layer.editBuffer().isAttributeDeleted(1))

        # delete attribute
        layer.deleteAttribute(0)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [0])
        self.assertTrue(layer.editBuffer().isAttributeDeleted(0))
        self.assertFalse(layer.editBuffer().isAttributeDeleted(1))

        # delete remaining attribute (now at position 0)
        layer.deleteAttribute(0)

        # test contents of buffer
        self.assertEqual(layer.editBuffer().deletedAttributeIds(), [0, 1])
        self.assertTrue(layer.editBuffer().isAttributeDeleted(0))
        self.assertTrue(layer.editBuffer().isAttributeDeleted(1))

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
