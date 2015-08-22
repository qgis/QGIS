# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
from qgis.core import NULL

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt4.QtCore import QSettings
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       unittest,
                       TestCase
                       )
from providertestbase import ProviderTestCase

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresProvider(TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.vl = QgsVectorLayer(u'dbname=\'qgis_test\' host=localhost port=5432 user=\'postgres\' password=\'postgres\' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/postgres/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/postgres/compileExpressions', False)

# HERE GO THE PROVIDER SPECIFIC TESTS
    def testDefaultValue(self):
        assert self.provider.defaultValue(0) == u'nextval(\'qgis_test."someData_pk_seq"\'::regclass)'
        assert self.provider.defaultValue(1) == NULL
        assert self.provider.defaultValue(2) == '\'qgis\'::text'

if __name__ == '__main__':
    unittest.main()
