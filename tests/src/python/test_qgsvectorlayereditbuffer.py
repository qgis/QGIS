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

import os
import qgis  # NOQA
from qgis.PyQt.QtCore import QVariant, QTemporaryDir
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (Qgis,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsProject,
                       QgsGeometry,
                       QgsPointXY,
                       QgsField,
                       QgsVectorFileWriter,
                       QgsCoordinateTransformContext)
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
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])
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
        self.assertCountEqual(layer.editBuffer().allAddedOrEditedFeatures(), [new_feature_ids[0], new_feature_ids[1]])

        # check if error in case adding not adaptable geometry
        # eg. a Multiline in a Line
        layer = createEmptyLinestringLayer()
        self.assertTrue(layer.startEditing())

        self.assertEqual(layer.editBuffer().addedFeatures(), {})
        self.assertFalse(layer.editBuffer().isFeatureAdded(1))
        self.assertFalse(layer.editBuffer().isFeatureAdded(3))
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

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
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])
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
        self.assertCountEqual(layer.editBuffer().allAddedOrEditedFeatures(), [new_feature_ids[0], new_feature_ids[1]])

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
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

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
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

        # change attribute values
        layer.changeAttributeValue(1, 0, 'a')

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedAttributeValues().keys()), [1])
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureAttributesChanged(2))
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1])

        layer.changeAttributeValue(2, 1, 5)

        # test contents of buffer
        self.assertEqual(set(layer.editBuffer().changedAttributeValues().keys()), set([1, 2]))
        self.assertEqual(layer.editBuffer().changedAttributeValues()[1], {0: 'a'})
        self.assertEqual(layer.editBuffer().changedAttributeValues()[2], {1: 5})
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(1))
        self.assertTrue(layer.editBuffer().isFeatureAttributesChanged(2))
        self.assertCountEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1, 2])

    def testChangeGeometry(self):
        # test changing geometries values from an edit buffer

        # make a layer with two features
        layer = createEmptyLayer()
        self.assertTrue(layer.startEditing())
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

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
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

        # change geometry
        layer.changeGeometry(1, QgsGeometry.fromPointXY(QgsPointXY(10, 20)))

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 10)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.undoStack().count(), 1)
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1])

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 10)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        # apply second change to same feature
        layer.beginEditCommand('second change')  # need to use an edit command to avoid the two geometry changes being merged
        layer.changeGeometry(1, QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        layer.endEditCommand()

        # test contents of buffer
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 100)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.undoStack().count(), 2)
        self.assertCountEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1])

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
        self.assertCountEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1, 2])

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 100)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 20)

        layer.undoStack().undo()

        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertEqual(layer.editBuffer().changedGeometries()[1].constGet().x(), 100)
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1])

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 100)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        layer.undoStack().undo()
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [1])
        self.assertTrue(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [1])

        self.assertEqual(layer.getFeature(1).geometry().constGet().x(), 10)
        self.assertEqual(layer.getFeature(2).geometry().constGet().x(), 2)

        layer.undoStack().undo()
        self.assertEqual(list(layer.editBuffer().changedGeometries().keys()), [])
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(1))
        self.assertFalse(layer.editBuffer().isFeatureGeometryChanged(2))
        self.assertEqual(layer.editBuffer().allAddedOrEditedFeatures(), [])

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

    def testTransactionGroup(self):
        """Test that the buffer works the same when used in transaction and when not"""

        def _test(transactionMode):
            """Test buffer methods within and without transactions

            - create a feature
            - save
            - retrieve the feature
            - change geom and attrs
            - test changes are seen in the buffer
            """

            def _check_feature(wkt):

                f = next(layer_a.getFeatures())
                self.assertEqual(f.geometry().asWkt().upper(), wkt)
                f = list(buffer.addedFeatures().values())[0]
                self.assertEqual(f.geometry().asWkt().upper(), wkt)

            ml = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer&field=int2:integer', 'test', 'memory')
            self.assertTrue(ml.isValid())

            d = QTemporaryDir()
            options = QgsVectorFileWriter.SaveVectorOptions()
            options.driverName = 'GPKG'
            options.layerName = 'layer_a'
            err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(ml, os.path.join(d.path(), 'transaction_test.gpkg'), QgsCoordinateTransformContext(), options)

            self.assertEqual(err, QgsVectorFileWriter.NoError)
            self.assertTrue(os.path.isfile(newFileName))

            layer_a = QgsVectorLayer(newFileName + '|layername=layer_a')

            self.assertTrue(layer_a.isValid())

            project = QgsProject()
            project.setTransactionMode(transactionMode)
            project.addMapLayers([layer_a])

            ###########################################
            # Tests with a new feature

            self.assertTrue(layer_a.startEditing())
            buffer = layer_a.editBuffer()

            f = QgsFeature(layer_a.fields())
            f.setAttribute('int', 123)
            f.setGeometry(QgsGeometry.fromWkt('point(7 45)'))
            self.assertTrue(layer_a.addFeatures([f]))

            _check_feature('POINT (7 45)')

            # Need to fetch the feature because its ID is NULL (-9223372036854775808)
            f = next(layer_a.getFeatures())

            self.assertEqual(len(buffer.addedFeatures()), 1)
            layer_a.undoStack().undo()
            self.assertEqual(len(buffer.addedFeatures()), 0)
            layer_a.undoStack().redo()
            self.assertEqual(len(buffer.addedFeatures()), 1)
            f = list(buffer.addedFeatures().values())[0]
            self.assertEqual(f.attribute('int'), 123)

            # Now change attribute
            self.assertEqual(buffer.changedAttributeValues(), {})
            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.changeAttributeValue(f.id(), 1, 321)
            self.assertEqual(len(spy_attribute_changed), 1)
            self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 321])

            self.assertEqual(len(buffer.addedFeatures()), 1)
            # This is surprising: because it was a new feature it has been changed directly
            self.assertEqual(buffer.changedAttributeValues(), {})
            f = list(buffer.addedFeatures().values())[0]
            self.assertEqual(f.attribute('int'), 321)

            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.undoStack().undo()
            self.assertEqual(len(spy_attribute_changed), 1)
            self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 123])
            self.assertEqual(buffer.changedAttributeValues(), {})
            f = list(buffer.addedFeatures().values())[0]
            self.assertEqual(f.attribute('int'), 123)
            f = next(layer_a.getFeatures())
            self.assertEqual(f.attribute('int'), 123)

            # Change multiple attributes
            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.changeAttributeValues(f.id(), {1: 321, 2: 456})
            self.assertEqual(len(spy_attribute_changed), 2)
            self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 321])
            self.assertEqual(spy_attribute_changed[1], [f.id(), 2, 456])
            buffer = layer_a.editBuffer()
            # This is surprising: because it was a new feature it has been changed directly
            self.assertEqual(buffer.changedAttributeValues(), {})

            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.undoStack().undo()
            # This is because QgsVectorLayerUndoCommandChangeAttribute plural
            if transactionMode != Qgis.TransactionMode.AutomaticGroups:
                layer_a.undoStack().undo()
            f = next(layer_a.getFeatures())
            self.assertEqual(f.attribute('int'), 123)
            self.assertEqual(f.attribute('int2'), None)
            self.assertEqual(len(spy_attribute_changed), 2)
            if transactionMode == Qgis.TransactionMode.AutomaticGroups:
                self.assertEqual(spy_attribute_changed[1], [f.id(), 2, None])
                self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 123])
            else:
                self.assertEqual(spy_attribute_changed[0], [f.id(), 2, None])
                self.assertEqual(spy_attribute_changed[1], [f.id(), 1, 123])

            # Change geometry
            f = next(layer_a.getFeatures())
            spy_geometry_changed = QSignalSpy(layer_a.geometryChanged)
            self.assertTrue(layer_a.changeGeometry(f.id(), QgsGeometry.fromWkt('point(9 43)')))
            self.assertTrue(len(spy_geometry_changed), 1)
            self.assertEqual(spy_geometry_changed[0][0], f.id())
            self.assertEqual(spy_geometry_changed[0][1].asWkt(), QgsGeometry.fromWkt('point(9 43)').asWkt())

            _check_feature('POINT (9 43)')
            self.assertEqual(buffer.changedGeometries(), {})

            layer_a.undoStack().undo()

            _check_feature('POINT (7 45)')
            self.assertEqual(buffer.changedGeometries(), {})

            self.assertTrue(layer_a.changeGeometry(f.id(), QgsGeometry.fromWkt('point(9 43)')))
            _check_feature('POINT (9 43)')

            self.assertTrue(layer_a.changeGeometry(f.id(), QgsGeometry.fromWkt('point(10 44)')))
            _check_feature('POINT (10 44)')

            # This is another surprise: geometry edits get collapsed into a single
            # one because they have the same hardcoded id
            layer_a.undoStack().undo()
            _check_feature('POINT (7 45)')

            self.assertTrue(layer_a.commitChanges())

            ###########################################
            # Tests with the existing feature

            # Get the feature
            f = next(layer_a.getFeatures())
            self.assertTrue(f.isValid())
            self.assertEqual(f.attribute('int'), 123)
            self.assertEqual(f.geometry().asWkt().upper(), 'POINT (7 45)')

            # Change single attribute
            self.assertTrue(layer_a.startEditing())
            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.changeAttributeValue(f.id(), 1, 321)
            self.assertEqual(len(spy_attribute_changed), 1)
            self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 321])
            buffer = layer_a.editBuffer()
            self.assertEqual(buffer.changedAttributeValues(), {1: {1: 321}})

            f = next(layer_a.getFeatures())
            self.assertEqual(f.attribute(1), 321)

            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.undoStack().undo()
            f = next(layer_a.getFeatures())
            self.assertEqual(f.attribute(1), 123)
            self.assertEqual(len(spy_attribute_changed), 1)
            self.assertEqual(spy_attribute_changed[0], [f.id(), 1, 123])
            self.assertEqual(buffer.changedAttributeValues(), {})

            # Change attributes
            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.changeAttributeValues(f.id(), {1: 111, 2: 654})
            self.assertEqual(len(spy_attribute_changed), 2)
            self.assertEqual(spy_attribute_changed[0], [1, 1, 111])
            self.assertEqual(spy_attribute_changed[1], [1, 2, 654])
            f = next(layer_a.getFeatures())
            self.assertEqual(f.attributes(), [1, 111, 654])
            self.assertEqual(buffer.changedAttributeValues(), {1: {1: 111, 2: 654}})

            spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
            layer_a.undoStack().undo()
            # This is because QgsVectorLayerUndoCommandChangeAttribute plural
            if transactionMode != Qgis.TransactionMode.AutomaticGroups:
                layer_a.undoStack().undo()
            self.assertEqual(len(spy_attribute_changed), 2)
            if transactionMode == Qgis.TransactionMode.AutomaticGroups:
                self.assertEqual(spy_attribute_changed[0], [1, 1, 123])
                self.assertEqual(spy_attribute_changed[1], [1, 2, None])
            else:
                self.assertEqual(spy_attribute_changed[1], [1, 1, 123])
                self.assertEqual(spy_attribute_changed[0], [1, 2, None])
            f = next(layer_a.getFeatures())
            self.assertEqual(f.attributes(), [1, 123, None])
            self.assertEqual(buffer.changedAttributeValues(), {})

            # Change geometry
            spy_geometry_changed = QSignalSpy(layer_a.geometryChanged)
            self.assertTrue(layer_a.changeGeometry(f.id(), QgsGeometry.fromWkt('point(9 43)')))
            self.assertEqual(spy_geometry_changed[0][0], 1)
            self.assertEqual(spy_geometry_changed[0][1].asWkt(), QgsGeometry.fromWkt('point(9 43)').asWkt())

            f = next(layer_a.getFeatures())
            self.assertEqual(f.geometry().asWkt().upper(), 'POINT (9 43)')
            self.assertEqual(buffer.changedGeometries()[1].asWkt().upper(), 'POINT (9 43)')

            spy_geometry_changed = QSignalSpy(layer_a.geometryChanged)
            layer_a.undoStack().undo()
            self.assertEqual(spy_geometry_changed[0][0], 1)
            self.assertEqual(spy_geometry_changed[0][1].asWkt(), QgsGeometry.fromWkt('point(7 45)').asWkt())
            self.assertEqual(buffer.changedGeometries(), {})
            f = next(layer_a.getFeatures())

            self.assertEqual(f.geometry().asWkt().upper(), 'POINT (7 45)')
            self.assertEqual(buffer.changedGeometries(), {})

            # Delete an existing feature
            self.assertTrue(layer_a.deleteFeature(f.id()))
            with self.assertRaises(StopIteration):
                next(layer_a.getFeatures())
            self.assertEqual(buffer.deletedFeatureIds(), [f.id()])

            layer_a.undoStack().undo()
            self.assertTrue(layer_a.getFeature(f.id()).isValid())
            self.assertEqual(buffer.deletedFeatureIds(), [])

            ###########################################
            # Test delete

            # Delete a new feature
            f = QgsFeature(layer_a.fields())
            f.setAttribute('int', 555)
            f.setGeometry(QgsGeometry.fromWkt('point(8 46)'))
            self.assertTrue(layer_a.addFeatures([f]))
            f = [f for f in layer_a.getFeatures() if f.attribute('int') == 555][0]
            self.assertTrue(f.id() in buffer.addedFeatures())
            self.assertTrue(layer_a.deleteFeature(f.id()))
            self.assertFalse(f.id() in buffer.addedFeatures())
            self.assertFalse(f.id() in buffer.deletedFeatureIds())

            layer_a.undoStack().undo()
            self.assertTrue(f.id() in buffer.addedFeatures())

            ###########################################
            # Add attribute

            field = QgsField('attr1', QVariant.String)
            self.assertTrue(layer_a.addAttribute(field))
            self.assertNotEqual(layer_a.fields().lookupField(field.name()), -1)
            self.assertEqual(buffer.addedAttributes(), [field])

            layer_a.undoStack().undo()
            self.assertEqual(layer_a.fields().lookupField(field.name()), -1)
            self.assertEqual(buffer.addedAttributes(), [])

            layer_a.undoStack().redo()
            self.assertNotEqual(layer_a.fields().lookupField(field.name()), -1)
            self.assertEqual(buffer.addedAttributes(), [field])

            self.assertTrue(layer_a.commitChanges())

            ###########################################
            # Remove attribute

            self.assertTrue(layer_a.startEditing())
            buffer = layer_a.editBuffer()

            attr_idx = layer_a.fields().lookupField(field.name())
            self.assertNotEqual(attr_idx, -1)

            self.assertTrue(layer_a.deleteAttribute(attr_idx))
            self.assertEqual(buffer.deletedAttributeIds(), [attr_idx])
            self.assertEqual(layer_a.fields().lookupField(field.name()), -1)

            layer_a.undoStack().undo()
            self.assertEqual(buffer.deletedAttributeIds(), [])
            self.assertEqual(layer_a.fields().lookupField(field.name()), attr_idx)

            # This is totally broken at least on OGR/GPKG: the rollback
            # does not restore the original fields
            if False:

                layer_a.undoStack().redo()
                self.assertEqual(buffer.deletedAttributeIds(), [attr_idx])
                self.assertEqual(layer_a.fields().lookupField(field.name()), -1)

                # Rollback!
                self.assertTrue(layer_a.rollBack())

                self.assertIn('attr1', layer_a.dataProvider().fields().names())
                self.assertIn('attr1', layer_a.fields().names())
                self.assertEqual(layer_a.fields().names(), layer_a.dataProvider().fields().names())

                attr_idx = layer_a.fields().lookupField(field.name())
                self.assertNotEqual(attr_idx, -1)

                self.assertTrue(layer_a.startEditing())
                attr_idx = layer_a.fields().lookupField(field.name())
                self.assertNotEqual(attr_idx, -1)

            ###########################################
            # Rename attribute

            attr_idx = layer_a.fields().lookupField(field.name())
            self.assertEqual(layer_a.fields().lookupField('new_name'), -1)
            self.assertTrue(layer_a.renameAttribute(attr_idx, 'new_name'))
            self.assertEqual(layer_a.fields().lookupField('new_name'), attr_idx)

            layer_a.undoStack().undo()
            self.assertEqual(layer_a.fields().lookupField(field.name()), attr_idx)
            self.assertEqual(layer_a.fields().lookupField('new_name'), -1)

            layer_a.undoStack().redo()
            self.assertEqual(layer_a.fields().lookupField('new_name'), attr_idx)
            self.assertEqual(layer_a.fields().lookupField(field.name()), -1)

            #############################################
            # Try hard to make this fail for transactions
            if (transactionMode == Qgis.TransactionMode.AutomaticGroups
                    or transactionMode == Qgis.TransactionMode.BufferedGroups):
                self.assertTrue(layer_a.commitChanges())
                self.assertTrue(layer_a.startEditing())
                f = next(layer_a.getFeatures())

                # Do
                for i in range(10):
                    spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
                    layer_a.changeAttributeValue(f.id(), 2, i)
                    self.assertEqual(len(spy_attribute_changed), 1)
                    self.assertEqual(spy_attribute_changed[0], [f.id(), 2, i])
                    buffer = layer_a.editBuffer()
                    self.assertEqual(buffer.changedAttributeValues(), {f.id(): {2: i}})
                    f = next(layer_a.getFeatures())
                    self.assertEqual(f.attribute(2), i)

                # Undo/redo
                for i in range(9):

                    # Undo
                    spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
                    layer_a.undoStack().undo()
                    f = next(layer_a.getFeatures())
                    self.assertEqual(f.attribute(2), 8 - i)
                    self.assertEqual(len(spy_attribute_changed), 1)
                    self.assertEqual(spy_attribute_changed[0], [f.id(), 2, 8 - i])
                    buffer = layer_a.editBuffer()
                    self.assertEqual(buffer.changedAttributeValues(), {f.id(): {2: 8 - i}})

                    # Redo
                    spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
                    layer_a.undoStack().redo()
                    f = next(layer_a.getFeatures())
                    self.assertEqual(f.attribute(2), 9 - i)
                    self.assertEqual(len(spy_attribute_changed), 1)
                    self.assertEqual(spy_attribute_changed[0], [f.id(), 2, 9 - i])

                    # Undo again
                    spy_attribute_changed = QSignalSpy(layer_a.attributeValueChanged)
                    layer_a.undoStack().undo()
                    f = next(layer_a.getFeatures())
                    self.assertEqual(f.attribute(2), 8 - i)
                    self.assertEqual(len(spy_attribute_changed), 1)
                    self.assertEqual(spy_attribute_changed[0], [f.id(), 2, 8 - i])
                    buffer = layer_a.editBuffer()
                    self.assertEqual(buffer.changedAttributeValues(), {f.id(): {2: 8 - i}})

                    # Last check
                    f = next(layer_a.getFeatures())
                    self.assertEqual(f.attribute(2), 8 - i)

                self.assertEqual(buffer.changedAttributeValues(), {f.id(): {2: 0}})
                layer_a.undoStack().undo()
                buffer = layer_a.editBuffer()
                self.assertEqual(buffer.changedAttributeValues(), {})
                f = next(layer_a.getFeatures())
                self.assertEqual(f.attribute(2), None)

        _test(Qgis.TransactionMode.Disabled)
        _test(Qgis.TransactionMode.AutomaticGroups)
        _test(Qgis.TransactionMode.BufferedGroups)


if __name__ == '__main__':
    unittest.main()
