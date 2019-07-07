# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '17/04/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import json
import re

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServerApi,
    QgsServerApiUtils,
    QgsServiceRegistry
)
from qgis.core import QgsProject, QgsRectangle
from qgis.testing import unittest
from utilities import unitTestDataPath
from urllib import parse

import tempfile

from test_qgsserver import QgsServerTestBase


class QgsServerAPIUtilsTest(QgsServerTestBase):
    """ QGIS API server utils tests"""

    def test_parse_bbox(self):
        bbox = QgsServerApiUtils.parseBbox('8.203495,44.901482,8.203497,44.901484')
        self.assertEquals(bbox.xMinimum(), 8.203495)
        self.assertEquals(bbox.yMinimum(), 44.901482)
        self.assertEquals(bbox.xMaximum(), 8.203497)
        self.assertEquals(bbox.yMaximum(), 44.901484)

        bbox = QgsServerApiUtils.parseBbox('8.203495,44.901482,8.203497,44.901484,100,120')
        self.assertEquals(bbox.xMinimum(), 8.203495)
        self.assertEquals(bbox.yMinimum(), 44.901482)
        self.assertEquals(bbox.xMaximum(), 8.203497)
        self.assertEquals(bbox.yMaximum(), 44.901484)

        bbox = QgsServerApiUtils.parseBbox('something_wrong_here')
        self.assertTrue(bbox.isEmpty())
        bbox = QgsServerApiUtils.parseBbox('8.203495,44.901482,8.203497,something_wrong_here')
        self.assertTrue(bbox.isEmpty())

    def test_parse_crs(self):
        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/def/crs/OGC/1.3/CRS84')
        self.assertTrue(crs.isValid())
        self.assertEquals(crs.postgisSrid(), 4326)

        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/def/crs/EPSG/9.6.2/32632')
        self.assertTrue(crs.isValid())
        self.assertEquals(crs.postgisSrid(), 32632)

        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/something_wrong_here')
        self.assertFalse(crs.isValid())


class API(QgsServerApi):

    def __init__(self, iface, version='1.0'):
        super().__init__(iface)
        self._version = version

    def name(self):
        return "TEST"

    def version(self):
        return self._version

    def rootPath(self):
        return "/testapi"

    def executeRequest(self, request_context):
        request_context.response().write(b"\"Test API\"")


