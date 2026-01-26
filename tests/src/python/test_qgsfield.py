"""QGIS Unit tests for QgsField.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/08/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.core import Qgis, QgsField, QgsCoordinateReferenceSystem
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsField(QgisTestCase):

    def test_metadata(self):
        field = QgsField()
        self.assertFalse(field.metadata())

        # set custom metadata
        field.setMetadata(Qgis.FieldMetadataProperty.CustomProperty + 2, "test")
        self.assertEqual(
            field.metadata(Qgis.FieldMetadataProperty.CustomProperty + 2), "test"
        )

        # set standard metadata
        field.setMetadata(
            Qgis.FieldMetadataProperty.GeometryWkbType, Qgis.WkbType.LineStringZ
        )
        self.assertEqual(
            field.metadata(Qgis.FieldMetadataProperty.GeometryWkbType),
            Qgis.WkbType.LineStringZ,
        )

        self.assertEqual(
            field.metadata(),
            {
                Qgis.FieldMetadataProperty.GeometryWkbType: Qgis.WkbType.LineStringZ,
                Qgis.FieldMetadataProperty.CustomProperty + 2: "test",
            },
        )

        field.setMetadata(
            {
                Qgis.FieldMetadataProperty.GeometryCrs: QgsCoordinateReferenceSystem(
                    "EPSG:3111"
                ),
                Qgis.FieldMetadataProperty.CustomProperty + 3: "test2",
            }
        )

        self.assertEqual(
            field.metadata(),
            {
                Qgis.FieldMetadataProperty.GeometryCrs: QgsCoordinateReferenceSystem(
                    "EPSG:3111"
                ),
                Qgis.FieldMetadataProperty.CustomProperty + 3: "test2",
            },
        )

    def test_convert_compatible_exceptions(self):
        field = QgsField("test", QVariant.Int)

        self.assertTrue(field.convertCompatible(5))
        with self.assertRaises(ValueError):
            field.convertCompatible("abc")

        field = QgsField("test", QVariant.DateTime)

        with self.assertRaises(ValueError):
            field.convertCompatible(5)
        with self.assertRaises(ValueError):
            field.convertCompatible("abc")


if __name__ == "__main__":
    unittest.main()
