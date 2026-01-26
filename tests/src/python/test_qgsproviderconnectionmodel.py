"""QGIS Unit tests for OGR GeoPackage QgsProviderConnectionModel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Nyall Dawson"
__date__ = "07/08/2020"
__copyright__ = "Copyright 2019, The QGIS Project"

import os
import shutil
import tempfile

from qgis.PyQt.QtCore import QCoreApplication, QModelIndex, Qt
from qgis.core import (
    QgsProviderConnectionModel,
    QgsProviderRegistry,
    QgsVectorLayer,
)
from qgis.testing import unittest

from utilities import start_app, unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnectionModel(unittest.TestCase):

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

    def test_model(self):
        """Test model functionality"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test1")

        model = QgsProviderConnectionModel("ogr")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConfiguration,
            ),
            {},
        )

        md.saveConnection(conn, "qgis_test1")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )

        conn2 = md.createConnection(self.gpkg_path2, {})
        md.saveConnection(conn2, "qgis_test2")
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path2,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test2",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path2,
        )

        md.deleteConnection("qgis_test1")
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )

        md.deleteConnection("qgis_test2")

    def test_model_allow_empty(self):
        """Test model with empty entry"""
        model = QgsProviderConnectionModel("ogr")
        self.assertEqual(model.rowCount(), 0)
        model.setAllowEmptyConnection(True)
        self.assertEqual(model.rowCount(), 1)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test1")

        model.setAllowEmptyConnection(False)
        model.setAllowEmptyConnection(False)

        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(model.columnCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConfiguration,
            ),
            {},
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )

        model.setAllowEmptyConnection(True)
        model.setAllowEmptyConnection(True)
        self.assertEqual(model.rowCount(), 2)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole)
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            )
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            )
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConfiguration,
            )
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConfiguration,
            ),
            {},
        )
        self.assertFalse(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )

        md.saveConnection(conn, "qgis_test1")
        self.assertEqual(model.rowCount(), 2)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertFalse(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )

        model.setAllowEmptyConnection(False)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        model.setAllowEmptyConnection(True)

        conn2 = md.createConnection(self.gpkg_path2, {})
        md.saveConnection(conn2, "qgis_test2")
        self.assertEqual(model.rowCount(), 3)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path,
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path,
        )
        self.assertFalse(
            model.data(
                model.index(1, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )
        self.assertEqual(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )
        self.assertEqual(
            model.data(model.index(2, 0, QModelIndex()), Qt.ItemDataRole.ToolTipRole),
            self.gpkg_path2,
        )
        self.assertEqual(
            model.data(
                model.index(2, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleConnectionName,
            ),
            "qgis_test2",
        )
        self.assertEqual(
            model.data(
                model.index(2, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleUri,
            ),
            self.gpkg_path2,
        )
        self.assertFalse(
            model.data(
                model.index(2, 0, QModelIndex()),
                QgsProviderConnectionModel.Role.RoleEmpty,
            )
        )

        model.setAllowEmptyConnection(False)
        self.assertEqual(model.rowCount(), 2)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test1",
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )
        model.setAllowEmptyConnection(True)

        md.deleteConnection("qgis_test1")
        self.assertEqual(model.rowCount(), 2)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertEqual(
            model.data(model.index(1, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )

        model.setAllowEmptyConnection(False)
        self.assertEqual(model.rowCount(), 1)
        self.assertEqual(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole),
            "qgis_test2",
        )


if __name__ == "__main__":
    unittest.main()
