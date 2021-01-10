# -*- coding: utf-8 -*-
"""QGIS Unit tests for the SAP HANA provider.

Read tests/README.md about writing/launching tests with HANA.

Run with ctest -V -R PyQgsHanaProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = 'Maxim Rylov'
__date__ = '2019-11-21'
__copyright__ = 'Copyright 2019, The QGIS Project'

import os

from providertestbase import ProviderTestCase
from PyQt5.QtCore import QVariant, QDate, QTime, QDateTime, QByteArray
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsDataProvider,
    QgsFeatureRequest,
    QgsFeature,
    QgsProviderRegistry,
    QgsRectangle,
    QgsSettings)
from qgis.testing import start_app, unittest
from test_hana_utils import QgsHanaProviderUtils
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsHanaProvider(unittest.TestCase, ProviderTestCase):
    # HANA connection object
    conn = None
    # Name of the schema
    schemaName = ''

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.uri = 'driver=\'/usr/sap/hdbclient/libodbcHDB.so\' host=localhost port=30015 user=SYSTEM ' \
                  'password=mypassword sslEnabled=true sslValidateCertificate=False'
        if 'QGIS_HANA_TEST_DB' in os.environ:
            cls.uri = os.environ['QGIS_HANA_TEST_DB']
        cls.conn = QgsHanaProviderUtils.createConnection(cls.uri)
        cls.schemaName = QgsHanaProviderUtils.generateSchemaName(cls.conn, 'qgis_test')

        QgsHanaProviderUtils.createAndFillDefaultTables(cls.conn, cls.schemaName)

        # Create test layers
        cls.vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri + f' key=\'pk\' srid=4326 type=POINT table="{cls.schemaName}"."some_data" (geom) sql=', 'test')
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri + f' key=\'pk\' srid=4326 type=POLYGON table="{cls.schemaName}"."some_poly_data" (geom) sql=', 'test')
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

        QgsHanaProviderUtils.cleanUp(cls.conn, cls.schemaName)
        cls.conn.close()

    def createVectorLayer(self, conn_parameters, layer_name):
        layer = QgsHanaProviderUtils.createVectorLayer(self.uri + ' ' + conn_parameters, layer_name)
        self.assertTrue(layer.isValid())
        return layer

    def prepareTestTable(self, table_name, create_sql, insert_sql, insert_args):
        res = QgsHanaProviderUtils.executeSQLFetchOne(self.conn,
                                                      f"SELECT COUNT(*) FROM SYS.TABLES WHERE "
                                                      f"SCHEMA_NAME='{self.schemaName}' AND TABLE_NAME='{table_name}'")
        if res != 0:
            QgsHanaProviderUtils.executeSQL(self.conn, f'DROP TABLE "{self.schemaName}"."{table_name}" CASCADE')
        QgsHanaProviderUtils.createAndFillTable(self.conn, create_sql, insert_sql, insert_args)

    def getSource(self):
        # create temporary table for edit tests
        create_sql = f'CREATE TABLE "{self.schemaName}"."edit_data" ( ' \
            '"pk" INTEGER NOT NULL PRIMARY KEY,' \
            '"cnt" INTEGER,' \
            '"name" NVARCHAR(100), ' \
            '"name2" NVARCHAR(100), ' \
            '"num_char" NVARCHAR(100),' \
            '"dt" TIMESTAMP,' \
            '"date" DATE,' \
            '"time" TIME,' \
            '"geom" ST_POINT(4326))'
        insert_sql = f'INSERT INTO "{self.schemaName}"."edit_data" ("pk", "cnt", "name", "name2", "num_char", "dt", "date", ' \
            '"time", "geom") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ST_GeomFromEWKB(?)) '
        insert_args = [
            [5, -200, None, 'NuLl', '5', '2020-05-04 12:13:14', '2020-05-02', '12:13:01',
             bytes.fromhex('0101000020E61000001D5A643BDFC751C01F85EB51B88E5340')],
            [3, 300, 'Pear', 'PEaR', '3', None, None, None, None],
            [1, 100, 'Orange', 'oranGe', '1', '2020-05-03 12:13:14', '2020-05-03', '12:13:14',
             bytes.fromhex('0101000020E61000006891ED7C3F9551C085EB51B81E955040')],
            [2, 200, 'Apple', 'Apple', '2', '2020-05-04 12:14:14', '2020-05-04', '12:14:14',
             bytes.fromhex('0101000020E6100000CDCCCCCCCC0C51C03333333333B35140')],
            [4, 400, 'Honey', 'Honey', '4', '2021-05-04 13:13:14', '2021-05-04', '13:13:14',
             bytes.fromhex('0101000020E610000014AE47E17A5450C03333333333935340')]]
        self.prepareTestTable('edit_data', create_sql, insert_sql, insert_args)

        return self.createVectorLayer(f'key=\'pk\' srid=4326 type=POINT table="{self.schemaName}"."edit_data" (geom) sql=',
                                      'test')

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
            'not null',
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
            'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
            '"dt" <= make_datetime(2020, 5, 4, 12, 13, 14)',
            '"dt" < make_date(2020, 5, 4)',
            '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
            '"date" <= make_datetime(2020, 5, 4, 12, 13, 14)',
            '"date" >= make_date(2020, 5, 4)',
            '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
            '"time" >= make_time(12, 14, 14)',
            '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')'
        ])
        return filters

    def partiallyCompiledFilters(self):
        return set([])

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testMetadata(self):
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
        create_sql = f'CREATE TABLE "{self.schemaName}"."boolean_type" ( ' \
            '"id" INTEGER NOT NULL PRIMARY KEY,' \
            '"fld1" BOOLEAN)'
        insert_sql = f'INSERT INTO "{self.schemaName}"."boolean_type" ("id", "fld1") VALUES (?, ?)'
        insert_args = [[1, 'TRUE'], [2, 'FALSE'], [3, None]]
        self.prepareTestTable('boolean_type', create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(f'table="{self.schemaName}"."boolean_type" sql=', 'testbool')

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('fld1')).type(), QVariant.Bool)

        values = {feat['id']: feat['fld1'] for feat in vl.getFeatures()}
        expected = {1: True, 2: False, 3: NULL}
        self.assertEqual(values, expected)

    def testDateTimeTypes(self):
        create_sql = f'CREATE TABLE "{self.schemaName}"."date_time_type" ( ' \
            '"id" INTEGER NOT NULL PRIMARY KEY,' \
            '"date_field" DATE,' \
            '"time_field" TIME,' \
            '"datetime_field" TIMESTAMP)'
        insert_sql = f'INSERT INTO "{self.schemaName}"."date_time_type" ("id", "date_field", "time_field", "datetime_field") ' \
            'VALUES (?, ?, ?, ?)'
        insert_args = [[1, '2004-03-04', '13:41:52', '2004-03-04 13:41:52']]
        self.prepareTestTable('date_time_type', create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(f'table="{self.schemaName}"."date_time_type" sql=', 'testdatetimes')

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
        create_sql = f'CREATE TABLE "{self.schemaName}"."binary_type" ( ' \
            '"id" INTEGER NOT NULL PRIMARY KEY,' \
            '"blob" VARBINARY(114))'
        insert_sql = f'INSERT INTO "{self.schemaName}"."binary_type" ("id", "blob") VALUES (?, ?)'
        insert_args = [[1, QByteArray(b'YmludmFsdWU=')], [2, None]]
        self.prepareTestTable('binary_type', create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(f'table="{self.schemaName}"."binary_type" sql=', 'testbinary')

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('blob')).type(), QVariant.ByteArray)
        self.assertEqual(fields.at(fields.indexFromName('blob')).length(), 114)

        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b'YmludmFsdWU='), 2: QByteArray()}
        self.assertEqual(values, expected)

    def testBinaryTypeEdit(self):
        create_sql = f'CREATE TABLE "{self.schemaName}"."binary_type_edit" ( ' \
            '"id" INTEGER NOT NULL PRIMARY KEY,' \
            '"blob" VARBINARY(1000))'
        insert_sql = f'INSERT INTO "{self.schemaName}"."binary_type_edit" ("id", "blob") VALUES (?, ?)'
        insert_args = [[1, QByteArray(b'YmJi')]]
        self.prepareTestTable('binary_type_edit', create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(f'key=\'id\' table="{self.schemaName}"."binary_type_edit" sql=', 'testbinaryedit')
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b'YmJi')}
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(vl.dataProvider().changeAttributeValues({1: {1: QByteArray(b'bbbvx')}}))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b'bbbvx')}
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([2, QByteArray(b'cccc')])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b'bbbvx'), 2: QByteArray(b'cccc')}
        self.assertEqual(values, expected)

        # change feature
        self.assertTrue(vl.dataProvider().changeFeatures({2: {1: QByteArray(b'dddd')}}, {}))
        values = {feat['id']: feat['blob'] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b'bbbvx'), 2: QByteArray(b'dddd')}
        self.assertEqual(values, expected)

    def testFilterRectOutsideSrsExtent(self):
        """Test filterRect which partially lies outside of the srs extent"""
        self.source.setSubsetString(None)
        extent = QgsRectangle(-103, 46, -25, 97)
        result = set([f[self.pk_name] for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))])
        expected = set([1, 2, 4, 5])
        assert set(expected) == result, f'Expected {expected} and got {result} when testing setFilterRect {extent}'

    def testEncodeDecodeUri(self):
        """Test HANA encode/decode URI"""
        md = QgsProviderRegistry.instance().providerMetadata('hana')
        self.assertEqual(md.decodeUri(
            "driver='/usr/sap/hdbclient/libodbcHDB.so' dbname='qgis_tests' host=localhost port=30015 "
            "user='myuser' password='mypwd' srid=2016 table=\"public\".\"gis\" (geom) type=MultiPolygon key='id' "
            "sslEnabled='true' sslCryptoProvider='commoncrypto' sslValidateCertificate='false' "
            "sslHostNameInCertificate='hostname.domain.com' sslKeyStore='mykey.pem' "
            "sslTrustStore='server_root.crt' "),
            {
                'driver': '/usr/sap/hdbclient/libodbcHDB.so',
                'dbname': 'qgis_tests',
                'host': 'localhost',
                'port': '30015',
                'username': 'myuser',
                'password': 'mypwd',
                'schema': 'public',
                'table': 'gis',
                'geometrycolumn': 'geom',
                'srid': '2016',
                'type': 6,
                'key': 'id',
                'sslEnabled': 'true',
                'sslCryptoProvider': 'commoncrypto',
                'sslValidateCertificate': 'false',
                'sslHostNameInCertificate': 'hostname.domain.com',
                'sslKeyStore': 'mykey.pem',
                'sslTrustStore': 'server_root.crt',
                'selectatid': False})

        self.assertEqual(md.encodeUri({'driver': '/usr/sap/hdbclient/libodbcHDB.so',
                                       'dbname': 'qgis_tests',
                                       'host': 'localhost',
                                       'port': '30015',
                                       'username': 'myuser',
                                       'password': 'mypwd',
                                       'schema': 'public',
                                       'table': 'gis',
                                       'geometrycolumn': 'geom',
                                       'srid': '2016',
                                       'type': 6,
                                       'key': 'id',
                                       'sslEnabled': 'true',
                                       'sslCryptoProvider': 'commoncrypto',
                                       'sslValidateCertificate': 'false',
                                       'sslHostNameInCertificate': 'hostname.domain.com',
                                       'sslKeyStore': 'mykey.pem',
                                       'sslTrustStore': 'server_root.crt',
                                       'selectatid': False}),
                         "dbname='qgis_tests' driver='/usr/sap/hdbclient/libodbcHDB.so' user='myuser' password='mypwd' "
                         "srid=2016 host='localhost' key='id' port='30015' selectatid='false' "
                         "sslCryptoProvider='commoncrypto' sslEnabled='true' "
                         "sslHostNameInCertificate='hostname.domain.com' sslKeyStore='mykey.pem' "
                         "sslTrustStore='server_root.crt' sslValidateCertificate='false' "
                         "type='MultiPolygon' table=\"public\".\"gis\" (geom)")


if __name__ == '__main__':
    unittest.main()
