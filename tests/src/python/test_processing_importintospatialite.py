"""QGIS Unit tests for Processing Export to SpatiaLite algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "2025-04-27"
__copyright__ = "Copyright 2025, Alexander Bruy"

import os
import tempfile
import unittest

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsApplication,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsSettings,
    QgsVectorLayer,
)
from qgis.analysis import QgsNativeAlgorithms
from qgis.testing import start_app, QgisTestCase
from qgis.utils import spatialite_connect

from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute

from utilities import unitTestDataPath

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):
    _errors = []

    def reportError(self, error, fatalError=False):
        print(error)
        self._errors.append(error)


class TestExportToSpatialite(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        # create test db
        cls.db_file = os.path.join(tempfile.gettempdir(), "test.sqlite")
        if os.path.exists(cls.db_file):
            os.remove(cls.db_file)
        con = spatialite_connect(cls.db_file, isolation_level=None)
        cur = con.cursor()
        cur.execute("BEGIN")
        sql = "SELECT InitSpatialMetadata()"
        cur.execute(sql)

        # simple table with primary key
        sql = "CREATE TABLE test(id INTEGER NOT NULL PRIMARY KEY, name TEXT NOT NULL)"
        cur.execute(sql)
        sql = "SELECT AddGeometryColumn('test', 'geometry', 4326, 'POLYGON', 'XY')"
        cur.execute(sql)
        sql = "INSERT INTO test(id, name, geometry) VALUES (1, 'test 1', GeomFromText('POLYGON((0 0,1 0,1 1,0 1,0 0))', 4326))"
        cur.execute(sql)
        cur.execute("COMMIT")
        con.close()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsExportToSpatialite.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsExportToSpatialite")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()

        # Create DB connection in the settings
        settings = QgsSettings()
        settings.beginGroup("/SpatiaLite/connections/qgis_test")
        settings.setValue("sqlitepath", cls.db_file)

    def test_import_unregistered_db(self):
        """Test import into unregistered database"""

        input_file = unitTestDataPath() + "/points.shp"
        alg = self.registry.createAlgorithmById("native:importintospatialite")
        self.assertIsNotNone(alg)

        table_name = "points1"

        parameters = {
            "INPUT": input_file,
            "DATABASE": self.db_file,
            "TABLENAME": table_name,
            "PRIMARY_KEY": None,
            "GEOMETRY_COLUMN": "geom",
            "ENCODING": "UTF-8",
            "OVERWRITE": True,
            "CREATEINDEX": True,
            "LOWERCASE_NAMES": True,
            "DROP_STRING_LENGTH": False,
            "FORCE_SINGLEPART": False,
        }

        feedback = ConsoleFeedBack()
        context = QgsProcessingContext()
        # Note: the following returns true also in case of errors
        self.assertTrue(execute(alg, parameters, context, feedback))
        # so we check the log
        self.assertEqual(feedback._errors, [])

        # Check that data have been imported correctly
        exported = QgsVectorLayer(input_file, "exported")
        self.assertTrue(exported.isValid())
        imported = QgsVectorLayer(
            f"dbname='{self.db_file}' table=\"{table_name}\" (geom) sql=",
            "test",
            "spatialite",
        )
        self.assertTrue(imported.isValid())
        imported_fields = [f.name().lower() for f in imported.fields()]
        for f in exported.fields():
            self.assertIn(f.name().lower(), imported_fields)

        # Check data
        imported_f = next(imported.getFeatures("class = 'Jet' AND heading = 85"))
        self.assertTrue(imported_f.isValid())
        exported_f = next(exported.getFeatures("class = 'Jet' AND heading = 85"))
        self.assertTrue(exported_f.isValid())
        self.assertEqual(exported_f.geometry().asWkt(), imported_f.geometry().asWkt())

        def test_import_registered_db(self):
            """Test import into registered database"""

            input_file = unitTestDataPath() + "/points.shp"
            alg = self.registry.createAlgorithmById(
                "native:importintospatialiteregistered"
            )
            self.assertIsNotNone(alg)

            table_name = "points2"

            parameters = {
                "INPUT": input_file,
                "DATABASE": "qgis_test",
                "TABLENAME": table_name,
                "PRIMARY_KEY": None,
                "GEOMETRY_COLUMN": "geom",
                "ENCODING": "UTF-8",
                "OVERWRITE": True,
                "CREATEINDEX": True,
                "LOWERCASE_NAMES": True,
                "DROP_STRING_LENGTH": False,
                "FORCE_SINGLEPART": False,
            }

            feedback = ConsoleFeedBack()
            context = QgsProcessingContext()
            # Note: the following returns true also in case of errors
            self.assertTrue(execute(alg, parameters, context, feedback))
            # so we check the log
            self.assertEqual(feedback._errors, [])

            # check that data have been imported correctly
            exported = QgsVectorLayer(input_file, "exported")
            self.assertTrue(exported.isValid())
            imported = QgsVectorLayer(
                f"dbname='{self.db_file}' table=\"{table_name}\" (geom) sql=",
                "test",
                "spatialite",
            )
            self.assertTrue(imported.isValid())
            imported_fields = [f.name().lower() for f in imported.fields()]
            for f in exported.fields():
                self.assertIn(f.name().lower(), imported_fields)

            # check data
            imported_f = next(imported.getFeatures("class = 'Jet' AND heading = 85"))
            self.assertTrue(imported_f.isValid())
            exported_f = next(exported.getFeatures("class = 'Jet' AND heading = 85"))
            self.assertTrue(exported_f.isValid())
            self.assertEqual(
                exported_f.geometry().asWkt(), imported_f.geometry().asWkt()
            )


if __name__ == "__main__":
    unittest.main()
