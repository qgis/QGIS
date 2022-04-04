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
from osgeo import gdal  # NOQA

from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.PyQt.QtCore import QTemporaryDir, QVariant
from qgis.core import (
    Qgis,
    QgsWkbTypes,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProviderRegistry,
    QgsFields,
    QgsCoordinateReferenceSystem,
    QgsRangeFieldDomain,
    QgsGlobFieldDomain,
    QgsCodedFieldDomain,
    QgsCodedValue
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class TestPyQgsProviderConnectionGpkg(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'ogr'

    # Provider test cases can define a slowQuery for executeSql cancellation test
    # Note: GDAL does not support GDALDatasetExecuteSQL interruption, so
    # let's disable this test for the time being
    slowQuery___disabled = """
    WITH RECURSIVE r(i) AS (
        VALUES(0)
        UNION ALL
        SELECT i FROM r
        LIMIT 10000000
        )
    SELECT i FROM r WHERE i = 1; """

    # Provider test cases can define a schema and table name for SQL query layers test
    sqlVectorLayerSchema = ''
    sqlVectorLayerTable = 'cdb_lines'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderConnectionBase.setUpClass()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.temp_dir = QTemporaryDir()
        cls.gpkg_path = '{}/test_project_wms_grouped_layers.gpkg'.format(cls.temp_dir.path())
        shutil.copy(gpkg_original_path, cls.gpkg_path)

        gpkg_domains_original_path = '{}/domains.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_domains_path = '{}/domains.gpkg'.format(cls.temp_dir.path())
        shutil.copy(gpkg_domains_original_path, cls.gpkg_domains_path)

        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    def test_gpkg_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(self.gpkg_path), 'test', 'ogr')
        conn = md.createConnection(vl.dataProvider().dataSourceUri(), {})
        self.assertEqual(conn.uri(), self.gpkg_path)

    def test_gpkg_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.tableUri('', 'cdb_lines'), '{}|layername=cdb_lines'.format(self.gpkg_path))
        vl = QgsVectorLayer(conn.tableUri('', 'cdb_lines'), 'lines', 'ogr')
        self.assertTrue(vl.isValid())

        # Test table(), throws if not found
        conn.table('', 'osm')
        conn.table('', 'cdb_lines')

        self.assertEqual(conn.tableUri('', 'osm'), "GPKG:%s:osm" % self.uri)
        rl = QgsRasterLayer(conn.tableUri('', 'osm'), 'r', 'gdal')
        self.assertTrue(rl.isValid())

    def test_gpkg_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable))
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameRasterTable))

        crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
        typ = QgsWkbTypes.LineString
        conn.createVectorTable('', 'myNewAspatialTable', QgsFields(), QgsWkbTypes.NoGeometry, crs, True, {})
        conn.createVectorTable('', 'myNewTable', QgsFields(), typ, crs, True, {})

        # Check filters and special cases
        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertTrue('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.View))
        self.assertFalse('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Aspatial))
        self.assertFalse('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertTrue('myNewAspatialTable' in table_names)

    def test_gpkg_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('', 'cdb_lines')
        table_info = conn.table('', 'cdb_lines')
        self.assertIn(table_info.geometryColumn(), fields.names())
        self.assertIn(table_info.primaryKeyColumns()[0], fields.names())
        self.assertEqual(fields.names(), ['fid', 'id', 'typ', 'name', 'ortsrat', 'id_long', 'geom'])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 5, 0), "GDAL 3.5 required")
    def test_gpkg_field_domain_names(self):
        """
        Test retrieving field domain names
        """
        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.gpkg_domains_path, {})

        domain_names = conn.fieldDomainNames()
        self.assertCountEqual(domain_names, ['enum_domain',
                                             'glob_domain',
                                             'range_domain_int',
                                             'range_domain_int64',
                                             'range_domain_real',
                                             'range_domain_real_inf'])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0), "GDAL 3.3 required")
    def test_gpkg_field_domain(self):
        """
        Test retrieving field domain
        """
        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.gpkg_domains_path, {})

        domain = conn.fieldDomain('enum_domain')
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Coded)
        self.assertEqual(domain.name(), 'enum_domain')

        domain = conn.fieldDomain('range_domain_int')
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(domain.name(), 'range_domain_int')
        self.assertEqual(domain.minimum(), 1)
        self.assertEqual(domain.maximum(), 2)

        domain = conn.fieldDomain('glob_domain')
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Glob)
        self.assertEqual(domain.name(), 'glob_domain')
        self.assertEqual(domain.glob(), '*')

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0), "GDAL 3.3 required")
    def test_gpkg_field_domain_create(self):
        """
        Test creating field domains
        """
        gpkg_domains_original_path = '{}/domains.gpkg'.format(TEST_DATA_DIR)
        temp_domains_path = '{}/domains_create.gpkg'.format(self.temp_dir.path())
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(temp_domains_path, {})

        domain = QgsRangeFieldDomain('my new domain', 'my new domain desc', QVariant.Int, 5, True, 15, True)
        conn.addFieldDomain(domain, '')

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain('my new domain')
        self.assertEqual(res.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(res.name(), 'my new domain')

        self.assertEqual(res.minimum(), 5)
        self.assertEqual(res.maximum(), 15)

        # try adding another with a duplicate name, should fail
        with self.assertRaises(QgsProviderConnectionException) as e:
            conn.addFieldDomain(domain, '')
        self.assertEqual(str(e.exception), 'Could not create field domain: A domain of identical name already exists')

        domain = QgsGlobFieldDomain('my new glob domain', 'my new glob desc', QVariant.String, '*aaabc*')
        conn.addFieldDomain(domain, '')

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain('my new glob domain')
        self.assertEqual(res.type(), Qgis.FieldDomainType.Glob)
        self.assertEqual(res.name(), 'my new glob domain')
        self.assertEqual(res.description(), 'my new glob desc')
        self.assertEqual(res.glob(), '*aaabc*')

        # coded value
        domain = QgsCodedFieldDomain('my new coded domain', 'my new coded desc', QVariant.String, [QgsCodedValue('a', 'aa'), QgsCodedValue('b', 'bb')])
        conn.addFieldDomain(domain, '')

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain('my new coded domain')
        self.assertEqual(res.type(), Qgis.FieldDomainType.Coded)
        self.assertEqual(res.name(), 'my new coded domain')
        self.assertEqual(res.description(), 'my new coded desc')
        self.assertCountEqual(res.values(), [QgsCodedValue('a', 'aa'), QgsCodedValue('b', 'bb')])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 3, 0), "GDAL 3.3 required")
    def test_gpkg_field_domain_set(self):
        """
        Test setting field domains
        """
        gpkg_domains_original_path = '{}/bug_17878.gpkg'.format(TEST_DATA_DIR)
        temp_domains_path = '{}/domain_set.gpkg'.format(self.temp_dir.path())
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(temp_domains_path, {})

        domain = QgsRangeFieldDomain('my new domain', 'my new domain desc', QVariant.Int, 5, True, 15, True)
        conn.addFieldDomain(domain, '')

        # field doesn't exist
        with self.assertRaises(QgsProviderConnectionException):
            conn.setFieldDomainName('xxx', '', 'bug_17878', 'my new domain')

        conn.setFieldDomainName('int_field', '', 'bug_17878', 'my new domain')

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        fields = conn.fields('', 'bug_17878')
        field = fields.field('int_field')
        self.assertEqual(field.constraints().domainName(), 'my new domain')

    def test_create_vector_layer(self):
        """Test query layers"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})

        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = 'SELECT fid, name, geom FROM cdb_lines WHERE name LIKE \'S%\' LIMIT 2'
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.geometryType(), QgsWkbTypes.PolygonGeometry)
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)
        self.assertEqual(features[0].attributes(), [8, 'Sülfeld'])

    def test_execute_sql_pk_geoms(self):
        """OGR hides fid and geom from attributes, check if we can still get them"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})

        # Check errors
        with self.assertRaises(QgsProviderConnectionException):
            sql = 'SELECT not_exists, name, geom FROM cdb_lines WHERE name LIKE \'S%\' LIMIT 2'
            results = conn.executeSql(sql)

        sql = 'SELECT fid, name, geom FROM cdb_lines WHERE name LIKE \'S%\' LIMIT 2'
        results = conn.executeSql(sql)
        self.assertEqual(results[0][:2], [8, 'Sülfeld'])
        self.assertEqual(results[1][:2], [16, 'Steimker Berg'])
        self.assertEqual(results[0][2][:20], 'Polygon ((612694.674')
        self.assertEqual(results[1][2][:20], 'Polygon ((622042.427')

        sql = 'SELECT name, st_astext(geom) FROM cdb_lines WHERE name LIKE \'S%\' LIMIT 2'
        results = conn.executeSql(sql)
        self.assertEqual(results[0], ['Sülfeld',
                                      'POLYGON((612694.674 5807839.658, 612668.715 5808176.815, 612547.354 5808414.452, 612509.527 5808425.73, 612522.932 5808473.02, 612407.901 5808519.082, 612505.836 5808632.763, 612463.449 5808781.115, 612433.57 5808819.061, 612422.685 5808980.281999, 612473.423 5808995.424999, 612333.856 5809647.731, 612307.316 5809781.446, 612267.099 5809852.803, 612308.221 5810040.995, 613920.397 5811079.478, 613947.16 5811129.3, 614022.726 5811154.456, 614058.436 5811260.36, 614194.037 5811331.972, 614307.176 5811360.06, 614343.842 5811323.238, 614443.449 5811363.03, 614526.199 5811059.031, 614417.83 5811057.603, 614787.296 5809648.422, 614772.062 5809583.246, 614981.93 5809245.35, 614811.885 5809138.271, 615063.452 5809100.954, 615215.476 5809029.413, 615469.441 5808883.282, 615569.846 5808829.522, 615577.239 5808806.242, 615392.964 5808736.873, 615306.34 5808662.171, 615335.445 5808290.588, 615312.192 5808290.397, 614890.582 5808077.956, 615018.854 5807799.895, 614837.326 5807688.363, 614435.698 5807646.847, 614126.351 5807661.841, 613555.813 5807814.801, 612826.66 5807964.828, 612830.113 5807856.315, 612694.674 5807839.658))'])


if __name__ == '__main__':
    unittest.main()
