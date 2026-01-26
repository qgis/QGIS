"""QGIS Unit tests for QgsProviderConnectionComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "8/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os
import shutil
import tempfile

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsProviderRegistry, QgsVectorLayer
from qgis.gui import QgsProviderConnectionComboBox
from qgis.testing import unittest

from utilities import start_app, unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsProviderConnectionComboBox(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

        gpkg_original_path = (
            f"{TEST_DATA_DIR}/qgis_server/test_project_wms_grouped_layers.gpkg"
        )
        cls.basetestpath = tempfile.mkdtemp()
        cls.gpkg_path = f"{cls.basetestpath}/test_gpkg.gpkg"
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer(f"{cls.gpkg_path}|layername=cdb_lines", "test", "ogr")
        assert vl.isValid()

        gpkg2_original_path = f"{TEST_DATA_DIR}/points_gpkg.gpkg"
        cls.gpkg_path2 = f"{cls.basetestpath}/test_gpkg2.gpkg"
        shutil.copy(gpkg2_original_path, cls.gpkg_path2)
        vl = QgsVectorLayer(f"{cls.gpkg_path2}", "test", "ogr")
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.gpkg_path)
        os.unlink(cls.gpkg_path2)
        super().tearDownClass()

    def testCombo(self):
        """test combobox functionality"""
        m = QgsProviderConnectionComboBox("ogr")
        spy = QSignalSpy(m.connectionChanged)
        self.assertEqual(m.count(), 0)
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test1")

        self.assertEqual(m.count(), 1)
        self.assertEqual(m.itemText(0), "qgis_test1")
        self.assertEqual(m.currentConnection(), "qgis_test1")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], "qgis_test1")

        m.setConnection("qgis_test1")
        self.assertEqual(len(spy), 1)
        m.setConnection("")
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())
        self.assertEqual(len(spy), 2)
        self.assertFalse(spy[-1][0])
        m.setConnection("")
        self.assertEqual(len(spy), 2)
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())

        m.setConnection("qgis_test1")
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentConnection(), "qgis_test1")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path)
        self.assertEqual(spy[-1][0], "qgis_test1")

        conn2 = md.createConnection(self.gpkg_path2, {})
        md.saveConnection(conn2, "aaa_qgis_test2")
        self.assertEqual(m.count(), 2)
        self.assertEqual(m.itemText(0), "aaa_qgis_test2")
        self.assertEqual(m.itemText(1), "qgis_test1")

        self.assertEqual(m.currentConnection(), "qgis_test1")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path)
        self.assertEqual(len(spy), 3)

        md.deleteConnection("qgis_test1")
        self.assertEqual(m.currentConnection(), "aaa_qgis_test2")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path2)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], "aaa_qgis_test2")

        md.deleteConnection("aaa_qgis_test2")

    def testComboSetProvider(self):
        """test combobox functionality with empty entry"""
        m = QgsProviderConnectionComboBox("ogr")

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test_zzz")

        self.assertEqual(m.count(), 1)
        m.setProvider("ogr")
        self.assertEqual(m.count(), 1)

        md.deleteConnection("qgis_test_zzz")

    def testComboWithEmpty(self):
        """test combobox functionality with empty entry"""
        m = QgsProviderConnectionComboBox("ogr")
        m.setAllowEmptyConnection(True)
        spy = QSignalSpy(m.connectionChanged)
        self.assertEqual(m.count(), 1)
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test1")

        self.assertEqual(m.count(), 2)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), "qgis_test1")
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())
        self.assertEqual(len(spy), 0)

        m.setConnection("qgis_test1")
        self.assertEqual(len(spy), 1)
        m.setConnection("")
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())
        self.assertEqual(len(spy), 2)
        self.assertFalse(spy[-1][0])
        m.setConnection("")
        self.assertEqual(m.currentIndex(), 0)
        self.assertEqual(len(spy), 2)
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())

        m.setConnection("qgis_test1")
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentConnection(), "qgis_test1")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path)
        self.assertEqual(spy[-1][0], "qgis_test1")

        conn2 = md.createConnection(self.gpkg_path2, {})
        md.saveConnection(conn2, "aaa_qgis_test2")
        self.assertEqual(m.count(), 3)
        self.assertFalse(m.itemText(0))
        self.assertEqual(m.itemText(1), "aaa_qgis_test2")
        self.assertEqual(m.itemText(2), "qgis_test1")

        self.assertEqual(m.currentConnection(), "qgis_test1")
        self.assertEqual(m.currentConnectionUri(), self.gpkg_path)
        self.assertEqual(len(spy), 3)

        # deleting the selected connection when we are allowing empty
        # connections should fallback to the empty item
        md.deleteConnection("qgis_test1")
        self.assertFalse(m.currentConnection())
        self.assertFalse(m.currentConnectionUri())
        self.assertEqual(len(spy), 4)
        self.assertFalse(spy[-1][0])


if __name__ == "__main__":
    unittest.main()
