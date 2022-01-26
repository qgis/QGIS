# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayerEditBufferGroup.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Damiano Lombardi'
__date__ = '13/01/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os
import qgis  # NOQA
from qgis.PyQt.QtCore import QVariant, QTemporaryDir
from qgis.core import (Qgis,
                       QgsGeometry,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsProject,
                       QgsField,
                       QgsVectorFileWriter,
                       QgsCoordinateTransformContext)
from qgis.testing import start_app, unittest

start_app()


class TestQgsVectorLayerEditBufferGroup(unittest.TestCase):

    def testStartEditingCommitRollBack(self):

        ml = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer&field=int2:integer', 'test', 'memory')
        self.assertTrue(ml.isValid())

        # Layer A geopackage A
        d = QTemporaryDir()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = 'GPKG'
        options.layerName = 'layer_a'
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(ml, os.path.join(d.path(), 'test_EditBufferGroup_A.gpkg'), QgsCoordinateTransformContext(), options)

        self.assertEqual(err, QgsVectorFileWriter.NoError)
        self.assertTrue(os.path.isfile(newFileName))

        layer_a = QgsVectorLayer(newFileName + '|layername=layer_a')

        self.assertTrue(layer_a.isValid())

        # Layer B geopackage B
        options.layerName = 'layer_b'
        err, msg, newFileName, newLayer = QgsVectorFileWriter.writeAsVectorFormatV3(ml, os.path.join(d.path(), 'test_EditBufferGroup_B.gpkg'), QgsCoordinateTransformContext(), options)

        self.assertEqual(err, QgsVectorFileWriter.NoError)
        self.assertTrue(os.path.isfile(newFileName))

        layer_b = QgsVectorLayer(newFileName + '|layername=layer_b')

        self.assertTrue(layer_b.isValid())

        # Layer C memory
        layer_c = QgsVectorLayer('Point?crs=epsg:4326&field=int:integer&field=int2:integer', 'test', 'memory')
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

        commitErrors = []
        self.assertTrue(editBufferGroup.commitChanges(commitErrors, False))
        self.assertTrue(editBufferGroup.isEditing())
        self.assertTrue(editBufferGroup.commitChanges(commitErrors, True))
        self.assertFalse(editBufferGroup.isEditing())

        self.assertTrue(editBufferGroup.startEditing())
        self.assertTrue(editBufferGroup.isEditing())

        f = QgsFeature(layer_a.fields())
        f.setAttribute('int', 123)
        f.setGeometry(QgsGeometry.fromWkt('point(7 45)'))
        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(len(editBufferGroup.modifiedLayers()), 1)
        self.assertIn(layer_a, editBufferGroup.modifiedLayers())

        # Check feature in layer edit buffer but not in provider till commit
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)

        rollbackErrors = []
        self.assertTrue(editBufferGroup.rollBack(rollbackErrors, False))
        self.assertTrue(editBufferGroup.isEditing())
        self.assertEqual(layer_a.featureCount(), 0)

        self.assertTrue(layer_a.addFeatures([f]))
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 0)

        self.assertTrue(editBufferGroup.commitChanges(commitErrors, True))
        self.assertFalse(editBufferGroup.isEditing())
        self.assertEqual(layer_a.featureCount(), 1)
        self.assertEqual(layer_a.dataProvider().featureCount(), 1)


if __name__ == '__main__':
    unittest.main()
