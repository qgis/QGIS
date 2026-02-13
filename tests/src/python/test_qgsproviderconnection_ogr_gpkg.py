"""QGIS Unit tests for OGR GeoPackage QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "10/08/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import shutil

from osgeo import gdal  # NOQA
from qgis.core import (
    Qgis,
    QgsAbstractDatabaseProviderConnection,
    QgsCodedFieldDomain,
    QgsCodedValue,
    QgsCoordinateReferenceSystem,
    QgsFields,
    QgsGeometry,
    QgsGlobFieldDomain,
    QgsMetadataSearchContext,
    QgsProviderConnectionException,
    QgsProviderRegistry,
    QgsRangeFieldDomain,
    QgsRasterLayer,
    QgsVectorLayer,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import QTemporaryDir, QVariant
from qgis.testing import unittest
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return (maj) * 1000000 + (min) * 10000 + (rev) * 100


class TestPyQgsProviderConnectionGpkg(
    unittest.TestCase, TestPyQgsProviderConnectionBase
):
    # Provider test cases must define the string URI for the test
    uri = ""
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = "ogr"

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
    sqlVectorLayerSchema = ""
    sqlVectorLayerTable = "cdb_lines"
    sqlVectorLayerCrs = "EPSG:25832"

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        TestPyQgsProviderConnectionBase.setUpClass()
        gpkg_original_path = (
            f"{TEST_DATA_DIR}/qgis_server/test_project_wms_grouped_layers.gpkg"
        )
        cls.temp_dir = QTemporaryDir()
        cls.gpkg_path = f"{cls.temp_dir.path()}/test_project_wms_grouped_layers.gpkg"
        shutil.copy(gpkg_original_path, cls.gpkg_path)

        gpkg_domains_original_path = f"{TEST_DATA_DIR}/domains.gpkg"
        cls.gpkg_domains_path = f"{cls.temp_dir.path()}/domains.gpkg"
        shutil.copy(gpkg_domains_original_path, cls.gpkg_domains_path)

        vl = QgsVectorLayer(f"{cls.gpkg_path}|layername=cdb_lines", "test", "ogr")
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    def test_gpkg_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        vl = QgsVectorLayer(f"{self.gpkg_path}|layername=cdb_lines", "test", "ogr")
        conn = md.createConnection(vl.dataProvider().dataSourceUri(), {})
        self.assertEqual(conn.uri(), self.gpkg_path)

    def test_gpkg_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        self.assertEqual(
            conn.tableUri("", "cdb_lines"), f"{self.gpkg_path}|layername=cdb_lines"
        )
        vl = QgsVectorLayer(conn.tableUri("", "cdb_lines"), "lines", "ogr")
        self.assertTrue(vl.isValid())

        # Test table(), throws if not found
        conn.table("", "osm")
        conn.table("", "cdb_lines")

        self.assertEqual(conn.tableUri("", "osm"), f"GPKG:{self.uri}:osm")
        rl = QgsRasterLayer(conn.tableUri("", "osm"), "r", "gdal")
        self.assertTrue(rl.isValid())

    def test_gpkg_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, "qgis_test1")

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(
            bool(capabilities & QgsAbstractDatabaseProviderConnection.Capability.Tables)
        )
        self.assertFalse(
            bool(
                capabilities & QgsAbstractDatabaseProviderConnection.Capability.Schemas
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.CreateVectorTable
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DropVectorTable
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.RenameVectorTable
            )
        )
        if int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 10, 0):
            self.assertFalse(
                bool(
                    capabilities
                    & QgsAbstractDatabaseProviderConnection.Capability.RenameRasterTable
                )
            )
        else:
            self.assertTrue(
                bool(
                    capabilities
                    & QgsAbstractDatabaseProviderConnection.Capability.RenameRasterTable
                )
            )

        crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
        typ = QgsWkbTypes.Type.LineString
        conn.createVectorTable(
            "",
            "myNewAspatialTable",
            QgsFields(),
            QgsWkbTypes.Type.NoGeometry,
            crs,
            True,
            {},
        )
        conn.createVectorTable("", "myNewTable", QgsFields(), typ, crs, True, {})

        # Check filters and special cases
        table_names = self._table_names(
            conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.Raster)
        )
        self.assertIn("osm", table_names)
        self.assertNotIn("myNewTable", table_names)
        self.assertNotIn("myNewAspatialTable", table_names)

        table_names = self._table_names(
            conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.View)
        )
        self.assertNotIn("osm", table_names)
        self.assertNotIn("myNewTable", table_names)
        self.assertNotIn("myNewAspatialTable", table_names)

        table_names = self._table_names(
            conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial)
        )
        self.assertNotIn("osm", table_names)
        self.assertNotIn("myNewTable", table_names)
        self.assertIn("myNewAspatialTable", table_names)

    def test_gpkg_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        fields = conn.fields("", "cdb_lines")
        table_info = conn.table("", "cdb_lines")
        self.assertIn(table_info.geometryColumn(), fields.names())
        self.assertIn(table_info.primaryKeyColumns()[0], fields.names())
        self.assertEqual(
            fields.names(), ["fid", "id", "typ", "name", "ortsrat", "id_long", "geom"]
        )

        # aspatial table
        fields = conn.fields("", "myNewAspatialTable")
        table_info = conn.table("", "myNewAspatialTable")
        self.assertFalse(table_info.geometryColumn())
        self.assertIn(table_info.primaryKeyColumns()[0], fields.names())
        self.assertEqual(fields.names(), ["fid"])

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 5, 0),
        "GDAL 3.5 required",
    )
    def test_gpkg_field_domain_names(self):
        """
        Test retrieving field domain names
        """
        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_domains_path, {})

        domain_names = conn.fieldDomainNames()
        self.assertCountEqual(
            domain_names,
            [
                "enum_domain",
                "glob_domain",
                "range_domain_int",
                "range_domain_int64",
                "range_domain_real",
                "range_domain_real_inf",
            ],
        )

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 3, 0),
        "GDAL 3.3 required",
    )
    def test_gpkg_field_domain(self):
        """
        Test retrieving field domain
        """
        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_domains_path, {})

        domain = conn.fieldDomain("enum_domain")
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Coded)
        self.assertEqual(domain.name(), "enum_domain")

        domain = conn.fieldDomain("range_domain_int")
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(domain.name(), "range_domain_int")
        self.assertEqual(domain.minimum(), 1)
        self.assertEqual(domain.maximum(), 2)

        domain = conn.fieldDomain("glob_domain")
        self.assertEqual(domain.type(), Qgis.FieldDomainType.Glob)
        self.assertEqual(domain.name(), "glob_domain")
        self.assertEqual(domain.glob(), "*")

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 3, 0),
        "GDAL 3.3 required",
    )
    def test_gpkg_field_domain_create(self):
        """
        Test creating field domains
        """
        gpkg_domains_original_path = f"{TEST_DATA_DIR}/domains.gpkg"
        temp_domains_path = f"{self.temp_dir.path()}/domains_create.gpkg"
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(temp_domains_path, {})

        domain = QgsRangeFieldDomain(
            "my new domain", "my new domain desc", QVariant.Int, 5, True, 15, True
        )
        conn.addFieldDomain(domain, "")

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain("my new domain")
        self.assertEqual(res.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(res.name(), "my new domain")

        self.assertEqual(res.minimum(), 5)
        self.assertEqual(res.maximum(), 15)

        # try adding another with a duplicate name, should fail
        with self.assertRaises(QgsProviderConnectionException) as e:
            conn.addFieldDomain(domain, "")
        self.assertEqual(
            str(e.exception),
            "Could not create field domain: A domain of identical name already exists",
        )

        domain = QgsGlobFieldDomain(
            "my new glob domain", "my new glob desc", QVariant.String, "*aaabc*"
        )
        conn.addFieldDomain(domain, "")

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain("my new glob domain")
        self.assertEqual(res.type(), Qgis.FieldDomainType.Glob)
        self.assertEqual(res.name(), "my new glob domain")
        self.assertEqual(res.description(), "my new glob desc")
        self.assertEqual(res.glob(), "*aaabc*")

        # coded value
        domain = QgsCodedFieldDomain(
            "my new coded domain",
            "my new coded desc",
            QVariant.String,
            [QgsCodedValue("a", "aa"), QgsCodedValue("b", "bb")],
        )
        conn.addFieldDomain(domain, "")

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        res = conn.fieldDomain("my new coded domain")
        self.assertEqual(res.type(), Qgis.FieldDomainType.Coded)
        self.assertEqual(res.name(), "my new coded domain")
        self.assertEqual(res.description(), "my new coded desc")
        self.assertCountEqual(
            res.values(), [QgsCodedValue("a", "aa"), QgsCodedValue("b", "bb")]
        )

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 3, 0),
        "GDAL 3.3 required",
    )
    def test_gpkg_field_domain_set(self):
        """
        Test setting field domains
        """
        gpkg_domains_original_path = f"{TEST_DATA_DIR}/bug_17878.gpkg"
        temp_domains_path = f"{self.temp_dir.path()}/domain_set.gpkg"
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(temp_domains_path, {})

        domain = QgsRangeFieldDomain(
            "my new domain", "my new domain desc", QVariant.Int, 5, True, 15, True
        )
        conn.addFieldDomain(domain, "")

        # field doesn't exist
        with self.assertRaises(QgsProviderConnectionException):
            conn.setFieldDomainName("xxx", "", "bug_17878", "my new domain")

        conn.setFieldDomainName("int_field", "", "bug_17878", "my new domain")

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        fields = conn.fields("", "bug_17878")
        field = fields.field("int_field")
        self.assertEqual(field.constraints().domainName(), "my new domain")

    def test_create_vector_layer(self):
        """Test query layers"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})

        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = (
            "SELECT fid, name, geom FROM cdb_lines WHERE name LIKE 'S%' LIMIT 2"
        )
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.geometryType(), QgsWkbTypes.GeometryType.PolygonGeometry)
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)
        self.assertEqual(features[0].attributes(), [8, "Sülfeld"])

    def test_execute_sql_pk_geoms(self):
        """OGR hides fid and geom from attributes, check if we can still get them"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})

        # Check errors
        with self.assertRaises(QgsProviderConnectionException):
            sql = "SELECT not_exists, name, geom FROM cdb_lines WHERE name LIKE 'S%' LIMIT 2"
            results = conn.executeSql(sql)

        sql = "SELECT fid, name, geom FROM cdb_lines WHERE name LIKE 'S%' LIMIT 2"
        results = conn.executeSql(sql)
        self.assertEqual(results[0][:2], [8, "Sülfeld"])
        self.assertEqual(results[1][:2], [16, "Steimker Berg"])
        g = QgsGeometry()
        g.fromWkb(results[0][2])
        self.assertIn("POLYGON ((612694", g.asWkt().upper())
        g.fromWkb(results[1][2])
        self.assertIn("POLYGON ((622042.427", g.asWkt().upper())

        sql = "SELECT name, st_astext(geom) FROM cdb_lines WHERE name LIKE 'S%' LIMIT 2"
        results = conn.executeSql(sql)
        self.assertEqual(
            results[0],
            [
                "Sülfeld",
                "POLYGON((612694.674 5807839.658, 612668.715 5808176.815, 612547.354 5808414.452, 612509.527 5808425.73, 612522.932 5808473.02, 612407.901 5808519.082, 612505.836 5808632.763, 612463.449 5808781.115, 612433.57 5808819.061, 612422.685 5808980.281999, 612473.423 5808995.424999, 612333.856 5809647.731, 612307.316 5809781.446, 612267.099 5809852.803, 612308.221 5810040.995, 613920.397 5811079.478, 613947.16 5811129.3, 614022.726 5811154.456, 614058.436 5811260.36, 614194.037 5811331.972, 614307.176 5811360.06, 614343.842 5811323.238, 614443.449 5811363.03, 614526.199 5811059.031, 614417.83 5811057.603, 614787.296 5809648.422, 614772.062 5809583.246, 614981.93 5809245.35, 614811.885 5809138.271, 615063.452 5809100.954, 615215.476 5809029.413, 615469.441 5808883.282, 615569.846 5808829.522, 615577.239 5808806.242, 615392.964 5808736.873, 615306.34 5808662.171, 615335.445 5808290.588, 615312.192 5808290.397, 614890.582 5808077.956, 615018.854 5807799.895, 614837.326 5807688.363, 614435.698 5807646.847, 614126.351 5807661.841, 613555.813 5807814.801, 612826.66 5807964.828, 612830.113 5807856.315, 612694.674 5807839.658))",
            ],
        )

    def test_rename_field(self):
        gpkg_domains_original_path = f"{TEST_DATA_DIR}/bug_17878.gpkg"
        temp_domains_path = f"{self.temp_dir.path()}/rename_field.gpkg"
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        fields = conn.fields("", "cdb_lines")
        self.assertEqual(
            fields.names(), ["fid", "id", "typ", "name", "ortsrat", "id_long", "geom"]
        )

        # invalid table name
        with self.assertRaises(QgsProviderConnectionException):
            conn.renameField("schema", "asdasd", "asd", "xyz")
        # invalid existing field name
        with self.assertRaises(QgsProviderConnectionException):
            conn.renameField("schema", "cdb_lines", "asd", "xyz")
        # try to rename over existing field
        with self.assertRaises(QgsProviderConnectionException):
            conn.renameField("schema", "cdb_lines", "name", "ortsrat")
        # try to rename geometry field
        with self.assertRaises(QgsProviderConnectionException):
            conn.renameField("schema", "cdb_lines", "geom", "the_geom")

        # good rename
        conn.renameField("schema", "cdb_lines", "name", "name2")
        fields = conn.fields("", "cdb_lines")
        self.assertEqual(
            fields.names(), ["fid", "id", "typ", "name2", "ortsrat", "id_long", "geom"]
        )

        # make sure schema is ignored
        conn.renameField("", "cdb_lines", "name2", "name3")
        fields = conn.fields("", "cdb_lines")
        self.assertEqual(
            fields.names(), ["fid", "id", "typ", "name3", "ortsrat", "id_long", "geom"]
        )
        # Restore
        conn.renameField("", "cdb_lines", "name3", "name")

    def test_searchLayerMetadata_buggy_extent(self):
        """Test fix for https://github.com/qgis/QGIS/issues/56203"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(f"{TEST_DATA_DIR}/provider/bug_56203.gpkg", {})
        res = conn.searchLayerMetadata(QgsMetadataSearchContext())
        self.assertTrue(res[0].geographicExtent().isEmpty())

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 10, 0),
        "GDAL 3.10 required",
    )
    def test_rename_raster_layer(self):
        """Test renaming a raster layer"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})

        tables = conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.Raster)
        osm = tables[0]
        self.assertEqual(osm.tableName(), "osm")
        conn.renameRasterTable("", "osm", "osm_new")
        tables = conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.Raster)
        osm = tables[0]
        self.assertEqual(osm.tableName(), "osm_new")

    def test_table_layer_with_comment(self):
        """Test a comment layer"""
        gpkg_test_comment_layers_path = f"{TEST_DATA_DIR}/test_comment_layers.gpkg"
        temp_comment_layers_path = f"{self.temp_dir.path()}/test_comment_layers.gpkg"
        shutil.copy(gpkg_test_comment_layers_path, temp_comment_layers_path)

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(temp_comment_layers_path, {})

        tables = conn.tables("", QgsAbstractDatabaseProviderConnection.TableFlag.Vector)
        table_with_comment = tables[0]
        self.assertEqual(table_with_comment.tableName(), "table_with_comment")
        self.assertEqual(table_with_comment.comment(), "table_with_comment")

    def test_createVectorLayerExporterDestinationUri(self):
        """
        Test createVectorLayerExporterDestinationUri
        """
        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})

        export_options = (
            QgsAbstractDatabaseProviderConnection.VectorLayerExporterOptions()
        )
        export_options.layerName = "new_layer"

        res, options = conn.createVectorLayerExporterDestinationUri(export_options)
        self.assertEqual(options, {"layerName": "new_layer"})
        self.assertEqual(res, self.uri)

        # ignored by provider
        export_options.wkbType = Qgis.WkbType.Point
        res, options = conn.createVectorLayerExporterDestinationUri(export_options)
        self.assertEqual(options, {"layerName": "new_layer"})
        self.assertEqual(res, self.uri)

        # ignored by provider
        export_options.geometryColumn = "geometry"
        res, options = conn.createVectorLayerExporterDestinationUri(export_options)
        self.assertEqual(options, {"layerName": "new_layer"})
        self.assertEqual(res, self.uri)

        # ignored by provider
        export_options.primaryKeyColumns = ["pk"]
        res, options = conn.createVectorLayerExporterDestinationUri(export_options)
        self.assertEqual(options, {"layerName": "new_layer"})
        self.assertEqual(res, self.uri)

    def test_ogr_CreateSqlVectorLayer_complex_queries(self):
        """
        Test createSqlVectorLayer with complex queries (bug #42132)
        """

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})

        # Test selecting all columns
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT * FROM cdb_lines"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 6)
        self.assertTrue(vl.isSpatial())

        # Test a query plus a filter
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT * FROM cdb_lines"
        options.filter = "name = 'Sülfeld'"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 6)
        self.assertTrue(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["name"], "Sülfeld")

        # Test a query with a where
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT * FROM cdb_lines WHERE name LIKE 'Sülfeld'"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 6)
        self.assertTrue(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["name"], "Sülfeld")

        # Test a query with a where without the name field
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = (
            "SELECT id, ortsrat, geom FROM cdb_lines WHERE name LIKE 'Sülfeld'"
        )
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 2)
        self.assertTrue(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["ortsrat"], "Fallersleben/Sülfeld")

        # Test a query with a where without the name field and the geom
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT id, ortsrat FROM cdb_lines WHERE name LIKE 'Sülfeld'"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 2)
        self.assertFalse(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["ortsrat"], "Fallersleben/Sülfeld")

        # Test a query with with a where without the name field and and additional filter
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT id, ortsrat, geom FROM cdb_lines WHERE name LIKE 'S%'"
        options.filter = "ortsrat = 'Fallersleben/Sülfeld'"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 2)
        self.assertTrue(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["ortsrat"], "Fallersleben/Sülfeld")

        # Check that the filter and sql have been merged
        uris_parts = md.decodeUri(vl.publicSource())
        self.assertEqual(
            uris_parts["subset"],
            "SELECT id, ortsrat, geom FROM cdb_lines WHERE ( name LIKE 'S%' ) AND ( ortsrat = 'Fallersleben/Sülfeld' )",
        )
        self.assertIsNone(uris_parts["layerName"])
        self.assertIsNone(uris_parts["layerId"])

        # Test a query with OR and an additional filter
        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.sql = "SELECT id, name, geom FROM cdb_lines WHERE ortsrat = 'Mitte-West' OR ortsrat = 'Fallersleben/Sülfeld'"
        options.filter = "name LIKE 'Sülfeld'"
        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 2)
        self.assertTrue(vl.isSpatial())
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0]["name"], "Sülfeld")

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 12, 0),
        "GDAL 3.12 required",
    )
    def test_gpkg_field_domain_update(self):
        """
        Test update field domains
        """
        gpkg_domains_original_path = f"{TEST_DATA_DIR}/domains.gpkg"
        temp_domains_path = f"{self.temp_dir.path()}/domains_delete.gpkg"
        shutil.copy(gpkg_domains_original_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(temp_domains_path, {})

        domain = QgsRangeFieldDomain(
            "my new domain", "my new domain desc", QVariant.Int, 5, True, 15, True
        )
        conn.addFieldDomain(domain, "")

        # try retrieving result
        del conn
        conn = md.createConnection(temp_domains_path, {})

        # get domain for modification
        read_domain = conn.fieldDomain("my new domain")

        # modify the field domain
        read_domain.setMinimum(7.5)
        read_domain.setMaximum(12.5)
        read_domain.setDescription("my updated domain desc")

        # update the field domain
        conn.updateFieldDomain(read_domain, "")

        # get the updated domain
        read_domain = conn.fieldDomain("my new domain")

        self.assertEqual(read_domain.type(), Qgis.FieldDomainType.Range)
        self.assertEqual(read_domain.name(), "my new domain")
        self.assertEqual(read_domain.description(), "my updated domain desc")

        self.assertEqual(read_domain.minimum(), 7.5)
        self.assertEqual(read_domain.maximum(), 12.5)

        # rename the domain
        read_domain.setName("my renamed domain")

        # try updating domain that does not exist, should fail
        with self.assertRaises(QgsProviderConnectionException) as e:
            conn.updateFieldDomain(read_domain, "")
        self.assertEqual(
            str(e.exception),
            "Could not update field domain: The domain should already exist to be updated",
        )

    @unittest.skipIf(
        int(gdal.VersionInfo("VERSION_NUM")) < GDAL_COMPUTE_VERSION(3, 12, 0),
        "GDAL 3.12 required",
    )
    def test_gpkg_field_domain_delete(self):
        """
        Test delete field domains
        """
        temp_domains_path = f"{self.temp_dir.path()}/domains_delete.gpkg"
        shutil.copy(self.gpkg_domains_path, temp_domains_path)

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(temp_domains_path, {})

        # delete the field domain
        conn.deleteFieldDomain("enum_domain", "")
        conn.deleteFieldDomain("glob_domain", "")

        # try updating domain that does not exist, should fail
        with self.assertRaises(QgsProviderConnectionException) as e:
            conn.deleteFieldDomain("non_existing_field_domain", "")
        self.assertEqual(
            str(e.exception),
            "Could not delete field domain: Domain does not exist",
        )

    def test_select_geometry(self):
        """Test complex queries and multiple geometries retrieval"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})

        # This would be a SQL parser error, test the fallback path
        sql = "SELECT HasSpatialIndex('cdb_lines', 'geom')"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["HasSpatialIndex"])
        results = exec_results.rows()
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0], [1])

        sql = "SELECT ST_StartPoint(ST_ExteriorRing(geom)) AS start_point, geom FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["start_point", "geom"])
        results = exec_results.rows()
        self.assertGreater(len(results), 0)
        self.assertEqual(len(results[0]), 2)
        g_start = QgsGeometry()
        g_start.fromWkb(results[0][0])
        g = QgsGeometry()
        g.fromWkb(results[0][1])
        self.assertTrue(g_start.asWkt().upper().startswith("POINT (625971"))
        self.assertIn("POLYGON ((625971", g.asWkt().upper())

        # Swapping the order of selected fields
        sql = "SELECT geom, ST_StartPoint(ST_ExteriorRing(geom)) AS start_point FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["geom", "start_point"])
        results = exec_results.rows()
        self.assertGreater(len(results), 0)
        self.assertEqual(len(results[0]), 2)
        g = QgsGeometry()
        g.fromWkb(results[0][0])
        g_start = QgsGeometry()
        g_start.fromWkb(results[0][1])
        self.assertIn("POLYGON ((625971", g.asWkt().upper())
        self.assertTrue(g_start.asWkt().upper().startswith("POINT (625971"))

        # Fields as reported by SQLite: fid, geom, id, typ, name, ortsrat, id_long
        # but Qgis internal OGR does not expose the geometry as a normal field
        # so we have it appended at the end of the field list
        # (fid, id, typ, name, ortsrat, id_long, geom)
        sql = "SELECT * FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(
            exec_results.columns(),
            ["fid", "id", "typ", "name", "ortsrat", "id_long", "geom"],
        )
        results = exec_results.rows()
        self.assertGreater(len(results), 0)
        self.assertEqual(results[0][0], 1)  # fid
        self.assertEqual(results[0][1], 1)  # id
        self.assertEqual(results[0][2], "Ortsteil")
        self.assertEqual(results[0][3], "Neindorf")
        self.assertEqual(results[0][4], "Almke/Neindorf")
        self.assertEqual(results[0][5], None)
        g = QgsGeometry()
        g.fromWkb(results[0][6])  # geom
        self.assertIn("POLYGON ((625971", g.asWkt().upper())

        sql = "SELECT geom, name FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["geom", "name"])
        results = exec_results.rows()
        self.assertGreater(len(results), 0)
        g = QgsGeometry()
        g.fromWkb(results[0][0])
        self.assertIn("POLYGON ((625971", g.asWkt().upper())
        self.assertEqual(results[0][1], "Neindorf")

        sql = "SELECT name, geom FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["name", "geom"])
        results = exec_results.rows()
        self.assertGreater(len(results), 0)
        g = QgsGeometry()
        g.fromWkb(results[0][1])
        self.assertIn("POLYGON ((625971", g.asWkt().upper())
        self.assertEqual(results[0][0], "Neindorf")

        sql = "SELECT name, geom, fid FROM cdb_lines ORDER BY fid LIMIT 1"
        exec_results = conn.execSql(sql)
        self.assertEqual(exec_results.columns(), ["name", "geom", "fid"])
        results = exec_results.rows()
        self.assertGreater(len(results), 0)

        self.assertEqual(results[0][2], 1)  # fid
        self.assertEqual(results[0][0], "Neindorf")
        g = QgsGeometry()
        g.fromWkb(results[0][1])
        self.assertIn("POLYGON ((625971", g.asWkt().upper())

    def test_gpkg_geometry_column_capabilities(self):
        """Test that GeoPackage provider has PolyhedralSurfaces geometry column capability"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})

        geom_caps = conn.geometryColumnCapabilities()

        # Check standard capabilities
        self.assertTrue(
            bool(
                geom_caps
                & QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.Z
            )
        )
        self.assertTrue(
            bool(
                geom_caps
                & QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.M
            )
        )
        self.assertTrue(
            bool(
                geom_caps
                & QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.Curves
            )
        )
        # Check PolyhedralSurfaces capability
        self.assertTrue(
            bool(
                geom_caps
                & QgsAbstractDatabaseProviderConnection.GeometryColumnCapability.PolyhedralSurfaces
            )
        )

    def test_gpkg_create_polyhedral_surface_and_tin(self):
        """Test creating tables with PolyhedralSurface and TIN geometry types"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)

        # Test PolyhedralSurface
        conn.createVectorTable(
            "",
            "test_polyhedral",
            QgsFields(),
            QgsWkbTypes.Type.PolyhedralSurface,
            crs,
            True,
            {"geometryColumn": "geom"},
        )

        tables = conn.tables("")
        table_names = [t.tableName() for t in tables]
        self.assertIn("test_polyhedral", table_names)

        # Verify geometry type
        table_info = conn.table("", "test_polyhedral")
        self.assertEqual(table_info.geometryColumn(), "geom")

        # Test TIN
        conn.createVectorTable(
            "",
            "test_tin",
            QgsFields(),
            QgsWkbTypes.Type.TIN,
            crs,
            True,
            {"geometryColumn": "geom"},
        )

        tables = conn.tables("")
        table_names = [t.tableName() for t in tables]
        self.assertIn("test_tin", table_names)

        # Clean up
        conn.dropVectorTable("", "test_polyhedral")
        conn.dropVectorTable("", "test_tin")

    def test_gpkg_create_spatial_index_with_geometry_column(self):
        """Test creating spatial index with explicit geometry column name"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)

        # Create table without spatial index
        conn.createVectorTable(
            "",
            "test_spatial_index",
            QgsFields(),
            QgsWkbTypes.Type.PolyhedralSurface,
            crs,
            True,
            {"layerOptions": "SPATIAL_INDEX=NO"},
        )

        # Reconnect to ensure metadata is flushed
        del conn
        conn = md.createConnection(self.uri, {})

        # Verify table was created
        table_info = conn.table("", "test_spatial_index")
        geom_column = table_info.geometryColumn()
        self.assertTrue(geom_column)  # Should have a geometry column

        # Create spatial index with explicit geometry column name
        options = QgsAbstractDatabaseProviderConnection.SpatialIndexOptions()
        options.geometryColumnName = geom_column
        conn.createSpatialIndex("", "test_spatial_index", options)

        # Verify spatial index exists
        self.assertTrue(conn.spatialIndexExists("", "test_spatial_index", geom_column))

        # Clean up
        conn.dropVectorTable("", "test_spatial_index")

    def test_gpkg_create_spatial_index_automatic(self):
        """Test that spatial index is created automatically with SPATIAL_INDEX=YES"""

        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.uri, {})
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)

        # Create table with automatic spatial index
        conn.createVectorTable(
            "",
            "test_spatial_index_auto",
            QgsFields(),
            QgsWkbTypes.Type.PolyhedralSurface,
            crs,
            True,
            {"layerOptions": "SPATIAL_INDEX=YES"},
        )

        # Reconnect to ensure metadata is flushed
        del conn
        conn = md.createConnection(self.uri, {})

        # Verify table was created
        table_info = conn.table("", "test_spatial_index_auto")
        geom_column = table_info.geometryColumn()
        self.assertTrue(geom_column)  # Should have a geometry column

        # Verify spatial index was created automatically
        self.assertTrue(
            conn.spatialIndexExists("", "test_spatial_index_auto", geom_column)
        )

        # Clean up
        conn.dropVectorTable("", "test_spatial_index_auto")


if __name__ == "__main__":
    unittest.main()
