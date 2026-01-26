"""QGIS Unit tests for QgsStoredQueryManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.PyQt.QtCore import QCoreApplication, QLocale
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsProject,
    QgsSettings,
)
from qgis.gui import QgsGui
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsStoredQueryManager(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsStoredQueryManager.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsStoredQueryManager")
        QgsSettings().clear()
        QLocale.setDefault(QLocale(QLocale.Language.English))
        start_app()

    def test_project_storage(self):
        """
        Test storage of queries in a project
        """
        p = QgsProject.instance()
        manager = QgsGui.storedQueryManager()
        QgsSettings().clear()
        QgsProject.instance().clear()

        self.assertFalse(manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject))
        self.assertFalse(manager.query("test"))

        signal_added = QSignalSpy(manager.queryAdded)
        signal_changed = QSignalSpy(manager.queryChanged)
        signal_removed = QSignalSpy(manager.queryRemoved)
        manager.storeQuery(
            "first query",
            "select * from something",
            Qgis.QueryStorageBackend.CurrentProject,
        )
        self.assertEqual(len(signal_added), 1)
        self.assertEqual(signal_added[-1][0], "first query")
        self.assertEqual(len(signal_changed), 0)
        self.assertEqual(len(signal_removed), 0)

        self.assertEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject),
            ["first query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.CurrentProject),
            "select * from something",
        )
        self.assertFalse(manager.query("test"))

        manager.storeQuery(
            "ANOTHER query",
            """INSERT INTO Students (name) VALUES ('Robert');
