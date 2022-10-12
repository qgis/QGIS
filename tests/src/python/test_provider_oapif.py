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

import copy
import json
import hashlib
import os
import re
import shutil
import tempfile

from osgeo import gdal
from qgis.PyQt.QtCore import QCoreApplication, Qt, QObject, QDateTime, QVariant
from qgis.PyQt.QtTest import QSignalSpy

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
    QgsCoordinateReferenceSystem,
    QgsBox3d,
    QgsMessageLog
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        ret = endpoint + hashlib.md5(x.replace('/', '_').encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    ret = endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"',
                                                                                                       '_').replace("'",
                                                                                                                    '_').replace(
        ' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')
    return ret


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


ACCEPT_LANDING = 'Accept=application/json'
ACCEPT_API = 'Accept=application/vnd.oai.openapi+json;version=3.0, application/openapi+json;version=3.0, application/json'
ACCEPT_COLLECTION = 'Accept=application/json'
ACCEPT_ITEMS = 'Accept=application/geo+json, application/json'


def create_landing_page_api_collection(endpoint,
                                       extraparam='',
                                       crs_url="http://www.opengis.net/def/crs/EPSG/0/4326",
                                       bbox=[-71.123, 66.33, -65.32, 78.3]):

    questionmark_extraparam = '?' + extraparam if extraparam else ''

    def add_params(x, y):
        if x:
            return x + '&' + y
        return y

    # Landing page
    with open(sanitize(endpoint, '?' + add_params(extraparam, ACCEPT_LANDING)), 'wb') as f:
        f.write(json.dumps({
            "links": [
                {"href": "http://" + endpoint + "/api" + questionmark_extraparam, "rel": "service-desc"},
                {"href": "http://" + endpoint + "/collections" + questionmark_extraparam, "rel": "data"}
            ]}).encode('UTF-8'))

    # API
    with open(sanitize(endpoint, '/api?' + add_params(extraparam, ACCEPT_API)), 'wb') as f:
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
    collection = {
        "id": "mycollection",
        "title": "my title",
        "description": "my description",
        "extent": {
            "spatial": {
                "bbox": [
                    bbox
                ]
            }
        }
    }
    if crs_url:
        collection["crs"] = [crs_url]
        collection["extent"]["spatial"]["crs"] = crs_url

    with open(sanitize(endpoint, '/collections/mycollection?' + add_params(extraparam, ACCEPT_COLLECTION)), 'wb') as f:
        f.write(json.dumps(collection).encode('UTF-8'))


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
                {"type": "Feature", "id": "feat.1",
                 "properties": {"pk": 1, "cnt": 100, "name": "Orange", "name2": "oranGe", "num_char": "1", "dt": "2020-05-03 12:13:14", "date": "2020-05-03", "time": "12:13:14"},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2",
                 "properties": {"pk": 2, "cnt": 200, "name": "Apple", "name2": "Apple", "num_char": "2", "dt": "2020-05-04 12:14:14", "date": "2020-05-04", "time": "12:14:14"},
                 "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}},
                {"type": "Feature", "id": "feat.3",
                 "properties": {"pk": 4, "cnt": 400, "name": "Honey", "name2": "Honey", "num_char": "4", "dt": "2021-05-04 13:13:14", "date": "2021-05-04", "time": "13:13:14"},
                 "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}},
                {"type": "Feature", "id": "feat.4",
                 "properties": {"pk": 3, "cnt": 300, "name": "Pear", "name2": "PEaR", "num_char": "3"},
                 "geometry": None},
                {"type": "Feature", "id": "feat.5",
                 "properties": {"pk": 5, "cnt": -200, "name": None, "name2": "NuLl", "num_char": "5", "dt": "2020-05-04 12:13:14", "date": "2020-05-02", "time": "12:13:01"},
                 "geometry": {"type": "Point", "coordinates": [-71.123, 78.23]}}
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

    def testExtentSubsetString(self):
        # can't run the base provider test suite here - WFS/OAPIF extent handling is different
        # to other providers
        pass

    def testFeaturePaging(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeaturePaging'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
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
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200},
                 "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}}
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
                {"type": "Feature", "id": "feat.3", "properties": {"a_non_expected": "foo", "pk": 4, "cnt": 400},
                 "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}}
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
                {"href": "http://" + endpoint + "/third_page", "rel": "next"}  # dummy link to ourselves
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
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200},
                 "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=65.5,-71,78,-65&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS),
                  'wb') as f:
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
        with open(
                sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=64.5,-180,78,-65&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS),
                'wb') as f:
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
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200},
                 "geometry": {"type": "Point", "coordinates": [-68.2, 70.8]}},
                {"type": "Feature", "id": "feat.3", "properties": {"pk": 4, "cnt": 400},
                 "geometry": {"type": "Point", "coordinates": [-65.32, 78.3]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=-90,-180,90,180&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        extent = QgsRectangle(-181, -91, 181, 91)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['pk'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1, 2, 4])

    def testLayerMetadata(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testLayerMetadata'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

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
                },
                "info":
                    {
                        "contact":
                            {
                                "name": "contact_name",
                                "email": "contact_email",
                                "url": "contact_url"
                            }
                }
            }).encode('UTF-8'))

        # collection
        base_collection = {
            "id": "mycollection",
            "title": "my title",
            "description": "my description",
            "extent": {
                "spatial": {
                    "bbox": [
                        [-71.123, 66.33, -65.32, 78.3],
                        None,  # invalid
                        [1, 2, 3],  # invalid
                        ["invalid", 1, 2, 3],  # invalid
                        [2, 49, -100, 3, 50, 100]
                    ]
                },
                "temporal":
                    {
                        "interval": [
                            [None, None],  # invalid
                            ["invalid", "invalid"],
                            "another_invalid",
                            ["1980-01-01T12:34:56.789Z", "2020-01-01T00:00:00Z"],
                            ["1980-01-01T12:34:56.789Z", None],
                            [None, "2020-01-01T00:00:00Z"]
                        ]
                },
                "crs": ["http://www.opengis.net/def/crs/EPSG/0/4326"]
            },
            "links": [
                {"href": "href_self", "rel": "self", "type": "application/json", "title": "my self link"},
                {"href": "href_parent", "rel": "parent", "title": "my parent link"},
                {"href": "http://download.example.org/buildings.gpkg",
                 "rel": "enclosure",
                 "type": "application/geopackage+sqlite3",
                 "title": "Bulk download (GeoPackage)",
                 "length": 123456789012345}
            ],
            # STAC specific
            "keywords": ["keyword_a", "keyword_b"]

        }

        collection = copy.deepcopy(base_collection)
        collection['links'].append(
            {"href": "https://creativecommons.org/publicdomain/zero/1.0/",
             "rel": "license", "type": "text/html",
             "title": "CC0-1.0"})
        collection['links'].append(
            {"href": "https://creativecommons.org/publicdomain/zero/1.0/rdf",
             "rel": "license", "type": "application/rdf+xml",
             "title": "CC0-1.0"})
        collection['links'].append(
            {"href": "https://example.com",
             "rel": "license", "type": "text/html",
             "title": "Public domain"})
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert md.identifier() == 'href_self'
        assert md.parentIdentifier() == 'href_parent'
        assert md.type() == 'dataset'
        assert md.title() == 'my title'
        assert md.abstract() == 'my description'

        contacts = md.contacts()
        assert len(contacts) == 1
        contact = contacts[0]
        assert contact.name == 'contact_name'
        assert contact.email == 'contact_email'
        assert contact.organization == 'contact_url'

        assert len(md.licenses()) == 2
        assert md.licenses()[0] == 'CC0-1.0'
        assert md.licenses()[1] == 'Public domain'

        assert 'keywords' in md.keywords()
        assert md.keywords()['keywords'] == ["keyword_a", "keyword_b"]

        assert md.crs().isValid()
        assert md.crs().isGeographic()
        assert not md.crs().hasAxisInverted()

        links = md.links()
        assert len(links) == 6, len(links)
        assert links[0].type == 'WWW:LINK'
        assert links[0].url == 'href_self'
        assert links[0].name == 'self'
        assert links[0].mimeType == 'application/json'
        assert links[0].description == 'my self link'
        assert links[0].size == ''
        assert links[2].size == '123456789012345'

        extent = md.extent()
        assert len(extent.spatialExtents()) == 2
        spatialExtent = extent.spatialExtents()[0]
        assert spatialExtent.extentCrs.isValid()
        assert spatialExtent.extentCrs.isGeographic()
        assert not spatialExtent.extentCrs.hasAxisInverted()
        assert spatialExtent.bounds == QgsBox3d(-71.123, 66.33, 0, -65.32, 78.3, 0)
        spatialExtent = extent.spatialExtents()[1]
        assert spatialExtent.bounds == QgsBox3d(2, 49, -100, 3, 50, 100)

        temporalExtents = extent.temporalExtents()
        assert len(temporalExtents) == 3
        assert temporalExtents[0].begin() == QDateTime.fromString("1980-01-01T12:34:56.789Z", Qt.ISODateWithMs), \
            temporalExtents[0].begin()
        assert temporalExtents[0].end() == QDateTime.fromString("2020-01-01T00:00:00Z", Qt.ISODateWithMs), \
            temporalExtents[0].end()
        assert temporalExtents[1].begin().isValid()
        assert not temporalExtents[1].end().isValid()
        assert not temporalExtents[2].begin().isValid()
        assert temporalExtents[2].end().isValid()

        # Variant using STAC license
        collection = copy.deepcopy(base_collection)
        collection['license'] = 'STAC license'
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert len(md.licenses()) == 1
        assert md.licenses()[0] == 'STAC license'

        # Variant using STAC license=various
        collection = copy.deepcopy(base_collection)
        collection['license'] = 'various'
        collection['links'].append(
            {"href": "https://creativecommons.org/publicdomain/zero/1.0/",
             "rel": "license", "type": "text/html",
             "title": "CC0-1.0"})
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert len(md.licenses()) == 1
        assert md.licenses()[0] == 'CC0-1.0'

        # Variant using STAC license=proprietary
        collection = copy.deepcopy(base_collection)
        collection['license'] = 'proprietary'
        collection['links'].append(
            {"href": "https://example.com",
             "rel": "license", "type": "text/html",
             "title": "my proprietary license"})
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert len(md.licenses()) == 1
        assert md.licenses()[0] == 'my proprietary license'

        # Variant using STAC license=proprietary (non conformant: missing a rel=license link)
        collection = copy.deepcopy(base_collection)
        collection['license'] = 'proprietary'
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert len(md.licenses()) == 1
        assert md.licenses()[0] == 'proprietary'

    def testDateTimeFiltering(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testDateTimeFiltering'
        create_landing_page_api_collection(endpoint)

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"my_dt_field": "2019-10-15T00:34:00Z", "foo": "bar"},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }

        no_items = {
            "type": "FeatureCollection",
            "features": [
            ]
        }

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='mycollection' filter='\"my_dt_field\" >= \\'2019-05-15T00:00:00Z\\''",
            'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-05-15T00:00:00Z/9999-12-31T00:00:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

        assert vl.setSubsetString(""""my_dt_field" < '2019-01-01T00:34:00Z'""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=0000-01-01T00:00:00Z/2019-01-01T00:34:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(no_items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, [])

        assert vl.setSubsetString(""""my_dt_field" = '2019-10-15T00:34:00Z'""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-10-15T00:34:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

        assert vl.setSubsetString(
            """("my_dt_field" >= '2019-01-01T00:34:00Z') AND ("my_dt_field" <= '2019-12-31T00:00:00Z')""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-01-01T00:34:00Z/2019-12-31T00:00:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

        # Partial on client side
        assert vl.setSubsetString("""("my_dt_field" >= '2019-01-01T00:34:00Z') AND ("foo" = 'bar')""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-01-01T00:34:00Z/9999-12-31T00:00:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

        # Same but with non-matching client-side part
        assert vl.setSubsetString("""("my_dt_field" >= '2019-01-01T00:34:00Z') AND ("foo" != 'bar')""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-01-01T00:34:00Z/9999-12-31T00:00:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, [])

        # Switch order
        assert vl.setSubsetString("""("foo" = 'bar') AND ("my_dt_field" >= '2019-01-01T00:34:00Z')""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&datetime=2019-01-01T00:34:00Z/9999-12-31T00:00:00Z&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

    def testStringList(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testStringList'
        create_landing_page_api_collection(endpoint)

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"my_stringlist_field": ["a", "b"]},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        features = [f for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(features[0]['my_stringlist_field'], ["a", "b"])

    def testApikey(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_apikey'
        create_landing_page_api_collection(endpoint, extraparam='apikey=mykey')

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }

        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&apikey=mykey&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&apikey=mykey&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        app_log = QgsApplication.messageLog()

        # signals should be emitted by application log
        app_spy = QSignalSpy(app_log.messageReceived)

        vl = QgsVectorLayer("url='http://" + endpoint + "?apikey=mykey' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, ['feat.1'])
        self.assertEqual(len(app_spy), 0, list(app_spy))

    def testDefaultCRS(self):

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = basetestpath + '/fake_qgis_http_endpoint_ogc84'

        create_landing_page_api_collection(endpoint,
                                           crs_url="",  # OGC norm says that if crs is not explicitly defined it is OGC:CRS84
                                           bbox=[66.33, -71.123, 78.3, -65.32])

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1",
                 "properties": {"pk": 1, "cnt": 100, "name": "Orange", "name2": "oranGe", "num_char": "1", "dt": "2020-05-03 12:13:14", "date": "2020-05-03", "time": "12:13:14"},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}},
                {"type": "Feature", "id": "feat.2",
                 "properties": {"pk": 2, "cnt": 200, "name": "Apple", "name2": "Apple", "num_char": "2", "dt": "2020-05-04 12:14:14", "date": "2020-05-04", "time": "12:14:14"},
                 "geometry": {"type": "Point", "coordinates": [70.8, -68.2]}},
                {"type": "Feature", "id": "feat.3",
                 "properties": {"pk": 4, "cnt": 400, "name": "Honey", "name2": "Honey", "num_char": "4", "dt": "2021-05-04 13:13:14", "date": "2021-05-04", "time": "13:13:14"},
                 "geometry": {"type": "Point", "coordinates": [78.3, -65.32]}},
                {"type": "Feature", "id": "feat.4",
                 "properties": {"pk": 3, "cnt": 300, "name": "Pear", "name2": "PEaR", "num_char": "3"},
                 "geometry": None},
                {"type": "Feature", "id": "feat.5",
                 "properties": {"pk": 5, "cnt": -200, "name": None, "name2": "NuLl", "num_char": "5", "dt": "2020-05-04 12:13:14", "date": "2020-05-02", "time": "12:13:01"},
                 "geometry": {"type": "Point", "coordinates": [78.23, -71.123]}}
            ]
        }

        # first items
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        assert vl.isValid()
        source = vl.dataProvider()

        self.assertEqual(source.sourceCrs().authid(), 'OGC:CRS84')

    def testCRS2056(self):

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = basetestpath + '/fake_qgis_http_endpoint_epsg_2056'

        create_landing_page_api_collection(endpoint,
                                           crs_url="http://www.opengis.net/def/crs/EPSG/0/2056",
                                           bbox=[2508500, 1152000, 2513450, 1156950])

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1",
                 "properties": {"pk": 1, "cnt": 100, "name": "Orange", "name2": "oranGe", "num_char": "1", "dt": "2020-05-03 12:13:14", "date": "2020-05-03", "time": "12:13:14"},
                 "geometry": {"type": "Point", "coordinates": [2510100, 1155050]}},
                {"type": "Feature", "id": "feat.2",
                 "properties": {"pk": 2, "cnt": 200, "name": "Apple", "name2": "Apple", "num_char": "2", "dt": "2020-05-04 12:14:14", "date": "2020-05-04", "time": "12:14:14"},
                 "geometry": {"type": "Point", "coordinates": [2511250, 1154600]}},
                {"type": "Feature", "id": "feat.3",
                 "properties": {"pk": 4, "cnt": 400, "name": "Honey", "name2": "Honey", "num_char": "4", "dt": "2021-05-04 13:13:14", "date": "2021-05-04", "time": "13:13:14"},
                 "geometry": {"type": "Point", "coordinates": [2511260, 1154610]}},
                {"type": "Feature", "id": "feat.4",
                 "properties": {"pk": 3, "cnt": 300, "name": "Pear", "name2": "PEaR", "num_char": "3"},
                 "geometry": None},
                {"type": "Feature", "id": "feat.5",
                 "properties": {"pk": 5, "cnt": -200, "name": None, "name2": "NuLl", "num_char": "5", "dt": "2020-05-04 12:13:14", "date": "2020-05-02", "time": "12:13:01"},
                 "geometry": {"type": "Point", "coordinates": [2511270, 1154620]}}
            ]
        }

        # first items
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        assert vl.isValid()
        source = vl.dataProvider()

        self.assertEqual(source.sourceCrs().authid(), 'EPSG:2056')


if __name__ == '__main__':
    unittest.main()
