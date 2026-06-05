"""QGIS Unit tests for QgsServer OGC API Schema Handler.

From build dir, run: ctest -R PyQgsServerApiSchema -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "2026-05-26"
__copyright__ = "Copyright 2026, The QGIS Project"

import json
import os
import re
import shutil

from osgeo import gdal, ogr
from provider_python import PyProvider

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from contextlib import contextmanager
from urllib import parse

from jsonschema import validate
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsDataProvider,
    QgsEditorWidgetSetup,
    QgsExpression,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsFieldConstraints,
    QgsFields,
    QgsGeometry,
    QgsMemoryProviderUtils,
    QgsProject,
    QgsProviderMetadata,
    QgsProviderRegistry,
    QgsRelation,
    QgsRelationContext,
    QgsVectorLayer,
    QgsVectorLayerServerProperties,
)
from qgis.PyQt import QtCore
from qgis.server import (
    QgsAccessControlFilter,
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerApi,
    QgsServerApiBadRequestException,
    QgsServerApiContext,
    QgsServerApiUtils,
    QgsServerOgcApi,
    QgsServerOgcApiHandler,
    QgsServerQueryStringParameter,
    QgsServiceRegistry,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase
from test_qgsserver_api import QgsServerAPITestBase
from utilities import unitTestDataPath


class EnumProvider(PyProvider):
    @classmethod
    def providerKey(cls):
        """Returns the memory provider key"""
        return "enumprovider"

    @classmethod
    def createProvider(cls, uri, providerOptions, flags=QgsDataProvider.ReadFlags()):
        return EnumProvider(uri, providerOptions, flags)

    def enumValues(self, fieldIdx):
        if self.fields()[fieldIdx].name().startswith("enum"):
            return ["value1", "value2", "value3"]
        return []


class QgsServerOgcApiSchemaTest(QgsServerAPITestBase):
    """QGIS API server tests"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        # Register the provider
        r = QgsProviderRegistry.instance()
        metadata = QgsProviderMetadata(
            EnumProvider.providerKey(),
            EnumProvider.description(),
            EnumProvider.createProvider,
        )
        assert r.registerProvider(metadata)

    # Set to True in child classes to re-generate reference files for this class
    regeregenerate_api_reference = True

    def _getJsonResponse(self, url, project):
        request = QgsBufferServerRequest(url)
        response = QgsBufferServerResponse()
        server = QgsServer()
        server.handleRequest(request, response, project)
        self.assertEqual(
            response.statusCode(),
            200,
            f"Request failed with status {response.statusCode()} and message: {bytes(response.body()).decode('utf8')} for URL: {url}",
        )
        response_str = bytes(response.body()).decode("utf8")
        j = json.loads(response_str)
        return j

    def testQgsServerOgcApiSchemaHandler(self):

        temp_dir = QtCore.QTemporaryDir()
        temp_dir_path = temp_dir.path()
        temp_db_path = os.path.join(temp_dir_path, "test.gpkg")
        # Create a test GeoPackage with a single layer using ogr
        driver = ogr.GetDriverByName("GPKG")
        ds = driver.CreateDataSource(temp_db_path)
        layer = ds.CreateLayer("layer1", geom_type=ogr.wkbPoint, srs=None)
        field_defn = ogr.FieldDefn("field1_string", ogr.OFTString)
        layer.CreateField(field_defn)

        # Add a feature to the layer
        feature_defn = layer.GetLayerDefn()
        feature = ogr.Feature(feature_defn)
        feature.SetField("field1_string", "test value")
        feature.SetGeometry(ogr.CreateGeometryFromWkt("POINT (1 1)"))
        layer.CreateFeature(feature)

        # Close
        ds = None

        layer = QgsVectorLayer(temp_db_path + "|layername=layer1", "layer1", "ogr")
        self.assertTrue(layer.isValid())

        project = QgsProject()
        project.addMapLayer(layer)
        # Expose to WFS
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/layer1/schema.json", project
        )

        self.assertEqual(j["title"], "layer1")
        self.assertEqual(j["type"], "object")
        self.assertEqual(
            j["$id"], "http://server.qgis.org/wfs3/collections/layer1/schema.json"
        )
        self.assertEqual(j["$schema"], "https://json-schema.org/draft/2020-12/schema")
        self.assertEqual(j["properties"]["fid"]["type"], "integer")
        self.assertEqual(j["properties"]["fid"]["format"], "int64")
        self.assertEqual(j["properties"]["fid"]["x-ogc-propertySeq"], 1)
        self.assertEqual(j["properties"]["field1_string"]["type"], "string")
        self.assertEqual(j["properties"]["field1_string"]["x-ogc-propertySeq"], 2)
        self.assertEqual(j["properties"]["geometry"]["format"], "geometry-point")
        self.assertEqual(j["properties"]["geometry"]["x-ogc-propertySeq"], 3)
        self.assertEqual(j["properties"]["geometry"]["x-ogc-role"], "primary-geometry")
        self.assertEqual(j["properties"]["id"]["type"], "integer")
        self.assertEqual(j["properties"]["id"]["readOnly"], True)
        self.assertEqual(j["properties"]["id"]["x-ogc-propertySeq"], 0)
        self.assertEqual(j["properties"]["id"]["x-ogc-role"], "id")
        self.assertEqual(j["required"], ["fid", "geometry"])

    def testFieldWidgetConfiguration(self):

        def _test_widget_conf(
            field,
            widget_type,
            widget_setup,
            expected_property,
            required=False,
            read_only=False,
            expression=None,
        ):
            temp_dir = QtCore.QTemporaryDir()
            temp_dir_path = temp_dir.path()
            temp_db_path = os.path.join(temp_dir_path, "test.gpkg")
            # Create a test GeoPackage with a single layer using ogr
            driver = ogr.GetDriverByName("GPKG")
            ds = driver.CreateDataSource(temp_db_path)
            layer = ds.CreateLayer("layer1", geom_type=ogr.wkbPoint, srs=None)
            ds = None
            layer = QgsVectorLayer(temp_db_path + "|layername=layer1", "layer1", "ogr")
            widget = QgsEditorWidgetSetup(widget_type, widget_setup)
            field.setEditorWidgetSetup(widget)
            layer.dataProvider().addAttributes([field])
            layer.updateFields()

            if expression:
                exp = QgsExpression(expression)
                self.assertTrue(
                    exp.isValid(),
                    f"Expression {expression} is not valid: {exp.parserErrorString()}",
                )
                layer.setConstraintExpression(1, expression)
                layer.setFieldConstraint(
                    1,
                    QgsFieldConstraints.ConstraintExpression,
                    QgsFieldConstraints.ConstraintStrength.ConstraintStrengthHard,
                )

            if required:
                layer.setFieldConstraint(
                    1,
                    QgsFieldConstraints.ConstraintNotNull,
                    QgsFieldConstraints.ConstraintStrength.ConstraintStrengthHard,
                )

            if read_only:
                ediform_config = layer.editFormConfig()
                ediform_config.setReadOnly(layer.fields().indexOf(field.name()))
                layer.setEditFormConfig(ediform_config)

            project = QgsProject()
            project.addMapLayer(layer)
            # Expose to WFS
            project.writeEntry("WFSLayers", "/", [layer.id()])

            j = self._getJsonResponse(
                "http://server.qgis.org/wfs3/collections/layer1/schema.json", project
            )
            field_name = field.alias() if field.alias() else field.name()
            self.assertEqual(j["properties"][field_name], expected_property)
            if required:
                self.assertIn("field1", j["required"])

        # Field with regexp expression constraint
        field = QgsField("field1", QtCore.QMetaType.Type.QString)
        _test_widget_conf(
            field,
            "TextEdit",
            {},
            {
                "type": "string",
                "readOnly": True,
                "x-ogc-propertySeq": 2,
                "pattern": r"^[A-Z]{3}$",
            },
            read_only=True,
            expression="regexp_match(\"field1\", '^[A-Z]{3}$')",
        )

        # Field with regexp expression constraint but on another field - should not be picked up as a pattern constraint
        field = QgsField("field1", QtCore.QMetaType.Type.QString)
        _test_widget_conf(
            field,
            "TextEdit",
            {},
            {
                "type": "string",
                "readOnly": True,
                "x-ogc-propertySeq": 2,
            },
            read_only=True,
            expression="regexp_match(\"field2\", '^[A-Z]{3}$')",
        )

        # Field with regexp expression constraint but taken from another field - should not be picked up as a pattern constraint
        field = QgsField("field1", QtCore.QMetaType.Type.QString)
        _test_widget_conf(
            field,
            "TextEdit",
            {},
            {
                "type": "string",
                "readOnly": True,
                "x-ogc-propertySeq": 2,
            },
            read_only=True,
            expression='regexp_match("field1", "refield")',
        )

        # ValueMap
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.QString),
            "ValueMap",
            {"map": {"a": "A", "b": "B"}},
            {
                "type": "string",
                "x-ogc-codelist": {
                    "oneOf": [
                        {"const": "A", "title": "a"},
                        {"const": "B", "title": "b"},
                    ],
                    "title": "field1",
                },
                "x-ogc-propertySeq": 2,
            },
            required=True,
        )

        # String JSON
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.QVariantMap),
            "TextEdit",
            {},
            {"format": "json", "type": "string", "x-ogc-propertySeq": 2},
        )

        # String with length constraint
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.QString, len=10),
            "TextEdit",
            {},
            {"type": "string", "maxLength": 10, "x-ogc-propertySeq": 2},
        )

        # Field with alias
        field = QgsField("field1", QtCore.QMetaType.Type.QString)
        field.setAlias("My Field")
        _test_widget_conf(
            field,
            "TextEdit",
            {},
            {"type": "string", "x-ogc-propertySeq": 2},
        )

        # Readonly field
        field = QgsField("field1", QtCore.QMetaType.Type.QString)
        _test_widget_conf(
            field,
            "TextEdit",
            {},
            {"type": "string", "readOnly": True, "x-ogc-propertySeq": 2},
            read_only=True,
        )

        ##### Widget configuration tests

        # Range widget with an integer field
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.Int),
            "Range",
            {"Min": 2, "Max": 10, "Suffix": "mm"},
            {
                "format": "int32",
                "type": "integer",
                "minimum": 2,
                "maximum": 10,
                "x-ogc-propertySeq": 2,
                "x-ogc-unit": "mm",
            },
        )

        # Range widget with a double field
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.Double),
            "Range",
            {"Min": 0.5, "Max": 1.5, "Step": 0.1, "Suffix": "inches"},
            {
                "format": "double",
                "type": "number",
                "minimum": 0.5,
                "maximum": 1.5,
                "x-ogc-propertySeq": 2,
                "x-ogc-unit": "inches",
            },
        )

        # Same test with default min/max values
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.Double),
            "Range",
            {
                "Suffix": "mm",
                "Max": 1.7976931348623157e308,
                "Min": -1.7976931348623157e308,
            },
            {
                "format": "double",
                "type": "number",
                "x-ogc-propertySeq": 2,
                "x-ogc-unit": "mm",
            },
        )

        # UUID
        _test_widget_conf(
            QgsField("field1", QtCore.QMetaType.Type.QString),
            "UuidGenerator",
            {},
            {"format": "uuid", "type": "string", "x-ogc-propertySeq": 2},
        )

    def test_enum(self):

        layer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&key=pk",
            "layer1",
            "enumprovider",
        )
        widget = QgsEditorWidgetSetup("Enumeration", {})
        field = QgsField("enum1", QtCore.QMetaType.Type.QString)
        field.setEditorWidgetSetup(widget)
        layer.dataProvider().addAttributes([field])
        layer.updateFields()
        project = QgsProject()
        project.addMapLayer(layer)
        # Expose to WFS
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/layer1/schema.json", project
        )
        self.assertEqual(
            j["properties"]["enum1"],
            {
                "enum": ["value1", "value2", "value3"],
                "type": "string",
                "x-ogc-propertySeq": 2,
            },
        )

    def test_geometry(self):

        layer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=pk:integer&key=pk",
            "layer1",
            "enumprovider",
        )
        widget = QgsEditorWidgetSetup("Geometry", {})
        field = QgsField("geom1", QtCore.QMetaType.Type.User)
        field.setEditorWidgetSetup(widget)
        layer.dataProvider().addAttributes([field])
        layer.updateFields()
        project = QgsProject()
        project.addMapLayer(layer)
        # Expose to WFS
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/layer1/schema.json", project
        )
        self.assertEqual(
            j["properties"]["geom1"],
            {"format": "geometry-any", "type": "string", "x-ogc-propertySeq": 2},
        )

    def test_relation(self):

        # Create two memory layers with a fk:pk relation
        referenced_layer = QgsVectorLayer(
            "Point?field=pk:integer&key=pk", "referenced", "memory"
        )
        feature = QgsFeature(referenced_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(0 0)"))
        feature.setAttribute("pk", 123)
        self.assertTrue(referenced_layer.dataProvider().addFeature(feature))
        feature = QgsFeature(referenced_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(1 1)"))
        feature.setAttribute("pk", 456)
        self.assertTrue(referenced_layer.dataProvider().addFeature(feature))

        referencing_layer = QgsVectorLayer(
            "Point?field=pk:integer&field=fk:integer&key=pk", "referencing", "memory"
        )
        feature = QgsFeature(referencing_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(2 2)"))
        feature.setAttribute("pk", 1)
        feature.setAttribute("fk", 123)
        self.assertTrue(referencing_layer.dataProvider().addFeature(feature))
        feature = QgsFeature(referencing_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(3 3)"))
        feature.setAttribute("pk", 2)
        feature.setAttribute("fk", 456)
        self.assertTrue(referencing_layer.dataProvider().addFeature(feature))

        project = QgsProject()
        project.addMapLayer(referenced_layer)
        project.addMapLayer(referencing_layer)

        relation = QgsRelation(QgsRelationContext(project))
        relation.setName("relation1")
        relation.setId("relation1")
        relation.addFieldPair("fk", "pk")
        relation.setReferencingLayer(referencing_layer.id())
        relation.setReferencedLayer(referenced_layer.id())

        r_manager = project.relationManager()
        r_manager.addRelation(relation)
        self.assertIn("relation1", r_manager.relations())

        # Expose to WFS
        project.writeEntry(
            "WFSLayers", "/", [referencing_layer.id(), referenced_layer.id()]
        )

        # First: test with no editor widget, the fk field should be exposed as a regular integer field
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/referencing/schema.json", project
        )

        self.assertEqual(
            j["properties"]["fk"],
            {"format": "int32", "type": "integer", "x-ogc-propertySeq": 2},
        )

        # Setup the widget
        referencing_layer.setEditorWidgetSetup(
            1, QgsEditorWidgetSetup("RelationReference", {"Relation": "relation1"})
        )

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/referencing/schema.json", project
        )

        self.assertEqual(
            j["properties"]["fk"],
            {
                "format": "int32",
                "title": "referenced",
                "type": "integer",
                "x-ogc-collectionId": "referenced",
                "x-ogc-propertySeq": 2,
                "x-ogc-role": "reference",
            },
        )

        # Use a value relation widget
        referencing_layer.setEditorWidgetSetup(
            1,
            QgsEditorWidgetSetup(
                "ValueRelation",
                {
                    # Note, Layer and LayerName are the only relevant information for OAPIF, the rest is just for the QGIS UI
                    "Layer": referenced_layer.id(),
                    "LayerName": "Test this is taken",
                    # Not relevant for OAPIF:
                    "LayerProviderName": referenced_layer.dataProvider().name(),
                    "LayerSource": referenced_layer.source(),
                    "Key": "pk",
                    "Value": "pk",  # Note, in a real project this would be a more user friendly field
                },
            ),
        )

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/referencing/schema.json", project
        )

        self.assertEqual(
            j["properties"]["fk"],
            {
                "format": "int32",
                "title": "referenced",
                "type": "integer",
                "x-ogc-collectionId": "referenced",
                "x-ogc-propertySeq": 2,
                "x-ogc-role": "reference",
            },
        )

    def test_extra_geometry(self):

        if "QGIS_PGTEST_DB" in os.environ:
            dbconn = os.environ["QGIS_PGTEST_DB"]
        else:
            dbconn = "service=qgis_test dbname=qgis_test sslmode=disable "

        # Test layer
        uri = dbconn + " dbname=qgis_test sslmode=disable "
        md = QgsProviderRegistry.instance().providerMetadata("postgres")

        if not md:
            self.skipTest("postgres provider not available")

        conn = md.createConnection(uri, {})
        conn.executeSql('DROP TABLE IF EXISTS "qgis_test"."TwoGeoms" CASCADE')
        conn.executeSql('CREATE TABLE "qgis_test"."TwoGeoms" (id serial primary key)')

        conn.executeSql(
            "SELECT AddGeometryColumn('qgis_test', 'TwoGeoms', 'geom1', 4326, 'POINT', 2 )"
        )
        conn.executeSql(
            "SELECT AddGeometryColumn('qgis_test', 'TwoGeoms', 'geom2', 4326, 'POINT', 2 )"
        )

        conn.executeSql(
            "INSERT INTO \"qgis_test\".\"TwoGeoms\" (geom1, geom2) VALUES (ST_GeomFromText('POINT(0 0)', 4326), ST_GeomFromText('POINT(1 1)', 4326))"
        )

        layer_source = conn.tableUri("qgis_test", "TwoGeoms") + " (geom2) sql="

        project = QgsProject()
        layer = QgsVectorLayer(layer_source, "TwoGeoms", "postgres")
        self.assertTrue(layer.isValid())

        project.addMapLayer(layer)
        # Expose to WFS
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/TwoGeoms/schema.json", project
        )

        self.assertEqual(
            j["properties"]["geom1"],
            {"format": "geometry-any", "type": "string", "x-ogc-propertySeq": 1},
        )

        # Get data and check geom1 is correctly exposed as WKT
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/TwoGeoms/items?f=json", project
        )
        self.assertEqual(j["features"][0]["properties"]["geom1"].upper(), "POINT (0 0)")


if __name__ == "__main__":
    unittest.main()