DROP TABLE Students;--');""",
            Qgis.QueryStorageBackend.CurrentProject,
        )
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(signal_added[-1][0], "ANOTHER query")
        self.assertEqual(len(signal_changed), 0)
        self.assertEqual(len(signal_removed), 0)

        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject),
            ["first query", "ANOTHER query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.CurrentProject),
            "select * from something",
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.CurrentProject),
            """INSERT INTO Students (name) VALUES ('Robert');
DROP TABLE Students;--');""",
        )
        self.assertFalse(manager.query("test"))

        # update an existing query
        manager.storeQuery(
            "ANOTHER query",
            "DROP TABLE Students",
            Qgis.QueryStorageBackend.CurrentProject,
        )
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(signal_changed[-1][0], "ANOTHER query")
        self.assertEqual(len(signal_removed), 0)

        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject),
            ["first query", "ANOTHER query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.CurrentProject),
            "select * from something",
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.CurrentProject),
            "DROP TABLE Students",
        )
        self.assertFalse(manager.query("test"))

        # remove a query which doesn't exist
        manager.removeQuery("xxx", Qgis.QueryStorageBackend.CurrentProject)
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(len(signal_removed), 0)
        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject),
            ["first query", "ANOTHER query"],
        )

        # remove a query
        manager.removeQuery("first query", Qgis.QueryStorageBackend.CurrentProject)
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(len(signal_removed), 1)
        self.assertEqual(signal_removed[-1][0], "first query")
        self.assertEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject),
            ["ANOTHER query"],
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.CurrentProject),
            "DROP TABLE Students",
        )
        p.clear()
        self.assertFalse(manager.allQueryNames(Qgis.QueryStorageBackend.CurrentProject))

    def test_local_profile_storage(self):
        """
        Test storage of queries in the local profile
        """
        QgsSettings().clear()
        QgsProject.instance().clear()
        manager = QgsGui.storedQueryManager()

        self.assertFalse(manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile))
        self.assertFalse(manager.query("test"))

        signal_added = QSignalSpy(manager.queryAdded)
        signal_changed = QSignalSpy(manager.queryChanged)
        signal_removed = QSignalSpy(manager.queryRemoved)
        manager.storeQuery(
            "first query",
            "select * from something",
            Qgis.QueryStorageBackend.LocalProfile,
        )
        self.assertEqual(len(signal_added), 1)
        self.assertEqual(signal_added[-1][0], "first query")
        self.assertEqual(len(signal_changed), 0)
        self.assertEqual(len(signal_removed), 0)

        self.assertEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile),
            ["first query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.LocalProfile),
            "select * from something",
        )
        self.assertFalse(manager.query("test"))

        manager.storeQuery(
            "ANOTHER query",
            """INSERT INTO Students (name) VALUES ('Robert');
DROP TABLE Students;--');""",
            Qgis.QueryStorageBackend.LocalProfile,
        )
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(signal_added[-1][0], "ANOTHER query")
        self.assertEqual(len(signal_changed), 0)
        self.assertEqual(len(signal_removed), 0)

        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile),
            ["first query", "ANOTHER query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.LocalProfile),
            "select * from something",
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.LocalProfile),
            """INSERT INTO Students (name) VALUES ('Robert');
DROP TABLE Students;--');""",
        )
        self.assertFalse(manager.query("test"))

        # update an existing query
        manager.storeQuery(
            "ANOTHER query",
            "DROP TABLE Students",
            Qgis.QueryStorageBackend.LocalProfile,
        )
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(signal_changed[-1][0], "ANOTHER query")
        self.assertEqual(len(signal_removed), 0)

        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile),
            ["first query", "ANOTHER query"],
        )
        self.assertEqual(
            manager.query("first query", Qgis.QueryStorageBackend.LocalProfile),
            "select * from something",
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.LocalProfile),
            "DROP TABLE Students",
        )
        self.assertFalse(manager.query("test"))

        # remove a query which doesn't exist
        manager.removeQuery("xxx", Qgis.QueryStorageBackend.LocalProfile)
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(len(signal_removed), 0)
        self.assertCountEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile),
            ["first query", "ANOTHER query"],
        )

        # remove a query
        manager.removeQuery("first query", Qgis.QueryStorageBackend.LocalProfile)
        self.assertEqual(len(signal_added), 2)
        self.assertEqual(len(signal_changed), 1)
        self.assertEqual(len(signal_removed), 1)
        self.assertEqual(signal_removed[-1][0], "first query")
        self.assertEqual(
            manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile),
            ["ANOTHER query"],
        )
        self.assertEqual(
            manager.query("ANOTHER query", Qgis.QueryStorageBackend.LocalProfile),
            "DROP TABLE Students",
        )
        QgsSettings().clear()
        self.assertFalse(manager.allQueryNames(Qgis.QueryStorageBackend.LocalProfile))

    def test_all_queries(self):
        """
        Test retrieving all queries.
        """
        QgsSettings().clear()
        QgsProject.instance().clear()
        manager = QgsGui.storedQueryManager()

        manager.storeQuery(
            "first project query",
            "select * from something",
            Qgis.QueryStorageBackend.CurrentProject,
        )
        manager.storeQuery(
            "SECOND project query",
            "select * from something_else",
            Qgis.QueryStorageBackend.CurrentProject,
        )

        manager.storeQuery(
            "first PROFILE query",
            "select * from something3",
            Qgis.QueryStorageBackend.LocalProfile,
        )
        manager.storeQuery(
            "2 PROFILE query",
            "select * from something4",
            Qgis.QueryStorageBackend.LocalProfile,
        )

        res = manager.allQueries()
        self.assertEqual(len(res), 4)
        self.assertEqual(res[0].name, "2 PROFILE query")
        self.assertEqual(res[0].definition, "select * from something4")
        self.assertEqual(res[0].backend, Qgis.QueryStorageBackend.LocalProfile)
        self.assertEqual(res[1].name, "first PROFILE query")
        self.assertEqual(res[1].definition, "select * from something3")
        self.assertEqual(res[1].backend, Qgis.QueryStorageBackend.LocalProfile)
        self.assertEqual(res[2].name, "first project query")
        self.assertEqual(res[2].definition, "select * from something")
        self.assertEqual(res[2].backend, Qgis.QueryStorageBackend.CurrentProject)
        self.assertEqual(res[3].name, "SECOND project query")
        self.assertEqual(res[3].definition, "select * from something_else")
        self.assertEqual(res[3].backend, Qgis.QueryStorageBackend.CurrentProject)


if __name__ == "__main__":
    unittest.main()
