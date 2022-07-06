# -*- coding: utf-8 -*-
"""QGIS Unit tests for Oracle QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Julien Cabieces'
__date__ = '28/12/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.core import (
    QgsVectorLayer,
    QgsProviderRegistry,
    QgsDataSourceUri,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,

)
from qgis.testing import unittest
from qgis.PyQt.QtSql import QSqlDatabase, QSqlQuery


class TestPyQgsProviderConnectionOracle(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'oracle'

    # there is no service for oracle provider test so we need to save user and password
    # to keep them when storing/loading connections in parent class _test_save_load method
    configuration = {"saveUsername": True, "savePassword": True, "onlyExistingTypes": True}

    defaultSchema = 'QGIS'

    # need to override this because tables with geometries need to be uppercase
    myNewTable = 'MYNEWTABLE'
    myVeryNewTable = 'MYVERYNEWTABLE'
    myUtf8Table = 'MYUTF8\U0001F604TABLE'
    geometryColumnName = 'GEOM'

    # Provider test cases can define a schema and table name for SQL query layers test
    sqlVectorLayerSchema = 'QGIS'
    sqlVectorLayerTable = 'SOME_DATA'

    def execSQLCommand(self, sql, ignore_errors=False):
        self.assertTrue(self.conn)
        query = QSqlQuery(self.conn)
        res = query.exec_(sql)
        if not ignore_errors:
            self.assertTrue(res, sql + ': ' + query.lastError().text())
        query.finish()

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        TestPyQgsProviderConnectionBase.setUpClass()

        # These are the connection details for the Docker Oracle instance running on Travis
        cls.dbconn = "host=localhost/XEPDB1 port=1521 user='QGIS' password='qgis'"
        if 'QGIS_ORACLETEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_ORACLETEST_DB']

        cls.uri = cls.dbconn

        cls.conn = QSqlDatabase.addDatabase('QOCISPATIAL', "oracletest")
        cls.conn.setDatabaseName('localhost/XEPDB1')
        if 'QGIS_ORACLETEST_DBNAME' in os.environ:
            cls.conn.setDatabaseName(os.environ['QGIS_ORACLETEST_DBNAME'])
        cls.conn.setUserName('QGIS')
        cls.conn.setPassword('qgis')

        # Start clean
        md = QgsProviderRegistry.instance().providerMetadata('oracle')
        conn = md.createConnection(cls.dbconn, {})

        for table_name in (cls.myNewTable, cls.myVeryNewTable):

            try:
                conn.dropVectorTable('QGIS', table_name)
            except QgsProviderConnectionException:
                pass

            try:
                conn.executeSql(f"DELETE FROM user_sdo_geom_metadata WHERE TABLE_NAME = '{table_name}'")
            except QgsProviderConnectionException:
                pass

        assert cls.conn.open()

    def test_tables_with_options(self):

        md = QgsProviderRegistry.instance().providerMetadata('oracle')

        def get_tables(schema, configuration, flags=QgsAbstractDatabaseProviderConnection.TableFlags()):
            conn = md.createConnection(self.uri, configuration)
            tables = conn.tables(schema, flags)
            return sorted([table.tableName() for table in tables if table.tableName() in [
                'DATE_TIMES', 'GENERATED_COLUMNS', 'LINE_DATA', 'OTHER_TABLE', 'POINT_DATA', 'POINT_DATA_IDENTITY', 'POLY_DATA', 'SOME_DATA', 'SOME_POLY_DATA']])

        # all tables
        self.assertEqual(get_tables('QGIS', {}),
                         ['DATE_TIMES', 'GENERATED_COLUMNS', 'LINE_DATA', 'POINT_DATA', 'POINT_DATA_IDENTITY', 'POLY_DATA', 'SOME_DATA', 'SOME_POLY_DATA'])

        # only non-spatial tables
        self.assertEqual(get_tables('QGIS', {}, QgsAbstractDatabaseProviderConnection.Aspatial),
                         ['DATE_TIMES', 'GENERATED_COLUMNS'])

        # only vector tables
        self.assertEqual(get_tables('QGIS', {}, QgsAbstractDatabaseProviderConnection.Vector),
                         ['LINE_DATA', 'POINT_DATA', 'POINT_DATA_IDENTITY', 'POLY_DATA', 'SOME_DATA', 'SOME_POLY_DATA'])

        # only table existing in sdo_geom_metadata table
        self.assertEqual(get_tables('QGIS', {"geometryColumnsOnly": True}, QgsAbstractDatabaseProviderConnection.Vector),
                         ['SOME_DATA', 'SOME_POLY_DATA'])

        self.execSQLCommand('DROP TABLE OTHER_USER.OTHER_TABLE', ignore_errors=True)
        self.execSQLCommand('DROP USER OTHER_USER CASCADE', ignore_errors=True)
        self.execSQLCommand('CREATE USER OTHER_USER')
        self.execSQLCommand('GRANT ALL PRIVILEGES TO OTHER_USER')
        self.execSQLCommand('CREATE TABLE OTHER_USER.OTHER_TABLE ( "pk" INTEGER PRIMARY KEY, GEOM SDO_GEOMETRY)')

        # if a schema is specified, schema (i.e. user) tables are returned, whatever userTablesOnly value
        self.assertEqual(get_tables('OTHER_USER', {"userTablesOnly": True}),
                         ['OTHER_TABLE'])

        self.assertEqual(get_tables('OTHER_USER', {"userTablesOnly": False}),
                         ['OTHER_TABLE'])

        # no schema is specified, all user tables (vector ones in this case) are returned
        self.assertEqual(get_tables('', {"userTablesOnly": True}, QgsAbstractDatabaseProviderConnection.Vector),
                         ['LINE_DATA', 'POINT_DATA', 'POINT_DATA_IDENTITY', 'POLY_DATA', 'SOME_DATA', 'SOME_POLY_DATA'])

        # no schema is specified, all tables (vector ones in this case) tables are returned
        self.assertEqual(get_tables('', {"userTablesOnly": False}, QgsAbstractDatabaseProviderConnection.Vector),
                         ['LINE_DATA', 'OTHER_TABLE', 'POINT_DATA', 'POINT_DATA_IDENTITY', 'POLY_DATA', 'SOME_DATA', 'SOME_POLY_DATA'])

    def test_configuration(self):
        """Test storage and retrieval for configuration parameters"""

        uri = ("authcfg='test_cfg' dbname='qgis_test' username='QGIS' password='qgis' dbworkspace='workspace' "
               "estimatedMetadata='true' host='localhost' port='1521' dboptions='test_opts' ")

        md = QgsProviderRegistry.instance().providerMetadata('oracle')
        conn = md.createConnection(uri, {"saveUsername": True, "savePassword": True})
        ds_uri = QgsDataSourceUri(conn.uri())
        self.assertEqual(ds_uri.username(), 'QGIS')
        self.assertEqual(ds_uri.host(), 'localhost')
        self.assertEqual(ds_uri.port(), '1521')
        self.assertEqual(ds_uri.database(), 'qgis_test')
        self.assertTrue(ds_uri.useEstimatedMetadata())
        self.assertEqual(ds_uri.password(), 'qgis')
        self.assertEqual(ds_uri.param('dboptions'), 'test_opts')
        self.assertEqual(ds_uri.param('dbworkspace'), 'workspace')

        conn.store('myconf')
        conn = md.findConnection('myconf', False)
        ds_uri = QgsDataSourceUri(conn.uri())
        self.assertEqual(ds_uri.username(), 'QGIS')
        self.assertEqual(ds_uri.host(), 'localhost')
        self.assertEqual(ds_uri.port(), '1521')
        self.assertEqual(ds_uri.database(), 'qgis_test')
        self.assertTrue(ds_uri.useEstimatedMetadata())
        self.assertEqual(ds_uri.password(), 'qgis')
        self.assertEqual(ds_uri.param('dboptions'), 'test_opts')
        self.assertEqual(ds_uri.param('dbworkspace'), 'workspace')
        conn.remove('myconf')

    def test_pkcols(self):
        """Test retrieval of primary columns"""

        self.execSQLCommand("""CREATE OR REPLACE VIEW "QGIS"."SOME_DATA_VIEW" AS SELECT * FROM "QGIS"."SOME_DATA" """)

        md = QgsProviderRegistry.instance().providerMetadata('oracle')
        conn = md.createConnection(self.uri, {})
        tables = conn.tables('QGIS')

        tables_dict = dict([(table.tableName(), table.primaryKeyColumns()) for table in tables])

        self.assertEqual(sorted(tables_dict['SOME_DATA_VIEW']), ['GEOM', 'cnt', 'date', 'dt', 'name', 'name2', 'num_char', 'pk', 'time'])
        self.assertEqual(sorted(tables_dict['SOME_DATA']), ['pk'])
        self.assertEqual(sorted(tables_dict['POINT_DATA_IDENTITY']), ['pk'])

    def test_schemas(self):
        """Test schemas retrieval"""

        # may be added by previous test
        self.execSQLCommand('DROP USER OTHER_USER CASCADE', ignore_errors=True)

        md = QgsProviderRegistry.instance().providerMetadata('oracle')
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.schemas(), ['QGIS'])


if __name__ == '__main__':
    unittest.main()
