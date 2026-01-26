"""QGIS Unit tests for QgsDatabaseSchemaModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Nyall Dawson"
__date__ = "07/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.PyQt.QtCore import QCoreApplication, QModelIndex, Qt
from qgis.core import QgsDatabaseSchemaModel, QgsProviderRegistry
import unittest
from qgis.testing import start_app, QgisTestCase


class TestPyQgsDatabaseSchemaModel(QgisTestCase):

    # Provider test cases must define the string URI for the test
    uri = ""
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = "postgres"

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        cls.postgres_conn = "service='qgis_test'"
        if "QGIS_PGTEST_DB" in os.environ:
            cls.postgres_conn = os.environ["QGIS_PGTEST_DB"]
        cls.uri = cls.postgres_conn + " sslmode=disable"

    def testModel(self):
        conn = (
            QgsProviderRegistry.instance()
            .providerMetadata("postgres")
            .createConnection(self.uri, {})
        )
        self.assertTrue(conn)
        model = QgsDatabaseSchemaModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        self.assertEqual(model.columnCount(), 1)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertEqual(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                Qt.ItemDataRole.ToolTipRole,
            ),
            "qgis_test",
        )
        self.assertIsNone(
            model.data(
                model.index(model.rowCount(), 0, QModelIndex()),
                Qt.ItemDataRole.DisplayRole,
            )
        )

        model.refresh()
        self.assertEqual(model.rowCount(), old_count)

        conn.createSchema("myNewSchema")
        self.assertEqual(model.rowCount(), old_count)
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertIn("myNewSchema", schemas)

        conn.createSchema("myNewSchema2")
        conn.createSchema("myNewSchema3")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertIn("myNewSchema", schemas)
        self.assertIn("myNewSchema2", schemas)
        self.assertIn("myNewSchema3", schemas)

        conn.createSchema("myNewSchema4")
        conn.dropSchema("myNewSchema2")
        conn.dropSchema("myNewSchema")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertNotIn("myNewSchema", schemas)
        self.assertNotIn("myNewSchema2", schemas)
        self.assertIn("myNewSchema3", schemas)
        self.assertIn("myNewSchema4", schemas)

        conn.dropSchema("myNewSchema3")
        conn.dropSchema("myNewSchema4")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertNotIn("myNewSchema3", schemas)
        self.assertNotIn("myNewSchema4", schemas)

    def test_model_allow_empty(self):
        """Test model with empty entry"""
        conn = (
            QgsProviderRegistry.instance()
            .providerMetadata("postgres")
            .createConnection(self.uri, {})
        )
        self.assertTrue(conn)
        model = QgsDatabaseSchemaModel(conn)
        self.assertGreaterEqual(model.rowCount(), 3)
        old_count = model.rowCount()
        model.setAllowEmptySchema(True)
        self.assertEqual(model.rowCount(), old_count + 1)

        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )
        self.assertIsNone(
            model.data(
                model.index(model.rowCount(), 0, QModelIndex()),
                Qt.ItemDataRole.DisplayRole,
            )
        )

        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)

        conn.createSchema("myNewSchema")
        self.assertEqual(model.rowCount(), old_count + 1)
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 2)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertIn("myNewSchema", schemas)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )

        model.setAllowEmptySchema(False)
        self.assertEqual(model.rowCount(), old_count + 1)
        self.assertTrue(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        model.setAllowEmptySchema(True)
        self.assertEqual(model.rowCount(), old_count + 2)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )

        conn.createSchema("myNewSchema2")
        conn.createSchema("myNewSchema3")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 4)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertIn("myNewSchema", schemas)
        self.assertIn("myNewSchema2", schemas)
        self.assertIn("myNewSchema3", schemas)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )

        conn.createSchema("myNewSchema4")
        conn.dropSchema("myNewSchema2")
        conn.dropSchema("myNewSchema")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 3)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertNotIn("myNewSchema", schemas)
        self.assertNotIn("myNewSchema2", schemas)
        self.assertIn("myNewSchema3", schemas)
        self.assertIn("myNewSchema4", schemas)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )

        conn.dropSchema("myNewSchema3")
        conn.dropSchema("myNewSchema4")
        model.refresh()
        self.assertEqual(model.rowCount(), old_count + 1)
        schemas = [
            model.data(model.index(r, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
            for r in range(model.rowCount())
        ]
        self.assertIn("public", schemas)
        self.assertIn("CamelCase'singlequote'Schema", schemas)
        self.assertIn("qgis_test", schemas)
        self.assertNotIn("myNewSchema3", schemas)
        self.assertNotIn("myNewSchema4", schemas)
        self.assertFalse(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertTrue(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )
        self.assertFalse(
            model.data(
                model.index(schemas.index("qgis_test"), 0, QModelIndex()),
                QgsDatabaseSchemaModel.Role.RoleEmpty,
            )
        )

        model.setAllowEmptySchema(False)
        self.assertEqual(model.rowCount(), old_count)
        self.assertTrue(
            model.data(model.index(0, 0, QModelIndex()), Qt.ItemDataRole.DisplayRole)
        )
        self.assertFalse(
            model.data(
                model.index(0, 0, QModelIndex()), QgsDatabaseSchemaModel.Role.RoleEmpty
            )
        )


if __name__ == "__main__":
    unittest.main()
