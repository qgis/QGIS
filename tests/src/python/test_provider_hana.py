"""QGIS Unit tests for the SAP HANA provider.

Read tests/README.md about writing/launching tests with HANA.

Run with ctest -V -R PyQgsHanaProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Maxim Rylov"
__date__ = "2019-11-21"
__copyright__ = "Copyright 2019, The QGIS Project"

import os

from qgis.PyQt.QtCore import QByteArray, QDate, QDateTime, QTime, QVariant
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsDataProvider,
    QgsDataSourceUri,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsFieldConstraints,
    QgsGeometry,
    QgsPointXY,
    QgsProviderRegistry,
    QgsRectangle,
    QgsSettings,
    QgsVectorDataProvider,
    QgsVectorLayer,
    QgsVectorLayerExporter,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from providertestbase import ProviderTestCase
from test_hana_utils import QgsHanaProviderUtils
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsHanaProvider(QgisTestCase, ProviderTestCase):
    # HANA connection object
    conn = None
    # Name of the schema
    schemaName = ""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        cls.uri = (
            "driver='/usr/sap/hdbclient/libodbcHDB.so' host=localhost port=30015 user=SYSTEM "
            "password=mypassword sslEnabled=true sslValidateCertificate=False"
        )
        if "QGIS_HANA_TEST_DB" in os.environ:
            cls.uri = os.environ["QGIS_HANA_TEST_DB"]
        cls.conn = QgsHanaProviderUtils.createConnection(cls.uri)

        schemaPrefix = "qgis_test"
        QgsHanaProviderUtils.dropOldTestSchemas(cls.conn, schemaPrefix)

        cls.schemaName = QgsHanaProviderUtils.generateSchemaName(cls.conn, schemaPrefix)

        QgsHanaProviderUtils.createAndFillDefaultTables(cls.conn, cls.schemaName)

        # Create test layers
        cls.vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri
            + f' key=\'pk\' srid=4326 type=POINT table="{cls.schemaName}"."some_data" (geom) sql=',
            "test",
        )
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri
            + f' key=\'pk\' srid=4326 type=POLYGON table="{cls.schemaName}"."some_poly_data" (geom) sql=',
            "test",
        )
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

        QgsHanaProviderUtils.dropSchemaIfExists(cls.conn, cls.schemaName)
        cls.conn.close()
        super().tearDownClass()

    def createVectorLayer(self, conn_parameters, layer_name):
        layer = QgsHanaProviderUtils.createVectorLayer(
            self.uri + " " + conn_parameters, layer_name
        )
        self.assertTrue(layer.isValid())
        return layer

    def prepareTestTable(self, table_name, create_sql, insert_sql, insert_args):
        res = QgsHanaProviderUtils.executeSQLFetchOne(
            self.conn,
            f"SELECT COUNT(*) FROM SYS.TABLES WHERE "
            f"SCHEMA_NAME='{self.schemaName}' AND TABLE_NAME='{table_name}'",
        )
        if res != 0:
            QgsHanaProviderUtils.executeSQL(
                self.conn, f'DROP TABLE "{self.schemaName}"."{table_name}" CASCADE'
            )
        QgsHanaProviderUtils.createAndFillTable(
            self.conn, create_sql, insert_sql, insert_args
        )

    def getSource(self):
        # create temporary table for edit tests
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."edit_data" ( '
            '"pk" INTEGER NOT NULL PRIMARY KEY,'
            '"cnt" INTEGER,'
            '"name" NVARCHAR(100), '
            '"name2" NVARCHAR(100), '
            '"num_char" NVARCHAR(100),'
            '"dt" TIMESTAMP,'
            '"date" DATE,'
            '"time" TIME,'
            '"geom" ST_POINT(4326))'
        )
        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."edit_data" ("pk", "cnt", "name", "name2", "num_char", "dt", "date", '
            '"time", "geom") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ST_GeomFromEWKB(?)) '
        )
        insert_args = [
            [
                5,
                -200,
                None,
                "NuLl",
                "5",
                "2020-05-04 12:13:14",
                "2020-05-02",
                "12:13:01",
                bytes.fromhex("0101000020E61000001D5A643BDFC751C01F85EB51B88E5340"),
            ],
            [3, 300, "Pear", "PEaR", "3", None, None, None, None],
            [
                1,
                100,
                "Orange",
                "oranGe",
                "1",
                "2020-05-03 12:13:14",
                "2020-05-03",
                "12:13:14",
                bytes.fromhex("0101000020E61000006891ED7C3F9551C085EB51B81E955040"),
            ],
            [
                2,
                200,
                "Apple",
                "Apple",
                "2",
                "2020-05-04 12:14:14",
                "2020-05-04",
                "12:14:14",
                bytes.fromhex("0101000020E6100000CDCCCCCCCC0C51C03333333333B35140"),
            ],
            [
                4,
                400,
                "Honey",
                "Honey",
                "4",
                "2021-05-04 13:13:14",
                "2021-05-04",
                "13:13:14",
                bytes.fromhex("0101000020E610000014AE47E17A5450C03333333333935340"),
            ],
        ]
        self.prepareTestTable("edit_data", create_sql, insert_sql, insert_args)

        return self.createVectorLayer(
            f'key=\'pk\' srid=4326 type=POINT table="{self.schemaName}"."edit_data" (geom) sql=',
            "test",
        )

    def getEditableLayer(self):
        return self.getSource()

    def enableCompiler(self):
        QgsSettings().setValue("/qgis/compileExpressions", True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue("/qgis/compileExpressions", False)

    def uncompiledFilters(self):
        filters = {
            "(name = 'Apple') is not null",
            "false and NULL",
            "true and NULL",
            "NULL and false",
            "NULL and true",
            "NULL and NULL",
            "false or NULL",
            "true or NULL",
            "NULL or false",
            "NULL or true",
            "NULL or NULL",
            "not null",
            "'x' || \"name\" IS NOT NULL",
            "'x' || \"name\" IS NULL",
            "radians(cnt) < 2",
            "degrees(pk) <= 200",
            "log10(pk) < 0.5",
            "x($geometry) < -70",
            "y($geometry) > 70",
            "xmin($geometry) < -70",
            "ymin($geometry) > 70",
            "xmax($geometry) < -70",
            "ymax($geometry) > 70",
            "disjoint($geometry,geom_from_wkt( 'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))'))",
            "intersects($geometry,geom_from_wkt( 'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))'))",
            "contains(geom_from_wkt( 'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))'),$geometry)",
            "distance($geometry,geom_from_wkt( 'Point (-70 70)')) > 7",
            "intersects($geometry,geom_from_gml( '<gml:Polygon srsName=\"EPSG:4326\"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>'))",
            "x($geometry) < -70",
            "y($geometry) > 79",
            "xmin($geometry) < -70",
            "ymin($geometry) < 76",
            "xmax($geometry) > -68",
            "ymax($geometry) > 80",
            "area($geometry) > 10",
            "perimeter($geometry) < 12",
            "relate($geometry,geom_from_wkt( 'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))')) = 'FF2FF1212'",
            "relate($geometry,geom_from_wkt( 'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))'), '****F****')",
            "crosses($geometry,geom_from_wkt( 'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)'))",
            "overlaps($geometry,geom_from_wkt( 'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))'))",
            "within($geometry,geom_from_wkt( 'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))'))",
            "overlaps(translate($geometry,-1,-1),geom_from_wkt( 'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))'))",
            "overlaps(buffer($geometry,1),geom_from_wkt( 'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))'))",
            "intersects(centroid($geometry),geom_from_wkt( 'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))'))",
            "intersects(point_on_surface($geometry),geom_from_wkt( 'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))'))",
            "\"dt\" = to_datetime('000www14ww13ww12www4ww5ww2020','zzzwwwsswwmmwwhhwwwdwwMwwyyyy')",
            "\"date\" = to_date('www4ww5ww2020','wwwdwwMwwyyyy')",
            "\"time\" = to_time('000www14ww13ww12www','zzzwwwsswwmmwwhhwww')",
        }
        return filters

    def partiallyCompiledFilters(self):
        return set()

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testMetadata(self):
        metadata = self.vl.metadata()
        self.assertEqual(metadata.crs(), QgsCoordinateReferenceSystem.fromEpsgId(4326))
        self.assertEqual(metadata.type(), "dataset")
        self.assertEqual(metadata.abstract(), "QGIS Test Table")

    def testDefaultValue(self):
        self.source.setProviderProperty(
            QgsDataProvider.ProviderProperty.EvaluateDefaultValues, True
        )
        self.assertEqual(self.source.defaultValue(0), NULL)
        self.assertEqual(self.source.defaultValue(1), NULL)
        self.assertEqual(self.source.defaultValue(2), "qgis")
        self.assertEqual(self.source.defaultValue(3), "qgis")
        self.assertEqual(self.source.defaultValue(4), NULL)
        self.source.setProviderProperty(
            QgsDataProvider.ProviderProperty.EvaluateDefaultValues, False
        )

    def testCompositeUniqueConstraints(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."unique_composite_constraints" ( '
            '"ID" INTEGER PRIMARY KEY,'
            '"VAL1" INTEGER,'
            '"VAL2" INTEGER,'
            '"VAL3" INTEGER,'
            "UNIQUE (VAL1, VAL2))"
        )
        QgsHanaProviderUtils.executeSQL(self.conn, create_sql)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."unique_composite_constraints" sql=',
            "testcompositeuniqueconstraints",
        )

        fields = vl.dataProvider().fields()
        id_field_idx = fields.indexFromName("ID")
        val1_field_idx = vl.fields().indexFromName("VAL1")
        val2_field_idx = vl.fields().indexFromName("VAL2")
        val3_field_idx = vl.fields().indexFromName("VAL3")
        self.assertGreaterEqual(id_field_idx, 0)
        self.assertGreaterEqual(val1_field_idx, 0)
        self.assertGreaterEqual(val2_field_idx, 0)
        self.assertGreaterEqual(val3_field_idx, 0)
        self.assertTrue(
            bool(
                vl.fieldConstraints(id_field_idx)
                & QgsFieldConstraints.Constraint.ConstraintUnique
            )
        )
        self.assertFalse(
            bool(
                vl.fieldConstraints(val1_field_idx)
                & QgsFieldConstraints.Constraint.ConstraintUnique
            )
        )
        self.assertFalse(
            bool(
                vl.fieldConstraints(val2_field_idx)
                & QgsFieldConstraints.Constraint.ConstraintUnique
            )
        )
        self.assertFalse(
            bool(
                vl.fieldConstraints(val3_field_idx)
                & QgsFieldConstraints.Constraint.ConstraintUnique
            )
        )

    def testQueryLayers(self):
        def test_query(
            query, key, geometry, attribute_names, wkb_type=QgsWkbTypes.Type.NoGeometry
        ):
            uri = QgsDataSourceUri()
            uri.setSchema(self.schemaName)
            uri.setTable(query)
            uri.setKeyColumn(key)
            uri.setGeometryColumn(geometry)
            vl = self.createVectorLayer(uri.uri(False), "testquery")

            for capability in [
                QgsVectorDataProvider.Capability.SelectAtId,
                QgsVectorDataProvider.Capability.TransactionSupport,
                QgsVectorDataProvider.Capability.CircularGeometries,
                QgsVectorDataProvider.Capability.ReadLayerMetadata,
            ]:
                self.assertTrue(vl.dataProvider().capabilities() & capability)

            for capability in [
                QgsVectorDataProvider.Capability.AddAttributes,
                QgsVectorDataProvider.Capability.ChangeAttributeValues,
                QgsVectorDataProvider.Capability.DeleteAttributes,
                QgsVectorDataProvider.Capability.RenameAttributes,
                QgsVectorDataProvider.Capability.AddFeatures,
                QgsVectorDataProvider.Capability.ChangeFeatures,
                QgsVectorDataProvider.Capability.DeleteFeatures,
                QgsVectorDataProvider.Capability.ChangeGeometries,
                QgsVectorDataProvider.Capability.FastTruncate,
            ]:
                self.assertFalse(vl.dataProvider().capabilities() & capability)

            fields = vl.dataProvider().fields()
            self.assertCountEqual(attribute_names, fields.names())
            for field_idx in vl.primaryKeyAttributes():
                self.assertIn(fields[field_idx].name(), key.split(","))
                self.assertEqual(
                    len(vl.primaryKeyAttributes()) == 1,
                    bool(
                        vl.fieldConstraints(field_idx)
                        & QgsFieldConstraints.Constraint.ConstraintUnique
                    ),
                )
            if fields.count() > 0:
                if vl.featureCount() == 0:
                    self.assertEqual(NULL, vl.maximumValue(0))
                    self.assertEqual(NULL, vl.minimumValue(0))
                else:
                    vl.maximumValue(0)
                    vl.minimumValue(0)
            self.assertEqual(vl.featureCount(), len([f for f in vl.getFeatures()]))
            self.assertFalse(vl.addFeatures([QgsFeature()]))
            self.assertFalse(vl.deleteFeatures([0]))
            self.assertEqual(wkb_type, vl.wkbType())
            self.assertEqual(
                wkb_type == QgsWkbTypes.Type.NoGeometry
                or wkb_type == QgsWkbTypes.Type.Unknown,
                vl.extent().isNull(),
            )

        test_query(
            "(SELECT * FROM DUMMY)", None, None, ["DUMMY"], QgsWkbTypes.Type.NoGeometry
        )
        test_query(
            "(SELECT CAST(NULL AS INT) ID1, CAST(NULL AS INT) ID2, CAST(NULL AS ST_GEOMETRY) SHAPE FROM DUMMY)",
            "ID1,ID2",
            None,
            ["ID1", "ID2", "SHAPE"],
            QgsWkbTypes.Type.NoGeometry,
        )
        test_query(
            "(SELECT CAST(1 AS INT) ID1, CAST(NULL AS BIGINT) ID2 FROM DUMMY)",
            "ID1",
            None,
            ["ID1", "ID2"],
            QgsWkbTypes.Type.NoGeometry,
        )
        test_query(
            "(SELECT CAST(NULL AS INT) ID1, CAST(NULL AS INT) ID2, CAST(NULL AS ST_GEOMETRY) SHAPE FROM DUMMY)",
            None,
            "SHAPE",
            ["ID1", "ID2"],
            QgsWkbTypes.Type.Unknown,
        )
        test_query(
            "(SELECT CAST(NULL AS INT) ID1, CAST(NULL AS BIGINT) ID2, CAST(NULL AS ST_GEOMETRY) SHAPE FROM "
            "DUMMY)",
            "ID2",
            "SHAPE",
            ["ID1", "ID2"],
            QgsWkbTypes.Type.Unknown,
        )
        test_query(
            "(SELECT CAST(NULL AS INT) ID1, CAST(NULL AS ST_GEOMETRY) SHAPE1, CAST(NULL AS ST_GEOMETRY) SHAPE2 "
            "FROM DUMMY)",
            "ID1",
            "SHAPE1",
            ["ID1", "SHAPE2"],
            QgsWkbTypes.Type.Unknown,
        )
        test_query(
            f'(SELECT "pk" AS "key", "cnt", "geom" AS "g" FROM "{self.schemaName}"."some_data")',
            "key",
            "g",
            ["key", "cnt"],
            QgsWkbTypes.Type.Point,
        )

    def testBooleanType(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."boolean_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"fld1" BOOLEAN)'
        )
        insert_sql = f'INSERT INTO "{self.schemaName}"."boolean_type" ("id", "fld1") VALUES (?, ?)'
        insert_args = [[1, "TRUE"], [2, "FALSE"], [3, None]]
        self.prepareTestTable("boolean_type", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."boolean_type" sql=', "testbool"
        )

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName("fld1")).type(), QVariant.Bool)

        values = {feat["id"]: feat["fld1"] for feat in vl.getFeatures()}
        expected = {1: True, 2: False, 3: NULL}
        self.assertEqual(values, expected)

    def testDecimalAndFloatTypes(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."decimal_and_float_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"decimal_field" DECIMAL(15,4),'
            '"float_field" FLOAT(12))'
        )
        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."decimal_and_float_type" ("id", "decimal_field", '
            f'"float_field") VALUES (?, ?, ?) '
        )
        insert_args = [[1, 1.1234, 1.76543]]
        self.prepareTestTable(
            "decimal_and_float_type", create_sql, insert_sql, insert_args
        )

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."decimal_and_float_type" sql=',
            "testdecimalfloat",
        )

        fields = vl.dataProvider().fields()
        decimal_field = fields.at(fields.indexFromName("decimal_field"))
        self.assertEqual(decimal_field.type(), QVariant.Double)
        self.assertEqual(decimal_field.length(), 15)
        self.assertEqual(decimal_field.precision(), 4)
        float_field = fields.at(fields.indexFromName("float_field"))
        self.assertEqual(float_field.type(), QVariant.Double)
        self.assertEqual(float_field.length(), 7)
        self.assertEqual(float_field.precision(), 0)

        feat = next(vl.getFeatures(QgsFeatureRequest()))

        decimal_idx = vl.fields().lookupField("decimal_field")
        self.assertIsInstance(feat.attributes()[decimal_idx], float)
        self.assertEqual(feat.attributes()[decimal_idx], 1.1234)
        float_idx = vl.fields().lookupField("float_field")
        self.assertIsInstance(feat.attributes()[float_idx], float)
        self.assertAlmostEqual(feat.attributes()[float_idx], 1.76543, 5)

    def testDateTimeTypes(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."date_time_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"date_field" DATE,'
            '"time_field" TIME,'
            '"datetime_field" TIMESTAMP)'
        )
        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."date_time_type" ("id", "date_field", "time_field", "datetime_field") '
            "VALUES (?, ?, ?, ?)"
        )
        insert_args = [[1, "2004-03-04", "13:41:52", "2004-03-04 13:41:52"]]
        self.prepareTestTable("date_time_type", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."date_time_type" sql=', "testdatetimes"
        )

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName("date_field")).type(), QVariant.Date
        )
        self.assertEqual(
            fields.at(fields.indexFromName("time_field")).type(), QVariant.Time
        )
        self.assertEqual(
            fields.at(fields.indexFromName("datetime_field")).type(), QVariant.DateTime
        )

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField("date_field")
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fields().lookupField("time_field")
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fields().lookupField("datetime_field")
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(
            f.attributes()[datetime_idx],
            QDateTime(QDate(2004, 3, 4), QTime(13, 41, 52)),
        )

    def testBinaryType(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."binary_type" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"blob" VARBINARY(114))'
        )
        insert_sql = f'INSERT INTO "{self.schemaName}"."binary_type" ("id", "blob") VALUES (?, ?)'
        insert_args = [[1, QByteArray(b"YmludmFsdWU=")], [2, None]]
        self.prepareTestTable("binary_type", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."binary_type" sql=', "testbinary"
        )

        fields = vl.dataProvider().fields()
        self.assertEqual(
            fields.at(fields.indexFromName("blob")).type(), QVariant.ByteArray
        )
        self.assertEqual(fields.at(fields.indexFromName("blob")).length(), 114)

        values = {feat["id"]: feat["blob"] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b"YmludmFsdWU="), 2: QByteArray()}
        self.assertEqual(values, expected)

    def testBinaryTypeEdit(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."binary_type_edit" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"blob" VARBINARY(1000))'
        )
        insert_sql = f'INSERT INTO "{self.schemaName}"."binary_type_edit" ("id", "blob") VALUES (?, ?)'
        insert_args = [[1, QByteArray(b"YmJi")]]
        self.prepareTestTable("binary_type_edit", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'key=\'id\' table="{self.schemaName}"."binary_type_edit" sql=',
            "testbinaryedit",
        )
        values = {feat["id"]: feat["blob"] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b"YmJi")}
        self.assertEqual(values, expected)

        # change attribute value
        self.assertTrue(
            vl.dataProvider().changeAttributeValues({1: {1: QByteArray(b"bbbvx")}})
        )
        values = {feat["id"]: feat["blob"] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b"bbbvx")}
        self.assertEqual(values, expected)

        # add feature
        f = QgsFeature()
        f.setAttributes([2, QByteArray(b"cccc")])
        self.assertTrue(vl.dataProvider().addFeature(f))
        values = {feat["id"]: feat["blob"] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b"bbbvx"), 2: QByteArray(b"cccc")}
        self.assertEqual(values, expected)

        # change feature
        self.assertTrue(
            vl.dataProvider().changeFeatures({2: {1: QByteArray(b"dddd")}}, {})
        )
        values = {feat["id"]: feat["blob"] for feat in vl.getFeatures()}
        expected = {1: QByteArray(b"bbbvx"), 2: QByteArray(b"dddd")}
        self.assertEqual(values, expected)

    def testRealVectorType(self):
        table_name = "real_vector_type"
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."{table_name}" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"emb" REAL_VECTOR(3))'
        )
        insert_sql = f'INSERT INTO "{self.schemaName}"."{table_name}" ("id", "emb") VALUES (?, TO_REAL_VECTOR(?))'
        insert_args = [[1, "[0.1,0.2,0.1]"], [2, None]]
        self.prepareTestTable("real_vector_type", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."{table_name}" sql=', "testrealvector"
        )

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName("emb")).type(), QVariant.String)
        self.assertEqual(fields.at(fields.indexFromName("emb")).length(), 3)

        values = {feat["id"]: feat["emb"] for feat in vl.getFeatures()}
        expected = {1: "[0.1,0.2,0.1]", 2: NULL}
        self.assertEqual(values, expected)

    def testRealVectorTypeEdit(self):
        table_name = "real_vector_type_edit"
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."{table_name}" ( '
            '"id" INTEGER NOT NULL PRIMARY KEY,'
            '"emb" REAL_VECTOR)'
        )
        insert_sql = f'INSERT INTO "{self.schemaName}"."{table_name}" ("id", "emb") VALUES (?, TO_REAL_VECTOR(?))'
        insert_args = [[1, "[0.1,0.2,0.3]"]]
        self.prepareTestTable(table_name, create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'key=\'id\' table="{self.schemaName}"."{table_name}" sql=',
            "testrealvectoredit",
        )

        def check_values(expected):
            actual = {feat["id"]: feat["emb"] for feat in vl.getFeatures()}
            self.assertEqual(actual, expected)

        check_values({1: "[0.1,0.2,0.3]"})

        # change attribute value
        self.assertTrue(
            vl.dataProvider().changeAttributeValues({1: {1: "[0.82,0.5,1]"}})
        )
        check_values({1: "[0.82,0.5,1]"})

        # add feature
        f = QgsFeature()
        f.setAttributes([2, "[1,1,1]"])
        self.assertTrue(vl.dataProvider().addFeature(f))
        check_values({1: "[0.82,0.5,1]", 2: "[1,1,1]"})

        # change feature
        self.assertTrue(vl.dataProvider().changeFeatures({2: {1: "[2,2,2]"}}, {}))
        check_values({1: "[0.82,0.5,1]", 2: "[2,2,2]"})

    def testGeometryAttributes(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."geometry_attribute" ( '
            "ID INTEGER NOT NULL PRIMARY KEY,"
            "GEOM1 ST_GEOMETRY(4326),"
            "GEOM2 ST_GEOMETRY(4326))"
        )
        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."geometry_attribute" (ID, GEOM1, GEOM2) '
            f"VALUES (?, ST_GeomFromText(?, 4326), ST_GeomFromText(?, 4326)) "
        )
        insert_args = [[1, "POINT (1 2)", "LINESTRING (0 0,1 1)"]]
        self.prepareTestTable("geometry_attribute", create_sql, insert_sql, insert_args)

        vl = self.createVectorLayer(
            f'table="{self.schemaName}"."geometry_attribute" (GEOM1) sql=',
            "testgeometryattribute",
        )
        fields = vl.dataProvider().fields()
        self.assertEqual(fields.names(), ["ID", "GEOM2"])
        self.assertEqual(fields.at(fields.indexFromName("ID")).type(), QVariant.Int)
        self.assertEqual(
            fields.at(fields.indexFromName("GEOM2")).type(), QVariant.String
        )
        values = {feat["ID"]: feat["GEOM2"] for feat in vl.getFeatures()}
        self.assertEqual(values, {1: "LINESTRING (0 0,1 1)"})

        # change attribute value
        self.assertTrue(
            vl.dataProvider().changeAttributeValues({1: {1: "LINESTRING (0 0,2 2)"}})
        )
        values = {feat["ID"]: feat["GEOM2"] for feat in vl.getFeatures()}
        self.assertEqual(values, {1: "LINESTRING (0 0,2 2)"})

    def testCreateLayerViaExport(self):
        def runTest(crs, primaryKey, attributeNames, attributeValues):
            self.assertTrue(crs.isValid())

            layer = QgsVectorLayer(f"Point?crs={crs.authid()}", "new_table", "memory")
            pr = layer.dataProvider()

            fields = [
                QgsField("fldid", QVariant.LongLong),
                QgsField("fldtxt", QVariant.String),
                QgsField("fldint", QVariant.Int),
            ]

            if primaryKey == "fldid":
                constraints = QgsFieldConstraints()
                constraints.setConstraint(
                    QgsFieldConstraints.Constraint.ConstraintNotNull,
                    QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider,
                )
                constraints.setConstraint(
                    QgsFieldConstraints.Constraint.ConstraintUnique,
                    QgsFieldConstraints.ConstraintOrigin.ConstraintOriginProvider,
                )
                fields[0].setConstraints(constraints)

            layer.startEditing()
            for f in fields:
                layer.addAttribute(f)
            layer.commitChanges(True)

            f1 = QgsFeature()
            f1.setAttributes([1, "test", 11])
            f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
            f2 = QgsFeature()
            f2.setAttributes([2, "test2", 13])
            f3 = QgsFeature()
            f3.setAttributes([3, "test2", NULL])
            f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 2)))
            f4 = QgsFeature()
            f4.setAttributes([4, NULL, 13])
            f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(4, 3)))
            pr.addFeatures([f1, f2, f3, f4])
            layer.commitChanges()

            QgsHanaProviderUtils.dropTableIfExists(
                self.conn, self.schemaName, "import_data"
            )
            params = f' key=\'{primaryKey}\' table="{self.schemaName}"."import_data" (geom) sql='
            error, message = QgsVectorLayerExporter.exportLayer(
                layer, self.uri + params, "hana", crs
            )
            self.assertEqual(error, QgsVectorLayerExporter.ExportError.NoError)

            import_layer = self.createVectorLayer(params, "testimportedlayer")
            self.assertEqual(import_layer.wkbType(), QgsWkbTypes.Type.Point)
            self.assertEqual([f.name() for f in import_layer.fields()], attributeNames)

            features = [f.attributes() for f in import_layer.getFeatures()]
            self.assertEqual(features, attributeValues)
            geom = [f.geometry().asWkt() for f in import_layer.getFeatures()]
            self.assertEqual(geom, ["Point (1 2)", "", "Point (3 2)", "Point (4 3)"])

            QgsHanaProviderUtils.dropTableIfExists(
                self.conn, self.schemaName, "import_data"
            )

        def is_crs_installed(srid):
            num_crs = QgsHanaProviderUtils.executeSQLFetchOne(
                self.conn,
                f"SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                f"WHERE SRS_ID = {srid}",
            )
            return num_crs == 1

        crs_4326 = QgsCoordinateReferenceSystem("EPSG:4326")
        # primary key already exists in the  imported layer
        runTest(
            crs_4326,
            "fldid",
            ["fldid", "fldtxt", "fldint"],
            [[1, "test", 11], [2, "test2", 13], [3, "test2", NULL], [4, NULL, 13]],
        )
        # primary key doesn't exist in the imported layer
        runTest(
            crs_4326,
            "pk",
            ["pk", "fldid", "fldtxt", "fldint"],
            [
                [1, 1, "test", 11],
                [2, 2, "test2", 13],
                [3, 3, "test2", NULL],
                [4, 4, NULL, 13],
            ],
        )
        # crs id that do not exist
        # unfortunately, we cannot test new units of measure as
        # QgsCoordinateReferenceSystem does not allow creating
        # a new crs object from WKT that contain custom AUTHORITY
        # or UNIT values.
        unknown_srid = 3395
        if not is_crs_installed(unknown_srid):
            crs = QgsCoordinateReferenceSystem.fromEpsgId(unknown_srid)

            runTest(
                crs,
                "fldid",
                ["fldid", "fldtxt", "fldint"],
                [[1, "test", 11], [2, "test2", 13], [3, "test2", NULL], [4, NULL, 13]],
            )
            self.assertTrue(is_crs_installed(unknown_srid))
            QgsHanaProviderUtils.executeSQL(
                self.conn, f'DROP SPATIAL REFERENCE SYSTEM "{crs.description()}"'
            )
            # QgsHanaProviderUtils.executeSQL(self.conn, 'DROP SPATIAL UNIT OF MEASURE degree_qgis')

    def testCreateLayerViaExportWithSpecialTypesHandledAsString(self):
        table_name = "binary_types_as_string"
        create_sql = f"""CREATE COLUMN TABLE "{self.schemaName}"."{table_name}" (
            "pk" INTEGER NOT NULL PRIMARY KEY,
            "vec" REAL_VECTOR(3),
            "geom1" ST_GEOMETRY(4326),
            "geom2" ST_GEOMETRY(4326))"""

        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."{table_name}" ("pk", "vec", "geom1", "geom2") '
            "VALUES (?, TO_REAL_VECTOR(?), ST_GeomFromWKT(?, 4326), ST_GeomFromWKT(?, 4326)) "
        )
        insert_args = [
            [1, "[0.1,0.3,0.2]", None, "POINT (-71.123 78.23)"],
            [2, None, None, None],
            [3, "[0.5,0.8,0.4]", "POINT (-70.332 66.33)", "POINT (-71.123 78.23)"],
        ]

        self.prepareTestTable(table_name, create_sql, insert_sql, insert_args)

        layer = self.createVectorLayer(
            f'table="{self.schemaName}"."{table_name}" (geom1) sql=',
            "testbinaryattributes",
        )
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        params = (
            f' key=\'pk\' table="{self.schemaName}"."import_data_binaries" (geom1) sql='
        )
        error, message = QgsVectorLayerExporter.exportLayer(
            layer, self.uri + params, "hana", crs
        )
        self.assertEqual(error, QgsVectorLayerExporter.ExportError.NoError)

        import_layer = self.createVectorLayer(params, "testimportedlayer")
        fields = import_layer.dataProvider().fields()
        self.assertEqual(fields.size(), 3)
        pk_field = fields.at(fields.indexFromName("pk"))
        self.assertEqual(pk_field.type(), QVariant.Int)
        self.assertEqual(pk_field.typeName(), "INTEGER")
        vec_field = fields.at(fields.indexFromName("vec"))
        self.assertEqual(vec_field.type(), QVariant.String)
        self.assertEqual(vec_field.typeName(), "REAL_VECTOR")
        self.assertEqual(vec_field.length(), 3)
        geom2_field = fields.at(fields.indexFromName("geom2"))
        self.assertEqual(geom2_field.type(), QVariant.String)
        self.assertEqual(geom2_field.typeName(), "ST_GEOMETRY")

        i = 0
        for feat in import_layer.getFeatures():
            self.assertEqual(feat["pk"], insert_args[i][0])
            self.assertEqual(feat["vec"], insert_args[i][1])
            self.assertEqual(feat["geom2"], insert_args[i][3])
            i += 1

    def testFilterRectOutsideSrsExtent(self):
        """Test filterRect which partially lies outside of the srs extent"""
        self.source.setSubsetString(None)
        extent = QgsRectangle(-103, 46, -25, 97)
        result = {
            f[self.pk_name]
            for f in self.source.getFeatures(QgsFeatureRequest().setFilterRect(extent))
        }
        expected = {1, 2, 4, 5}
        self.assertEqual(set(expected), result)

    def testExtentWithEstimatedMetadata(self):
        create_sql = (
            f'CREATE TABLE "{self.schemaName}"."test_extent" ( '
            "ID INTEGER NOT NULL PRIMARY KEY,"
            "GEOM1 ST_GEOMETRY(0),"
            "GEOM2 ST_GEOMETRY(4326))"
        )
        insert_sql = (
            f'INSERT INTO "{self.schemaName}"."test_extent" (ID, GEOM1, GEOM2) '
            f"VALUES (?, ST_GeomFromText(?), ST_GeomFromText(?, 4326)) "
        )
        insert_args = [
            [
                1,
                "POLYGON ((0 0, 20 0, 20 20, 0 20, 0 0))",
                "POLYGON ((0 0, 20 0, 20 20, 0 20, 0 0))",
            ]
        ]
        self.prepareTestTable("test_extent", create_sql, insert_sql, insert_args)

        vl_geom1 = self.createVectorLayer(
            f'estimatedmetadata=true table="{self.schemaName}"."test_extent" (GEOM1) sql=',
            "test_extent",
        )
        vl_geom2 = self.createVectorLayer(
            f'estimatedmetadata=true table="{self.schemaName}"."test_extent" (GEOM2) sql=',
            "test_extent",
        )

        self.assertEqual(QgsRectangle(0, 0, 20, 20), vl_geom1.dataProvider().extent())
        self.assertEqual(
            QgsRectangle(0, 0, 20, 20.284).toString(3),
            vl_geom2.dataProvider().extent().toString(3),
        )

    def testEncodeDecodeUri(self):
        """Test HANA encode/decode URI"""
        md = QgsProviderRegistry.instance().providerMetadata("hana")
        self.maxDiff = None
        self.assertEqual(
            md.decodeUri(
                "connectionType=0 dsn='HANADB1' "
                "driver='/usr/sap/hdbclient/libodbcHDB.so' dbname='qgis_tests' host=localhost port=30015 "
                "user='myuser' password='mypwd' srid=2016 table=\"public\".\"gis\" (geom) type=MultiPolygon key='id' "
                "sslEnabled='true' sslCryptoProvider='commoncrypto' sslValidateCertificate='false' "
                "sslHostNameInCertificate='hostname.domain.com' sslKeyStore='mykey.pem' "
                "sslTrustStore='server_root.crt' "
                "proxyEnabled='true' proxyHttp='true' proxyHost='h' proxyPort=2 proxyUsername='u' proxyPassword='p' "
            ),
            {
                "connectionType": "0",
                "dsn": "HANADB1",
                "driver": "/usr/sap/hdbclient/libodbcHDB.so",
                "dbname": "qgis_tests",
                "host": "localhost",
                "port": "30015",
                "username": "myuser",
                "password": "mypwd",
                "schema": "public",
                "table": "gis",
                "geometrycolumn": "geom",
                "srid": "2016",
                "type": 6,
                "key": "id",
                "sslEnabled": "true",
                "sslCryptoProvider": "commoncrypto",
                "sslValidateCertificate": "false",
                "sslHostNameInCertificate": "hostname.domain.com",
                "sslKeyStore": "mykey.pem",
                "sslTrustStore": "server_root.crt",
                "selectatid": False,
                "proxyEnabled": "true",
                "proxyHttp": "true",
                "proxyHost": "h",
                "proxyPort": "2",
                "proxyUsername": "u",
                "proxyPassword": "p",
            },
        )

        self.assertEqual(
            md.encodeUri(
                {
                    "connectionType": "0",
                    "dsn": "HANADB1",
                    "driver": "/usr/sap/hdbclient/libodbcHDB.so",
                    "dbname": "qgis_tests",
                    "host": "localhost",
                    "port": "30015",
                    "username": "myuser",
                    "password": "mypwd",
                    "schema": "public",
                    "table": "gis",
                    "geometrycolumn": "geom",
                    "srid": "2016",
                    "type": 6,
                    "key": "id",
                    "sslEnabled": "true",
                    "sslCryptoProvider": "commoncrypto",
                    "sslValidateCertificate": "false",
                    "sslHostNameInCertificate": "hostname.domain.com",
                    "sslKeyStore": "mykey.pem",
                    "sslTrustStore": "server_root.crt",
                    "selectatid": False,
                    "proxyEnabled": "true",
                    "proxyHttp": "false",
                    "proxyHost": "h",
                    "proxyPort": "3",
                    "proxyUsername": "u",
                    "proxyPassword": "p",
                }
            ),
            "dbname='qgis_tests' driver='/usr/sap/hdbclient/libodbcHDB.so' user='myuser' password='mypwd' "
            "key='id' srid=2016 connectionType='0' dsn='HANADB1' host='localhost' port='30015' "
            "proxyEnabled='true' proxyHost='h' proxyHttp='false' proxyPassword='p' proxyPort='3' "
            "proxyUsername='u' selectatid='false' sslCryptoProvider='commoncrypto' sslEnabled='true' "
            "sslHostNameInCertificate='hostname.domain.com' sslKeyStore='mykey.pem' "
            "sslTrustStore='server_root.crt' sslValidateCertificate='false' type='MultiPolygon' "
            'table="public"."gis" (geom)',
        )


if __name__ == "__main__":
    unittest.main()
