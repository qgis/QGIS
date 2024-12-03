"""QGIS Unit tests for QgsBatchGeocodeAlgorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "02/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.analysis import QgsBatchGeocodeAlgorithm
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeocoderInterface,
    QgsGeocoderResult,
    QgsGeometry,
    QgsPointXY,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsRectangle,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestGeocoder(QgsGeocoderInterface):

    def flags(self):
        return QgsGeocoderInterface.Flag.GeocodesStrings

    def wkbType(self):
        return QgsWkbTypes.Type.Point

    def geocodeString(self, string, context, feedback):
        if string == "a":
            result = QgsGeocoderResult(
                "res 1",
                QgsGeometry.fromPointXY(QgsPointXY(1, 2)),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
            result.setAdditionalAttributes({"b": 123, "c": "xyz"})
            return [result]

        if string == "b":
            result1 = QgsGeocoderResult(
                "res 1",
                QgsGeometry.fromPointXY(QgsPointXY(11, 12)),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
            result1.setAdditionalAttributes({"b": 123, "c": "xyz"})
            result1.setDescription("desc")
            result1.setGroup("group")
            result2 = QgsGeocoderResult(
                "res 2",
                QgsGeometry.fromPointXY(QgsPointXY(13, 14)),
                QgsCoordinateReferenceSystem("EPSG:3857"),
            )
            result2.setAdditionalAttributes({"d": 456})
            result2.setViewport(QgsRectangle(1, 2, 3, 4))
            return [result1, result2]

        return []


class TestGeocoderExtraFields(QgsGeocoderInterface):

    def flags(self):
        return QgsGeocoderInterface.Flag.GeocodesStrings

    def wkbType(self):
        return QgsWkbTypes.Type.Point

    def appendedFields(self):
        fields = QgsFields()
        fields.append(QgsField("parsed", QVariant.String))
        fields.append(QgsField("accuracy", QVariant.Int))
        return fields

    def geocodeString(self, string, context, feedback):
        if string == "a":
            result = QgsGeocoderResult(
                "res 1",
                QgsGeometry.fromPointXY(QgsPointXY(1, 2)),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
            result.setAdditionalAttributes({"accuracy": 123, "parsed": "xyz"})
            return [result]

        if string == "b":
            result1 = QgsGeocoderResult(
                "res 1",
                QgsGeometry.fromPointXY(QgsPointXY(11, 12)),
                QgsCoordinateReferenceSystem("EPSG:4326"),
            )
            result1.setAdditionalAttributes({"accuracy": 456, "parsed": "xyz2"})
            result1.setDescription("desc")
            result1.setGroup("group")
            result2 = QgsGeocoderResult(
                "res 2",
                QgsGeometry.fromPointXY(QgsPointXY(13, 14)),
                QgsCoordinateReferenceSystem("EPSG:3857"),
            )
            result2.setAdditionalAttributes({"d": 456})
            result2.setViewport(QgsRectangle(1, 2, 3, 4))
            return [result1, result2]

        return []


class TestGeocoderAlgorithm(QgsBatchGeocodeAlgorithm):

    def __init__(self, geocoder):
        super().__init__(geocoder)
        self.geocoder = geocoder

    def displayName(self):
        return "Test Geocoder"

    def name(self):
        return "test_geocoder_alg"

    def createInstance(self):
        return TestGeocoderAlgorithm(self.geocoder)


class TestQgsBatchGeocodeAlgorithm(QgisTestCase):

    def test_algorithm(self):
        geocoder = TestGeocoder()

        alg = TestGeocoderAlgorithm(geocoder)
        alg.initParameters()

        fields = QgsFields()
        fields.append(QgsField("some_pk", QVariant.Int))
        fields.append(QgsField("address", QVariant.String))

        f = QgsFeature(fields)
        f.initAttributes(2)
        f["some_pk"] = 17

        params = {"FIELD": "address"}

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        self.assertTrue(alg.prepareAlgorithm(params, context, feedback))

        output_fields = alg.outputFields(fields)
        self.assertEqual([f.name() for f in output_fields], ["some_pk", "address"])
        self.assertEqual(
            [f.type() for f in output_fields], [QVariant.Int, QVariant.String]
        )

        # empty field
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, NULL])

        # no result
        f["address"] = "c"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, "c"])

        f["address"] = "a"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (1 2)")
        self.assertEqual(res[0].attributes(), [17, "a"])

        f["address"] = "b"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (11 12)")
        self.assertEqual(res[0].attributes(), [17, "b"])

    def testAppendedFields(self):
        geocoder = TestGeocoderExtraFields()

        alg = TestGeocoderAlgorithm(geocoder)
        alg.initParameters()

        fields = QgsFields()
        fields.append(QgsField("some_pk", QVariant.Int))
        fields.append(QgsField("address", QVariant.String))

        output_fields = alg.outputFields(fields)
        self.assertEqual(
            [f.name() for f in output_fields],
            ["some_pk", "address", "parsed", "accuracy"],
        )
        self.assertEqual(
            [f.type() for f in output_fields],
            [QVariant.Int, QVariant.String, QVariant.String, QVariant.Int],
        )

        f = QgsFeature(fields)
        f.initAttributes(2)
        f["some_pk"] = 17

        params = {"FIELD": "address"}

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        self.assertTrue(alg.prepareAlgorithm(params, context, feedback))

        # empty field
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, NULL, NULL, NULL])

        # no result
        f["address"] = "c"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, "c", NULL, NULL])

        f["address"] = "a"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (1 2)")
        self.assertEqual(res[0].attributes(), [17, "a", "xyz", 123])

        f["address"] = "b"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (11 12)")
        self.assertEqual(res[0].attributes(), [17, "b", "xyz2", 456])

    def testInPlace(self):
        geocoder = TestGeocoderExtraFields()

        alg = TestGeocoderAlgorithm(geocoder)
        alg.initParameters({"IN_PLACE": True})

        fields = QgsFields()
        fields.append(QgsField("some_pk", QVariant.Int))
        fields.append(QgsField("address", QVariant.String))
        fields.append(QgsField("parsedx", QVariant.String))
        fields.append(QgsField("accuracyx", QVariant.Int))

        output_fields = alg.outputFields(fields)
        self.assertEqual(
            [f.name() for f in output_fields],
            ["some_pk", "address", "parsedx", "accuracyx"],
        )
        self.assertEqual(
            [f.type() for f in output_fields],
            [QVariant.Int, QVariant.String, QVariant.String, QVariant.Int],
        )

        f = QgsFeature(fields)
        f.initAttributes(4)
        f["some_pk"] = 17

        # not storing additional attributes
        params = {"FIELD": "address"}

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        self.assertTrue(alg.prepareAlgorithm(params, context, feedback))

        # empty field
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, NULL, NULL, NULL])

        f["address"] = "a"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (1 2)")
        self.assertEqual(res[0].attributes(), [17, "a", None, None])

        f.clearGeometry()
        f["address"] = NULL

        # storing additional attributes
        params = {"FIELD": "address", "parsed": "parsedx", "accuracy": "accuracyx"}

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        self.assertTrue(alg.prepareAlgorithm(params, context, feedback))

        # empty field
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertTrue(res[0].geometry().isNull())
        self.assertEqual(res[0].attributes(), [17, NULL, NULL, NULL])

        f["address"] = "b"
        res = alg.processFeature(f, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].geometry().asWkt(), "Point (11 12)")
        self.assertEqual(res[0].attributes(), [17, "b", "xyz2", 456])


if __name__ == "__main__":
    unittest.main()
