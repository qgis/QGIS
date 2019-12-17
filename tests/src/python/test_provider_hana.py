# -*- coding: utf-8 -*-
"""QGIS Unit tests for the SAP HANA provider.

Read tests/README.md about writing/launching tests with HANA.

Run with ctest -V -R PyQgsHanaProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = 'Maksim Rylov'
__date__ = '2019-11-21'
__copyright__ = 'Copyright 2019, The QGIS Project'

import base64
import os

from hdbcli import dbapi
from providertestbase import ProviderTestCase
from PyQt5.QtCore import QVariant, QDate, QTime, QDateTime, QByteArray
from qgis.core import (
    NULL,
    QgsVectorLayer,
    QgsDataProvider,
    QgsDataSourceUri,
    QgsSettings,
    QgsCoordinateReferenceSystem,
    QgsFeatureRequest,
    QgsFeature)
from qgis.gui import QgsGui
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


def execSQLCommand(conn, sql):
    assert conn
    cursor = conn.cursor()
    assert cursor
    res = None
    try:
        cursor.execute(sql)
        cursor.fetchone()
        res = True
    except:
        pass
    cursor.close()
    conn.commit()
    return res


def execSQLCommands(conn, sql, parameters):
    assert conn
    cursor = conn.cursor()
    assert cursor
    res = None
    try:
        cursor.executemany(sql, parameters)
    except Exception as ex:
        print(ex)
    cursor.close()
    conn.commit()
    return res


def createDefaultTables(conn):
    execSQLCommand(conn, 'DROP SCHEMA "qgis_test" CASCADE')
    execSQLCommand(conn, 'CREATE SCHEMA "qgis_test"')

    execSQLCommand(conn,
                   'CREATE TABLE "qgis_test"."some_data" ( '
                   '"pk" INTEGER NOT NULL PRIMARY KEY,'
                   '"cnt" INTEGER,'
                   '"name" NVARCHAR(32) DEFAULT \'qgis\','
                   '"name2" NVARCHAR(32) DEFAULT \'qgis\','
                   '"num_char" NVARCHAR(1),'
                   '"geom" ST_GEOMETRY(4326))')
    execSQLCommand(conn, 'COMMENT ON TABLE "qgis_test"."some_data" IS \'QGIS Test Table\'')
    sql = 'INSERT INTO "qgis_test"."some_data" ("pk", "cnt", "name", "name2", "num_char", "geom") VALUES (?, ?, ' \
          '?, ?, ?, ST_GeomFromEWKB(?)) '
    args = [[5, -200, None, 'NuLl', '5', bytes.fromhex('0101000020E61000001D5A643BDFC751C01F85EB51B88E5340')],
            [3, 300, 'Pear', 'PEaR', '3', None],
            [1, 100, 'Orange', 'oranGe', '1', bytes.fromhex('0101000020E61000006891ED7C3F9551C085EB51B81E955040')],
            [2, 200, 'Apple', 'Apple', '2', bytes.fromhex('0101000020E6100000CDCCCCCCCC0C51C03333333333B35140')],
            [4, 400, 'Honey', 'Honey', '4', bytes.fromhex('0101000020E610000014AE47E17A5450C03333333333935340')]]
    execSQLCommands(conn, sql, args)

    execSQLCommand(conn,
                   'CREATE TABLE "qgis_test"."some_poly_data" ( '
                   '"pk" INTEGER NOT NULL PRIMARY KEY,'
                   '"geom" ST_GEOMETRY(4326))')
    sql = 'INSERT INTO "qgis_test"."some_poly_data" ("pk", "geom") VALUES (?, ST_GeomFromText(?, 4326))'
    args = [[1, 'Polygon ((-69.0 81.4, -69.0 80.2, -73.7 80.2, -73.7 76.3, -74.9 76.3, -74.9 81.4, -69.0 81.4))'],
            [2, 'Polygon ((-67.6 81.2, -66.3 81.2, -66.3 76.9, -67.6 76.9, -67.6 81.2))'],
            [3, 'Polygon ((-68.4 75.8, -67.5 72.6, -68.6 73.7, -70.2 72.9, -68.4 75.8))'],
            [4, None]]
    execSQLCommands(conn, sql, args)


class TestPyQgsHanaProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'driver=\'/usr/sap/hdbclient/libodbcHDB.so\' host=localhost port=30015 dbname=\'\' ' \
                     'user=\'SYSTEM\' password=\'mypassword\' '
        if 'QGIS_HANA_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_HANA_DB']
        uri = QgsDataSourceUri(cls.dbconn)
        cls.conn = dbapi.connect(
            address=uri.host(),
            port=uri.port(),
            user=uri.username(),
            password=uri.password()
        )

        createDefaultTables(cls.conn)

        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' key=\'pk\' srid=4326 type=POINT table="qgis_test"."some_data" (geom) sql=', 'test', 'hana')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn + ' key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test',
            'hana')
        assert cls.poly_vl.isValid()
        cls.poly_provider = cls.poly_vl.dataProvider()
        QgsGui.editorWidgetRegistry().initEditors()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def execSQLCommand(self, sql):
        return execSQLCommand(self.conn, sql)

    def execSQLCommands(self, sql, parameters):
        return execSQLCommands(self.conn, sql, parameters)

    def dropTableIfExist(self, tableName):
        res = self.execSQLCommand(
            "SELECT * FROM SYS.TABLES WHERE SCHEMA_NAME='qgis_test' AND TABLE_NAME='{}'".format(tableName))
        if res:
            self.execSQLCommand('DROP TABLE "qgis_test"."{}" CASCADE'.format(tableName))

    def getSource(self):
        # create temporary table for edit tests
        self.dropTableIfExist('edit_data')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."edit_data" ( '
            '"pk" INTEGER NOT NULL PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,'
            '"cnt" INTEGER,'
            '"name" NVARCHAR(100), '
            '"name2" NVARCHAR(100), '
            '"num_char" NVARCHAR(100),'
            '"geom" ST_GEOMETRY(4326))')
        sql = 'INSERT INTO "qgis_test"."edit_data" ("pk", "cnt", "name", "name2", "num_char", "geom") VALUES (?, ' \
              '?, ?, ?, ?, ST_GeomFromEWKB(?)) '
        args = [[5, -200, None, 'NuLl', '5', bytes.fromhex('0101000020E61000001D5A643BDFC751C01F85EB51B88E5340')],
                [3, 300, 'Pear', 'PEaR', '3', None],
                [1, 100, 'Orange', 'oranGe', '1', bytes.fromhex('0101000020E61000006891ED7C3F9551C085EB51B81E955040')],
                [2, 200, 'Apple', 'Apple', '2', bytes.fromhex('0101000020E6100000CDCCCCCCCC0C51C03333333333B35140')],
                [4, 400, 'Honey', 'Honey', '4', bytes.fromhex('0101000020E610000014AE47E17A5450C03333333333935340')]]
        self.execSQLCommands(sql, args)
        vl = QgsVectorLayer(
            self.dbconn + ' key=\'pk\' srid=4326 type=POINT table="qgis_test"."edit_data" (geom) sql=',
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
        filters = set([
            '(name = \'Apple\') is not null',
            'false and NULL',
            'true and NULL',
            'NULL and false',
            'NULL and true',
            'NULL and NULL',
            'false or NULL',
            'true or NULL',
            'NULL or false',
            'NULL or true',
            'NULL or NULL',
            '\'x\' || "name" IS NOT NULL',
            '\'x\' || "name" IS NULL',
            'radians(cnt) < 2',
            'degrees(pk) <= 200',
            'log10(pk) < 0.5',
            'x($geometry) < -70',
            'y($geometry) > 70',
            'xmin($geometry) < -70',
            'ymin($geometry) > 70',
            'xmax($geometry) < -70',
            'ymax($geometry) > 70',
            'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
            'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
            'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
            'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7',
            'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
            'x($geometry) < -70',
            'y($geometry) > 79',
            'xmin($geometry) < -70',
            'ymin($geometry) < 76',
            'xmax($geometry) > -68',
            'ymax($geometry) > 80',
            'area($geometry) > 10',
            'perimeter($geometry) < 12',
            'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
            'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
            'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
            'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
            'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
            'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))'
        ])
        return filters

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testMetadata(self):
        """ Test that metadata is correctly acquired from provider """
        metadata = self.vl.metadata()
        self.assertEqual(metadata.crs(), QgsCoordinateReferenceSystem.fromEpsgId(4326))
        self.assertEqual(metadata.type(), 'dataset')
        self.assertEqual(metadata.abstract(), 'QGIS Test Table')

    def testDefaultValue(self):
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, True)
        self.assertEqual(self.source.defaultValue(0), NULL)
        self.assertEqual(self.source.defaultValue(1), NULL)
        self.assertEqual(self.source.defaultValue(2), 'qgis')
        self.assertEqual(self.source.defaultValue(3), 'qgis')
        self.assertEqual(self.source.defaultValue(4), NULL)
        self.source.setProviderProperty(QgsDataProvider.EvaluateDefaultValues, False)

    def testBooleanType(self):
        self.dropTableIfExist('date_times')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."boolean_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"fld1" BOOLEAN)')

        sql = 'INSERT INTO "qgis_test"."boolean_type" ("id", "fld1") VALUES (?, ?)'
        args = [[1, 'TRUE'],
                [2, 'FALSE'],
                [3, None]]
        self.execSQLCommands(sql, args)

        vl = QgsVectorLayer(self.dbconn + ' table="qgis_test"."boolean_type" sql=', "testbool", "hana")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('fld1')).type(), QVariant.Bool)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {
            1: True,
            2: False,
            3: NULL
        }
        self.assertEqual(values, expected)

    def testDateTimeTypes(self):
        self.dropTableIfExist('date_time_type')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."date_time_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"date_field" DATE,'
            '"time_field" TIME,'
            '"datetime_field" TIMESTAMP)')

        sql = 'INSERT INTO "qgis_test"."date_time_type" ("id", "date_field", "time_field", "datetime_field") VALUES (?, ' \
              '?, ?, ?)'
        args = [[1, '2004-03-04', '13:41:52', '2004-03-04 13:41:52']]
        self.execSQLCommands(sql, args)

        vl = QgsVectorLayer(self.dbconn + ' table="qgis_test"."date_time_type" sql=', "testdatetimes", "hana")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('datetime_field')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField('date_field')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fields().lookupField('time_field')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fields().lookupField('datetime_field')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testBinaryType(self):
        self.dropTableIfExist('binary_type')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."binary_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"blob" VARBINARY(114))')

        sql = 'INSERT INTO "qgis_test"."binary_type" ("id", "blob") VALUES (?, ?)'
        args = [[1, base64.b64encode(b'binvalue')],
                [2, None]]
        self.execSQLCommands(sql, args)

        vl = QgsVectorLayer(self.dbconn + ' table="qgis_test"."binary_type" sql=', "testbinary", "hana")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('blob')).type(), QVariant.ByteArray)
        self.assertEqual(fields.at(fields.indexFromName('blob')).length(), 114)

        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'YmludmFsdWU='),
            2: QByteArray()
        }
        self.assertEqual(values, expected)

        # editing binary values
        self.dropTableIfExist('binary_type_edit')
        self.execSQLCommand(
            'CREATE TABLE "qgis_test"."binary_type_edit" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"blob" VARBINARY(1000))')

        self.execSQLCommands('INSERT INTO "qgis_test"."binary_type_edit" ("id", "blob") VALUES (?, ?)',
                             [[1, base64.b64encode(b'bbb')]])

        vl = QgsVectorLayer(self.dbconn + ' key=\'id\' table="qgis_test"."binary_type_edit" sql=', 'testbinaryedit', 'hana')
        self.assertTrue(vl.isValid())
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'YmJi')
        }
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(vl.dataProvider().changeAttributeValues({1: {1: QByteArray(b'bbbvx')}}))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx')
        }
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([2, QByteArray(b'cccc')])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(b'cccc')
        }
        self.assertEqual(values, expected)

        # change feature
        self.assertTrue(vl.dataProvider().changeFeatures({2: {1: QByteArray(b'dddd')}}, {}))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {
            1: QByteArray(b'bbbvx'),
            2: QByteArray(b'dddd')
        }
        self.assertEqual(values, expected)


if __name__ == '__main__':
    unittest.main()
