# -*- coding: utf-8 -*-
"""QGIS Unit tests for the HANA provider.

Note: to prepare the DB, you need to run the sql files specified in
tests/testdata/provider/testdata_pg.sh

Read tests/README.md about writing/launching tests with HANA.

Run with ctest -V -R PyQgsHanaProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next
__author__ = 'Maksim Rylov'
__date__ = '2019-11-21'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import psycopg2

import os
import time

from qgis.core import (
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsFeatureRequest,
    QgsFeature,
    QgsFieldConstraints,
    QgsDataProvider,
    NULL,
    QgsVectorLayerUtils,
    QgsSettings,
    QgsTransactionGroup,
    QgsReadWriteContext,
    QgsRectangle,
    QgsDefaultValue,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsWkbTypes,
    QgsGeometry
)
from qgis.gui import QgsGui, QgsAttributeForm
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir, QObject, QByteArray
from qgis.PyQt.QtWidgets import QLabel
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsHanaProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def getSource(self):
        # create temporary table for edit tests
        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test."editData" CASCADE')
        self.execSQLCommand('CREATE TABLE qgis_test."editData" ( pk SERIAL NOT NULL PRIMARY KEY, cnt integer, name text, name2 text, num_char text, geom public.geometry(Point, 4326))')
        self.execSQLCommand("INSERT INTO qgis_test.\"editData\" (pk, cnt, name, name2, num_char, geom) VALUES "
                            "(5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),"
                            "(3, 300, 'Pear', 'PEaR', '3', NULL),"
                            "(1, 100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),"
                            "(2, 200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),"
                            "(4, 400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')")
        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."editData" (geom) sql=',
            'test', 'hana')
        return vl

    def getEditableLayer(self):
        return self.getSource()

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        return set([])

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS


if __name__ == '__main__':
    unittest.main()
