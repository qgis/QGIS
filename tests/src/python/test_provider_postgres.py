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

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt4.QtCore import QSettings
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
from providertestutils import runGetFeatureTests

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()

class TestPyQgsPostgresProvider(TestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # Create test database
        cls.vl = QgsVectorLayer( u'service=\'qgis_test\' key=\'pk\' table="qgis_test"."someData" sql=', 'test', 'postgres' )


    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

        # Delete test database

    def testGetFeaturesUncompiled(self):
        QSettings().setValue( "/qgis/postgres/compileExpressions", False )
        runGetFeatureTests( self.vl )

    def testGetFeaturesCompiled(self):
        QSettings().setValue( "/qgis/postgres/compileExpressions", True )
        runGetFeatureTests( self.vl )

if __name__ == '__main__':
    unittest.main()
