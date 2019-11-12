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
    QgsServerApiBadRequestException,
    QgsServerQueryStringParameter,
    QgsServerApiContext,
    QgsServerOgcApi,
    QgsServerOgcApiHandler,
    QgsServerApiUtils,
    QgsServiceRegistry
)
from qgis.core import QgsProject, QgsRectangle
from qgis.PyQt import QtCore

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

        bbox = QgsServerApiUtils.parseBbox('8.203495,44.901482,100,8.203497,44.901484,120')
        self.assertEquals(bbox.xMinimum(), 8.203495)
        self.assertEquals(bbox.yMinimum(), 44.901482)
        self.assertEquals(bbox.xMaximum(), 8.203497)
        self.assertEquals(bbox.yMaximum(), 44.901484)

        bbox = QgsServerApiUtils.parseBbox('something_wrong_here')
        self.assertTrue(bbox.isEmpty())
        bbox = QgsServerApiUtils.parseBbox('8.203495,44.901482,8.203497,something_wrong_here')
        self.assertTrue(bbox.isEmpty())

    def test_published_crs(self):
        """Test published WMS CRSs"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        crss = QgsServerApiUtils.publishedCrsList(project)
        self.assertTrue('http://www.opengis.net/def/crs/OGC/1.3/CRS84' in crss)
        self.assertTrue('http://www.opengis.net/def/crs/EPSG/9.6.2/3857' in crss)
        self.assertTrue('http://www.opengis.net/def/crs/EPSG/9.6.2/4326' in crss)

    def test_parse_crs(self):

        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/def/crs/OGC/1.3/CRS84')
        self.assertTrue(crs.isValid())
        self.assertEquals(crs.postgisSrid(), 4326)

        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/def/crs/EPSG/9.6.2/3857')
        self.assertTrue(crs.isValid())
        self.assertEquals(crs.postgisSrid(), 3857)

        crs = QgsServerApiUtils.parseCrs('http://www.opengis.net/something_wrong_here')
        self.assertFalse(crs.isValid())

    def test_append_path(self):

        path = QgsServerApiUtils.appendMapParameter('/wfs3', QtCore.QUrl('https://www.qgis.org/wfs3?MAP=/some/path'))
        self.assertEquals(path, '/wfs3?MAP=/some/path')


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


class QgsServerAPITestBase(QgsServerTestBase):
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

    def compareContentType(self, url, headers, content_type, project=QgsProject()):
        request = QgsBufferServerRequest(url, headers=headers)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.headers()['Content-Type'], content_type)

    @classmethod
    def setUpClass(cls):
        super(QgsServerAPITestBase, cls).setUpClass()
        cls.maxDiff = None


class QgsServerAPITest(QgsServerAPITestBase):
    """ QGIS API server tests"""

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
        # Alias request (we ask for json but we get geojson)
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareContentType(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484',
            {'Accept': 'application/json'}, 'application/geo+json',
            project=project
        )
        self.compareContentType(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484',
            {'Accept': 'application/vnd.geo+json'}, 'application/geo+json',
            project=project
        )
        self.compareContentType(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484',
            {'Accept': 'application/geojson'}, 'application/geo+json',
            project=project
        )

    def test_wfs3_landing_page_json(self):
        """Test WFS3 API landing page in JSON format"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3.json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3')
        request.setHeader('Accept', 'application/json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')

    def test_wfs3_api(self):
        """Test WFS3 API"""

        # No project: error
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/api.openapi3')
        self.compareApi(request, None, 'test_wfs3_api.json')

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/api.openapi3')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_api_project.json')

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
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.json')

    def test_wfs3_collections_html(self):
        """Test WFS3 API collections in html format"""
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections.html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.html')

    def test_wfs3_collections_content_type(self):
        """Test WFS3 API collections in html format with Accept header"""

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections')
        request.setHeader('Accept', 'text/html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.headers()['Content-Type'], 'text/html')

    def test_wfs3_collection_json(self):
        """Test WFS3 API collection"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé')
        self.compareApi(request, project, 'test_wfs3_collection_testlayer_èé.json')

    def test_wfs3_collection_html(self):
        """Test WFS3 API collection"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé.html')
        self.compareApi(request, project, 'test_wfs3_collection_testlayer_èé.html')

    def test_wfs3_collection_items(self):
        """Test WFS3 API items"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé.json')

    def test_wfs3_collection_items_crs(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_crs_3857.json')

    def test_wfs3_collection_items_as_areas_crs_4326(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_wms_grouped_nested_layers.qgs')
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/4326', safe='')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/as-areas-short-name/items?crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_as-areas-short-name_4326.json')

    def test_wfs3_collection_items_as_areas_crs_3857(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_wms_grouped_nested_layers.qgs')
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/as-areas-short-name/items?crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_as-areas-short-name_3857.json')

    def test_invalid_args(self):
        """Test wrong args"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]') # Bad request

        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=10001')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]') # Bad request

    def test_wfs3_collection_items_limit(self):
        """Test WFS3 API item limits"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1.json')

    def test_wfs3_collection_items_limit_offset(self):
        """Test WFS3 API offset"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1&offset=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1_offset_1.json')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1&offset=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Argument \'offset\' is not valid. Offset for features to retrieve [0-3]"}]') # Bad request
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1&offset=1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]') # Bad request

    def test_wfs3_collection_items_bbox(self):
        """Test WFS3 API bbox"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox.json')

        # Test with a different CRS
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=913191,5606014,913234,5606029&bbox-crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox_3857.json')

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
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        # Check not published
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer3/items?name=two')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 404) # Not found
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?name=two')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        self.compareApi(request, project, 'test_wfs3_collections_items_layer1_with_short_name_eq_two.json')

    def test_wfs3_field_filters_star(self):
        """Test field filters"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?name=tw*')
        response = self.compareApi(request, project, 'test_wfs3_collections_items_layer1_with_short_name_eq_tw_star.json')
        self.assertEqual(response.statusCode(), 200)

    def test_wfs3_excluded_attributes(self):
        """Test excluded attributes"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/exclude_attribute/items/0.geojson')
        response = self.compareApi(request, project, 'test_wfs3_collections_items_exclude_attribute_0.json')
        self.assertEqual(response.statusCode(), 200)


class Handler1(QgsServerOgcApiHandler):

    def path(self):
        return QtCore.QRegularExpression("/handlerone")

    def operationId(self):
        return "handlerOne"

    def summary(self):
        return "First of its name"

    def description(self):
        return "The first handler ever"

    def linkTitle(self):
        return "Handler One Link Title"

    def linkType(self):
        return QgsServerOgcApi.data

    def handleRequest(self, context):
        """Simple mirror: returns the parameters"""

        params = self.values(context)
        self.write(params, context)

    def parameters(self, context):
        return [QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value')]


class Handler2(QgsServerOgcApiHandler):

    def path(self):
        return QtCore.QRegularExpression(r"/handlertwo/(?P<code1>\d{2})/(\d{3})")

    def operationId(self):
        return "handlerTwo"

    def summary(self):
        return "Second of its name"

    def description(self):
        return "The second handler ever"

    def linkTitle(self):
        return "Handler Two Link Title"

    def linkType(self):
        return QgsServerOgcApi.data

    def handleRequest(self, context):
        """Simple mirror: returns the parameters"""

        params = self.values(context)
        self.write(params, context)

    def parameters(self, context):
        return [QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value'),
                QgsServerQueryStringParameter('value2', False, QgsServerQueryStringParameter.Type.String, 'a string value'), ]


class Handler3(QgsServerOgcApiHandler):
    """Custom content types: only accept JSON"""

    templatePathOverride = None

    def __init__(self):
        super(Handler3, self).__init__()
        self.setContentTypes([QgsServerOgcApi.JSON])

    def path(self):
        return QtCore.QRegularExpression(r"/handlerthree")

    def operationId(self):
        return "handlerThree"

    def summary(self):
        return "Third of its name"

    def description(self):
        return "The third handler ever"

    def linkTitle(self):
        return "Handler Three Link Title"

    def linkType(self):
        return QgsServerOgcApi.data

    def handleRequest(self, context):
        """Simple mirror: returns the parameters"""

        params = self.values(context)
        self.write(params, context)

    def parameters(self, context):
        return [QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value')]

    def templatePath(self, context):
        if self.templatePathOverride is None:
            return super(Handler3, self).templatePath(context)
        else:
            return self.templatePathOverride


class QgsServerOgcAPITest(QgsServerAPITestBase):
    """ QGIS OGC API server tests"""

    def testOgcApi(self):
        """Test OGC API"""

        api = QgsServerOgcApi(self.server.serverInterface(), '/api1', 'apione', 'an api', '1.1')
        self.assertEqual(api.name(), 'apione')
        self.assertEqual(api.description(), 'an api')
        self.assertEqual(api.version(), '1.1')
        self.assertEqual(api.rootPath(), '/api1')
        url = 'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1'
        self.assertEqual(api.sanitizeUrl(QtCore.QUrl(url)).toString(), 'http://server.qgis.org/wfs3/collections/testlayer \xe8\xe9/items?limit=-1')
        self.assertEqual(api.sanitizeUrl(QtCore.QUrl('/path//double//slashes//#fr')).toString(), '/path/double/slashes#fr')
        self.assertEqual(api.relToString(QgsServerOgcApi.data), 'data')
        self.assertEqual(api.relToString(QgsServerOgcApi.alternate), 'alternate')
        self.assertEqual(api.contentTypeToString(QgsServerOgcApi.JSON), 'JSON')
        self.assertEqual(api.contentTypeToStdString(QgsServerOgcApi.JSON), 'JSON')
        self.assertEqual(api.contentTypeToExtension(QgsServerOgcApi.JSON), 'json')
        self.assertEqual(api.contentTypeToExtension(QgsServerOgcApi.GEOJSON), 'geojson')

    def testOgcApiHandler(self):
        """Test OGC API Handler"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()

        ctx = QgsServerApiContext('/services/api1', request, response, project, self.server.serverInterface())
        h = Handler1()
        self.assertTrue(h.staticPath(ctx).endswith('/resources/server/api/ogc/static'))
        self.assertEqual(h.path(), QtCore.QRegularExpression("/handlerone"))
        self.assertEqual(h.description(), 'The first handler ever')
        self.assertEqual(h.operationId(), 'handlerOne')
        self.assertEqual(h.summary(), 'First of its name')
        self.assertEqual(h.linkTitle(), 'Handler One Link Title')
        self.assertEqual(h.linkType(), QgsServerOgcApi.data)
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            h.handleRequest(ctx)
        self.assertEqual(str(ex.exception), 'Missing required argument: \'value1\'')

        r = ctx.response()
        self.assertEqual(r.data(), '')

        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            h.values(ctx)
        self.assertEqual(str(ex.exception), 'Missing required argument: \'value1\'')

        # Add handler to API and test for /api2
        ctx = QgsServerApiContext('/services/api2', request, response, project, self.server.serverInterface())
        api = QgsServerOgcApi(self.server.serverInterface(), '/api2', 'apitwo', 'a second api', '1.2')
        api.registerHandler(h)
        # Add a second handler (will be tested later)
        h2 = Handler2()
        api.registerHandler(h2)

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api1'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Requested URI does not match any registered API handler')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Requested URI does not match any registered API handler')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlerone'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Missing required argument: \'value1\'')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlerone?value1=not+a+double'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Argument \'value1\' could not be converted to Double')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlerone?value1=1.2345'))
        params = h.values(ctx)
        self.assertEqual(params, {'value1': 1.2345})
        api.executeRequest(ctx)
        self.assertEqual(json.loads(bytes(ctx.response().data()))['value1'], 1.2345)

        # Test path fragments extraction
        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345'))
        params = h2.values(ctx)
        self.assertEqual(params, {'code1': '00', 'value1': 1.2345, 'value2': None})

        # Test string encoding
        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345&value2=a%2Fstring%20some'))
        params = h2.values(ctx)
        self.assertEqual(params, {'code1': '00', 'value1': 1.2345, 'value2': 'a/string some'})

        # Test links
        self.assertEqual(h2.href(ctx), 'http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345&value2=a%2Fstring%20some')
        self.assertEqual(h2.href(ctx, '/extra'), 'http://www.qgis.org/services/api2/handlertwo/00/555/extra?value1=1.2345&value2=a%2Fstring%20some')
        self.assertEqual(h2.href(ctx, '/extra', 'json'), 'http://www.qgis.org/services/api2/handlertwo/00/555/extra.json?value1=1.2345&value2=a%2Fstring%20some')

        # Test template path
        self.assertTrue(h2.templatePath(ctx).endswith('/resources/server/api/ogc/templates/services/api2/handlerTwo.html'))

    def testOgcApiHandlerContentType(self):
        """Test OGC API Handler content types"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('http://server.qgis.org/api3/handlerthree?value1=9.5')
        response = QgsBufferServerResponse()

        # Add handler to API and test for /api3
        ctx = QgsServerApiContext('/services/api3', request, response, project, self.server.serverInterface())
        api = QgsServerOgcApi(self.server.serverInterface(), '/api3', 'apithree', 'a third api', '1.2')
        h3 = Handler3()
        api.registerHandler(h3)

        ctx = QgsServerApiContext('/services/api3/', request, response, project, self.server.serverInterface())
        api.executeRequest(ctx)
        self.assertEqual(json.loads(bytes(ctx.response().data()))['value1'], 9.5)

        # Call HTML
        ctx.request().setUrl(QtCore.QUrl('http://server.qgis.org/api3/handlerthree.html?value1=9.5'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Unsupported Content-Type: HTML')

        h3.setContentTypes([QgsServerOgcApi.HTML])
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Template not found: handlerThree.html')

        # Define a template path
        dir = QtCore.QTemporaryDir()
        with open(dir.path() + '/handlerThree.html', 'w+') as f:
            f.write("Hello world")
        h3.templatePathOverride = dir.path() + '/handlerThree.html'
        ctx.response().clear()
        api.executeRequest(ctx)
        self.assertEqual(bytes(ctx.response().data()), b"Hello world")


if __name__ == '__main__':
    unittest.main()
