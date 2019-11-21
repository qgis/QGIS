# -*- coding: utf-8 -*-
"""QGIS Unit tests for AFS QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '21/11/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
import tempfile
from test_qgsproviderconnection_base import TestPyQgsProviderWebServiceConnectionBase
from qgis.core import (
    QgsWkbTypes,
    QgsAbstractWebServiceProviderConnection,
    QgsProviderConnectionException,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProviderRegistry,
    QgsFields,
    QgsCoordinateReferenceSystem,
)
from qgis.testing import unittest
from utilities import unitTestDataPath
from test_provider_afs import createTestLayer

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnectionAfs(unittest.TestCase, TestPyQgsProviderWebServiceConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'arcgisfeatureserver'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderWebServiceConnectionBase.setUpClass()
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        cls.vl = createTestLayer(cls.basetestpath)
        cls.uri = cls.vl.publicSource()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass


if __name__ == '__main__':
    unittest.main()
