"""QGIS Unit tests for QgsDatabaseTableComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "8/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsField,
    QgsFields,
    QgsProviderRegistry,
    QgsWkbTypes,
)
from qgis.gui import QgsDatabaseTableComboBox
from qgis.testing import unittest

from utilities import start_app, unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsDatabaseTableComboBox(unittest.TestCase):

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

    def testCombo(self):
        """test combobox functionality"""
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, "mycon")

        m = QgsDatabaseTableComboBox("postgres", "mycon")
        self.assertGreaterEqual(m.comboBox().count(), 3)

        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertIn("information_schema.attributes", text)
        self.assertIn("qgis_test.some_poly_data", text)
        self.assertLess(
            text.index("information_schema.attributes"),
            text.index("qgis_test.some_poly_data"),
        )
        self.assertTrue(m.currentSchema())
        self.assertTrue(m.currentTable())

        m.setSchema("information_schema")
        m.setTable("attributes")
        spy = QSignalSpy(m.tableChanged)

        m.setSchema("qgis_test")
        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("information_schema.attributes", text)
        self.assertNotIn("attributes", text)
        self.assertIn("some_poly_data", text)

        self.assertEqual(m.currentTable(), "")
        self.assertEqual(m.currentSchema(), "")
        self.assertEqual(len(spy), 1)
        self.assertFalse(spy[-1][0])

        m.setTable("")
        self.assertEqual(m.currentTable(), "")
        self.assertEqual(m.currentSchema(), "")
        self.assertEqual(len(spy), 1)
        self.assertFalse(spy[-1][0])
        m.setTable("someData")
        self.assertEqual(len(spy), 2)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        fields = QgsFields()
        fields.append(QgsField("test", QVariant.String))
        conn.createVectorTable(
            "qgis_test",
            "myNewTable",
            fields,
            QgsWkbTypes.Type.Point,
            QgsCoordinateReferenceSystem("EPSG:3857"),
            False,
            {},
        )

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        # tables are not automatically refreshed
        self.assertEqual(text2, text)

        # but setting a new connection should fix this!
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn2 = md.createConnection(self.uri, {})
        md.saveConnection(conn2, "another")
        m.setConnectionName("another", "postgres")
        # ideally there'd be no extra signal here, but it's a minor issue...
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotEqual(text2, text)
        self.assertIn("myNewTable", text2)

        m.setTable("myNewTable")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        # no auto drop
        conn.dropVectorTable("qgis_test", "myNewTable")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        m.refreshTables()
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("myNewTable", text2)
        self.assertEqual(len(spy), 5)
        self.assertFalse(m.currentSchema())
        self.assertFalse(spy[-1][0])

    def testComboAllSchemas(self):
        """test combobox functionality showing all schemas"""
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, "mycon2")

        m = QgsDatabaseTableComboBox("postgres", "mycon2")
        self.assertGreaterEqual(m.comboBox().count(), 3)

        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertIn("information_schema.attributes", text)
        self.assertIn("qgis_test.some_poly_data", text)
        self.assertLess(
            text.index("information_schema.attributes"),
            text.index("qgis_test.some_poly_data"),
        )
        self.assertTrue(m.currentSchema())
        self.assertTrue(m.currentTable())

        spy = QSignalSpy(m.tableChanged)

        m.setTable("")
        self.assertEqual(m.currentTable(), "")
        self.assertEqual(m.currentSchema(), "")
        self.assertEqual(len(spy), 1)
        self.assertFalse(spy[-1][0])
        m.setTable("someData", "qgis_test")
        self.assertEqual(len(spy), 2)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        fields = QgsFields()
        fields.append(QgsField("test", QVariant.String))
        conn.createVectorTable(
            "qgis_test",
            "myNewTable",
            fields,
            QgsWkbTypes.Type.Point,
            QgsCoordinateReferenceSystem("EPSG:3857"),
            False,
            {},
        )

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        # tables are not automatically refreshed
        self.assertEqual(text2, text)

        # but setting a new connection should fix this!
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn2 = md.createConnection(self.uri, {})
        md.saveConnection(conn2, "another")
        m.setConnectionName("another", "postgres")
        # ideally there'd be no extra signal here, but it's a minor issue...
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotEqual(text2, text)
        self.assertIn("qgis_test.myNewTable", text2)

        m.setTable("myNewTable", "qgis_test")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        # no auto drop
        conn.dropVectorTable("qgis_test", "myNewTable")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        m.refreshTables()
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("qgis_test.myNewTable", text2)
        self.assertEqual(len(spy), 5)
        self.assertFalse(m.currentSchema())
        self.assertFalse(spy[-1][0])

    def testComboWithEmpty(self):
        """test combobox functionality with empty choice"""
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, "mycon")

        m = QgsDatabaseTableComboBox("postgres", "mycon")
        old_count = m.comboBox().count()
        self.assertGreaterEqual(old_count, 3)

        m.setAllowEmptyTable(True)
        self.assertEqual(m.comboBox().count(), old_count + 1)

        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertFalse(text[0])
        self.assertIn("information_schema.attributes", text)
        self.assertIn("qgis_test.some_poly_data", text)
        self.assertLess(
            text.index("information_schema.attributes"),
            text.index("qgis_test.some_poly_data"),
        )
        self.assertTrue(m.currentSchema())
        self.assertTrue(m.currentTable())

        m.setSchema("information_schema")
        m.setTable("attributes")
        spy = QSignalSpy(m.tableChanged)

        m.setSchema("qgis_test")
        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("information_schema.attributes", text)
        self.assertNotIn("attributes", text)
        self.assertIn("some_poly_data", text)

        self.assertEqual(m.currentTable(), "")
        self.assertEqual(m.currentSchema(), "")
        self.assertEqual(len(spy), 1)
        self.assertFalse(spy[-1][0])

        m.setTable("")
        self.assertEqual(m.currentTable(), "")
        self.assertEqual(m.currentSchema(), "")
        self.assertEqual(len(spy), 1)
        self.assertFalse(spy[-1][0])
        m.setTable("someData")
        self.assertEqual(len(spy), 2)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        fields = QgsFields()
        fields.append(QgsField("test", QVariant.String))
        conn.createVectorTable(
            "qgis_test",
            "myNewTable",
            fields,
            QgsWkbTypes.Type.Point,
            QgsCoordinateReferenceSystem("EPSG:3857"),
            False,
            {},
        )

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        # tables are not automatically refreshed
        self.assertEqual(text2, text)

        # but setting a new connection should fix this!
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn2 = md.createConnection(self.uri, {})
        md.saveConnection(conn2, "another")
        m.setConnectionName("another", "postgres")
        # ideally there'd be no extra signal here, but it's a minor issue...
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentTable(), "someData")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "someData")
        self.assertEqual(spy[-1][1], "qgis_test")

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotEqual(text2, text)
        self.assertIn("myNewTable", text2)

        m.setTable("myNewTable")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        # no auto drop
        conn.dropVectorTable("qgis_test", "myNewTable")
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentTable(), "myNewTable")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "myNewTable")
        self.assertEqual(spy[-1][1], "qgis_test")

        m.refreshTables()
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("myNewTable", text2)
        self.assertEqual(len(spy), 5)
        self.assertFalse(m.currentSchema())
        self.assertFalse(spy[-1][0])


if __name__ == "__main__":
    unittest.main()
