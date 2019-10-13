# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OAPIF provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2019-10-12'
__copyright__ = 'Copyright 2019, Even Rouault'

import json
import hashlib
import os
import re
import shutil
import tempfile

from qgis.PyQt.QtCore import QCoreApplication, Qt, QObject, QDateTime

from qgis.core import (
    QgsWkbTypes,
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsRectangle,
    QgsPointXY,
    QgsVectorDataProvider,
    QgsFeatureRequest,
    QgsApplication,
    QgsSettings,
    QgsExpression,
    QgsExpressionContextUtils,
    QgsExpressionContext,
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        ret = endpoint + hashlib.md5(x.replace('/', '_').encode()).hexdigest()
        #print('Before: ' + endpoint + x)
        #print('After:  ' + ret)
        return ret
    ret = endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"', '_').replace("'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')
    return ret


ACCEPT_LANDING = 'Accept=application/json'
ACCEPT_API = 'Accept=application/vnd.oai.openapi+json;version=3.0, application/openapi+json;version=3.0, application/json'
ACCEPT_COLLECTION = 'Accept=application/json'
ACCEPT_ITEMS = 'Accept=application/geo+json, application/json'


def create_landing_page_api_collection(endpoint):

    # Landing page
    with open(sanitize(endpoint, '?' + ACCEPT_LANDING), 'wb') as f:
        f.write(json.dumps({
            "links": [
                {"href": "http://" + endpoint + "/api", "rel": "service-desc"},
                {"href": "http://" + endpoint + "/collections", "rel": "data"}
            ]}).encode('UTF-8'))

    # API
    with open(sanitize(endpoint, '/api?' + ACCEPT_API), 'wb') as f:
        f.write(json.dumps({
            "components": {
                "parameters": {
                    "limit": {
                        "schema": {
                            "maximum": 1000,
                            "default": 100
                        }
                    }
                }
            }
        }).encode('UTF-8'))

    # collection
    with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
        f.write(json.dumps({
            "id": "mycollection",
            "title": "my title",
            "description": "my description",
            "extent": {
                "spatial": {
                    "bbox": [
                        [-71.123, 66.33, -65.32, 78.3]
                    ]
                }
            }
        }).encode('UTF-8'))


class TestPyQgsOapifProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsOapifProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsOapifProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'

        create_landing_page_api_collection(endpoint)

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100, "name": "Orange", "name2": "oranGe", "num_char": "1"}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200, "name": "Apple", "name2": "Apple", "num_char": "2"}, "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}},
                {"type": "Feature", "id": "feat.3", "properties": {"pk": 4, "cnt": 400, "name": "Honey", "name2": "Honey", "num_char": "4"}, "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}},
                {"type": "Feature", "id": "feat.4", "properties": {"pk": 3, "cnt": 300, "name": "Pear", "name2": "PEaR", "num_char": "3"}, "geometry": None},
                {"type": "Feature", "id": "feat.5", "properties": {"pk": 5, "cnt": -200, "name": None, "name2": "NuLl", "num_char": "5"}, "geometry": {"type": "Point", "coordinates": [-71.123, 78.23]}}
            ]
        }

        # first items
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # Create test layer
        cls.vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        shutil.rmtree(cls.basetestpath, True)
        cls.vl = None  # so as to properly close the provider and remove any temporary file

    def testFeaturePaging(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeaturePaging'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())

        # first real page
        first_page = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200}, "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}}
            ],
            "links": [
                # Test multiple media types for next
                {"href": "http://" + endpoint + "/second_page.html", "rel": "next", "type": "text/html"},
                {"href": "http://" + endpoint + "/second_page", "rel": "next", "type": "application/geo+json"},
                {"href": "http://" + endpoint + "/second_page.xml", "rel": "next", "type": "text/xml"}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_page).encode('UTF-8'))

        # second page
        second_page = {
            "type": "FeatureCollection",
            "features": [
                # Also add a non expected property
                {"type": "Feature", "id": "feat.3", "properties": {"a_non_expected": "foo", "pk": 4, "cnt": 400}, "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}}
            ],
            "links": [
                {"href": "http://" + endpoint + "/third_page", "rel": "next"}
            ]
        }
        with open(sanitize(endpoint, '/second_page?' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(second_page).encode('UTF-8'))

        # third page
        third_page = {
            "type": "FeatureCollection",
            "features": [],
            "links": [
                {"href": "http://" + endpoint + "/third_page", "rel": "next"} # dummy link to ourselves
            ]
        }
        with open(sanitize(endpoint, '/third_page?' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(third_page).encode('UTF-8'))

        values = [f['pk'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2, 4])

        values = [f['pk'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2, 4])

    def testBbox(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testBbox'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200}, "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=-71,65.5,-65,78&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        extent = QgsRectangle(-71, 65.5, -65, 78)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1, 2])

        # Test request inside above one
        EPS = 0.1
        extent = QgsRectangle(-71 + EPS, 65.5 + EPS, -65 - EPS, 78 - EPS)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1, 2])

        # Test clamping of bbox
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=-180,64.5,-65,78&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        extent = QgsRectangle(-190, 64.5, -65, 78)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1, 2])

        # Test request completely outside of -180,-90,180,90
        extent = QgsRectangle(-1000, -1000, -900, -900)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [])

        # Test request containing -180,-90,180,90
        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100}, "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200}, "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}},
                {"type": "Feature", "id": "feat.3", "properties": {"pk": 4, "cnt": 400}, "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        extent = QgsRectangle(-181, -91, 181, 91)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1, 2, 4])


if __name__ == '__main__':
    unittest.main()
