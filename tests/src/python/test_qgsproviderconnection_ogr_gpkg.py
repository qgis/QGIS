# -*- coding: utf-8 -*-
"""QGIS Unit tests for OGR GeoPackage QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '10/08/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.core import (
    QgsWkbTypes,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
    QgsVectorLayer,
    QgsProviderRegistry,
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnectionGpkg(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'ogr'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderConnectionBase.setUpClass()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_path = '{}/qgis_server/test_project_wms_grouped_layers_test.gpkg'.format(TEST_DATA_DIR)
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.gpkg_path)


if __name__ == '__main__':
    unittest.main()
