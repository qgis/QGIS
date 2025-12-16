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

try:
    import geopandas
except ImportError:
    geopandas = None

try:
    import pyarrow as pa
except ImportError:
    pa = None

try:
    import shapely
except ImportError:
    shapely = None

from qgis.core import (
    QgsArrowIterator,
    QgsArrowSchema,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsMemoryProviderUtils,
    QgsPointXY,
    QgsWkbTypes,
    QgsException,
)

from qgis.PyQt.QtCore import (
    QMetaType,
    QByteArray,
    QDate,
    QTime,
    QDateTime,
    QTimeZone,
)
from qgis.testing import QgisTestCase


@unittest.skipIf(geopandas is None, "geopandas is not available")
@unittest.skipIf(pa is None, "pyarrow is not available")
@unittest.skipIf(shapely is None, "shapely is not available")
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
        iterator.setSchema(schema)

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

        batch_invalid = iterator.nextFeatures(4)
        assert batch_invalid.isValid() is False

    def test_layer_to_stream(self):
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        layer = self.create_test_layer_with_geometry(crs)

        schema = QgsArrowIterator.inferSchema(layer)
        iterator = QgsArrowIterator(layer.getFeatures())
        iterator.setSchema(schema)

        stream = iterator.toArrayStream()
        reader = pa.RecordBatchReader._import_from_c(stream.cArrayStreamAddress())
        df = geopandas.GeoDataFrame.from_arrow(reader)

        assert list(df.id) == list(range(1, 11))
        assert df.crs == "EPSG:4326"
        assert list(df.geometry.to_wkt()) == [
            "POINT (0 20)",
            "POINT (1 21)",
            "POINT (2 22)",
            "POINT (3 23)",
            "POINT (4 24)",
            "POINT (5 25)",
            "POINT (6 26)",
            "POINT (7 27)",
            "POINT (8 28)",
            "POINT (9 29)",
        ]

    def test_layer_to_stream_error(self):
        crs = QgsCoordinateReferenceSystem()
        layer = self.create_test_layer_with_geometry(crs)

        # With an incompatible schema, this should throw in get_next()
        pa_schema = pa.schema({"name": pa.union([], "dense")})
        schema = QgsArrowSchema()
        pa_schema._export_to_c(schema.cSchemaAddress())

        iterator = QgsArrowIterator(layer.getFeatures())
        iterator.setSchema(schema)
        stream = iterator.toArrayStream()
        reader = pa.RecordBatchReader._import_from_c(stream.cArrayStreamAddress())

        with self.assertRaises(pa.lib.ArrowInvalid) as ctx:
            reader.read_next_batch() is stream
        assert (
            str(ctx.exception)
            == "Can't convert variant of type 'QString' to Arrow type 'dense_union'"
        )

    def test_type_int(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Int, [1, 2, None, 4, 5]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.int32()})

        for pa_type in [
            pa.int8(),
            pa.int16(),
            pa.int32(),
            pa.int64(),
            pa.uint8(),
            pa.uint16(),
            pa.uint32(),
            pa.uint64(),
        ]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch(
                {"f": [1, 2, None, 4, 5]}, schema=pa_schema
            )

    def test_type_double(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Double, [1.0, 2.0, None, 4.0, 5.0]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.float64()})

        for pa_type in [pa.float32(), pa.float64()]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch(
                {"f": [1.0, 2.0, None, 4.0, 5.0]}, schema=pa_schema
            )

    def test_type_string(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.QString, ["a", "b", None, "d", "e"]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.string()})

        for pa_type in [pa.string(), pa.large_string(), pa.string_view()]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch(
                {"f": ["a", "b", None, "d", "e"]}, schema=pa_schema
            )

    def test_type_binary(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.QByteArray,
            [
                QByteArray(b"a"),
                QByteArray(b"b"),
                None,
                QByteArray(b"d"),
                QByteArray(b"e"),
            ],
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.binary()})

        for pa_type in [pa.binary(), pa.large_binary(), pa.binary_view(), pa.binary(1)]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch(
                {"f": [b"a", b"b", None, b"d", b"e"]}, schema=pa_schema
            )

    def test_type_boolean(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Bool, [True, False, None, True, False]
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.bool_()})

        for pa_type in [pa.bool_()]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch(
                {"f": [True, False, None, True, False]}, schema=pa_schema
            )

    def test_type_date(self):
        dates = [datetime.date(2020, 1, i) for i in range(1, 6)]
        dates[2] = None

        q_dates = [QDate(2020, 1, i) for i in range(1, 6)]
        q_dates[2] = None

        layer = self.create_test_layer_single_field(QMetaType.Type.QDate, q_dates)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.date32()})

        for pa_type in [pa.date32(), pa.date64()]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": dates}, schema=pa_schema)

    def test_type_time(self):
        times = [datetime.time(17, 0, i) for i in range(5)]
        times[2] = None

        q_times = [QTime(17, 0, i) for i in range(5)]
        q_times[2] = None

        layer = self.create_test_layer_single_field(QMetaType.Type.QTime, q_times)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.time32("ms")})

        for pa_type in [
            pa.time32("s"),
            pa.time32("ms"),
            pa.time64("us"),
            pa.time64("ns"),
        ]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": times}, schema=pa_schema)

    def test_type_datetime(self):
        datetimes = [datetime.datetime(2020, 1, 1, 17, 0, i) for i in range(5)]
        datetimes[2] = None

        q_datetimes = [
            QDateTime(QDate(2020, 1, 1), QTime(17, 0, i), QTimeZone.utc())
            for i in range(5)
        ]
        q_datetimes[2] = None

        layer = self.create_test_layer_single_field(
            QMetaType.Type.QDateTime, q_datetimes
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.timestamp("ms", tz="UTC")})

        for pa_type in [
            pa.timestamp("s", "UTC"),
            pa.timestamp("ms", "UTC"),
            pa.timestamp("us", "UTC"),
            pa.timestamp("ns", "UTC"),
            pa.timestamp("ms", "America/Halifax"),
        ]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": datetimes}, schema=pa_schema)

    def test_type_string_list(self):
        items = [["a", "b"], ["c", "d"], None, ["e", "f"], ["g", "h"]]

        layer = self.create_test_layer_single_field(QMetaType.Type.QStringList, items)
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.string())})

        for pa_type in [pa.list_(pa.string())]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": items}, schema=pa_schema)

    def test_type_double_list(self):
        items = [[1.0, 2.0], [3.0, 4.0], None, [5.0, 6.0], [7.0, 8.0]]

        layer = self.create_test_layer_single_field(
            QMetaType.Type.QVariantList, items, subtype=QMetaType.Type.Double
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.float64())})

        for pa_type in [
            pa.list_(pa.float64()),
            pa.list_(pa.float64(), 2),
            pa.large_list(pa.float64()),
            pa.list_(pa.float32()),
        ]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": items}, schema=pa_schema)

    def test_type_int_list(self):
        items = [[1, 2], [3, 4], None, [5, 6], [7, 8]]

        layer = self.create_test_layer_single_field(
            QMetaType.Type.QVariantList, items, subtype=QMetaType.Type.Int
        )
        inferred = QgsArrowIterator.inferSchema(layer)
        pa_inferred = pa.Schema._import_from_c(inferred.cSchemaAddress())
        assert pa_inferred == pa.schema({"f": pa.list_(pa.int32())})

        for pa_type in [
            pa.list_(pa.int32()),
            pa.list_(pa.int32(), 2),
            pa.large_list(pa.int32()),
            pa.list_(pa.int16()),
        ]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)
            batch = iterator.nextFeatures(5)
            pa_batch = pa.RecordBatch._import_from_c(batch.cArrayAddress(), pa_schema)
            assert pa_batch == pa.record_batch({"f": items}, schema=pa_schema)

    def test_type_unsupported_conversion(self):
        layer = self.create_test_layer_single_field(
            QMetaType.Type.Int, [1, 2, None, 4, 5]
        )

        for pa_type in [pa.list_(pa.time32("s")), pa.month_day_nano_interval()]:
            pa_schema = pa.schema({"f": pa_type})
            schema = QgsArrowSchema()
            pa_schema._export_to_c(schema.cSchemaAddress())

            iterator = QgsArrowIterator(layer.getFeatures())
            iterator.setSchema(schema)

            with self.assertRaisesRegex(QgsException, "Can't convert"):
                iterator.nextFeatures(5)


if __name__ == "__main__":
    unittest.main()
