"""QGIS Unit tests for QgsVectorLayerEditBufferGroup.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Damiano Lombardi"
__date__ = "13/01/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import os

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import (
    Qgis,
    QgsCoordinateTransformContext,
    QgsFeature,
    QgsGeometry,
    QgsProject,
    QgsRelation,
    QgsRelationContext,
    QgsVectorFileWriter,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsVectorLayerEditBufferGroup(QgisTestCase):

    def tearDown(self):
        """Run after each test."""
        QgsProject.instance().removeAllMapLayers()

    def testStartEditingCommitRollBack(self):

        ml = QgsVectorLayer(
            "Point?crs=epsg:4326&field=int:integer&field=int2:integer", "test", "memory"
        )
        self.assertTrue(ml.isValid())

        # Layer A geopackage A
        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        options.layerName = "layer_a"
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            ml,
            os.path.join(d.path(), "test_EditBufferGroup_A.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        self.assertEqual(err, QgsVectorFileWriter.WriterError.NoError)
        self.assertTrue(os.path.isfile(newFileName))

        layer_a = QgsVectorLayer(newFileName + "|layername=layer_a")

        self.assertTrue(layer_a.isValid())

        # Layer B geopackage B
        options.layerName = "layer_b"
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            ml,
            os.path.join(d.path(), "test_EditBufferGroup_B.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        self.assertEqual(err, QgsVectorFileWriter.WriterError.NoError)
        self.assertTrue(os.path.isfile(newFileName))

        layer_b = QgsVectorLayer(newFileName + "|layername=layer_b")

        self.assertTrue(layer_b.isValid())

        # Layer C memory
        layer_c = QgsVectorLayer(
            "Point?crs=epsg:4326&field=int:integer&field=int2:integer", "test", "memory"
        )
        self.assertTrue(layer_c.isValid())

        project = QgsProject()
        project.addMapLayers([layer_a, layer_b, layer_c])
        project.setTransactionMode(Qgis.TransactionMode.BufferedGroups)

        editBufferGroup = project.editBufferGroup()

        # Check layers in group
        self.assertIn(layer_a, editBufferGroup.layers())
        self.assertIn(layer_b, editBufferGroup.layers())
        self.assertIn(layer_c, editBufferGroup.layers())

        self.assertFalse(editBufferGroup.isEditing())

        self.assertTrue(editBufferGroup.startEditing())
        self.assertTrue(editBufferGroup.isEditing())
        self.assertTrue(layer_a.editBuffer())
        self.assertTrue(layer_b.editBuffer())
        self.assertTrue(layer_c.editBuffer())
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 0)

        success, commitErrors = editBufferGroup.commitChanges(False)
        self.assertTrue(success)
        self.assertTrue(editBufferGroup.isEditing())
        success, commitErrors = editBufferGroup.commitChanges(True)
        self.assertTrue(success)
        self.assertFalse(editBufferGroup.isEditing())

        self.assertTrue(editBufferGroup.startEditing())
        self.assertTrue(editBufferGroup.isEditing())

        f = QgsFeature(layer_a.fields())
        f.setAttribute("int", 123)
        f.setGeometry(QgsGeometry.fromWkt("point(7 45)"))
        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 1)
        self.assertIn(layer_a, editBufferGroup.modifiedLayers())

        # Check feature in layer edit buffer but not in provider till commit
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)

        success, rollbackErrors = editBufferGroup.rollBack(False)
        self.assertTrue(success)
        self.assertTrue(editBufferGroup.isEditing())
        self.assertEqual(layer_a.featureCount(), 0)

        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)

        success, commitErrors = editBufferGroup.commitChanges(True)
        self.assertTrue(success)
        self.assertFalse(editBufferGroup.isEditing())
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 1)

    def testSetBufferedGroupsAfterAutomaticGroups(self):

        ml = QgsVectorLayer(
            "Point?crs=epsg:4326&field=int:integer&field=int2:integer", "test", "memory"
        )

        # Load 2 layer from a geopackage
        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        options.layerName = "layer_a"
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            ml,
            os.path.join(d.path(), "test_EditBufferGroup.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        options.layerName = "layer_b"
        options.actionOnExistingFile = (
            QgsVectorFileWriter.ActionOnExistingFile.CreateOrOverwriteLayer
        )
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            ml,
            os.path.join(d.path(), "test_EditBufferGroup.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        layer_a = QgsVectorLayer(newFileName + "|layername=layer_a")
        self.assertTrue(layer_a.isValid())
        layer_b = QgsVectorLayer(newFileName + "|layername=layer_b")
        self.assertTrue(layer_b.isValid())

        project = QgsProject()
        project.addMapLayers([layer_a, layer_b, ml])

        project.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        project.setTransactionMode(Qgis.TransactionMode.BufferedGroups)

        project.startEditing()
        success, rollbackErrors = project.rollBack(True)

        self.assertTrue(success)

    def testReadOnlyLayers(self):

        memoryLayer_a = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer&field=id_b", "test", "memory"
        )
        self.assertTrue(memoryLayer_a.isValid())
        memoryLayer_b = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer", "test", "memory"
        )
        self.assertTrue(memoryLayer_b.isValid())

        # Load 2 layer from a geopackage
        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        options.layerName = "layer_a"
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            memoryLayer_a,
            os.path.join(d.path(), "test_EditBufferGroupReadOnly.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        options.layerName = "layer_b"
        options.actionOnExistingFile = (
            QgsVectorFileWriter.ActionOnExistingFile.CreateOrOverwriteLayer
        )
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            memoryLayer_b,
            os.path.join(d.path(), "test_EditBufferGroupReadOnly.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        layer_a = QgsVectorLayer(newFileName + "|layername=layer_a")
        self.assertTrue(layer_a.isValid())
        layer_b = QgsVectorLayer(newFileName + "|layername=layer_b")
        self.assertTrue(layer_b.isValid())
        layer_b.setReadOnly(True)

        project = QgsProject.instance()
        project.addMapLayers([layer_a, layer_b])

        relationContext = QgsRelationContext(project)
        relation = QgsRelation(relationContext)
        relation.setId("relation")
        relation.setName("Relation Number One")
        relation.setReferencingLayer(layer_a.id())
        relation.setReferencedLayer(layer_b.id())
        relation.addFieldPair("id_b", "id")

        self.assertEqual(relation.validationError(), "")
        self.assertTrue(relation.isValid())

        project.relationManager().addRelation(relation)

        project.setTransactionMode(Qgis.TransactionMode.BufferedGroups)
        project.startEditing()

        editBufferGroup = project.editBufferGroup()
        self.assertTrue(editBufferGroup.isEditing())

        f = QgsFeature(layer_a.fields())
        f.setAttribute("id", 123)
        f.setAttribute("id_b", 1)
        f.setGeometry(QgsGeometry.fromWkt("point(7 45)"))
        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 1)
        self.assertIn(layer_a, editBufferGroup.modifiedLayers())

        # Check feature in layer edit buffer but not in provider till commit
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)

        success, commitErrors = editBufferGroup.commitChanges(True)
        self.assertTrue(success)
        self.assertFalse(editBufferGroup.isEditing())

    def testCircularRelations(self):

        memoryLayer_a = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer&field=id_b", "test", "memory"
        )
        self.assertTrue(memoryLayer_a.isValid())
        memoryLayer_b = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer&field=id_a", "test", "memory"
        )
        self.assertTrue(memoryLayer_b.isValid())

        # Load 2 layer from a geopackage
        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "GPKG"
        options.layerName = "layer_a"
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            memoryLayer_a,
            os.path.join(d.path(), "test_EditBufferGroupCircularRelations.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        options.layerName = "layer_b"
        options.actionOnExistingFile = (
            QgsVectorFileWriter.ActionOnExistingFile.CreateOrOverwriteLayer
        )
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(
            memoryLayer_b,
            os.path.join(d.path(), "test_EditBufferGroupCircularRelations.gpkg"),
            QgsCoordinateTransformContext(),
            options,
        )

        layer_a = QgsVectorLayer(newFileName + "|layername=layer_a")
        self.assertTrue(layer_a.isValid())
        layer_b = QgsVectorLayer(newFileName + "|layername=layer_b")
        self.assertTrue(layer_b.isValid())

        project = QgsProject.instance()
        project.addMapLayers([layer_a, layer_b])

        relationContext = QgsRelationContext(project)

        relation_ab = QgsRelation(relationContext)
        relation_ab.setId("relation_ab")
        relation_ab.setName("Relation a b")
        relation_ab.setReferencingLayer(layer_a.id())
        relation_ab.setReferencedLayer(layer_b.id())
        relation_ab.addFieldPair("id_b", "id")
        self.assertEqual(relation_ab.validationError(), "")
        self.assertTrue(relation_ab.isValid())
        project.relationManager().addRelation(relation_ab)

        relation_ba = QgsRelation(relationContext)
        relation_ba.setId("relation_ba")
        relation_ba.setName("Relation b a")
        relation_ba.setReferencingLayer(layer_b.id())
        relation_ba.setReferencedLayer(layer_a.id())
        relation_ba.addFieldPair("id_a", "id")
        self.assertEqual(relation_ba.validationError(), "")
        self.assertTrue(relation_ba.isValid())
        project.relationManager().addRelation(relation_ba)

        project.setTransactionMode(Qgis.TransactionMode.BufferedGroups)
        project.startEditing()

        editBufferGroup = project.editBufferGroup()
        self.assertTrue(editBufferGroup.isEditing())

        f = QgsFeature(layer_a.fields())
        f.setAttribute("id", 123)
        f.setAttribute("id_b", 1)
        f.setGeometry(QgsGeometry.fromWkt("point(7 45)"))
        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 1)
        self.assertIn(layer_a, editBufferGroup.modifiedLayers())

        f = QgsFeature(layer_b.fields())
        f.setAttribute("id", 1)
        f.setAttribute("id_a", 123)
        f.setGeometry(QgsGeometry.fromWkt("point(8 46)"))
        self.assertTrue(layer_b.addFeatures([f]))
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 2)
        self.assertIn(layer_b, editBufferGroup.modifiedLayers())

        # Check feature in layer edit buffer but not in provider till commit
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)
        self.assertEqual(layer_b.featureCount(), 1)
        self.assertEqual(layer_b.dataProvider().featureCount(), 0)

        success, commitErrors = editBufferGroup.commitChanges(True)
        self.assertTrue(success)
        self.assertFalse(editBufferGroup.isEditing())

    def testRemoveLayer(self):
        memoryLayer_a = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer&field=id_b", "testA", "memory"
        )
        self.assertTrue(memoryLayer_a.isValid())

        memoryLayer_b = QgsVectorLayer(
            "Point?crs=epsg:4326&field=id:integer&field=id_a", "testB", "memory"
        )
        self.assertTrue(memoryLayer_b.isValid())

        project = QgsProject.instance()
        project.addMapLayer(memoryLayer_a)
        project.addMapLayer(memoryLayer_b)

        project.setTransactionMode(Qgis.TransactionMode.BufferedGroups)

        editBufferGroup = project.editBufferGroup()

        project.removeMapLayer(memoryLayer_a.id())

        self.assertNotIn(memoryLayer_a, editBufferGroup.layers())
        self.assertIn(memoryLayer_b, editBufferGroup.layers())

        # Chack that no crash happens (#59828)
        project.startEditing()
        project.commitChanges()


if __name__ == "__main__":
    unittest.main()
