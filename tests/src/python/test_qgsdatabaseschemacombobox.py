"""QGIS Unit tests for QgsDatabaseSchemaComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "8/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsProviderRegistry
from qgis.gui import QgsDatabaseSchemaComboBox
from qgis.testing import unittest

from utilities import start_app, unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsDatabaseSchemaComboBox(unittest.TestCase):

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
        conn = (
            QgsProviderRegistry.instance()
            .providerMetadata("postgres")
            .createConnection(self.uri, {})
        )
        self.assertTrue(conn)

        m = QgsDatabaseSchemaComboBox(conn)
        spy = QSignalSpy(m.schemaChanged)
        self.assertGreaterEqual(m.comboBox().count(), 3)

        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertIn("CamelCase'singlequote'Schema", text)
        self.assertIn("qgis_test", text)
        self.assertLess(
            text.index("CamelCase'singlequote'Schema"), text.index("qgis_test")
        )
        self.assertEqual(m.currentSchema(), "CamelCase'singlequote'Schema")

        m.setSchema("qgis_test")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], "qgis_test")

        m.setSchema("")
        self.assertFalse(m.currentSchema())
        self.assertEqual(len(spy), 2)
        self.assertFalse(spy[-1][0])
        m.setSchema("")
        self.assertEqual(len(spy), 2)
        self.assertFalse(m.currentSchema())

        m.setSchema("qgis_test")
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "qgis_test")

        conn.createSchema("myNewSchema")
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        # schemas are not automatically refreshed
        self.assertEqual(text2, text)

        # but setting a new connection should fix this!
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn2 = md.createConnection(self.uri, {})
        md.saveConnection(conn2, "another")
        m.setConnectionName("another", "postgres")
        # ideally there'd be no extra signal here, but it's a minor issue...
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "qgis_test")

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotEqual(text2, text)
        self.assertIn("myNewSchema", text2)

        m.setSchema("myNewSchema")
        self.assertEqual(len(spy), 5)
        self.assertEqual(m.currentSchema(), "myNewSchema")
        self.assertEqual(spy[-1][0], "myNewSchema")

        # no auto drop
        conn.dropSchema("myNewSchema")
        self.assertEqual(len(spy), 5)
        self.assertEqual(m.currentSchema(), "myNewSchema")
        self.assertEqual(spy[-1][0], "myNewSchema")

        m.refreshSchemas()
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("myNewSchema", text2)
        self.assertEqual(len(spy), 6)
        self.assertFalse(m.currentSchema())
        self.assertFalse(spy[-1][0])

    def testComboWithEmpty(self):
        """test combobox functionality with the empty row"""
        conn = (
            QgsProviderRegistry.instance()
            .providerMetadata("postgres")
            .createConnection(self.uri, {})
        )
        self.assertTrue(conn)

        m = QgsDatabaseSchemaComboBox(conn)
        spy = QSignalSpy(m.schemaChanged)
        old_count = m.comboBox().count()
        self.assertGreaterEqual(m.comboBox().count(), 3)
        m.setAllowEmptySchema(True)
        self.assertEqual(m.comboBox().count(), old_count + 1)

        text = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertFalse(text[0])
        self.assertIn("CamelCase'singlequote'Schema", text)
        self.assertIn("qgis_test", text)
        self.assertLess(
            text.index("CamelCase'singlequote'Schema"), text.index("qgis_test")
        )
        self.assertEqual(m.currentSchema(), "CamelCase'singlequote'Schema")

        m.setSchema("qgis_test")
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], "qgis_test")

        m.setSchema("")
        self.assertEqual(m.comboBox().currentIndex(), 0)
        self.assertFalse(m.currentSchema())
        self.assertEqual(len(spy), 2)
        self.assertFalse(spy[-1][0])
        m.setSchema("")
        self.assertEqual(len(spy), 2)
        self.assertFalse(m.currentSchema())

        m.setSchema("qgis_test")
        self.assertEqual(len(spy), 3)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "qgis_test")

        conn.createSchema("myNewSchema")
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        # schemas are not automatically refreshed
        self.assertEqual(text2, text)

        # but setting a new connection should fix this!
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        conn2 = md.createConnection(self.uri, {})
        md.saveConnection(conn2, "another")
        m.setConnectionName("another", "postgres")
        # ideally there'd be no extra signal here, but it's a minor issue...
        self.assertEqual(len(spy), 4)
        self.assertEqual(m.currentSchema(), "qgis_test")
        self.assertEqual(spy[-1][0], "qgis_test")

        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotEqual(text2, text)
        self.assertIn("myNewSchema", text2)
        self.assertFalse(text2[0])

        m.setSchema("myNewSchema")
        self.assertEqual(len(spy), 5)
        self.assertEqual(m.currentSchema(), "myNewSchema")
        self.assertEqual(spy[-1][0], "myNewSchema")

        # no auto drop
        conn.dropSchema("myNewSchema")
        self.assertEqual(len(spy), 5)
        self.assertEqual(m.currentSchema(), "myNewSchema")
        self.assertEqual(spy[-1][0], "myNewSchema")

        m.refreshSchemas()
        text2 = [m.comboBox().itemText(i) for i in range(m.comboBox().count())]
        self.assertNotIn("myNewSchema", text2)
        self.assertEqual(len(spy), 6)
        self.assertFalse(m.currentSchema())
        self.assertFalse(spy[-1][0])
        self.assertFalse(text2[0])


if __name__ == "__main__":
    unittest.main()
