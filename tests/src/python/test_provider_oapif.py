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
import hashlib
import json
import os
import shutil
import tempfile

from osgeo import gdal

from qgis.PyQt.QtCore import QCoreApplication, QDateTime, Qt, QVariant
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsApplication,
    QgsBox3d,
    QgsFeature,
    QgsFeatureRequest,
    QgsGeometry,
    QgsRectangle,
    QgsSettings,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from providertestbase import ProviderTestCase


def sanitize(endpoint, query_params):
    # Implement the logic of QgsBaseNetworkRequest::sendGET()
    # Note query_params can actually contain subpaths, so create the full url
    # by concatenating boths, and then figure out things...

    url = endpoint + query_params
    # For REST API using URL subpaths, normalize the subpaths
    afterEndpointStartPos = url.find("fake_qgis_http_endpoint") + len("fake_qgis_http_endpoint")
    afterEndpointStart = url[afterEndpointStartPos:]
    afterEndpointStart = afterEndpointStart.replace('/', '_')
    url = url[0:afterEndpointStartPos] + afterEndpointStart
    posQuotationMark = url.find('?')
    endpoint = url[0:posQuotationMark]
    query_params = url[posQuotationMark:]

    if len(endpoint + query_params) > 256:
        ret = endpoint + hashlib.md5(query_params.encode()).hexdigest()
        # print('Before: ' + endpoint + query_params)
        # print('After:  ' + ret)
        return ret
    ret = endpoint + query_params.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"',
                                                                                                                  '_').replace("'",
                                                                                                                               '_').replace(
        ' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')
    return ret


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


ACCEPT_LANDING = 'Accept=application/json'
ACCEPT_API = 'Accept=application/vnd.oai.openapi+json;version=3.0, application/openapi+json;version=3.0, application/json'
ACCEPT_COLLECTION = 'Accept=application/json'
ACCEPT_CONFORMANCE = 'Accept=application/json'
ACCEPT_ITEMS = 'Accept=application/geo+json, application/json'
ACCEPT_QUERYABLES = 'Accept=application/schema+json'


def mergeDict(d1, d2):
    res = copy.deepcopy(d1)
    for k in d2:
        if k not in res:
            res[k] = d2[k]
        else:
            res[k] = mergeDict(res[k], d2[k])
    return res


