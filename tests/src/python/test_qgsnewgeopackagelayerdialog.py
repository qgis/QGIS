"""QGIS Unit tests for QgsNewGeoPackageLayerDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Even Rouault"
__date__ = "2016-04-21"
__copyright__ = "Copyright 2016, Even Rouault"

import os
import shutil
import tempfile

from qgis.PyQt.QtCore import QCoreApplication, Qt
from qgis.PyQt.QtTest import QTest
from qgis.PyQt.QtWidgets import (
    QComboBox,
    QDialogButtonBox,
    QLineEdit,
    QToolButton,
    QTreeWidget,
)
from qgis.core import QgsProject, QgsSettings, QgsWkbTypes
from qgis.gui import QgsFileWidget, QgsNewGeoPackageLayerDialog
import unittest
from qgis.testing import start_app, QgisTestCase


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return (maj) * 1000000 + (min) * 10000 + (rev) * 100


class TestPyQgsNewGeoPackageLayerDialog(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsNewGeoPackageLayerDialog.com"
        )
        QCoreApplication.setApplicationName("QGIS_TestPyQgsNewGeoPackageLayerDialog")
        QgsSettings().clear()
        start_app()
        cls.basetestpath = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        if cls.basetestpath is not None:
            shutil.rmtree(cls.basetestpath, True)

        super().tearDownClass()

    def test(self):

        # Skip if GDAL python bindings are not available
        try:
            from osgeo import gdal, ogr
        except:
            return

        dialog = QgsNewGeoPackageLayerDialog()
        dialog.setProperty("hideDialogs", True)

        mDatabase = dialog.findChild(QgsFileWidget, "mDatabase")
        buttonBox = dialog.findChild(QDialogButtonBox, "buttonBox")
        ok_button = buttonBox.button(QDialogButtonBox.StandardButton.Ok)
        mTableNameEdit = dialog.findChild(QLineEdit, "mTableNameEdit")
        mLayerIdentifierEdit = dialog.findChild(QLineEdit, "mLayerIdentifierEdit")
        mLayerDescriptionEdit = dialog.findChild(QLineEdit, "mLayerDescriptionEdit")
        mFeatureIdColumnEdit = dialog.findChild(QLineEdit, "mFeatureIdColumnEdit")
        mGeometryTypeBox = dialog.findChild(QComboBox, "mGeometryTypeBox")
        mGeometryColumnEdit = dialog.findChild(QLineEdit, "mGeometryColumnEdit")
        mFieldNameEdit = dialog.findChild(QLineEdit, "mFieldNameEdit")
        mFieldTypeBox = dialog.findChild(QComboBox, "mFieldTypeBox")
        mFieldLengthEdit = dialog.findChild(QLineEdit, "mFieldLengthEdit")
        mAddAttributeButton = dialog.findChild(QToolButton, "mAddAttributeButton")
        mRemoveAttributeButton = dialog.findChild(QToolButton, "mRemoveAttributeButton")
        mAttributeView = dialog.findChild(QTreeWidget, "mAttributeView")
        dialog.accepted.connect(self.accepted_slot)

        mGeometryTypeBox.setCurrentIndex(mGeometryTypeBox.findData(ogr.wkbPoint))
        self.assertEqual(mGeometryTypeBox.currentText(), "Point")

        self.assertFalse(ok_button.isEnabled())

        dbname = os.path.join(self.basetestpath, "test.gpkg")
        mDatabase.setFilePath(dbname)
        self.assertEqual(mTableNameEdit.text(), "test")
        self.assertEqual(mLayerIdentifierEdit.text(), "test")
        self.assertTrue(ok_button.isEnabled())

        mGeometryColumnEdit.setText("my_geom")
        mFeatureIdColumnEdit.setText("my_fid")

        self.assertFalse(mAddAttributeButton.isEnabled())
        self.assertFalse(mRemoveAttributeButton.isEnabled())

        mFieldNameEdit.setText("strfield")
        self.assertTrue(mAddAttributeButton.isEnabled())
        mFieldLengthEdit.setText("10")
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        mFieldNameEdit.setText("intfield")
        mFieldTypeBox.setCurrentIndex(mFieldTypeBox.findData("integer"))
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        mFieldNameEdit.setText("realfield")
        mFieldTypeBox.setCurrentIndex(mFieldTypeBox.findData("real"))
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        mFieldNameEdit.setText("datefield")
        mFieldTypeBox.setCurrentIndex(mFieldTypeBox.findData("date"))
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        mFieldNameEdit.setText("datetimefield")
        mFieldTypeBox.setCurrentIndex(mFieldTypeBox.findData("datetime"))
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        mFieldNameEdit.setText("int64field")
        mFieldTypeBox.setCurrentIndex(mFieldTypeBox.findData("integer64"))
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        # Add and remove field
        mFieldNameEdit.setText("dummy")
        self.assertFalse(mFieldLengthEdit.isEnabled())
        QTest.mouseClick(mAddAttributeButton, Qt.MouseButton.LeftButton)

        index = mAttributeView.model().index(mAttributeView.model().rowCount() - 1, 0)
        mAttributeView.setCurrentIndex(index)

        QTest.mouseClick(mRemoveAttributeButton, Qt.MouseButton.LeftButton)

        self.accepted = False
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        self.assertTrue(self.accepted)

        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 1)
        layer = layers[list(layers.keys())[0]]
        self.assertEqual(layer.name(), "test")
        self.assertEqual(layer.geometryType(), QgsWkbTypes.GeometryType.PointGeometry)
        QgsProject.instance().removeAllMapLayers()

        ds = ogr.Open(dbname)
        lyr = ds.GetLayer(0)
        self.assertEqual(lyr.GetFIDColumn(), "my_fid")
        self.assertEqual(lyr.GetGeometryColumn(), "my_geom")
        self.assertEqual(lyr.GetGeomType(), ogr.wkbPoint)
        self.assertEqual(lyr.GetLayerDefn().GetFieldCount(), 6)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(0).GetNameRef(), "strfield")
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(0).GetType(), ogr.OFTString)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(0).GetWidth(), 10)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(1).GetNameRef(), "intfield")
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(1).GetType(), ogr.OFTInteger)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(1).GetWidth(), 0)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(2).GetNameRef(), "realfield")
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(2).GetType(), ogr.OFTReal)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(2).GetWidth(), 0)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(3).GetNameRef(), "datefield")
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(3).GetType(), ogr.OFTDate)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(3).GetWidth(), 0)
        self.assertEqual(
            lyr.GetLayerDefn().GetFieldDefn(4).GetNameRef(), "datetimefield"
        )
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(4).GetType(), ogr.OFTDateTime)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(4).GetWidth(), 0)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(5).GetNameRef(), "int64field")
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(5).GetType(), ogr.OFTInteger64)
        self.assertEqual(lyr.GetLayerDefn().GetFieldDefn(5).GetWidth(), 0)
        ds = None

        # Try re-adding with different table. It should ask if we want to
        # overwrite the DB, and we'll implicitly answer cancel, hence failure
        mTableNameEdit.setText("table2")

        self.accepted = False
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        self.assertFalse(self.accepted)

        # Retry, and ask to keep the DB
        self.accepted = False
        dialog.setProperty("question_existing_db_answer_add_new_layer", True)
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        dialog.setProperty("question_existing_db_answer_add_new_layer", None)
        self.assertTrue(self.accepted)

        QgsProject.instance().removeAllMapLayers()
        ds = ogr.Open(dbname)
        self.assertEqual(ds.GetLayerCount(), 2)
        ds = None

        # Retry, and ask to overwrite the DB
        self.accepted = False
        dialog.setProperty("question_existing_db_answer_overwrite", True)
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        dialog.setProperty("question_existing_db_answer_overwrite", None)
        self.assertTrue(self.accepted)

        QgsProject.instance().removeAllMapLayers()
        ds = ogr.Open(dbname)
        self.assertEqual(ds.GetLayerCount(), 1)
        ds = None

        # Try re-adding with same parameters. It should ask if we want to
        # overwrite the layer, and we'll implicitly answer no, hence failure
        # since it already exists with that name
        self.accepted = False
        dialog.setProperty("question_existing_db_answer_add_new_layer", True)
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        dialog.setProperty("question_existing_db_answer_add_new_layer", None)
        self.assertFalse(self.accepted)

        # Now answer yes, and change a few things
        mLayerIdentifierEdit.setText("my_identifier")
        mLayerDescriptionEdit.setText("my_description")
        dialog.setProperty("question_existing_db_answer_add_new_layer", True)
        dialog.setProperty("question_existing_layer_answer_overwrite", True)
        self.accepted = False
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        dialog.setProperty("question_existing_db_answer_add_new_layer", None)
        dialog.setProperty("question_existing_layer_answer_overwrite", None)
        self.assertTrue(self.accepted)

        # Only check with OGR 2.0 since the IDENTIFIER and DESCRIPTION creation options don't exist in OGR 1.11
        layers = QgsProject.instance().mapLayers()
        self.assertEqual(len(layers), 1)
        layer = layers[list(layers.keys())[0]]
        self.assertEqual(layer.name(), "my_identifier")
        QgsProject.instance().removeAllMapLayers()

        ds = ogr.Open(dbname)
        sql_lyr = ds.ExecuteSQL("SELECT * FROM gpkg_contents")
        self.assertEqual(sql_lyr.GetFeatureCount(), 1)
        f = sql_lyr.GetNextFeature()
        identifier = f.GetField("identifier")
        description = f.GetField("description")
        f = None
        ds.ReleaseResultSet(sql_lyr)
        ds = None
        self.assertEqual(identifier, "my_identifier")
        self.assertEqual(description, "my_description")

        # Try invalid path
        mDatabase.setFilePath("/this/is/invalid/test.gpkg")
        self.accepted = False
        QTest.mouseClick(ok_button, Qt.MouseButton.LeftButton)
        self.assertFalse(self.accepted)

        # dialog.exec_()

    def accepted_slot(self):
        self.accepted = True


if __name__ == "__main__":
    unittest.main()
