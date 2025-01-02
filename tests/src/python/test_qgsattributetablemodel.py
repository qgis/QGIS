"""QGIS Unit tests for the attribute table model.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "27/05/2015"
__copyright__ = "Copyright 2015, The QGIS Project"


import os

from qgis.PyQt.QtCore import QSortFilterProxyModel, Qt, QTemporaryDir, QVariant
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsConditionalStyle,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMemoryProviderUtils,
    QgsPointXY,
    QgsPoint,
    QgsProject,
    QgsVectorLayer,
    QgsVectorLayerCache,
    QgsVectorLayerExporter,
)
from qgis.gui import (
    QgsAttributeTableModel,
    QgsAttributeTableFilterModel,
    QgsEditorWidgetFactory,
    QgsGui,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsAttributeTableModel(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QgsGui.editorWidgetRegistry().initEditors()

        # to track down whether or not we have created widget regarding the field
        class TestEditorWidgetFactory(QgsEditorWidgetFactory):

            def __init__(self):
                super().__init__("test")
                self.widgetLoaded = 0

            def create(self, vl, fieldIdx, editor, parent):
                return None

            def configWidget(self, vl, fieldIdx, parent):
                return None

            def fieldScore(self, vl, fieldIdx):
                self.widgetLoaded += 1
                return 0

        cls.testWidgetFactory = TestEditorWidgetFactory()
        QgsGui.editorWidgetRegistry().registerWidget(
            "testWidget", cls.testWidgetFactory
        )

    def setUp(self):
        self.layer = self.createLayer()
        self.cache = QgsVectorLayerCache(self.layer, 100)
        self.testWidgetFactory.widgetLoaded = 0
        self.am = QgsAttributeTableModel(self.cache)
        self.am.loadLayer()

    def tearDown(self):
        del self.am
        del self.cache
        del self.layer

    def createLayer(self):
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
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
        self.assertEqual(
            feature.attribute(field_idx), feature_model.attribute(field_idx)
        )

        # change attribute value for a feature and commit
        self.layer.startEditing()

        spy = QSignalSpy(self.am.dataChanged)

        self.layer.changeAttributeValue(fid, field_idx, new_value)

        # ensure that dataChanged signal was raised
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0].row(), model_index.row())
        self.assertEqual(spy[-1][0].column(), field_idx)
        self.assertEqual(spy[-1][1].row(), model_index.row())
        self.assertEqual(spy[-1][1].column(), field_idx)
        self.assertEqual(spy[-1][2], [Qt.ItemDataRole.DisplayRole])

        self.layer.commitChanges()

        # check the feature in layer is good
        feature = self.layer.getFeature(fid)
        self.assertEqual(feature.attribute(field_idx), new_value)

        # get the same feature from model and layer
        model_index = self.am.idToIndex(fid)
        feature_model = self.am.feature(model_index)

        # check that index from layer and model are sync
        self.assertEqual(
            feature.attribute(field_idx), feature_model.attribute(field_idx)
        )

    def testEditWithFilter(self):
        fid = 2
        field_idx = 1
        new_value = 334

        # get the same feature from model and layer
        feature = self.layer.getFeature(fid)
        am = QgsAttributeTableModel(self.cache)
        am.setRequest(QgsFeatureRequest().setFilterFid(fid))
        am.loadLayer()

        model_index = am.idToIndex(fid)
        feature_model = am.feature(model_index)

        # check that feature from layer and model are sync
        self.assertEqual(
            feature.attribute(field_idx), feature_model.attribute(field_idx)
        )

        # change attribute value for a feature and commit
        self.layer.startEditing()

        spy = QSignalSpy(am.dataChanged)

        self.layer.changeAttributeValue(fid, field_idx, new_value)

        # ensure that dataChanged signal was raised
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0].row(), model_index.row())
        self.assertEqual(spy[-1][0].column(), field_idx)
        self.assertEqual(spy[-1][1].row(), model_index.row())
        self.assertEqual(spy[-1][1].column(), field_idx)
        self.assertEqual(spy[-1][2], [Qt.ItemDataRole.DisplayRole])

        self.layer.commitChanges()

        # check the feature in layer is good
        feature = self.layer.getFeature(fid)
        self.assertEqual(feature.attribute(field_idx), new_value)

        # get the same feature from model and layer
        model_index = am.idToIndex(fid)
        feature_model = am.feature(model_index)

        # check that index from layer and model are sync
        self.assertEqual(
            feature.attribute(field_idx), feature_model.attribute(field_idx)
        )

    def testStyle(self):
        style_threshold = 2
        color = QColor(133, 133, 133)
        style = QgsConditionalStyle()
        style.setRule(f'"fldint" <= {style_threshold}')
        style.setTextColor(color)
        self.layer.conditionalStyles().setRowStyles([style])

        for f in self.layer.getFeatures():
            model_index = self.am.idToIndex(f.id())
            text_color = self.am.data(model_index, Qt.ItemDataRole.ForegroundRole)

            if f["fldint"] <= style_threshold:
                self.assertEqual(text_color, color)
            else:
                self.assertIsNone(text_color)

        self.assertTrue(self.layer.startEditing())

        feature1 = self.layer.getFeature(2)
        feature1["fldint"] = style_threshold + 1
        feature2 = self.layer.getFeature(8)
        feature2["fldint"] = style_threshold

        self.assertTrue(self.layer.updateFeature(feature1))
        self.assertTrue(self.layer.updateFeature(feature2))
        self.assertTrue(self.layer.commitChanges())

        for f in self.layer.getFeatures():
            model_index = self.am.idToIndex(f.id())
            text_color = self.am.data(model_index, Qt.ItemDataRole.ForegroundRole)

            if f["fldint"] <= style_threshold:
                self.assertEqual(
                    color, text_color, f"Feature {f.id()} should have color"
                )
            else:
                self.assertIsNone(text_color, f"Feature {f.id()} should have no color")

        self.layer.conditionalStyles().setRowStyles([])

    def testTransactionRollback(self):
        """Test issue https://github.com/qgis/QGIS/issues/48171#issuecomment-1132709901"""

        d = QTemporaryDir()
        path = d.path()

        source_fields = QgsFields()
        source_fields.append(QgsField("int", QVariant.Int))
        vl = QgsMemoryProviderUtils.createMemoryLayer("test", source_fields)
        f = QgsFeature()
        f.setAttributes([1])
        vl.dataProvider().addFeature(f)

        tmpfile = os.path.join(path, "testTransactionRollback.sqlite")

        options = {"driverName": "SpatiaLite", "layerName": "test"}

        err = QgsVectorLayerExporter.exportLayer(
            vl, tmpfile, "ogr", vl.crs(), False, options
        )
        self.assertEqual(
            err[0],
            QgsVectorLayerExporter.ExportError.NoError,
            f"unexpected import error {err}",
        )

        vl = QgsVectorLayer(
            f"dbname='{tmpfile}' table=\"test\" () sql=", "test", "spatialite"
        )

        self.assertTrue(vl.isValid())

        p = QgsProject.instance()
        p.setTransactionMode(Qgis.TransactionMode.AutomaticGroups)
        self.assertTrue(p.addMapLayer(vl))

        cache = QgsVectorLayerCache(vl, 100)
        am = QgsAttributeTableModel(cache)
        am.loadLayer()
        self.assertEqual(am.rowCount(), 1)

        self.assertTrue(vl.startEditing())
        vl.beginEditCommand("edit1")

        f = QgsFeature()
        f.setAttributes([2])
        self.assertTrue(vl.addFeature(f))
        self.assertEqual(am.rowCount(), 2)
        self.assertEqual(len([f for f in vl.getFeatures()]), 2)

        vl.endEditCommand()
        self.assertTrue(vl.rollBack())
        self.assertEqual(len([f for f in vl.getFeatures()]), 1)
        self.assertEqual(am.rowCount(), 1)

    def testColumnLazyLoading(self):
        """
        Test that widget are loaded only when column is needed
        and that models handles correctly extra columns
        """

        twf = self.testWidgetFactory
        self.assertEqual(twf.widgetLoaded, 0)

        # to track down if column have been inserted or removed
        colsInserted = 0
        colsRemoved = 0

        def onColsInserted(parent, first, last):
            nonlocal colsInserted
            colsInserted = last - first + 1

        def onColsRemoved(parent, first, last):
            nonlocal colsRemoved
            colsRemoved = last - first + 1

        self.am.columnsInserted.connect(onColsInserted)
        self.am.columnsRemoved.connect(onColsRemoved)

        # to check our extra column is working
        class TestFilterModel(QSortFilterProxyModel):

            def __init__(self):
                super().__init__()

            def data(self, index, role):
                if (
                    role == Qt.ItemDataRole.DisplayRole
                    and self.sourceModel().extraColumns() > 0
                    and index.column() > 1
                ):
                    return f"extra_{index.column()}"

                return super().data(index, role)

        fm = TestFilterModel()
        fm.setSourceModel(self.am)

        self.assertEqual(fm.data(fm.index(2, 0), Qt.ItemDataRole.DisplayRole), "test")
        self.assertEqual(fm.data(fm.index(2, 1), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(fm.data(fm.index(2, 2), Qt.ItemDataRole.DisplayRole), None)

        # 2 columns have been loaded since we call data
        self.assertEqual(twf.widgetLoaded, 2)
        twf.widgetLoaded = 0

        # only one column inserted, no widget loaded
        self.am.setExtraColumns(1)
        self.assertEqual(twf.widgetLoaded, 0)
        self.assertEqual(colsInserted, 1)
        colsInserted = 0
        self.assertEqual(colsRemoved, 0)

        self.assertEqual(fm.data(fm.index(2, 0), Qt.ItemDataRole.DisplayRole), "test")
        self.assertEqual(fm.data(fm.index(2, 1), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(
            fm.data(fm.index(2, 2), Qt.ItemDataRole.DisplayRole), "extra_2"
        )

        self.assertEqual(twf.widgetLoaded, 0)

        # only one column removed, no widget loaded
        self.am.setExtraColumns(0)
        self.assertEqual(twf.widgetLoaded, 0)
        self.assertEqual(colsInserted, 0)
        self.assertEqual(colsRemoved, 1)
        colsRemoved = 0

        self.assertEqual(fm.data(fm.index(2, 0), Qt.ItemDataRole.DisplayRole), "test")
        self.assertEqual(fm.data(fm.index(2, 1), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(fm.data(fm.index(2, 2), Qt.ItemDataRole.DisplayRole), None)

        self.assertEqual(twf.widgetLoaded, 0)

        # nothing has changed, nothing should happened
        self.am.loadLayer()
        self.assertEqual(twf.widgetLoaded, 0)
        self.assertEqual(colsInserted, 0)
        self.assertEqual(colsRemoved, 0)

        # add field, widget will be reloaded when data will be called
        self.layer.addExpressionField(
            "'newfield_' || \"fldtxt\"", QgsField("newfield", QVariant.String)
        )
        self.assertEqual(twf.widgetLoaded, 0)
        self.assertEqual(colsInserted, 1)
        self.assertEqual(colsRemoved, 0)
        colsInserted = 0
        twf.widgetLoaded = 0

        self.assertEqual(fm.data(fm.index(2, 0), Qt.ItemDataRole.DisplayRole), "test")
        self.assertEqual(fm.data(fm.index(2, 1), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(
            fm.data(fm.index(2, 2), Qt.ItemDataRole.DisplayRole), "newfield_test"
        )
        twf.widgetLoaded = 0

        # remove field, widget will be reloaded again
        self.layer.removeExpressionField(2)
        self.assertEqual(twf.widgetLoaded, 0)
        self.assertEqual(colsInserted, 0)
        self.assertEqual(colsRemoved, 1)
        colsRemoved = 0

        self.assertEqual(fm.data(fm.index(2, 0), Qt.ItemDataRole.DisplayRole), "test")
        self.assertEqual(fm.data(fm.index(2, 1), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(fm.data(fm.index(2, 2), Qt.ItemDataRole.DisplayRole), None)
        self.assertEqual(twf.widgetLoaded, 2)
        twf.widgetLoaded = 0

    def test_sort_requires_geometry(self):
        layer = QgsVectorLayer("Linestring?field=fldint:integer", "addfeat", "memory")
        pr = layer.dataProvider()
        features = list()
        f = QgsFeature(layer.fields())
        f.setAttributes([2])
        f.setGeometry(QgsGeometry.fromPolyline([QgsPoint(0, 0), QgsPoint(1, 1)]))
        features.append(f)

        f = QgsFeature(layer.fields())
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromPolyline([QgsPoint(0, 0), QgsPoint(2, 2)]))
        features.append(f)

        self.assertTrue(pr.addFeatures(features))
        cache = QgsVectorLayerCache(layer, 100)
        am = QgsAttributeTableModel(cache)
        am.loadLayer()

        fm = QgsAttributeTableFilterModel(None, am, am)

        fm.sort('"fldint"', Qt.SortOrder.AscendingOrder)
        self.assertEqual(fm.data(fm.index(0, 0), Qt.ItemDataRole.DisplayRole), "1")
        self.assertEqual(fm.data(fm.index(1, 0), Qt.ItemDataRole.DisplayRole), "2")

        fm.sort('"fldint"', Qt.SortOrder.DescendingOrder)
        self.assertEqual(fm.data(fm.index(0, 0), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(fm.data(fm.index(1, 0), Qt.ItemDataRole.DisplayRole), "1")

        fm.sort("$length", Qt.SortOrder.DescendingOrder)
        self.assertEqual(fm.data(fm.index(0, 0), Qt.ItemDataRole.DisplayRole), "1")
        self.assertEqual(fm.data(fm.index(1, 0), Qt.ItemDataRole.DisplayRole), "2")

        fm.sort("$length", Qt.SortOrder.AscendingOrder)
        self.assertEqual(fm.data(fm.index(0, 0), Qt.ItemDataRole.DisplayRole), "2")
        self.assertEqual(fm.data(fm.index(1, 0), Qt.ItemDataRole.DisplayRole), "1")


if __name__ == "__main__":
    unittest.main()
