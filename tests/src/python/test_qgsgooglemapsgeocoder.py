"""QGIS Unit tests for QgsGeocoderLocatorFilter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "02/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import tempfile

from qgis.PyQt.QtCore import QCoreApplication, QUrl
from qgis.core import (
    QgsCoordinateTransformContext,
    QgsGeocoderContext,
    QgsGoogleMapsGeocoder,
    QgsRectangle,
    QgsSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsGeocoderLocatorFilter(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestQgsGeocoderLocatorFilter.com")
        QCoreApplication.setApplicationName("TestQgsGeocoderLocatorFilter")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")
        cls.endpoint = "http://" + cls.basetestpath + "/fake_qgis_http_endpoint"

    @classmethod
    def generate_url(cls, params):
        res = cls.endpoint + "_" + params
        res = res.replace("&", "_")
        res = res.replace(" ", "_")
        return "file:" + res.replace("http://", "")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)
        super().tearDownClass()

    def test_basic(self):
        """
        Basic tests
        """
        geocoder = QgsGoogleMapsGeocoder("my key")
        self.assertEqual(geocoder.apiKey(), "my key")
        geocoder.setApiKey("ggggg")
        self.assertEqual(geocoder.apiKey(), "ggggg")

        self.assertFalse(geocoder.region())
        geocoder.setRegion("xx")
        self.assertEqual(geocoder.region(), "xx")

        self.assertEqual(
            geocoder.requestUrl("20 green st, twaddlingham").toString(),
            "https://maps.googleapis.com/maps/api/geocode/json?region=xx&sensor=false&address=20 green st, twaddlingham&key=ggggg",
        )

    def test_url(self):
        geocoder = QgsGoogleMapsGeocoder("my key")
        geocoder.setEndpoint(self.endpoint)

        self.assertEqual(
            geocoder.requestUrl("20 green st, twaddlingham").toString(),
            QUrl(
                self.generate_url(
                    "sensor=false&address=20 green st, twaddlingham&key=my key"
                )
            ).toString(),
        )
        self.assertEqual(
            geocoder.requestUrl(
                "20 green st, twaddlingham", QgsRectangle(3, 5, 6, 8)
            ).toString(),
            QUrl(
                self.generate_url(
                    "bounds=5,3|8,5&sensor=false&address=20 green st, twaddlingham&key=my key"
                )
            ).toString(),
        )

        geocoder = QgsGoogleMapsGeocoder("my key", regionBias="au")
        geocoder.setEndpoint(self.endpoint)

        self.assertEqual(
            geocoder.requestUrl("20 green st, twaddlingham").toString(),
            QUrl(
                self.generate_url(
                    "region=au&sensor=false&address=20 green st, twaddlingham&key=my key"
                )
            ).toString(),
        )
        self.assertEqual(
            geocoder.requestUrl(
                "20 green st, twaddlingham", QgsRectangle(3, 5, 6, 8)
            ).toString(),
            QUrl(
                self.generate_url(
                    "bounds=5,3|8,5&region=au&sensor=false&address=20 green st, twaddlingham&key=my key"
                )
            ).toString(),
        )

    def test_json_to_result(self):
        geocoder = QgsGoogleMapsGeocoder("my key")
        geocoder.setEndpoint(self.endpoint)

        json = {
            "address_components": [
                {"long_name": "1600", "short_name": "1600", "types": ["street_number"]},
                {
                    "long_name": "Amphitheatre Pkwy",
                    "short_name": "Amphitheatre Pkwy",
                    "types": ["route"],
                },
                {
                    "long_name": "Mountain View",
                    "short_name": "Mountain View",
                    "types": ["locality", "political"],
                },
                {
                    "long_name": "Santa Clara County",
                    "short_name": "Santa Clara County",
                    "types": ["administrative_area_level_2", "political"],
                },
                {
                    "long_name": "California",
                    "short_name": "CA",
                    "types": ["administrative_area_level_1", "political"],
                },
                {
                    "long_name": "United States",
                    "short_name": "US",
                    "types": ["country", "political"],
                },
                {"long_name": "94043", "short_name": "94043", "types": ["postal_code"]},
            ],
            "formatted_address": "Toledo, OH, USA",
            "geometry": {
                "location": {"lat": 41.6639383, "lng": -83.55521200000001},
                "location_type": "APPROXIMATE",
                "viewport": {
                    "northeast": {"lat": 42.1, "lng": -87.7},
                    "southwest": {"lat": 42.0, "lng": -87.7},
                },
            },
            "place_id": "ChIJeU4e_C2HO4gRRcM6RZ_IPHw",
            "types": ["locality", "political"],
        }

        res = geocoder.jsonToResult(json)
        self.assertEqual(res.identifier(), "Toledo, OH, USA")
        self.assertEqual(res.geometry().asWkt(1), "Point (-83.6 41.7)")
        self.assertEqual(
            res.additionalAttributes(),
            {
                "administrative_area_level_1": "California",
                "administrative_area_level_2": "Santa Clara County",
                "country": "United States",
                "formatted_address": "Toledo, OH, USA",
                "locality": "Mountain View",
                "location_type": "APPROXIMATE",
                "place_id": "ChIJeU4e_C2HO4gRRcM6RZ_IPHw",
                "postal_code": "94043",
                "route": "Amphitheatre Pkwy",
                "street_number": "1600",
            },
        )
        self.assertEqual(res.viewport(), QgsRectangle(-87.7, 42, -87.7, 42.1))
        self.assertEqual(res.group(), "California")

    def test_geocode(self):
        geocoder = QgsGoogleMapsGeocoder("my key")
        geocoder.setEndpoint(self.endpoint)

        with open(
            geocoder.requestUrl("20 green st, twaddlingham")
            .toString()
            .replace("file://", ""),
            "wb",
        ) as f:
            f.write(
                b"""
        {
   "results" : [
      {
         "address_components" : [
            {
               "long_name" : "1600",
               "short_name" : "1600",
               "types" : [ "street_number" ]
            },
            {
               "long_name" : "Amphitheatre Pkwy",
               "short_name" : "Amphitheatre Pkwy",
               "types" : [ "route" ]
            },
            {
               "long_name" : "Mountain View",
               "short_name" : "Mountain View",
               "types" : [ "locality", "political" ]
            },
            {
               "long_name" : "Santa Clara County",
               "short_name" : "Santa Clara County",
               "types" : [ "administrative_area_level_2", "political" ]
            },
            {
               "long_name" : "California",
               "short_name" : "CA",
               "types" : [ "administrative_area_level_1", "political" ]
            },
            {
               "long_name" : "United States",
               "short_name" : "US",
               "types" : [ "country", "political" ]
            },
            {
               "long_name" : "94043",
               "short_name" : "94043",
               "types" : [ "postal_code" ]
            }
         ],
         "formatted_address" : "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
         "geometry" : {
            "location" : {
               "lat" : 37.4224764,
               "lng" : -122.0842499
            },
            "location_type" : "ROOFTOP",
            "viewport" : {
               "northeast" : {
                  "lat" : 37.4238253802915,
                  "lng" : -122.0829009197085
               },
               "southwest" : {
                  "lat" : 37.4211274197085,
                  "lng" : -122.0855988802915
               }
            }
         },
         "place_id" : "ChIJ2eUgeAK6j4ARbn5u_wAGqWA",
         "plus_code": {
            "compound_code": "CWC8+W5 Mountain View, California, United States",
            "global_code": "849VCWC8+W5"
         },
         "types" : [ "street_address" ]
      }
   ],
   "status" : "OK"
}
        """
            )

        context = QgsGeocoderContext(QgsCoordinateTransformContext())
        results = geocoder.geocodeString("20 green st, twaddlingham", context)
        self.assertEqual(len(results), 1)

        self.assertEqual(
            results[0].identifier(),
            "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
        )
        self.assertEqual(results[0].geometry().asWkt(1), "Point (-122.1 37.4)")
        self.assertEqual(
            results[0].additionalAttributes(),
            {
                "administrative_area_level_1": "California",
                "administrative_area_level_2": "Santa Clara County",
                "country": "United States",
                "formatted_address": "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA",
                "locality": "Mountain View",
                "location_type": "ROOFTOP",
                "place_id": "ChIJ2eUgeAK6j4ARbn5u_wAGqWA",
                "postal_code": "94043",
                "route": "Amphitheatre Pkwy",
                "street_number": "1600",
            },
        )


if __name__ == "__main__":
    unittest.main()
