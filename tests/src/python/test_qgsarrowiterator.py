"""QGIS Unit tests for QgsArrowIterator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2025 by Dewey Dunnington"
__date__ = "04/11/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

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
from qgis.testing import start_app, QgisTestCase


# Crashes pytest for me locally
# start_app()


# if field_type_parameter == 0:  # Integer
#     field_type = QMetaType.Type.Int
# elif field_type_parameter == 1:  # Float
#     field_type = QMetaType.Type.Double
# elif field_type_parameter == 2:  # String
#     field_type = QMetaType.Type.QString
# elif field_type_parameter == 3:  # Boolean
#     field_type = QMetaType.Type.Bool
# elif field_type_parameter == 4:  # Date
#     field_type = QMetaType.Type.QDate
# elif field_type_parameter == 5:  # Time
#     field_type = QMetaType.Type.QTime
# elif field_type_parameter == 6:  # DateTime
#     field_type = QMetaType.Type.QDateTime
# elif field_type_parameter == 7:  # Binary
#     field_type = QMetaType.Type.QByteArray
# elif field_type_parameter == 8:  # StringList
#     field_type = QMetaType.Type.QStringList
#     field_sub_type = QMetaType.Type.QString
# elif field_type_parameter == 9:  # IntegerList
#     field_type = QMetaType.Type.QVariantList
#     field_sub_type = QMetaType.Type.Int
# elif field_type_parameter == 10:  # DoubleList
#     field_type = QMetaType.Type.QVariantList
#     field_sub_type = QMetaType.Type.Double


class TestQgsArrowIterator(QgisTestCase):

    def create_test_layer(self, crs):
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

    def test_infer_schema(self):
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        layer = self.create_test_layer(crs)

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
        layer = self.create_test_layer(QgsCoordinateReferenceSystem())
        schema = QgsArrowIterator.inferSchema(layer)
        pa_schema = pa.Schema._import_from_c(schema.cSchemaAddress())
        geometry_field_metadata = pa_schema.field("geometry").metadata
        geoarrow_metadata = json.loads(
            geometry_field_metadata[b"ARROW:extension:metadata"]
        )
        assert geoarrow_metadata == {}

    def test_iterate(self):
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        layer = self.create_test_layer(crs)
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


if __name__ == "__main__":
    unittest.main()