class QgsServerAPITest(QgsServerTestBase):
    """ QGIS API server tests"""

    # Set to True in child classes to re-generate reference files for this class
    regeregenerate_api_reference = False

    def dump(self, response):
        """Returns the response body as str"""

        result = []
        for n, v in response.headers().items():
            if n == 'Content-Length':
                continue
            result.append("%s: %s" % (n, v))
        result.append('')
        result.append(bytes(response.body()).decode('utf8'))
        return '\n'.join(result)

    def compareApi(self, request, project, reference_file):
        response = QgsBufferServerResponse()
        # Add json to accept it reference_file is JSON
        if reference_file.endswith('.json'):
            request.setHeader('Accept', 'application/json')
        self.server.handleRequest(request, response, project)
        result = bytes(response.body()).decode('utf8') if reference_file.endswith('html') else self.dump(response)
        path = unitTestDataPath('qgis_server') + '/api/' + reference_file
        if self.regeregenerate_api_reference:
            f = open(path.encode('utf8'), 'w+', encoding='utf8')
            f.write(result)
            f.close()
            print("Reference file %s regenerated!" % path.encode('utf8'))

        def __normalize_json(content):
            reference_content = content.split('\n')
            j = ''.join(reference_content[reference_content.index('') + 1:])
            # Do not test timeStamp
            j = json.loads(j)
            try:
                j['timeStamp'] = '2019-07-05T12:27:07Z'
            except:
                pass
            json_content = json.dumps(j)
            headers_content = '\n'.join(reference_content[:reference_content.index('') + 1])
            return headers_content + '\n' + json_content

        with open(path.encode('utf8'), 'r', encoding='utf8') as f:
            if reference_file.endswith('json'):
                self.assertEqual(__normalize_json(result), __normalize_json(f.read()))
            else:
                self.assertEqual(f.read(), result)

        return response

    def compareContentType(self, url, headers, content_type):
        request = QgsBufferServerRequest(url, headers=headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, QgsProject())
        self.assertEqual(response.headers()['Content-Type'], content_type)

    @classmethod
    def setUpClass(cls):
        super(QgsServerAPITest, cls).setUpClass()
        cls.maxDiff = None

    def test_api(self):
        """Test API registering"""

        api = API(self.server.serverInterface())
        self.server.serverInterface().serviceRegistry().registerApi(api)
        request = QgsBufferServerRequest('http://server.qgis.org/testapi')
        self.compareApi(request, None, 'test_api.json')
        self.server.serverInterface().serviceRegistry().unregisterApi(api.name())

    def test_0_version_registration(self):

        reg = QgsServiceRegistry()
        api = API(self.server.serverInterface())
        api1 = API(self.server.serverInterface(), '1.1')

        # 1.1 comes first
        reg.registerApi(api1)
        reg.registerApi(api)

        rapi = reg.getApi("TEST")
        self.assertIsNotNone(rapi)
        self.assertEqual(rapi.version(), "1.1")

        rapi = reg.getApi("TEST", "2.0")
        self.assertIsNotNone(rapi)
        self.assertEqual(rapi.version(), "1.1")

        rapi = reg.getApi("TEST", "1.0")
        self.assertIsNotNone(rapi)
        self.assertEqual(rapi.version(), "1.0")

    def test_1_unregister_services(self):

        reg = QgsServiceRegistry()
        api = API(self.server.serverInterface(), '1.0a')
        api1 = API(self.server.serverInterface(), '1.0b')
        api2 = API(self.server.serverInterface(), '1.0c')

        reg.registerApi(api)
        reg.registerApi(api1)
        reg.registerApi(api2)

        # Check we get the default version
        rapi = reg.getApi("TEST")
        self.assertEqual(rapi.version(), "1.0a")

        # Remove one service
        removed = reg.unregisterApi("TEST", "1.0a")
        self.assertEqual(removed, 1)

        # Check that we get the highest version
        rapi = reg.getApi("TEST")
        self.assertEqual(rapi.version(), "1.0c")

        # Remove all services
        removed = reg.unregisterApi("TEST")
        self.assertEqual(removed, 2)

        # Check that there is no more services available
        api = reg.getApi("TEST")
        self.assertIsNone(api)

    def test_wfs3_landing_page(self):
        """Test WFS3 API landing page in HTML format"""

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3.html')
        self.compareApi(request, None, 'test_wfs3_landing_page.html')

    def test_content_type_negotiation(self):
        """Test content-type negotiation and conflicts"""

        # Default: json
        self.compareContentType('http://server.qgis.org/wfs3', {}, 'application/json')
        # Explicit request
        self.compareContentType('http://server.qgis.org/wfs3', {'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'}, 'text/html')
        self.compareContentType('http://server.qgis.org/wfs3', {'Accept': 'application/json'}, 'application/json')
        # File suffix
        self.compareContentType('http://server.qgis.org/wfs3.json', {}, 'application/json')
        self.compareContentType('http://server.qgis.org/wfs3.html', {}, 'text/html')
        # File extension must take precedence over Accept header
        self.compareContentType('http://server.qgis.org/wfs3.html', {'Accept': 'application/json'}, 'text/html')
        self.compareContentType('http://server.qgis.org/wfs3.json', {'Accept': 'text/html'}, 'application/json')

    def test_wfs3_landing_page_json(self):
        """Test WFS3 API landing page in JSON format"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3.json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3')
        request.setHeader('Accept', 'application/json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')

    def test_wfs3_api(self):
        """Test WFS3 API"""

        self.compareContentType('http://server.qgis.org/wfs3/api', {}, 'application/openapi+json;version=3.0')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/api.openapi3')
        self.compareApi(request, None, 'test_wfs3_api.json')

    def test_wfs3_conformance(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/conformance')
        self.compareApi(request, None, 'test_wfs3_conformance.json')

    def test_wfs3_collections_empty(self):
        """Test WFS3 collections API"""

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections.json')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections.html')
        self.compareApi(request, None, 'test_wfs3_collections_empty.html')

    def test_wfs3_collections_json(self):
        """Test WFS3 API collections in json format"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections.json')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.json')

    def test_wfs3_collections_html(self):
        """Test WFS3 API collections in html format"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections.html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.html')

    def test_wfs3_collections_content_type(self):
        """Test WFS3 API collections in html format with Accept header"""

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections')
        request.setHeader('Accept', 'text/html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.headers()['Content-Type'], 'text/html')

    def test_wfs3_collection_items(self):
        """Test WFS3 API items"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé.json')

    def test_invalid_args(self):
        """Test wrong args"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Limit is not valid (0-10000)"}]') # Bad request

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=10001')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Limit is not valid (0-10000)"}]') # Bad request

    def test_wfs3_collection_items_limit(self):
        """Test WFS3 API item limits"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1.json')

    def test_wfs3_collection_items_limit_offset(self):
        """Test WFS3 API offset"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1&offset=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1_offset_1.json')

    def test_wfs3_collection_items_bbox(self):
        """Test WFS3 API bbox"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox.json')

        # Test with a different CRS
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/32632', safe='')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=437106.99096772138727829,4972307.72834488749504089,437129.50390358484582976,4972318.1108037903904914&bbox-crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox_32632.json')

    def test_wfs3_static_handler(self):
        """Test static handler"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/static/style.css')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        body = bytes(response.body()).decode('utf8')
        self.assertTrue('Content-Length' in response.headers())
        self.assertEqual(response.headers()['Content-Type'], 'text/css')
        self.assertTrue(len(body) > 0)

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/static/does_not_exists.css')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        body = bytes(response.body()).decode('utf8')
        self.assertEqual(body, '[{"code":"API not found error","description":"Static file does_not_exists.css was not found"}]')

    def test_wfs3_field_filters(self):
        """Test field filters"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer3/items?name=two')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer3_name_eq_two.json')

    def test_wfs3_field_filters_star(self):
        """Test field filters"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer3/items?name=tw*')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer3_name_eq_tw_star.json')


if __name__ == '__main__':
    unittest.main()
