"""QGIS Unit tests for QgsArrowIterator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2025 by Dewey Dunnington"
__date__ = "04/11/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

import datetime
import json
import unittest

import pyarrow as pa
import shapely

from qgis.core import (
    QgsArrowIterator,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMemoryProviderUtils,
    QgsPointXY,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import QMetaType
from qgis.testing import QgisTestCase


# Crashes pytest for me locally
# start_app()


class TestQgsArrowIterator(QgisTestCase):

    def create_test_layer_with_geometry(self, crs):
        # Create fields
        fields = QgsFields()
        fields.append(QgsField("id", QMetaType.Type.Int))
        fields.append(QgsField("name", QMetaType.Type.QString))

        layer = QgsMemoryProviderUtils.createMemoryLayer(
            "test_layer", fields, QgsWkbTypes.Point, crs
        )

        # Add features
        features = []
        for i in range(10):
            feature = QgsFeature(fields)
            feature.setAttributes([i + 1, f"feat_{i + 1}"])

            # Create point geometry
            point = QgsPointXY(i, i + 20)
            feature.setGeometry(QgsGeometry.fromPointXY(point))

            features.append(feature)

        # Add features to layer
        layer.dataProvider().addFeatures(features)
        layer.updateExtents()

        return layer

    def create_test_layer_single_field(
        self, meta_type, py_values, subtype=QMetaType.Type.UnknownType
    ):
        fields = QgsFields()
        fields.append(QgsField("f", meta_type, subType=subtype))
        layer = QgsMemoryProviderUtils.createMemoryLayer("test_layer", fields)

        features = []
        for py_value in py_values:
            feature = QgsFeature(fields)
            feature.setAttributes([py_value])
            features.append(feature)

        layer.dataProvider().addFeatures(features)
        return layer

    def test_infer_schema(self):
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        layer = self.create_test_layer_with_geometry(crs)

        schema = QgsArrowIterator.inferSchema(layer)
        self.assertTrue(schema.isValid())

        pa_schema = pa.Schema._import_from_c(schema.cSchemaAddress())
        assert pa_schema.names == ["id", "name", "geometry"]
        assert pa_schema.types == [pa.int32(), pa.string(), pa.binary()]

        geometry_field_metadata = pa_schema.field("geometry").metadata
        assert geometry_field_metadata[b"ARROW:extension:name"] == b"geoarrow.wkb"
        geoarrow_metadata = json.loads(
            geometry_field_metadata[b"ARROW:extension:metadata"]
        )
        assert geoarrow_metadata["crs"]["type"] == "GeographicCRS"

    def test_infer_schema_no_crs(self):
        layer = self.create_test_layer_with_geometry(QgsCoordinateReferenceSystem())
        schema = QgsArrowIterator.inferSchema(layer)
        pa_schema = pa.Schema._import_from_c(schema.cSchemaAddress())
        geometry_field_metadata = pa_schema.field("geometry").metadata
        geoarrow_metadata = json.loads(
            geometry_field_metadata[b"ARROW:extension:metadata"]
        )
        assert geoarrow_metadata == {}

    def test_iterate(self):
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        layer = self.create_test_layer_with_geometry(crs)
        schema = QgsArrowIterator.inferSchema(layer)

        iterator = QgsArrowIterator(layer.getFeatures())
        iterator.setSchema(schema, 2)

        pa_schema = pa.Schema._import_from_c(schema.cSchemaAddress())
        batch0 = iterator.nextFeatures(4)
        pa_batch0 = pa.RecordBatch._import_from_c(batch0.cArrayAddress(), pa_schema)
        batch1 = iterator.nextFeatures(4)
        pa_batch1 = pa.RecordBatch._import_from_c(batch1.cArrayAddress(), pa_schema)
        batch2 = iterator.nextFeatures(4)
        pa_batch2 = pa.RecordBatch._import_from_c(batch2.cArrayAddress(), pa_schema)

        assert len(pa_batch0) == 4
        assert len(pa_batch1) == 4
        assert len(pa_batch2) == 2

        tab = pa.Table.from_batches([pa_batch0, pa_batch1, pa_batch2])
        assert tab["id"].to_pylist() == [i + 1 for i in range(10)]
        assert tab["name"].to_pylist() == [f"feat_{i + 1}" for i in range(10)]

        expected_geometries = [shapely.Point(i, i + 20) for i in range(10)]
        shapely_geometries = [shapely.from_wkb(g) for g in tab["geometry"].to_pylist()]
        assert expected_geometries == shapely_geometries

    def test_type_int(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Int, [1, 2, None, 4, 5]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.int32()})

    def test_type_double(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Double, [1.0, 2.0, None, 4.0, 5.0]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.float64()})

    def test_type_string(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.QString, ["a", "b", None, "d", "e"]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.string()})

    def test_type_binary(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.QByteArray, [b"a", b"b", None, b"d", b"e"]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.binary()})

    def test_type_boolean(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Bool, [True, False, None, True, False]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.bool_()})

    def test_type_date(self):
        dates = [datetime.date(2020, 1, i) for i in range(1, 6)]
        dates[2] = None

        layer = self.create_test_layer_single_field(QMetaType.Type.QDate, dates)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.date32()})

    def test_type_time(self):
        times = [datetime.time(17, 0, i) for i in range(5)]
        times[2] = None

        layer = self.create_test_layer_single_field(QMetaType.Type.QTime, times)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.time32("ms")})

    def test_type_datetime(self):
        datetimes = [datetime.datetime(2020, 1, 1, 17, 0, i) for i in range(5)]
        datetimes[2] = None

        layer = self.create_test_layer_single_field(QMetaType.Type.QDateTime, datetimes)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.timestamp("ms", tz=None)})

    def test_type_string_list(self):
        items = [["a", "b"], ["c"], None, ["d", "e", "f"], ["g"]]

        layer = self.create_test_layer_single_field(QMetaType.Type.QStringList, items)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.string())})

    def test_type_double_list(self):
        items = [[1.0, 2.0], [3.0], None, [4.0, 5.0, 6.0], [7.0]]

        layer = self.create_test_layer_single_field(
            QMetaType.Type.QVariantList, items, subtype=QMetaType.Type.Double
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.float64())})

    def test_type_int_list(self):
        items = [[1, 2], [3], None, [4, 5, 6], [7]]

        layer = self.create_test_layer_single_field(
            QMetaType.Type.QVariantList, items, subtype=QMetaType.Type.Int
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.int32())})


if __name__ == "__main__":
    unittest.main()
