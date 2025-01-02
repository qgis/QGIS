"""QGIS Unit tests for GdalUtils class

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stefanos Natsis"
__date__ = "03/07/2024"
__copyright__ = "Copyright 2024, The QGIS Project"


import os
import tempfile
from shutil import rmtree

import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.core import (
    QgsApplication,
    QgsRasterLayer,
    QgsDataSourceUri,
    QgsAuthMethodConfig,
)
from processing.algs.gdal.GdalUtils import GdalUtils, GdalConnectionDetails

QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()
os.environ["QGIS_AUTH_DB_DIR_PATH"] = QGIS_AUTH_DB_DIR_PATH

start_app()


class TestProcessingAlgsGdalGdalUtils(QgisTestCase):

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        del os.environ["QGIS_AUTH_DB_DIR_PATH"]
        super().tearDownClass()

    def test_gdal_connection_details_from_layer_postgresraster(self):
        """
        Test GdalUtils.gdal_connection_details_from_layer
        """

        rl = QgsRasterLayer(
            "dbname='mydb' host=localhost port=5432 user='asdf' password='42'"
            " sslmode=disable table=some_table schema=some_schema column=rast sql=pk = 2",
            "pg_layer",
            "postgresraster",
        )

        self.assertEqual(rl.providerType(), "postgresraster")

        connection_details = GdalUtils.gdal_connection_details_from_layer(rl)
        s = connection_details.connection_string

        self.assertTrue(s.lower().startswith("pg:"))
        self.assertTrue("schema='some_schema'" in s)
        self.assertTrue("password='42'" in s)
        self.assertTrue("column='rast'" in s)
        self.assertTrue("mode=1" in s)
        self.assertTrue("where='pk = 2'" in s)
        self.assertEqual(connection_details.format, '"PostGISRaster"')

        # test different uri:
        # - authcfg is expanded
        # - column is parsed
        # - where is skipped
        authm = QgsApplication.authManager()
        self.assertTrue(authm.setMasterPassword("masterpassword", True))
        config = QgsAuthMethodConfig()
        config.setName("Basic")
        config.setMethod("Basic")
        config.setConfig("username", "asdf")
        config.setConfig("password", "42")
        self.assertTrue(authm.storeAuthenticationConfig(config, True))

        rl = QgsRasterLayer(
            f"dbname='mydb' host=localhost port=5432 authcfg={config.id()}"
            f' sslmode=disable table="some_schema"."some_table" (rast)',
            "pg_layer",
            "postgresraster",
        )

        self.assertEqual(rl.providerType(), "postgresraster")

        connection_details = GdalUtils.gdal_connection_details_from_layer(rl)
        s = connection_details.connection_string

        self.assertTrue(s.lower().startswith("pg:"))
        self.assertTrue("schema='some_schema'" in s)
        self.assertTrue("user='asdf'" in s)
        self.assertTrue("password='42'" in s)
        self.assertTrue("column='rast'" in s)
        self.assertTrue("mode=1" in s)
        self.assertFalse("where=" in s)


if __name__ == "__main__":
    unittest.main()