def create_landing_page_api_collection(endpoint,
                                       extraparam='',
                                       storageCrs=None,
                                       crsList=None,
                                       bbox=[-71.123, 66.33, -65.32, 78.3],
                                       additionalApiResponse={},
                                       additionalConformance=[]):

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
                {"href": "http://" + endpoint + "/collections" + questionmark_extraparam, "rel": "data"},
                {"href": "http://" + endpoint + "/conformance" + questionmark_extraparam, "rel": "conformance"},
            ]}).encode('UTF-8'))

    # API
    with open(sanitize(endpoint, '/api?' + add_params(extraparam, ACCEPT_API)), 'wb') as f:
        j = mergeDict(additionalApiResponse, {"components": {
            "parameters": {
                "limit": {
                    "schema": {
                        "maximum": 1000,
                        "default": 100
                    }
                }
            }
        }
        })
        f.write(json.dumps(j).encode('UTF-8'))

    # conformance
    with open(sanitize(endpoint, '/conformance?' + add_params(extraparam, ACCEPT_CONFORMANCE)), 'wb') as f:
        f.write(json.dumps({
            "conformsTo": ["http://www.opengis.net/spec/ogcapi-features-2/1.0/conf/crs"] + additionalConformance
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
    if bbox is None:
        del collection["extent"]
    if storageCrs:
        collection["storageCrs"] = storageCrs
    if crsList:
        collection["crs"] = crsList

    with open(sanitize(endpoint, '/collections/mycollection?' + add_params(extraparam, ACCEPT_COLLECTION)), 'wb') as f:
        f.write(json.dumps(collection).encode('UTF-8'))

    # Options
    with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
        f.write("HEAD, GET".encode("UTF-8"))


class TestPyQgsOapifProvider(QgisTestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super(TestPyQgsOapifProvider, cls).setUpClass()

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
            "numberMatched": 5,
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

        # limit 1 for getting count
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

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
        super(TestPyQgsOapifProvider, cls).tearDownClass()

    def testCrs(self):
        self.assertEqual(self.source.sourceCrs().authid(), 'OGC:CRS84')

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
        create_landing_page_api_collection(endpoint, storageCrs="http://www.opengis.net/def/crs/EPSG/0/4326")

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
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
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200},
                 "geometry": {"type": "Point", "coordinates": [70.8, -68.2]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=65.5,-71,78,-65&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS),
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
                sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=64.5,-180,78,-65&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS),
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
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}},
                {"type": "Feature", "id": "feat.2", "properties": {"pk": 2, "cnt": 200},
                 "geometry": {"type": "Point", "coordinates": [70.8, -68.2]}},
                {"type": "Feature", "id": "feat.3", "properties": {"pk": 4, "cnt": 400},
                 "geometry": {"type": "Point", "coordinates": [78.3, -65.32]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&bbox=-90,-180,90,180&bbox-crs=http://www.opengis.net/def/crs/EPSG/0/4326&crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS), 'wb') as f:
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
                }
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
        assert md.crs().authid() == "OGC:CRS84"
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
        assert spatialExtent.bounds == QgsBox3d(-71.123, 66.33, float("nan"), -65.32, 78.3, float("nan"))
        spatialExtent = extent.spatialExtents()[1]
        assert spatialExtent.bounds == QgsBox3d(2, 49, -100, 3, 50, 100)

        temporalExtents = extent.temporalExtents()
        assert len(temporalExtents) == 3
        assert temporalExtents[0].begin() == QDateTime.fromString("1980-01-01T12:34:56.789Z", Qt.DateFormat.ISODateWithMs), \
            temporalExtents[0].begin()
        assert temporalExtents[0].end() == QDateTime.fromString("2020-01-01T00:00:00Z", Qt.DateFormat.ISODateWithMs), \
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

        # Variant with storageCrs
        collection = copy.deepcopy(base_collection)
        collection['storageCrs'] = "http://www.opengis.net/def/crs/EPSG/0/4258"
        collection['storageCrsCoordinateEpoch'] = 2020.0
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert vl.sourceCrs().isValid()
        assert vl.sourceCrs().authid() == "EPSG:4258"
        assert vl.sourceCrs().isGeographic()
        assert vl.sourceCrs().coordinateEpoch() == 2020.0
        assert vl.sourceCrs().hasAxisInverted()

        # Variant with a list of crs
        collection = copy.deepcopy(base_collection)
        collection['crs'] = ["http://www.opengis.net/def/crs/EPSG/0/4258", "http://www.opengis.net/def/crs/EPSG/0/4326"]
        with open(sanitize(endpoint, '/collections/mycollection?' + ACCEPT_COLLECTION), 'wb') as f:
            f.write(json.dumps(collection).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        md = vl.metadata()
        assert vl.sourceCrs().isValid()
        assert vl.sourceCrs().authid() == "EPSG:4258"
        assert vl.sourceCrs().isGeographic()
        assert vl.sourceCrs().hasAxisInverted()

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

    def testSimpleQueryableFiltering(self):
        """Test simple filtering capabilities, not requiring Part 3"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_encoded_query_testSimpleQueryableFiltering'
        additionalApiResponse = {
            "paths": {
                "/collections/mycollection/items": {
                    "get": {
                        "parameters": [
                            {
                                "$ref": "#/components/parameters/mycollection_strfield_param"
                            },
                            {
                                "name": "intfield",
                                "in": "query",
                                "style": "form",
                                "explode": False,
                                "schema": {
                                    "type": "integer"
                                }
                            },
                            {
                                "name": "doublefield",
                                "in": "query",
                                "style": "form",
                                "explode": False,
                                "schema": {
                                    "type": "number"
                                }
                            },
                            {
                                "name": "boolfield",
                                "in": "query",
                                "style": "form",
                                "explode": False,
                                "schema": {
                                    "type": "boolean"
                                }
                            }
                        ]
                    }
                }
            },
            "components": {
                "parameters": {
                    "mycollection_strfield_param": {
                        "name": "strfield",
                        "in": "query",
                        "style": "form",
                        "explode": False,
                        "schema": {
                            "type": "string"
                        }
                    }
                }
            }
        }
        create_landing_page_api_collection(endpoint, additionalApiResponse=additionalApiResponse)

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties":
                 {"strfield": "foo=bar",
                  "intfield": 1,
                  "doublefield": 1.5,
                  "boolfield": True},
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
            "url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        assert vl.setSubsetString(""""strfield" = 'foo=bar' and intfield = 1 and doublefield = 1.5 and boolfield = true""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&strfield=foo%3Dbar&intfield=1&doublefield=1.5&boolfield=true&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, ['feat.1'])

        assert vl.setSubsetString(""""strfield" = 'bar'""")

        filename = sanitize(endpoint,
                            '/collections/mycollection/items?limit=1000&strfield=bar&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(no_items).encode('UTF-8'))
        values = [f['id'] for f in vl.getFeatures()]
        os.unlink(filename)
        self.assertEqual(values, [])

    def testCQL2TextFiltering(self):
        """Test Part 3 CQL2-Text filtering"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_encoded_query_testCQL2TextFiltering'
        additionalConformance = [
            "http://www.opengis.net/spec/cql2/1.0/conf/advanced-comparison-operators",
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2",
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-operators",
            "http://www.opengis.net/spec/cql2/1.0/conf/case-insensitive-comparison",
            "http://www.opengis.net/spec/cql2/1.0/conf/cql2-text",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter",
        ]

        create_landing_page_api_collection(endpoint, additionalConformance=additionalConformance)

        filename = sanitize(endpoint, '/collections/mycollection/queryables?' + ACCEPT_QUERYABLES)
        queryables = {
            "properties": {
                "strfield": {
                    "type": "string"
                },
                "strfield2": {
                    "type": "string"
                },
                "intfield": {
                    "type": "integer"
                },
                "doublefield": {
                    "type": "number"
                },
                "boolfield": {
                    "type": "boolean"
                },
                "boolfield2": {
                    "type": "boolean"
                },
                "datetimefield": {
                    "type": "string",
                    "format": "date-time",
                },
                "datefield": {
                    "type": "string",
                    "format": "date",
                },
                "geometry": {
                    "$ref": "https://geojson.org/schema/Point.json"
                },
            }
        }
        with open(filename, 'wb') as f:
            f.write(json.dumps(queryables).encode('UTF-8'))

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties":
                 {"strfield": "foo=bar",
                  "strfield2": None,
                  "intfield": 1,
                  "doublefield": 1.5,
                  "not_a_queryable": 3,
                  "datetimefield": "2023-04-19T12:34:56Z",
                  "datefield": "2023-04-19",
                  "boolfield": True,
                  "boolfield2": False},
                 "geometry": {"type": "Point", "coordinates": [-70.5, 66.5]}}
            ]
        }

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        tests = [
            (""""strfield" = 'foo=bar' and intfield = 1""",
             """filter=((strfield%20%3D%20'foo%3Dbar')%20AND%20(intfield%20%3D%201))&filter-lang=cql2-text"""),
            ("""doublefield = 1.5 or boolfield = true""",
             """filter=((doublefield%20%3D%201.5)%20OR%20(boolfield%20%3D%20TRUE))&filter-lang=cql2-text"""),
            ("boolfield2 = false", "filter=(boolfield2%20%3D%20FALSE)&filter-lang=cql2-text"""),
            ("NOT(intfield = 0)", """filter=(NOT%20((intfield%20%3D%200)))&filter-lang=cql2-text"""),
            ("intfield <> 0", """filter=(intfield%20%3C%3E%200)&filter-lang=cql2-text"""),
            ("intfield > 0", """filter=(intfield%20%3E%200)&filter-lang=cql2-text"""),
            ("intfield >= 1", """filter=(intfield%20%3E%3D%201)&filter-lang=cql2-text"""),
            ("intfield < 2", """filter=(intfield%20%3C%202)&filter-lang=cql2-text"""),
            ("intfield <= 1", """filter=(intfield%20%3C%3D%201)&filter-lang=cql2-text"""),
            ("intfield IN (1, 2)", """filter=intfield%20IN%20(1,2)&filter-lang=cql2-text"""),
            ("intfield NOT IN (3, 4)", """filter=intfield%20NOT%20IN%20(3,4)&filter-lang=cql2-text"""),
            ("intfield BETWEEN 0 AND 2", "filter=intfield%20BETWEEN%200%20AND%202&filter-lang=cql2-text"),
            ("intfield NOT BETWEEN 3 AND 4", "filter=intfield%20NOT%20BETWEEN%203%20AND%204&filter-lang=cql2-text"),
            ("strfield2 IS NULL", """filter=(strfield2%20IS%20NULL)&filter-lang=cql2-text"""),
            ("intfield IS NOT NULL", """filter=(intfield%20IS%20NOT%20NULL)&filter-lang=cql2-text"""),
            ("datetimefield = make_datetime(2023, 4, 19, 12, 34, 56)",
             "filter=(datetimefield%20%3D%20TIMESTAMP('2023-04-19T12:34:56.000Z'))&filter-lang=cql2-text"),
            ("datetimefield = '2023-04-19T12:34:56.000Z'",
             "filter=(datetimefield%20%3D%20TIMESTAMP('2023-04-19T12:34:56.000Z'))&filter-lang=cql2-text"),
            ("datefield = make_date(2023, 4, 19)",
             "filter=(datefield%20%3D%20DATE('2023-04-19'))&filter-lang=cql2-text"),
            ("datefield = '2023-04-19'",
             "filter=(datefield%20%3D%20DATE('2023-04-19'))&filter-lang=cql2-text"),
            (""""strfield" LIKE 'foo%'""",
             """filter=(strfield%20LIKE%20'foo%25')&filter-lang=cql2-text"""),
            (""""strfield" NOT LIKE 'bar'""",
             """filter=(strfield%20NOT%20LIKE%20'bar')&filter-lang=cql2-text"""),
            (""""strfield" ILIKE 'fo%'""",
             """filter=(CASEI(strfield)%20LIKE%20CASEI('fo%25'))&filter-lang=cql2-text"""),
            (""""strfield" NOT ILIKE 'bar'""",
             """filter=(CASEI(strfield)%20NOT%20LIKE%20CASEI('bar'))&filter-lang=cql2-text"""),
            ("""intersects_bbox($geometry, geomFromWkt('POLYGON((-180 -90,-180 90,180 90,180 -90,-180 -90))'))""",
             """filter=S_INTERSECTS(geometry,BBOX(-180,-90,180,90))&filter-lang=cql2-text"""),
            ("""intersects($geometry, geomFromWkt('POINT(-70.5 66.5))'))""",
             """filter=S_INTERSECTS(geometry,POINT(-70.5%2066.5))&filter-lang=cql2-text"""),
            # Partially evaluated on server
            ("intfield >= 1 AND not_a_queryable = 3", """filter=(intfield%20%3E%3D%201)&filter-lang=cql2-text"""),
            ("not_a_queryable = 3 AND intfield >= 1", """filter=(intfield%20%3E%3D%201)&filter-lang=cql2-text"""),
            # Only evaluated on client
            ("intfield >= 1 OR not_a_queryable = 3", ""),
            ("not_a_queryable = 3 AND not_a_queryable = 3", ""),
        ]
        for (expr, cql_filter) in tests:
            assert vl.setSubsetString(expr)

            filename = sanitize(endpoint,
                                "/collections/mycollection/items?limit=1000&" + cql_filter + ("&" if cql_filter else "") + ACCEPT_ITEMS)
            with open(filename, 'wb') as f:
                f.write(json.dumps(items).encode('UTF-8'))
            values = [f['id'] for f in vl.getFeatures()]
            os.unlink(filename)
            self.assertEqual(values, ['feat.1'], expr)

    def testCQL2TextFilteringAndPart2(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_encoded_query_testCQL2TextFilteringAndPart2'
        additionalConformance = [
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2",
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-operators",
            "http://www.opengis.net/spec/cql2/1.0/conf/cql2-text",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter",
        ]

        create_landing_page_api_collection(endpoint,
                                           storageCrs="http://www.opengis.net/def/crs/EPSG/0/4258",
                                           crsList=["http://www.opengis.net/def/crs/OGC/0/CRS84",
                                                    "http://www.opengis.net/def/crs/EPSG/0/4258"],
                                           additionalConformance=additionalConformance)

        filename = sanitize(endpoint, '/collections/mycollection/queryables?' + ACCEPT_QUERYABLES)
        queryables = {
            "properties": {
                "geometry": {
                    "$ref": "https://geojson.org/schema/Point.json"
                },
            }
        }
        with open(filename, 'wb') as f:
            f.write(json.dumps(queryables).encode('UTF-8'))

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties":
                 {},
                 # lat, long order
                 "geometry": {"type": "Point", "coordinates": [49, 2]}}
            ]
        }

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        tests = [
            ("""intersects($geometry, geomFromWkt('POINT(2 49))'))""",
             """filter=S_INTERSECTS(geometry,POINT(49%202))&filter-lang=cql2-text&filter-crs=http://www.opengis.net/def/crs/EPSG/0/4258"""),
        ]
        for (expr, cql_filter) in tests:
            assert vl.setSubsetString(expr)

            filename = sanitize(endpoint,
                                "/collections/mycollection/items?limit=1000&" + cql_filter + ("&" if cql_filter else "") + "crs=http://www.opengis.net/def/crs/EPSG/0/4258&" + ACCEPT_ITEMS)
            with open(filename, 'wb') as f:
                f.write(json.dumps(items).encode('UTF-8'))
            values = [f['id'] for f in vl.getFeatures()]
            os.unlink(filename)
            self.assertEqual(values, ['feat.1'], expr)

    def testCQL2TextFilteringGetFeaturesExpression(self):
        """Test Part 3 CQL2-Text filtering"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_encoded_query_testCQL2TextFilteringGetFeaturesExpression'
        additionalConformance = [
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2",
            "http://www.opengis.net/spec/cql2/1.0/conf/cql2-text",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter",
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter",
        ]

        create_landing_page_api_collection(endpoint, additionalConformance=additionalConformance)

        filename = sanitize(endpoint, '/collections/mycollection/queryables?' + ACCEPT_QUERYABLES)
        queryables = {
            "properties": {
                "strfield": {
                    "type": "string"
                },
                "geometry": {
                    "$ref": "https://geojson.org/schema/Point.json"
                },
            }
        }
        with open(filename, 'wb') as f:
            f.write(json.dumps(queryables).encode('UTF-8'))

        items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties":
                 {
                     "strfield": "foo=bar",
                 },
                 "geometry": {"type": "Point", "coordinates": [-70.5, 66.5]}}
            ]
        }

        filename = sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS)
        with open(filename, 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        self.assertTrue(vl.isValid())
        os.unlink(filename)

        tests = [
            ("", """"strfield" = 'foo=bar'""",
             """filter=(strfield%20%3D%20'foo%3Dbar')&filter-lang=cql2-text"""),
            ("strfield <> 'x'", """"strfield" = 'foo=bar'""",
             """filter=((strfield%20%3C%3E%20'x'))%20AND%20((strfield%20%3D%20'foo%3Dbar'))&filter-lang=cql2-text"""),
        ]
        for (substring_expr, getfeatures_expr, cql_filter) in tests:
            assert vl.setSubsetString(substring_expr)
            filename = sanitize(endpoint,
                                "/collections/mycollection/items?limit=1000&" + cql_filter + ("&" if cql_filter else "") + ACCEPT_ITEMS)
            with open(filename, 'wb') as f:
                f.write(json.dumps(items).encode('UTF-8'))
            request = QgsFeatureRequest()
            if getfeatures_expr:
                request.setFilterExpression(getfeatures_expr)
            values = [f['id'] for f in vl.getFeatures(request)]
            os.unlink(filename)
            self.assertEqual(values, ['feat.1'], (substring_expr, getfeatures_expr))

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
                                           storageCrs="http://www.opengis.net/def/crs/EPSG/0/2056",
                                           crsList=["http://www.opengis.net/def/crs/OGC/0/CRS84", "http://www.opengis.net/def/crs/EPSG/0/2056"])

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

        # Test srsname parameter overrides default CRS
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' srsname='OGC:CRS84'", 'test', 'OAPIF')
        assert vl.isValid()
        source = vl.dataProvider()

        self.assertEqual(source.sourceCrs().authid(), 'OGC:CRS84')

    def testFeatureCountFallbackAndNoBboxInCollection(self):

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = basetestpath + '/fake_qgis_http_endpoint_feature_count_fallback'

        create_landing_page_api_collection(endpoint, storageCrs="http://www.opengis.net/def/crs/EPSG/0/2056", bbox=None)

        items = {
            "type": "FeatureCollection",
            "features": [],
            "links": [
                # should not be hit
                {"href": "http://" + endpoint + "/next_page", "rel": "next"}
            ]
        }
        for i in range(10):
            items["features"].append({"type": "Feature", "id": f"feat.{i}",
                                      "properties": {},
                                      "geometry": {"type": "Point", "coordinates": [23, 63]}})

        # first items
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # first items
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # real page

        items = {
            "type": "FeatureCollection",
            "features": [],
            "links": [
                # should not be hit
                {"href": "http://" + endpoint + "/next_page", "rel": "next"}
            ]
        }
        for i in range(1001):
            items["features"].append({"type": "Feature", "id": f"feat.{i}",
                                      "properties": {},
                                      "geometry": None})

        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&crs=http://www.opengis.net/def/crs/EPSG/0/2056&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(items).encode('UTF-8'))

        # Create test layer

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection'", 'test', 'OAPIF')
        assert vl.isValid()
        source = vl.dataProvider()

        # Extent got from first fetched features
        reference = QgsGeometry.fromRect(
            QgsRectangle(3415684, 3094884,
                         3415684, 3094884))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   10), f'Expected {reference.asWkt()}, got {vl_extent.asWkt()}'

        app_log = QgsApplication.messageLog()
        # signals should be emitted by application log
        app_spy = QSignalSpy(app_log.messageReceived)

        self.assertEqual(source.featureCount(), 1000)

        self.assertEqual(len(app_spy), 0, list(app_spy))

    def testFeatureInsertionDeletion(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureInsertion'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        # Basic OPTIONS response: no AddFeatures capability
        self.assertEqual(vl.dataProvider().capabilities() & vl.dataProvider().AddFeatures,
                         vl.dataProvider().NoCapabilities)

        # POST on /items, but no DELETE on /items/id
        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().AddFeatures,
                            vl.dataProvider().NoCapabilities)
        self.assertEqual(vl.dataProvider().capabilities() & vl.dataProvider().DeleteFeatures,
                         vl.dataProvider().NoCapabilities)

        # DELETE on /items/id
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, DELETE".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().AddFeatures,
                            vl.dataProvider().NoCapabilities)
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().DeleteFeatures,
                            vl.dataProvider().NoCapabilities)

        with open(sanitize(endpoint, '/collections/mycollection/items?POSTDATA={"geometry":{"coordinates":[2.0,49.0],"type":"Point"},"properties":{"cnt":1234567890123,"pk":1},"type":"Feature"}'), 'wb') as f:
            f.write(b"Location: /collections/mycollection/items/new_id\r\n")

        with open(sanitize(endpoint, '/collections/mycollection/items?POSTDATA={"geometry":null,"properties":{"cnt":null,"pk":null},"type":"Feature"}'), 'wb') as f:
            f.write(b"Location: /collections/mycollection/items/other_id\r\n")

        new_id = {"type": "Feature", "id": "new_id", "properties": {"pk": 1, "cnt": 1234567890123},
                  "geometry": {"type": "Point", "coordinates": [2, 49]}}
        with open(sanitize(endpoint, '/collections/mycollection/items/new_id?' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(new_id).encode('UTF-8'))

        other_id = {"type": "Feature", "id": "other_id", "properties": {"pk": 2, "cnt": 123},
                    "geometry": None}
        with open(sanitize(endpoint, '/collections/mycollection/items/other_id?' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(other_id).encode('UTF-8'))

        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([None, 1, 1234567890123])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))

        f2 = QgsFeature()
        f2.setFields(vl.fields())

        ret, fl = vl.dataProvider().addFeatures([f, f2])
        self.assertTrue(ret)

        self.assertEqual(fl[0].id(), 1)
        self.assertEqual(fl[0]["id"], "new_id")
        self.assertEqual(fl[0]["pk"], 1)
        self.assertEqual(fl[0]["cnt"], 1234567890123)

        self.assertEqual(fl[1].id(), 2)
        self.assertEqual(fl[1]["id"], "other_id")
        self.assertEqual(fl[1]["pk"], 2)
        self.assertEqual(fl[1]["cnt"], 123)

        # Failed attempt
        self.assertFalse(vl.dataProvider().deleteFeatures([1]))

        with open(sanitize(endpoint, '/collections/mycollection/items/new_id?VERB=DELETE'), 'wb') as f:
            f.write(b"")

        with open(sanitize(endpoint, '/collections/mycollection/items/other_id?VERB=DELETE'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().deleteFeatures([1, 2]))

    def testFeatureInsertionNonDefaultCrs(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureInsertionNonDefaultCrs'
        create_landing_page_api_collection(endpoint, storageCrs="http://www.opengis.net/def/crs/EPSG/0/4326")

        # first items (lat, long) order
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [-70.332, 66.33]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        # POST on /items, but no DELETE on /items/id
        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        # (lat, long) order
        with open(sanitize(endpoint, '/collections/mycollection/items?POSTDATA={"geometry":{"coordinates":[49.0,2.0],"type":"Point"},"properties":{"cnt":1234567890123,"pk":1},"type":"Feature"}&Content-Crs=http://www.opengis.net/def/crs/EPSG/0/4326'), 'wb') as f:
            f.write(b"Location: /collections/mycollection/items/new_id\r\n")

        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([None, 1, 1234567890123])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))

        ret, _ = vl.dataProvider().addFeatures([f])
        self.assertTrue(ret)

    def testFeatureGeometryChange(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureGeometryChange'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, PUT, DELETE".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().ChangeGeometries,
                            vl.dataProvider().NoCapabilities)

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?PUTDATA={"geometry":{"coordinates":[3.0,50.0],"type":"Point"},"id":"feat.1","properties":{"cnt":100,"pk":1},"type":"Feature"}'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')}))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 50.0))

    def testFeatureGeometryChangeNonDefaultCrs(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureGeometryChangeNonDefaultCrs'
        create_landing_page_api_collection(endpoint, storageCrs="http://www.opengis.net/def/crs/EPSG/0/4326")

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

        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, PUT, DELETE".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().ChangeGeometries,
                            vl.dataProvider().NoCapabilities)

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        # (Lat, Long) order
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?PUTDATA={"geometry":{"coordinates":[50.0,3.0],"type":"Point"},"id":"feat.1","properties":{"cnt":100,"pk":1},"type":"Feature"}&Content-Crs=http://www.opengis.net/def/crs/EPSG/0/4326'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')}))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 50.0))

    def testFeatureGeometryChangePatch(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureGeometryChangePatch'
        create_landing_page_api_collection(endpoint, storageCrs="http://www.opengis.net/def/crs/EPSG/0/4326")

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

        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, PUT, DELETE, PATCH".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().ChangeGeometries,
                            vl.dataProvider().NoCapabilities)

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&crs=http://www.opengis.net/def/crs/EPSG/0/4326&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        # (Lat, Long) order
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?PATCHDATA={"geometry":{"coordinates":[50.0,3.0],"type":"Point"}}&Content-Crs=http://www.opengis.net/def/crs/EPSG/0/4326&Content-Type=application_merge-patch+json'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')}))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 50.0))

    def testFeatureAttributeChange(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureAttributeChange'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, PUT, DELETE".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().ChangeGeometries,
                            vl.dataProvider().NoCapabilities)

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?PUTDATA={"geometry":{"coordinates":[66.33,-70.332],"type":"Point"},"id":"feat.1","properties":{"cnt":200,"pk":1},"type":"Feature"}'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().changeAttributeValues({1: {2: 200}}))

        values = [f['cnt'] for f in vl.getFeatures()]
        self.assertEqual(values, [200])

    def testFeatureAttributeChangePatch(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureAttributeChangePatch'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"pk": 1, "cnt": 100},
                 "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        with open(sanitize(endpoint, '/collections/mycollection/items?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, POST".encode("UTF-8"))
        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?VERB=OPTIONS'), 'wb') as f:
            f.write("HEAD, GET, PUT, DELETE, PATCH".encode("UTF-8"))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().ChangeGeometries,
                            vl.dataProvider().NoCapabilities)

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        with open(sanitize(endpoint, '/collections/mycollection/items/feat.1?PATCHDATA={"properties":{"cnt":200}}&Content-Type=application_merge-patch+json'), 'wb') as f:
            f.write(b"")

        self.assertTrue(vl.dataProvider().changeAttributeValues({1: {2: 200}}))

        values = [f['cnt'] for f in vl.getFeatures()]
        self.assertEqual(values, [200])

    # GDAL 3.5.0 is required since it is the first version that tags "complex"
    # fields as OFSTJSON
    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 5, 0), "GDAL 3.5.0 required")
    def testFeatureComplexAttribute(self):

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testFeatureComplexAttribute'
        create_landing_page_api_collection(endpoint)

        # first items
        first_items = {
            "type": "FeatureCollection",
            "features": [
                {"type": "Feature", "id": "feat.1", "properties": {"center": {
                    "type": "Point",
                    "coordinates": [
                        6.50,
                        51.80
                    ]
                }},
                    "geometry": {"type": "Point", "coordinates": [66.33, -70.332]}}
            ]
        }
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=10&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        # real page
        with open(sanitize(endpoint, '/collections/mycollection/items?limit=1000&' + ACCEPT_ITEMS), 'wb') as f:
            f.write(json.dumps(first_items).encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='mycollection' restrictToRequestBBOX=1", 'test',
                            'OAPIF')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.fields().field("center").type(), QVariant.Map)

        # First time we getFeatures(): comes directly from the GeoJSON layer
        values = [f["center"] for f in vl.getFeatures()]
        self.assertEqual(values, [{'coordinates': [6.5, 51.8], 'type': 'Point'}])

        # Now, that comes from the Spatialite cache, through
        # serialization and deserialization
        values = [f["center"] for f in vl.getFeatures()]
        self.assertEqual(values, [{'coordinates': [6.5, 51.8], 'type': 'Point'}])


if __name__ == '__main__':
    unittest.main()
