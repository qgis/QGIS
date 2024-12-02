"""QGIS Base Unit tests for QgsAbastractProviderConnection API.

Providers must implement a test based on TestPyQgsProviderConnectionBase

Provider test cases can define a "slowQuery" member with the SQL code for executeSql cancellation test

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "05/08/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import os
import time

from qgis.PyQt import QtCore
from qgis.PyQt.QtCore import QCoreApplication, QVariant, QDir
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsAbstractDatabaseProviderConnection,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFeedback,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMapLayerUtils,
    QgsProviderConnectionException,
    QgsProviderRegistry,
    QgsSettings,
    QgsTask,
    QgsVectorLayer,
    QgsWkbTypes,
)
from qgis.testing import start_app


class TestPyQgsProviderConnectionBase:
    # Provider test cases must define the string URI for the test
    uri = ""
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = ""

    configuration = {}

    defaultSchema = "public"

    myNewTable = "myNewTable"
    myVeryNewTable = "myVeryNewTable"
    myUtf8Table = "myUtf8\U0001f604Table"
    geometryColumnName = "geom"

    # Provider test cases can define a schema and table name for SQL query layers test
    sqlVectorLayerSchema = None  # string, empty string for schema-less DBs (SQLite)
    sqlVectorLayerTable = None  # string
    sqlVectorLayerCrs = None  # string

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

    def setUp(self):
        QgsSettings().clear()

    def tearDown(self):
        report_file_path = f"{QDir.tempPath()}/qgistest.html"
        with open(report_file_path, "a") as report_file:
            report_file.write(self.report)

    def treat_date_as_string(self):
        """Provider test case can override this to treat DATE type as STRING"""

        return False

    def getUniqueSchemaName(self, name):
        """This function must return a schema name with unique prefix/postfix,
        if the tests are run simultaneously on the same machine by different CI instances
        """
        return name

    def _test_save_load(self, md, uri, configuration):
        """Common tests on connection save and load"""

        conn = md.createConnection(uri, configuration)

        md.saveConnection(conn, "qgis_test1")
        # Check that we retrieve the new connection
        self.assertIn("qgis_test1", md.connections().keys())
        self.assertIn("qgis_test1", md.dbConnections().keys())

        return md.connections()["qgis_test1"]

    def _table_names(self, table_properties):
        """Return table default names from table property list"""
        return [p.defaultName() for p in table_properties]

    def _table_by_name(self, table_properties, name):
        """Filter table properties by table name"""
        try:
            return [p for p in table_properties if p.defaultName() == name][0]
        except IndexError:
            return None

    def _test_operations(self, md, conn):
        """Common tests"""

        capabilities = conn.capabilities()

        # Schema operations
        if (
            capabilities & QgsAbstractDatabaseProviderConnection.Capability.CreateSchema
            and capabilities & QgsAbstractDatabaseProviderConnection.Capability.Schemas
            and capabilities
            & QgsAbstractDatabaseProviderConnection.Capability.DropSchema
        ):

            myNewSchema = self.getUniqueSchemaName("myNewSchema")
            # Start clean
            if myNewSchema in conn.schemas():
                conn.dropSchema(myNewSchema, True)

            # Create
            conn.createSchema(myNewSchema)
            schemas = conn.schemas()
            self.assertIn(myNewSchema, schemas)

            # Create again
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.createSchema(myNewSchema)

            # Test rename
            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.RenameSchema
            ):
                # Rename
                myVeryNewSchema = self.getUniqueSchemaName("myVeryNewSchema")
                conn.renameSchema(myNewSchema, myVeryNewSchema)
                schemas = conn.schemas()
                self.assertIn(myVeryNewSchema, schemas)
                self.assertNotIn(myNewSchema, schemas)
                conn.renameSchema(myVeryNewSchema, myNewSchema)
                schemas = conn.schemas()
                self.assertNotIn(myVeryNewSchema, schemas)
                self.assertIn(myNewSchema, schemas)

            # Drop
            conn.dropSchema(myNewSchema)
            schemas = conn.schemas()
            self.assertNotIn(myNewSchema, schemas)

            # UTF8 schema
            myUtf8NewSchema = self.getUniqueSchemaName("myUtf8\U0001f604NewSchema")
            conn.createSchema(myUtf8NewSchema)
            schemas = conn.schemas()
            conn.dropSchema(myUtf8NewSchema)
            schemas = conn.schemas()
            self.assertNotIn(myUtf8NewSchema, schemas)

        # Table operations
        schema = None
        if (
            capabilities
            & QgsAbstractDatabaseProviderConnection.Capability.CreateVectorTable
            and capabilities & QgsAbstractDatabaseProviderConnection.Capability.Tables
            and capabilities
            & QgsAbstractDatabaseProviderConnection.Capability.DropVectorTable
        ):

            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.CreateSchema
            ):
                schema = self.getUniqueSchemaName("myNewSchema")
                conn.createSchema(schema)

            elif (
                capabilities & QgsAbstractDatabaseProviderConnection.Capability.Schemas
            ):
                schema = self.getUniqueSchemaName(self.defaultSchema)

            # Start clean
            if self.myNewTable in self._table_names(conn.tables(schema)):
                conn.dropVectorTable(schema, self.myNewTable)

            fields = QgsFields()
            fields.append(QgsField("string_t", QVariant.String))
            fields.append(QgsField("long_t", QVariant.LongLong))
            fields.append(QgsField("double_t", QVariant.Double))
            fields.append(QgsField("integer_t", QVariant.Int))
            fields.append(QgsField("date_t", QVariant.Date))
            fields.append(QgsField("datetime_t", QVariant.DateTime))
            fields.append(QgsField("time_t", QVariant.Time))
            options = {}
            crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
            typ = QgsWkbTypes.Type.LineString

            # Create
            conn.createVectorTable(
                schema, self.myNewTable, fields, typ, crs, True, options
            )
            table_names = self._table_names(conn.tables(schema))
            self.assertIn(self.myNewTable, table_names)

            # Create UTF8 table
            conn.createVectorTable(
                schema, self.myUtf8Table, fields, typ, crs, True, options
            )
            table_names = self._table_names(conn.tables(schema))
            self.assertIn(self.myNewTable, table_names)
            self.assertIn(self.myUtf8Table, table_names)
            conn.dropVectorTable(schema, self.myUtf8Table)
            table_names = self._table_names(conn.tables(schema))
            self.assertNotIn(self.myUtf8Table, table_names)
            self.assertIn(self.myNewTable, table_names)

            # insert something, because otherwise some databases cannot guess
            if self.providerKey in ["hana", "mssql", "oracle"]:
                f = QgsFeature(fields)
                f.setGeometry(
                    QgsGeometry.fromWkt("LineString (-72.345 71.987, -80 80)")
                )
                vl = QgsVectorLayer(
                    conn.tableUri(schema, self.myNewTable), "vl", self.providerKey
                )
                vl.dataProvider().addFeatures([f])

            # Check table information
            table_properties = conn.tables(schema)
            table_property = self._table_by_name(table_properties, self.myNewTable)
            self.assertEqual(table_property.maxCoordinateDimensions(), 2)
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.tableName(), self.myNewTable)
            self.assertEqual(table_property.geometryColumnCount(), 1)

            # with oracle line and curve have the same type, so it defaults to curve https://docs.oracle.com/database/121/SPATL/sdo_geometry-object-type.htm#SPATL494
            line_wkb_type = (
                QgsWkbTypes.Type.LineString
                if self.providerKey != "oracle"
                else QgsWkbTypes.Type.CompoundCurve
            )

            self.assertEqual(
                table_property.geometryColumnTypes()[0].wkbType, line_wkb_type
            )
            cols = table_property.geometryColumnTypes()
            self.assertEqual(cols[0].crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(table_property.defaultName(), self.myNewTable)

            # Check aspatial tables
            conn.createVectorTable(
                schema,
                "myNewAspatialTable",
                fields,
                QgsWkbTypes.Type.NoGeometry,
                crs,
                True,
                options,
            )
            table_properties = conn.tables(
                schema, QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial
            )
            table_property = self._table_by_name(table_properties, "myNewAspatialTable")
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.maxCoordinateDimensions(), 0)
            self.assertEqual(table_property.tableName(), "myNewAspatialTable")
            self.assertEqual(table_property.geometryColumnCount(), 0)
            self.assertEqual(table_property.geometryColumn(), "")
            self.assertEqual(table_property.defaultName(), "myNewAspatialTable")
            cols = table_property.geometryColumnTypes()
            # We always return geom col types, even when there is no geometry
            self.assertEqual(cols[0].wkbType, QgsWkbTypes.Type.NoGeometry)
            self.assertFalse(cols[0].crs.isValid())
            self.assertFalse(
                table_property.flags()
                & QgsAbstractDatabaseProviderConnection.TableFlag.Raster
            )
            self.assertFalse(
                table_property.flags()
                & QgsAbstractDatabaseProviderConnection.TableFlag.Vector
            )
            self.assertTrue(
                table_property.flags()
                & QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial
            )

            # Check executeSql
            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.ExecuteSql
            ):
                if schema:
                    table = f'"{schema}"."myNewAspatialTable"'
                else:
                    table = "myNewAspatialTable"

                # MSSQL literal syntax for UTF8 requires 'N' prefix
                # Oracle date time definition needs some prefix
                sql = 'INSERT INTO {} ("string_t", "long_t", "double_t", "integer_t", "date_t", "datetime_t", "time_t") VALUES ({}\'QGIS Rocks - \U0001f604\', 666, 1.234, 1234, {} \'2019-07-08\', {}, \'12:00:13.00\')'.format(
                    table,
                    "N" if self.providerKey == "mssql" else "",
                    "DATE" if self.providerKey == "oracle" else "",
                    (
                        "TIMESTAMP '2019-07-08 12:00:12'"
                        if self.providerKey == "oracle"
                        else "'2019-07-08T12:00:12'"
                    ),
                )
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = f'SELECT "string_t", "long_t", "double_t", "integer_t", "date_t", "datetime_t" FROM {table}'
                res = conn.executeSql(sql)

                expected_date = QtCore.QDate(2019, 7, 8)

                # GPKG and spatialite have no type for time
                if self.treat_date_as_string():
                    expected_date = "2019-07-08"

                # Oracle DATE type contains date and time and so returns a QDateTime object
                elif self.providerKey == "oracle":
                    expected_date = QtCore.QDateTime(QtCore.QDate(2019, 7, 8))

                self.assertEqual(
                    res,
                    [
                        [
                            "QGIS Rocks - \U0001f604",
                            666,
                            1.234,
                            1234,
                            expected_date,
                            QtCore.QDateTime(2019, 7, 8, 12, 0, 12),
                        ]
                    ],
                )

                # Test column names
                res = conn.execSql(sql)

                row_count = res.rowCount()
                # Some providers do not support rowCount and return -1
                if row_count != -1:
                    self.assertEqual(row_count, 1)

                rows = res.rows()
                self.assertEqual(
                    rows,
                    [
                        [
                            "QGIS Rocks - \U0001f604",
                            666,
                            1.234,
                            1234,
                            expected_date,
                            QtCore.QDateTime(2019, 7, 8, 12, 0, 12),
                        ]
                    ],
                )
                self.assertEqual(
                    res.columns(),
                    [
                        "string_t",
                        "long_t",
                        "double_t",
                        "integer_t",
                        "date_t",
                        "datetime_t",
                    ],
                )

                self.assertEqual(res.fetchedRowCount(), 1)

                # Test iterator
                old_rows = rows
                res = conn.execSql(sql)
                rows = []
                self.assertTrue(res.hasNextRow())

                for row in res:
                    rows.append(row)

                self.assertEqual(rows, old_rows)

                # Java style
                res = conn.execSql(sql)
                rows = []
                self.assertTrue(res.hasNextRow())
                while res.hasNextRow():
                    rows.append(res.nextRow())

                self.assertFalse(res.hasNextRow())

                # Test time_t
                sql = f'SELECT "time_t" FROM {table}'
                res = conn.executeSql(sql)

                # This does not work in MSSQL and returns a QByteArray, we have no way to know that it is a time
                # value and there is no way we can convert it.
                if self.providerKey != "mssql":
                    self.assertIn(res, ([[QtCore.QTime(12, 0, 13)]], [["12:00:13.00"]]))

                sql = "DELETE FROM {} WHERE \"string_t\" = {}'QGIS Rocks - \U0001f604'".format(
                    table, "N" if self.providerKey == "mssql" else ""
                )
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = f'SELECT "string_t", "integer_t" FROM {table}'
                res = conn.executeSql(sql)
                self.assertEqual(res, [])

            # Check that we do NOT get the aspatial table when querying for vectors
            table_names = self._table_names(
                conn.tables(
                    schema, QgsAbstractDatabaseProviderConnection.TableFlag.Vector
                )
            )
            self.assertIn(self.myNewTable, table_names)
            self.assertNotIn("myNewAspatialTable", table_names)

            # Query for rasters (in qgis_test schema or no schema for GPKG, spatialite has no support)
            if self.providerKey not in ("spatialite", "mssql", "hana", "oracle"):
                table_properties = conn.tables(
                    "qgis_test", QgsAbstractDatabaseProviderConnection.TableFlag.Raster
                )
                # At least one raster should be there (except for spatialite)
                self.assertGreaterEqual(len(table_properties), 1)
                table_property = table_properties[0]
                self.assertTrue(
                    table_property.flags()
                    & QgsAbstractDatabaseProviderConnection.TableFlag.Raster
                )
                self.assertFalse(
                    table_property.flags()
                    & QgsAbstractDatabaseProviderConnection.TableFlag.Vector
                )
                self.assertFalse(
                    table_property.flags()
                    & QgsAbstractDatabaseProviderConnection.TableFlag.Aspatial
                )

            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.RenameVectorTable
            ):
                # Rename
                conn.renameVectorTable(schema, self.myNewTable, self.myVeryNewTable)
                tables = self._table_names(conn.tables(schema))
                self.assertNotIn(self.myNewTable, tables)
                self.assertIn(self.myVeryNewTable, tables)
                # Rename it back
                conn.renameVectorTable(schema, self.myVeryNewTable, self.myNewTable)
                tables = self._table_names(conn.tables(schema))
                self.assertIn(self.myNewTable, tables)
                self.assertNotIn(self.myVeryNewTable, tables)

            # Vacuum
            if capabilities & QgsAbstractDatabaseProviderConnection.Capability.Vacuum:
                conn.vacuum(schema, self.myNewTable)

            # Spatial index
            spatial_index_exists = False
            # we don't initially know if a spatial index exists -- some formats may create them by default, others don't
            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.SpatialIndexExists
            ):
                spatial_index_exists = conn.spatialIndexExists(
                    schema, self.myNewTable, self.geometryColumnName
                )
            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DeleteSpatialIndex
            ):
                if spatial_index_exists:
                    conn.deleteSpatialIndex(
                        schema, self.myNewTable, self.geometryColumnName
                    )
                if (
                    capabilities
                    & QgsAbstractDatabaseProviderConnection.Capability.SpatialIndexExists
                ):
                    self.assertFalse(
                        conn.spatialIndexExists(
                            schema, self.myNewTable, self.geometryColumnName
                        )
                    )

            if capabilities & (
                QgsAbstractDatabaseProviderConnection.Capability.CreateSpatialIndex
                | QgsAbstractDatabaseProviderConnection.Capability.SpatialIndexExists
            ):
                options = QgsAbstractDatabaseProviderConnection.SpatialIndexOptions()
                options.geometryColumnName = self.geometryColumnName

                if not conn.spatialIndexExists(
                    schema, self.myNewTable, options.geometryColumnName
                ):
                    conn.createSpatialIndex(schema, self.myNewTable, options)

                self.assertTrue(
                    conn.spatialIndexExists(
                        schema, self.myNewTable, self.geometryColumnName
                    )
                )

                # now we know for certain a spatial index exists, let's retry dropping it
                if (
                    capabilities
                    & QgsAbstractDatabaseProviderConnection.Capability.DeleteSpatialIndex
                ):
                    conn.deleteSpatialIndex(
                        schema, self.myNewTable, self.geometryColumnName
                    )
                    if (
                        capabilities
                        & QgsAbstractDatabaseProviderConnection.Capability.SpatialIndexExists
                    ):
                        self.assertFalse(
                            conn.spatialIndexExists(
                                schema, self.myNewTable, self.geometryColumnName
                            )
                        )

            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DropSchema
            ):
                # Drop schema (should fail)
                with self.assertRaises(QgsProviderConnectionException) as ex:
                    conn.dropSchema(schema)

            # Check some column types operations
            table = self._table_by_name(conn.tables(schema), self.myNewTable)
            self.assertEqual(len(table.geometryColumnTypes()), 1)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, line_wkb_type)
            # Add a new (existing type)
            table.addGeometryColumnType(
                line_wkb_type, QgsCoordinateReferenceSystem.fromEpsgId(3857)
            )
            self.assertEqual(len(table.geometryColumnTypes()), 1)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, line_wkb_type)
            # Add a new one
            table.addGeometryColumnType(
                line_wkb_type, QgsCoordinateReferenceSystem.fromEpsgId(4326)
            )
            self.assertEqual(len(table.geometryColumnTypes()), 2)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, line_wkb_type)
            ct = table.geometryColumnTypes()[1]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(4326))
            self.assertEqual(ct.wkbType, line_wkb_type)

            # Check fields
            fields = conn.fields(schema, self.myNewTable)
            for f in [
                "string_t",
                "long_t",
                "double_t",
                "integer_t",
                "date_t",
                "datetime_t",
                "time_t",
            ]:
                self.assertIn(f, fields.names())

            if capabilities & QgsAbstractDatabaseProviderConnection.Capability.AddField:
                field = QgsField("short_lived_field", QVariant.Int, "integer")
                conn.addField(field, schema, self.myNewTable)
                fields = conn.fields(schema, self.myNewTable)
                self.assertIn("short_lived_field", fields.names())

                if (
                    capabilities
                    & QgsAbstractDatabaseProviderConnection.Capability.DeleteField
                ):
                    conn.deleteField("short_lived_field", schema, self.myNewTable)
                    # This fails on Travis for spatialite, for no particular reason
                    if self.providerKey == "spatialite" and not os.environ.get(
                        "TRAVIS", False
                    ):
                        fields = conn.fields(schema, self.myNewTable)
                        self.assertNotIn("short_lived_field", fields.names())

            # Drop table
            conn.dropVectorTable(schema, self.myNewTable)
            conn.dropVectorTable(schema, "myNewAspatialTable")
            table_names = self._table_names(conn.tables(schema))
            self.assertNotIn(self.myNewTable, table_names)

            if (
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DropSchema
            ):
                # Drop schema
                conn.dropSchema(schema)
                self.assertNotIn(schema, conn.schemas())

        conns = md.connections()
        self.assertTrue(
            isinstance(list(conns.values())[0], QgsAbstractDatabaseProviderConnection)
        )

        # Remove connection
        spy_deleted = QSignalSpy(md.connectionDeleted)
        md.deleteConnection("qgis_test1")
        self.assertEqual(list(md.connections().values()), [])
        self.assertEqual(len(spy_deleted), 1)

    def test_errors(self):
        """Test SQL errors"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = self._test_save_load(md, self.uri, self.configuration)

        if (
            conn.capabilities()
            & QgsAbstractDatabaseProviderConnection.Capability.Schemas
        ):
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.createVectorTable(
                    "notExists",
                    "notReally",
                    QgsFields(),
                    QgsWkbTypes.Type.Point,
                    QgsCoordinateReferenceSystem(),
                    False,
                    {},
                )

        if (
            conn.capabilities()
            & QgsAbstractDatabaseProviderConnection.Capability.DropVectorTable
        ):
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.executeSql('DROP TABLE "notExists"')

        # Remove connection
        md.deleteConnection("qgis_test1")
        self.assertEqual(list(md.connections().values()), [])

    def test_connections(self):
        """Main test"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)

        # Run common tests
        created_spy = QSignalSpy(md.connectionCreated)
        changed_spy = QSignalSpy(md.connectionChanged)

        conn = self._test_save_load(md, self.uri, self.configuration)

        self.assertEqual(len(created_spy), 1)
        self.assertEqual(len(changed_spy), 0)

        # if we try to save again, the connectionChanged signal should be emitted instead of connectionCreated
        md.saveConnection(conn, "qgis_test1")
        self.assertEqual(len(created_spy), 1)
        self.assertEqual(len(changed_spy), 1)

        self._test_operations(md, conn)

    def test_native_types(self):
        """Test native types retrieval"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        native_types = conn.nativeTypes()
        names = [nt.mTypeName.lower() for nt in native_types]
        self.assertTrue(
            "integer" in names or "decimal" in names or "number" in names, names
        )
        self.assertTrue(
            "string" in names
            or "text" in names
            or "nvarchar" in names
            or "varchar2" in names,
            names,
        )

    def testExecuteSqlCancel(self):
        """Test that feedback can cancel an executeSql query"""

        if hasattr(self, "slowQuery"):

            md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
            conn = md.createConnection(self.uri, {})
            feedback = QgsFeedback()

            def _run(task):
                conn.executeSql(self.slowQuery, feedback=feedback)

            def _cancel():
                feedback.cancel()

            start = time.time()
            QtCore.QTimer.singleShot(500, _cancel)
            task = QgsTask.fromFunction("test long running query", _run)
            QgsApplication.taskManager().addTask(task)
            while task.status() not in [
                QgsTask.TaskStatus.Complete,
                QgsTask.TaskStatus.Terminated,
            ]:
                QgsApplication.processEvents()
            end = time.time()
            self.assertLess(end - start, 1)

    def testCreateSqlVectorLayer(self):
        """Test vector layer creation from SQL query"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})

        if (
            not conn.capabilities()
            & QgsAbstractDatabaseProviderConnection.Capability.SqlLayers
        ):
            print(
                f"FIXME: {self.providerKey} data provider does not support query layers!"
            )
            return

        schema = getattr(self, "sqlVectorLayerSchema", None)
        if schema is None:
            print(
                f"FIXME: {self.providerKey} data provider test case does not define self.sqlVectorLayerSchema for query layers test!"
            )
            return

        table = getattr(self, "sqlVectorLayerTable", None)
        if table is None:
            print(
                f"FIXME: {self.providerKey} data provider test case does not define self.sqlVectorLayerTable for query layers test!"
            )
            return

        crs = getattr(self, "sqlVectorLayerCrs", None)
        if crs is None:
            print(
                f"FIXME: {self.providerKey} data provider test case does not define self.sqlVectorLayerCrs for query layers test!"
            )
            return

        sql_layer_capabilities = conn.sqlLayerDefinitionCapabilities()

        # Try a simple select first
        table_info = conn.table(schema, table)

        options = QgsAbstractDatabaseProviderConnection.SqlVectorLayerOptions()
        options.layerName = "My SQL Layer"

        # Some providers do not support schema
        if schema != "":
            options.sql = (
                f'SELECT * FROM "{table_info.schema()}"."{table_info.tableName()}"'
            )
        else:
            options.sql = f'SELECT * FROM "{table_info.tableName()}"'

        options.geometryColumn = table_info.geometryColumn()
        options.primaryKeyColumns = table_info.primaryKeyColumns()

        vl = conn.createSqlVectorLayer(options)
        self.assertTrue(vl.isSqlQuery())
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.isSpatial())
        self.assertEqual(vl.name(), options.layerName)
        self.assertEqual(vl.sourceCrs().authid(), crs)

        # Test that a database connection can be retrieved from an existing layer
        vlconn = QgsMapLayerUtils.databaseConnection(vl)
        self.assertIsNotNone(vlconn)
        self.assertEqual(vlconn.uri(), conn.uri())

        # Some providers can also create SQL layer without an explicit PK
        if sql_layer_capabilities & Qgis.SqlLayerDefinitionCapability.PrimaryKeys:
            options.primaryKeyColumns = []
            vl = conn.createSqlVectorLayer(options)
            self.assertTrue(vl.isSqlQuery())
            self.assertTrue(vl.isValid())
            self.assertTrue(vl.isSpatial())

        # Some providers can also create SQL layer without an explicit geometry column
        if sql_layer_capabilities & Qgis.SqlLayerDefinitionCapability.GeometryColumn:
            options.primaryKeyColumns = table_info.primaryKeyColumns()
            options.geometryColumn = ""
            vl = conn.createSqlVectorLayer(options)
            self.assertTrue(vl.isValid())
            self.assertTrue(vl.isSqlQuery())
            # This may fail for OGR where the provider is smart enough to guess the geometry column
            if self.providerKey != "ogr":
                self.assertFalse(vl.isSpatial())

        # Some providers can also create SQL layer with filters
        if (
            sql_layer_capabilities
            & Qgis.SqlLayerDefinitionCapability.SubsetStringFilter
        ):
            options.primaryKeyColumns = table_info.primaryKeyColumns()
            options.geometryColumn = table_info.geometryColumn()
            options.filter = f'"{options.primaryKeyColumns[0]}" > 0'
            vl = conn.createSqlVectorLayer(options)
            self.assertTrue(vl.isValid())
            self.assertTrue(vl.isSqlQuery())
            self.assertTrue(vl.isSpatial())
