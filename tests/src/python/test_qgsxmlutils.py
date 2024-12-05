"""QGIS Unit tests for QgsXmlUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "18/11/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QTime, QVariant
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsFeatureRequest,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingOutputLayerDefinition,
    QgsProperty,
    QgsRemappingSinkDefinition,
    QgsWkbTypes,
    QgsXmlUtils,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsXmlUtils(QgisTestCase):

    def test_invalid(self):
        """
        Test that invalid attributes are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(None, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertIsNone(prop2)

    def test_integer(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {"a": 1, "b": 2, "c": 3, "d": -1}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(my_properties, prop2)

    def test_long(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        # not sure if this actually does map to a long?
        my_properties = {"a": 9223372036854775808}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(my_properties, prop2)

    def test_string(self):
        """
        Test that strings are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {"a": "a", "b": "b", "c": "something_else", "empty": ""}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(
            prop2, {"a": "a", "b": "b", "c": "something_else", "empty": None}
        )

    def test_double(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {"a": 0.27, "b": 1.0, "c": 5}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_boolean(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {"a": True, "b": False}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_list(self):
        """
        Test that lists are correctly loaded and written
        """
        doc = QDomDocument("properties")
        my_properties = [1, 4, "a", "test", 7.9]
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_complex(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {"boolean": True, "integer": False, "map": {"a": 1}}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_property(self):
        """
        Test that QgsProperty values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        prop = QgsProperty.fromValue(1001)
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

        prop = QgsProperty.fromExpression("1+2=5")
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

        prop = QgsProperty.fromField("oid")
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

    def test_crs(self):
        """
        Test that QgsCoordinateReferenceSystem values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        crs = QgsCoordinateReferenceSystem("epsg:3111")
        elem = QgsXmlUtils.writeVariant(crs, doc)

        crs2 = QgsXmlUtils.readVariant(elem)
        self.assertTrue(crs2.isValid())
        self.assertEqual(crs2.authid(), "EPSG:3111")

        crs = QgsCoordinateReferenceSystem()
        elem = QgsXmlUtils.writeVariant(crs, doc)

        crs2 = QgsXmlUtils.readVariant(elem)
        self.assertIsNone(crs2)

    def test_geom(self):
        """
        Test that QgsGeometry values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        g = QgsGeometry.fromWkt("Point(3 4)")
        elem = QgsXmlUtils.writeVariant(g, doc)

        g2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(g2.asWkt(), "Point (3 4)")

    def test_color(self):
        """
        Test that QColor values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(QColor(100, 200, 210), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QColor(100, 200, 210))
        elem = QgsXmlUtils.writeVariant(QColor(100, 200, 210, 50), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QColor(100, 200, 210, 50))
        elem = QgsXmlUtils.writeVariant(QColor(), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertIsNone(c)

    def test_datetime(self):
        """
        Test that QDateTime values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(
            QDateTime(QDate(2019, 5, 7), QTime(12, 11, 10)), doc
        )
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QDateTime(QDate(2019, 5, 7), QTime(12, 11, 10)))
        elem = QgsXmlUtils.writeVariant(QDateTime(), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, NULL)

    def test_date(self):
        """
        Test that QDate values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(QDate(2019, 5, 7), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QDate(2019, 5, 7))
        elem = QgsXmlUtils.writeVariant(QDate(), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, NULL)

    def test_time(self):
        """
        Test that QTime values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(QTime(12, 11, 10), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QTime(12, 11, 10))
        elem = QgsXmlUtils.writeVariant(QTime(), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, NULL)

    def test_feature_source_definition(self):
        """
        Test that QgsProcessingFeatureSourceDefinition values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        definition = QgsProcessingFeatureSourceDefinition(
            QgsProperty.fromValue("my source")
        )
        definition.selectedFeaturesOnly = True
        definition.featureLimit = 27
        definition.flags = (
            QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature
        )
        definition.geometryCheck = (
            QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
        )

        elem = QgsXmlUtils.writeVariant(definition, doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c.source.staticValue(), "my source")
        self.assertTrue(c.selectedFeaturesOnly)
        self.assertEqual(c.featureLimit, 27)
        self.assertEqual(
            c.flags,
            QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature,
        )
        self.assertEqual(
            c.geometryCheck, QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid
        )

    def test_output_layer_definition(self):
        """
        Test that QgsProcessingOutputLayerDefinition values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        definition = QgsProcessingOutputLayerDefinition(
            QgsProperty.fromValue("my sink")
        )
        definition.createOptions = {"opt": 1, "opt2": 2}

        elem = QgsXmlUtils.writeVariant(definition, doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c.sink.staticValue(), "my sink")
        self.assertEqual(c.createOptions, {"opt": 1, "opt2": 2})

    def testRemappingDefinition(self):
        fields = QgsFields()
        fields.append(QgsField("fldtxt", QVariant.String))
        fields.append(QgsField("fldint", QVariant.Int))
        fields.append(QgsField("fldtxt2", QVariant.String))

        mapping_def = QgsRemappingSinkDefinition()
        mapping_def.setDestinationWkbType(QgsWkbTypes.Type.Point)
        mapping_def.setSourceCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        mapping_def.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapping_def.setDestinationFields(fields)
        mapping_def.addMappedField("fldtxt2", QgsProperty.fromField("fld1"))
        mapping_def.addMappedField(
            "fldint", QgsProperty.fromExpression("@myval * fldint")
        )

        doc = QDomDocument("properties")
        elem = QgsXmlUtils.writeVariant(mapping_def, doc)
        c = QgsXmlUtils.readVariant(elem)

        self.assertEqual(c.destinationWkbType(), QgsWkbTypes.Type.Point)
        self.assertEqual(c.sourceCrs().authid(), "EPSG:4326")
        self.assertEqual(c.destinationCrs().authid(), "EPSG:3857")
        self.assertEqual(c.destinationFields()[0].name(), "fldtxt")
        self.assertEqual(c.destinationFields()[1].name(), "fldint")
        self.assertEqual(c.fieldMap()["fldtxt2"].field(), "fld1")
        self.assertEqual(c.fieldMap()["fldint"].expressionString(), "@myval * fldint")


if __name__ == "__main__":
    unittest.main()
