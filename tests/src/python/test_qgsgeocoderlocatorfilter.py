"""QGIS Unit tests for QgsGeocoderLocatorFilter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "02/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFeedback,
    QgsGeocoderInterface,
    QgsGeocoderResult,
    QgsGeometry,
    QgsLocatorContext,
    QgsPointXY,
    QgsRectangle,
    QgsWkbTypes,
)
from qgis.gui import QgsGeocoderLocatorFilter, QgsMapCanvas
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


class TestQgsGeocoderLocatorFilter(QgisTestCase):

    def test_geocode(self):
        geocoder = TestGeocoder()
        canvas = QgsMapCanvas()
        filter = QgsGeocoderLocatorFilter(
            "testgeocoder",
            "my geocoder",
            "pref",
            geocoder,
            canvas,
            QgsRectangle(-1, -1, 1, 1),
        )

        self.assertEqual(filter.name(), "testgeocoder")
        self.assertEqual(filter.displayName(), "my geocoder")
        self.assertEqual(filter.prefix(), "pref")
        self.assertEqual(filter.geocoder(), geocoder)
        self.assertEqual(filter.boundingBox(), QgsRectangle(-1, -1, 1, 1))

        spy = QSignalSpy(filter.resultFetched)

        context = QgsLocatorContext()
        feedback = QgsFeedback()

        # no results
        filter.fetchResults("cvxbcvb", context, feedback)
        self.assertEqual(len(spy), 0)

        # one result
        filter.fetchResults("a", context, feedback)
        self.assertEqual(len(spy), 1)
        res = spy[-1][0]
        self.assertEqual(res.displayString, "res 1")
        # some sip weirdness here -- if we directly access the QgsLocatorResult object here then we get segfaults!
        # so instead convert back to QgsGeocoderResult. This makes the test more robust anyway...
        geocode_result = filter.locatorResultToGeocoderResult(res)
        self.assertEqual(geocode_result.identifier(), "res 1")
        self.assertEqual(geocode_result.geometry().asWkt(), "Point (1 2)")
        self.assertEqual(geocode_result.crs().authid(), "EPSG:4326")
        self.assertEqual(geocode_result.additionalAttributes(), {"b": 123, "c": "xyz"})
        self.assertTrue(geocode_result.viewport().isNull())
        self.assertFalse(geocode_result.description())
        self.assertFalse(geocode_result.group())

        # two possible results
        filter.fetchResults("b", context, feedback)
        self.assertEqual(len(spy), 3)
        res1 = spy[-2][0]
        res2 = spy[-1][0]
        self.assertEqual(res1.displayString, "res 1")
        geocode_result = filter.locatorResultToGeocoderResult(res1)
        self.assertEqual(geocode_result.identifier(), "res 1")
        self.assertEqual(geocode_result.geometry().asWkt(), "Point (11 12)")
        self.assertEqual(geocode_result.crs().authid(), "EPSG:4326")
        self.assertEqual(geocode_result.additionalAttributes(), {"b": 123, "c": "xyz"})
        self.assertTrue(geocode_result.viewport().isNull())
        self.assertEqual(geocode_result.description(), "desc")
        self.assertEqual(geocode_result.group(), "group")
        self.assertEqual(res2.displayString, "res 2")
        geocode_result = filter.locatorResultToGeocoderResult(res2)
        self.assertEqual(geocode_result.identifier(), "res 2")
        self.assertEqual(geocode_result.geometry().asWkt(), "Point (13 14)")
        self.assertEqual(geocode_result.crs().authid(), "EPSG:3857")
        self.assertEqual(geocode_result.additionalAttributes(), {"d": 456})
        self.assertEqual(geocode_result.viewport(), QgsRectangle(1, 2, 3, 4))
        self.assertFalse(geocode_result.description())
        self.assertFalse(geocode_result.group())


if __name__ == "__main__":
    unittest.main()
