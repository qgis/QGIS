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
from hdbcli import dbapi

import os
import time

from qgis.core import (
    QgsApplication,
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsFeatureRequest,
    QgsFeature,
    QgsFieldConstraints,
    QgsDataProvider,
    QgsDataSourceUri,
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
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsHanaProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'driver=\'/usr/sap/hdbclient/libodbcHDB.so\' host=10.18.25.248 port=30115 dbname=\'\' user=\'SYSTEM\' password=\'manager\''
        if 'QGIS_HANA_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_HANA_DB']
        uri = QgsDataSourceUri(cls.dbconn)
        cls.conn = dbapi.connect(
            address=uri.host(),
            port=uri.port(),
            user=uri.username(),
            password=uri.password()
        )

        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' key=\'pk\' srid=4326 type=POINT table="qgis_test"."some_data" (GEOM) sql=', 'test', 'hana')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn + ' key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (GEOM) sql=', 'test',
            'hana')
        assert cls.poly_vl.isValid()
        cls.poly_provider = cls.poly_vl.dataProvider()
        QgsGui.editorWidgetRegistry().initEditors()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def execSQLCommand(self, sql):
        self.assertTrue(self.conn)
        cursor = self.conn.cursor()
        self.assertTrue(cursor)
        res = None
        try:
            cursor.execute(sql)
            res = cursor.fetchone
        except Exception as ex:
            print(ex)
        cursor.close()
        self.conn.commit()
        return res

    def execSQLCommands(self, sql, parameters):
        self.assertTrue(self.conn)
        cursor = self.conn.cursor()
        self.assertTrue(cursor)
        res = None
        try:
            cursor.executemany(sql, parameters)
        except Exception as ex:
            print(ex)
        cursor.close()
        self.conn.commit()
        return res

    def dropTableIfExist(self, tableName):
        res = self.execSQLCommand("SELECT * FROM SYS.TABLES WHERE SCHEMA_NAME='qgis_test' AND TABLE_NAME='{}'".format(tableName))
        if res:
            self.execSQLCommand('DROP TABLE "qgis_test"."{}" CASCADE'.format(tableName))

    def getSource(self):
        # create temporary table for edit tests
        self.dropTableIfExist('edit_data')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."edit_data" ( pk INTEGER NOT NULL PRIMARY KEY, cnt INTEGER, name NVARCHAR(100), name2 NVARCHAR(100), num_char NVARCHAR(100), geom ST_GEOMETRY(4326))')
        sql = "INSERT INTO \"qgis_test\".\"edit_data\" (pk, cnt, name, name2, num_char, geom) VALUES (?, ?, ?, ?, ?, ST_GeomFromEWKB(?))"
        args = [[5, -200, None, 'NuLl', '5', bytes.fromhex('0101000020E61000001D5A643BDFC751C01F85EB51B88E5340')],
                [3, 300, 'Pear', 'PEaR', '3', None],
                [1, 100, 'Orange', 'oranGe', '1', bytes.fromhex('0101000020E61000006891ED7C3F9551C085EB51B81E955040')],
                [2, 200, 'Apple', 'Apple', '2', bytes.fromhex('0101000020E6100000CDCCCCCCCC0C51C03333333333B35140')],
                [4, 400, 'Honey', 'Honey', '4', bytes.fromhex('0101000020E610000014AE47E17A5450C03333333333935340')]]
        self.execSQLCommands(sql, args)
        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."edit_data" (GEOM) sql=',
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
    def testDefaultValue(self):
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, True)
        self.assertIsInstance(self.source.defaultValue(0), int)
        self.assertEqual(self.source.defaultValue(1), NULL)
        self.assertEqual(self.source.defaultValue(2), 'qgis')
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)

if __name__ == '__main__':
    unittest.main()
