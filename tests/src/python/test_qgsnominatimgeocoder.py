# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNominatimGeocoder.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '14/12/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import tempfile

import qgis  # NOQA
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QUrl
)
from qgis.core import (
    QgsSettings,
    QgsRectangle,
    QgsNominatimGeocoder,
    QgsGeocoderContext,
    QgsCoordinateTransformContext
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsNominatimGeocoder(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestQgsGeocoderLocatorFilter.com")
        QCoreApplication.setApplicationName("TestQgsGeocoderLocatorFilter")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        cls.endpoint = 'http://' + cls.basetestpath + '/fake_qgis_http_endpoint'

    @classmethod
    def generate_url(cls, params):
        res = cls.endpoint + '_' + params
        res = res.replace('&', '_')
        res = res.replace(' ', '_')
        return 'file:' + res.replace('http://', '')

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)

    def test_basic(self):
        """
        Basic tests
        """
        geocoder = QgsNominatimGeocoder()

        self.assertFalse(geocoder.countryCodes())
        geocoder.setCountryCodes('ca,km')
        self.assertEqual(geocoder.countryCodes(), 'ca,km')
        self.assertEqual(geocoder.requestUrl('20 green st, twaddlingham').toString(), 'https://nominatim.qgis.org/search?format=json&addressdetails=1&countrycodes=ca,km&q=20 green st, twaddlingham')

        geocoder.setEndpoint('https://my.server/search')
        self.assertEqual(geocoder.requestUrl('20 green st, twaddlingham').toString(), 'https://my.server/search?format=json&addressdetails=1&countrycodes=ca,km&q=20 green st, twaddlingham')

    def test_url(self):
        geocoder = QgsNominatimGeocoder('')
        self.assertEqual(geocoder.requestUrl('20 green st, twaddlingham', QgsRectangle(3, 5, 6, 8)).toString(),
                         'https://nominatim.qgis.org/search?format=json&addressdetails=1&viewbox=3,5,6,8&q=20 green st, twaddlingham')

        self.assertEqual(geocoder.requestUrl('20 green st, twaddlingham', QgsRectangle(float('-inf'), float('-inf'), float('inf'), float('inf'))).toString(),
                         'https://nominatim.qgis.org/search?format=json&addressdetails=1&q=20 green st, twaddlingham')

        geocoder = QgsNominatimGeocoder(countryCodes='ca,km', endpoint='https://my.server/search')
        self.assertEqual(geocoder.requestUrl('20 green st, twaddlingham', QgsRectangle(3, 5, 6, 8)).toString(),
                         'https://my.server/search?format=json&addressdetails=1&viewbox=3,5,6,8&countrycodes=ca,km&q=20 green st, twaddlingham')

    def test_json_to_result(self):
        geocoder = QgsNominatimGeocoder()

        json = {
            "place_id": 157298780,
            "licence": "Data © OpenStreetMap contributors, ODbL 1.0. https://osm.org/copyright",
            "osm_type": "way",
            "osm_id": 264390219,
            "boundingbox": [
                "46.887",
                "46.889",
                "-71.201",
                "-71.199"
            ],
            "lat": "46.888",
            "lon": "-71.200",
            "display_name": "École Primaire La Ribambelle, 500, Rue Anick, Saint-Michel, Beauport, Quebec City, Québec (Agglomération), Capitale-Nationale, Quebec, G1C 4X5, Canada",
            "class": "amenity",
            "type": "school",
            "importance": 0.201,
            "icon": "https://nominatim.qgis.org/ui/mapicons//education_school.p.20.png",
            "address": {
                "amenity": "École Primaire La Ribambelle",
                "house_number": "500",
                "road": "Rue Anick",
                "suburb": "Saint-Michel",
                "city_district": "Beauport",
                "city": "Quebec City",
                "county": "Québec (Agglomération)",
                "region": "Capitale-Nationale",
                "state": "Quebec",
                "postcode": "G1C 4X5",
                "country": "Canada",
                "country_code": "ca"
            }
        }

        res = geocoder.jsonToResult(json)
        self.assertEqual(res.identifier(), 'École Primaire La Ribambelle, 500, Rue Anick, Saint-Michel, Beauport, Quebec City, Québec (Agglomération), Capitale-Nationale, Quebec, G1C 4X5, Canada')
        self.assertEqual(res.geometry().asWkt(1), 'Point (-71.2 46.9)')
        self.assertEqual(res.additionalAttributes(), {'state': 'Quebec',
                                                      'city': 'Quebec City',
                                                      'country': 'Canada',
                                                      'display_name': 'École Primaire La Ribambelle, 500, Rue Anick, Saint-Michel, Beauport, Quebec City, Québec (Agglomération), Capitale-Nationale, Quebec, G1C 4X5, Canada',
                                                      'city_district': 'Beauport',
                                                      'place_id': '157298780', 'postcode': 'G1C 4X5',
                                                      'road': 'Rue Anick', 'osm_type': 'way',
                                                      'type': 'school', 'class': 'amenity'})
        self.assertEqual(res.viewport(), QgsRectangle(-71.201, 46.887, -71.199, 46.889))
        self.assertEqual(res.group(), 'Quebec')


if __name__ == '__main__':
    unittest.main()
