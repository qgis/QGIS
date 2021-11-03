# -*- coding: utf-8 -*-
"""QGIS Unit tests for Spatialite QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '28/10/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
import tempfile
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
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
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnectionSpatialite(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'spatialite'

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
        cls.basetestpath = tempfile.mkdtemp()
        spatialite_original_path = '{}/qgis_server/test_project_wms_grouped_layers.sqlite'.format(TEST_DATA_DIR)
        cls.spatialite_path = os.path.join(cls.basetestpath, 'test.sqlite')
        shutil.copy(spatialite_original_path, cls.spatialite_path)
        cls.uri = "dbname=\'%s\'" % cls.spatialite_path
        vl = QgsVectorLayer('{} table=\'cdb_lines\''.format(cls.uri), 'test', 'spatialite')
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.spatialite_path)

    def test_spatialite_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        vl = QgsVectorLayer('{} table=\'cdb_lines\''.format(self.uri), 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        self.assertEqual(conn.uri(), self.uri)
        conn.tables()

    def test_spatialite_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.tableUri('', 'cdb_lines'), '{} table="cdb_lines"'.format(self.uri))
        vl = QgsVectorLayer(conn.tableUri('', 'cdb_lines'), 'lines', 'spatialite')
        self.assertTrue(vl.isValid())

        # Test table(), throws if not found
        table_info = conn.table('', 'cdb_lines')

    def test_spatialite_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')

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

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.View))
        self.assertTrue('my_view' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Aspatial))
        self.assertFalse('myNewTable' in table_names)
        self.assertTrue('myNewAspatialTable' in table_names)

    def test_spatialite_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('', 'cdb_lines')
        table_info = conn.table('', 'cdb_lines')
        self.assertIn(table_info.geometryColumn(), fields.names())
        self.assertIn(table_info.primaryKeyColumns()[0], fields.names())
        self.assertEqual(fields.names(), ['pk', 'geom', 'fid', 'id', 'typ', 'name', 'ortsrat', 'id_long'])

    def test_create_vector_layer(self):
        """Test query layers"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        conn = md.createConnection(self.uri, {})

        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = 'SELECT fid, name, geom FROM cdb_lines WHERE name LIKE \'S%\' LIMIT 2'
        options.geometryColumn = 'geom'
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isSqlQuery())
        self.assertEqual(vl.geometryType(), QgsWkbTypes.PolygonGeometry)
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)
        self.assertEqual(features[0].attributes(), [8, 'Sülfeld'])

        options.filter = 'name == \'Sülfeld\''
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isSqlQuery())
        # Test flags
        self.assertTrue(vl.vectorLayerTypeFlags() & Qgis.VectorLayerTypeFlag.SqlQuery)
        self.assertEqual(vl.geometryType(), QgsWkbTypes.PolygonGeometry)
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].attributes(), [8, 'Sülfeld'])

    def test_execute_sql_pk_geoms(self):
        """OGR hides fid and geom from attributes, check if we can still get them"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
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
