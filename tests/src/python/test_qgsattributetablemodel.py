# -*- coding: utf-8 -*-
"""QGIS Unit tests for the attribute table model.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '27/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.gui import (
    QgsAttributeTableModel,
    QgsGui
)
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsVectorLayer,
    QgsVectorLayerCache
)

from qgis.testing import (start_app,
                          unittest
                          )

start_app()


class TestQgsAttributeTableModel(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        QgsGui.editorWidgetRegistry().initEditors()

    def setUp(self):
        self.layer = self.createLayer()
        self.cache = QgsVectorLayerCache(self.layer, 100)
        self.am = QgsAttributeTableModel(self.cache)
        self.am.loadLayer()

    def tearDown(self):
        del self.am
        del self.cache
        del self.layer

    def createLayer(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        features = list()
        for i in range(10):
            f = QgsFeature()
            f.setAttributes(["test", i])
            f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100 * i, 2 ^ i)))
            features.append(f)

        self.assertTrue(pr.addFeatures(features))
        return layer

    def testLoad(self):
        self.assertEqual(self.am.rowCount(), 10)
        self.assertEqual(self.am.columnCount(), 2)

    def testRemove(self):
        self.layer.startEditing()
        self.layer.deleteFeature(5)
        self.assertEqual(self.am.rowCount(), 9)
        self.layer.selectByIds([1, 3, 6, 7])
        self.layer.deleteSelectedFeatures()
        self.assertEqual(self.am.rowCount(), 5)

    def testAdd(self):
        self.layer.startEditing()

        f = QgsFeature()
        f.setAttributes(["test", 8])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(100, 200)))
        self.layer.addFeature(f)

        self.assertEqual(self.am.rowCount(), 11)

    def testRemoveColumns(self):
        self.assertTrue(self.layer.startEditing())

        self.assertTrue(self.layer.deleteAttribute(1))

        self.assertEqual(self.am.columnCount(), 1)

    def testEdit(self):
        fid = 2
        field_idx = 1
        new_value = 333

        # get the same feature from model and layer
        feature = self.layer.getFeature(fid)
        model_index = self.am.idToIndex(fid)
        feature_model = self.am.feature(model_index)

        # check that feature from layer and model are sync
        self.assertEqual(feature.attribute(field_idx), feature_model.attribute(field_idx))

        # change attribute value for a feature and commit
        self.layer.startEditing()
        self.layer.changeAttributeValue(fid, field_idx, new_value)
        self.layer.commitChanges()

        # check the feature in layer is good
        feature = self.layer.getFeature(fid)
        self.assertEqual(feature.attribute(field_idx), new_value)

        # get the same feature from model and layer
        model_index = self.am.idToIndex(fid)
        feature_model = self.am.feature(model_index)

        # check that index from layer and model are sync
        self.assertEqual(feature.attribute(field_idx), feature_model.attribute(field_idx))


if __name__ == '__main__':
    unittest.main()
