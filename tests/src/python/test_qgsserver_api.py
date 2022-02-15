# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer API.

From build dir, run: ctest -R PyQgsServerApi -V

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
import shutil

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerApi,
    QgsServerApiBadRequestException,
    QgsServerQueryStringParameter,
    QgsServerApiContext,
    QgsServerOgcApi,
    QgsServerOgcApiHandler,
    QgsServerApiUtils,
    QgsServiceRegistry
)

from qgis.core import (
    QgsProject,
    QgsRectangle,
    QgsVectorLayerServerProperties,
    QgsFeatureRequest,
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
)

from qgis.PyQt import QtCore

from qgis.testing import unittest
from utilities import unitTestDataPath
from urllib import parse

import tempfile

from test_qgsserver import QgsServerTestBase


class QgsServerAPIUtilsTest(QgsServerTestBase):
    """ QGIS API server utils tests"""

    def test_parse_bbox(self):
        bbox = QgsServerApiUtils.parseBbox(
            '8.203495,44.901482,8.203497,44.901484')
        self.assertEqual(bbox.xMinimum(), 8.203495)
        self.assertEqual(bbox.yMinimum(), 44.901482)
        self.assertEqual(bbox.xMaximum(), 8.203497)
        self.assertEqual(bbox.yMaximum(), 44.901484)

        bbox = QgsServerApiUtils.parseBbox(
            '8.203495,44.901482,100,8.203497,44.901484,120')
        self.assertEqual(bbox.xMinimum(), 8.203495)
        self.assertEqual(bbox.yMinimum(), 44.901482)
        self.assertEqual(bbox.xMaximum(), 8.203497)
        self.assertEqual(bbox.yMaximum(), 44.901484)

        bbox = QgsServerApiUtils.parseBbox('something_wrong_here')
        self.assertTrue(bbox.isEmpty())
        bbox = QgsServerApiUtils.parseBbox(
            '8.203495,44.901482,8.203497,something_wrong_here')
        self.assertTrue(bbox.isEmpty())

    def test_published_crs(self):
        """Test published WMS CRSs"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        crss = QgsServerApiUtils.publishedCrsList(project)
        self.assertTrue('http://www.opengis.net/def/crs/OGC/1.3/CRS84' in crss)
        self.assertTrue(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/3857' in crss)
        self.assertTrue(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/4326' in crss)

    def test_parse_crs(self):
        crs = QgsServerApiUtils.parseCrs(
            'http://www.opengis.net/def/crs/OGC/1.3/CRS84')
        self.assertTrue(crs.isValid())

        crs = QgsServerApiUtils.parseCrs(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/4326')
        self.assertEqual(crs.postgisSrid(), 4326)

        crs = QgsServerApiUtils.parseCrs(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/3857')
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.postgisSrid(), 3857)

        crs = QgsServerApiUtils.parseCrs(
            'http://www.opengis.net/something_wrong_here')
        self.assertFalse(crs.isValid())

    def test_append_path(self):
        path = QgsServerApiUtils.appendMapParameter(
            '/wfs3', QtCore.QUrl('https://www.qgis.org/wfs3?MAP=/some/path'))
        self.assertEqual(path, '/wfs3?MAP=/some/path')

    def test_temporal_extent(self):
        project = QgsProject()

        tempDir = QtCore.QTemporaryDir()
        source_project_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.qgs'
        source_data_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.gpkg'
        dest_project_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.qgs')
        dest_data_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.gpkg')
        shutil.copy(source_data_path, dest_data_path)
        shutil.copy(source_project_path, dest_project_path)
        project.read(dest_project_path)

        layer = list(project.mapLayers().values())[0]

        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('time', 'updated_string')))
        self.assertEqual(QgsServerApiUtils.temporalExtent(layer), [
                         ['2010-01-01T01:01:01', '2020-01-01T01:01:01']])

        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'created')))
        self.assertEqual(QgsServerApiUtils.temporalExtent(layer), [
                         ['2010-01-01T00:00:00', '2019-01-01T00:00:00']])

        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'created_string')))
        self.assertEqual(QgsServerApiUtils.temporalExtent(layer), [
                         ['2010-01-01T00:00:00', '2019-01-01T00:00:00']])

        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('time', 'updated')))
        self.assertEqual(QgsServerApiUtils.temporalExtent(layer), [
                         ['2010-01-01T01:01:01', '2022-01-01T01:01:01']])

        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'begin', 'end')))
        self.assertEqual(QgsServerApiUtils.temporalExtent(layer), [
                         ['2010-01-01T00:00:00', '2022-01-01T00:00:00']])


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

    def assertEqualBrackets(self, actual, expected):
        """Also counts parenthesis"""

        self.assertEqual(actual.count('('), actual.count(')'))
        self.assertEqual(actual, expected)

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

    def assertLinesEqual(self, actual, expected, reference_file):
        """Break on first different line"""

        actual_lines = actual.split('\n')
        expected_lines = expected.split('\n')
        for i in range(len(actual_lines)):
            self.assertEqual(actual_lines[i], expected_lines[i], "File: %s\nLine: %s\nActual  : %s\nExpected: %s" % (
                reference_file, i, actual_lines[i], expected_lines[i]))

    def normalize_json(self, content):
        """Normalize a json string"""

        reference_content = content.split('\n')
        j = ''.join(reference_content[reference_content.index('') + 1:])
        # Do not test timeStamp
        j = json.loads(j)
        try:
            j['timeStamp'] = '2019-07-05T12:27:07Z'
        except:
            pass
        # Fix coordinate precision differences in Travis
        try:
            bbox = j['extent']['spatial']['bbox'][0]
            bbox = [round(c, 4) for c in bbox]
            j['extent']['spatial']['bbox'][0] = bbox
        except:
            pass
        json_content = json.dumps(j, indent=4)
        # Rounding errors
        json_content = re.sub(r'(\d{5})\d+\.\d+', r'\1', json_content)
        json_content = re.sub(r'(\d+\.\d{4})\d+', r'\1', json_content)
        # Poject hash
        json_content = re.sub(
            r'[a-f0-9]{32}', r'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF', json_content)
        headers_content = '\n'.join(
            reference_content[:reference_content.index('') + 1])
        return headers_content + '\n' + json_content

    def compareApi(self, request, project, reference_file, subdir='api'):
        response = QgsBufferServerResponse()
        # Add json to accept it reference_file is JSON
        if reference_file.endswith('.json'):
            request.setHeader('Accept', 'application/json')
        self.server.handleRequest(request, response, project)
        result = bytes(response.body()).decode(
            'utf8') if reference_file.endswith('html') else self.dump(response)
        path = os.path.join(unitTestDataPath(
            'qgis_server'), subdir, reference_file)
        if self.regeregenerate_api_reference:
            # Try to change timestamp
            try:
                content = result.split('\n')
                j = ''.join(content[content.index('') + 1:])
                j = json.loads(j)
                j['timeStamp'] = '2019-07-05T12:27:07Z'
                result = '\n'.join(content[:2]) + '\n' + \
                    json.dumps(j, ensure_ascii=False, indent=2)
            except:
                pass
            f = open(path.encode('utf8'), 'w+', encoding='utf8')
            f.write(result)
            f.close()
            print("Reference file %s regenerated!" % path.encode('utf8'))

        with open(path.encode('utf8'), 'r', encoding='utf8') as f:
            if reference_file.endswith('json'):
                self.assertLinesEqual(self.normalize_json(
                    result), self.normalize_json(f.read()), path.encode('utf8'))
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
        self.compareContentType(
            'http://server.qgis.org/wfs3', {}, 'application/json')
        # Explicit request
        self.compareContentType('http://server.qgis.org/wfs3',
                                {'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'},
                                'text/html')
        self.compareContentType('http://server.qgis.org/wfs3',
                                {'Accept': 'application/json'}, 'application/json')
        # File suffix
        self.compareContentType(
            'http://server.qgis.org/wfs3.json', {}, 'application/json')
        self.compareContentType(
            'http://server.qgis.org/wfs3.html', {}, 'text/html')
        # File extension must take precedence over Accept header
        self.compareContentType(
            'http://server.qgis.org/wfs3.html', {'Accept': 'application/json'}, 'text/html')
        self.compareContentType(
            'http://server.qgis.org/wfs3.json', {'Accept': 'text/html'}, 'application/json')
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
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/api.openapi3')
        self.compareApi(request, None, 'test_wfs3_api.json')

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/api.openapi3')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_api_project.json')

    def test_wfs3_conformance(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/conformance')
        self.compareApi(request, None, 'test_wfs3_conformance.json')

    def test_wfs3_collections_empty(self):
        """Test WFS3 collections API"""

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections.json')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections.html')
        self.compareApi(request, None, 'test_wfs3_collections_empty.html')

    def test_wfs3_collections_json(self):
        """Test WFS3 API collections in json format"""
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections.json')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.json')

    def test_wfs3_collections_html(self):
        """Test WFS3 API collections in html format"""
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections.html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.html')

    def test_wfs3_collections_content_type(self):
        """Test WFS3 API collections in html format with Accept header"""

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections')
        request.setHeader('Accept', 'text/html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.headers()['Content-Type'], 'text/html')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections')
        request.setHeader('Accept', 'text/html')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.headers()['Content-Type'], 'text/html')

    def test_wfs3_collection_json(self):
        """Test WFS3 API collection"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé')
        self.compareApi(request, project,
                        'test_wfs3_collection_testlayer_èé.json')

    def test_wfs3_collection_temporal_extent_json(self):
        """Test collection with timefilter"""
        project = QgsProject()
        tempDir = QtCore.QTemporaryDir()
        source_project_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.qgs'
        source_data_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.gpkg'
        dest_project_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.qgs')
        dest_data_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.gpkg')
        shutil.copy(source_data_path, dest_data_path)
        shutil.copy(source_project_path, dest_project_path)
        project.read(dest_project_path)
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/points')
        self.compareApi(request, project,
                        'test_wfs3_collection_points_timefilters.json')

    def test_wfs3_collection_html(self):
        """Test WFS3 API collection"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé.html')
        self.compareApi(request, project,
                        'test_wfs3_collection_testlayer_èé.html')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/')
        request.setHeader('Accept', 'text/html')
        self.compareApi(request, project,
                        'test_wfs3_collection_testlayer_èé.html')

    def test_wfs3_collection_items(self):
        """Test WFS3 API items"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items')
        self.compareApi(request, project,
                        'test_wfs3_collections_items_testlayer_èé.json')

    def test_wfs3_collection_items_html(self):
        """Test WFS3 API items"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items.html')
        self.compareApi(request, project,
                        'test_wfs3_collections_items_testlayer_èé.html')

    def test_wfs3_collection_items_crs(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        encoded_crs = parse.quote(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?crs={}'.format(encoded_crs))
        self.compareApi(
            request, project, 'test_wfs3_collections_items_testlayer_èé_crs_3857.json')

    def test_wfs3_collection_items_as_areas_crs_4326(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') +
                     '/test_project_wms_grouped_nested_layers.qgs')
        encoded_crs = parse.quote(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/4326', safe='')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/as-areas-short-name/items?crs={}'.format(encoded_crs))
        self.compareApi(
            request, project, 'test_wfs3_collections_items_as-areas-short-name_4326.json')

    def test_wfs3_collection_items_as_areas_crs_3857(self):
        """Test WFS3 API items with CRS"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') +
                     '/test_project_wms_grouped_nested_layers.qgs')
        encoded_crs = parse.quote(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/as-areas-short-name/items?crs={}'.format(encoded_crs))
        self.compareApi(
            request, project, 'test_wfs3_collections_items_as-areas-short-name_3857.json')

    def test_invalid_args(self):
        """Test wrong args"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)  # Bad request
        self.assertEqual(response.body(),
                         b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]')  # Bad request

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=10001')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)  # Bad request
        self.assertEqual(response.body(),
                         b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]')  # Bad request

    def test_wfs3_collection_items_limit(self):
        """Test WFS3 API item limits"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1')
        self.compareApi(
            request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1.json')

    def test_wfs3_collection_items_limit_offset(self):
        """Test WFS3 API offset"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1&offset=1')
        self.compareApi(
            request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1_offset_1.json')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=1&offset=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)  # Bad request
        self.assertEqual(response.body(),
                         b'[{"code":"Bad request error","description":"Argument \'offset\' is not valid. Offset for features to retrieve [0-3]"}]')  # Bad request
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1&offset=1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)  # Bad request
        self.assertEqual(response.body(),
                         b'[{"code":"Bad request error","description":"Argument \'limit\' is not valid. Number of features to retrieve [0-10000]"}]')  # Bad request

    def test_wfs3_collection_items_bbox(self):
        """Test WFS3 API bbox"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484')
        self.compareApi(request, project,
                        'test_wfs3_collections_items_testlayer_èé_bbox.json')

        # Test with a different CRS
        encoded_crs = parse.quote(
            'http://www.opengis.net/def/crs/EPSG/9.6.2/3857', safe='')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?bbox=913191,5606014,913234,5606029&bbox-crs={}'.format(
                encoded_crs))
        self.compareApi(
            request, project, 'test_wfs3_collections_items_testlayer_èé_bbox_3857.json')

    def test_wfs3_collection_items_bbox_25832(self):
        """Test WFS3 API bbox with reprojection"""

        project = QgsProject()
        vl = QgsVectorLayer("Point?crs=EPSG:25832&field=fldint:integer",
                            "testlayer25832", "memory")

        f = QgsFeature(vl.fields())
        f.setAttribute(0, 1)
        f.setGeometry(QgsGeometry.fromWkt('point(361774 4963545)'))
        vl.dataProvider().addFeatures((f,))
        project.addMapLayers([vl])

        project.writeEntry("WFSLayers", "/", (vl.id(),))
        project.writeEntry("WMSCrsList", "/", ("EPSG:25832", "EPSG:4326",))

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer25832/items?bbox=7.16305252070271603,44.75906320523620963,7.3418755610416051,44.87555723151492515')

        self.compareApi(request, project,
                        'test_wfs3_collections_items_testlayer25832_bbox.json')

    def test_wfs3_static_handler(self):
        """Test static handler"""
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/static/style.css')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        body = bytes(response.body()).decode('utf8')
        self.assertTrue('Content-Length' in response.headers())
        self.assertEqual(response.headers()['Content-Type'], 'text/css')
        self.assertTrue(len(body) > 0)

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/static/does_not_exists.css')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        body = bytes(response.body()).decode('utf8')
        self.assertEqual(body,
                         '[{"code":"API not found error","description":"Static file does_not_exists.css was not found"}]')

    def test_wfs3_collection_items_post(self):
        """Test WFS3 API items POST"""

        tmpDir = QtCore.QTemporaryDir()
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.qgs',
                    tmpDir.path() + '/test_project_api_editing.qgs')
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.gpkg',
                    tmpDir.path() + '/test_project_api_editing.gpkg')

        project = QgsProject()
        project.read(tmpDir.path() + '/test_project_api_editing.qgs')

        # Project layers with different permissions
        insert_layer = r'test%20layer%20èé%203857%20published%20insert'
        update_layer = r'test%20layer%20èé%203857%20published%20update'
        delete_layer = r'test%20layer%20èé%203857%20published%20delete'
        unpublished_layer = r'test%20layer%203857%20èé%20unpublished'
        hidden_text_2_layer = r'test%20layer%20èé%203857%20published%20hidden%20text_2'

        # Invalid request
        data = b'not json!'
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items' % insert_layer,
                                         QgsBufferServerRequest.PostMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        self.assertTrue(
            '[{"code":"Bad request error","description":"JSON parse error' in bytes(response.body()).decode('utf8'))

        # Valid request
        data = """{
        "geometry": {
            "coordinates": [[
            7.247,
            44.814
            ]],
            "type": "MultiPoint"
        },
        "properties": {
            "text_1": "Text 1",
            "text_2": "Text 2",
            "int_1": 123,
            "float_1": 12345.678,
            "datetime_1": "2019-11-07T12:34:56",
            "date_1": "2019-11-07",
            "blob_1": "dGVzdA==",
            "bool_1": true
        },
        "type": "Feature"
        }""".encode('utf8')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items' % insert_layer,
                                         QgsBufferServerRequest.PostMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 201)
        self.assertEqual(response.body(), '"string"')
        # Get last feature
        req = QgsFeatureRequest()
        order_by_clause = QgsFeatureRequest.OrderByClause('$id', False)
        req.setOrderBy(QgsFeatureRequest.OrderBy([order_by_clause]))
        feature = next(project.mapLayersByName(
            'test layer èé 3857 published insert')[0].getFeatures(req))
        self.assertEqual(response.headers()['Location'],
                         'http://server.qgis.org/wfs3/collections/%s/items/%s' % (insert_layer, feature.id()))
        self.assertEqual(feature.attribute('text_1'), 'Text 1')
        self.assertEqual(feature.attribute('text_2'), 'Text 2')
        self.assertEqual(feature.attribute('int_1'), 123)
        self.assertEqual(feature.attribute('float_1'), 12345.678)
        self.assertEqual(feature.attribute('bool_1'), True)
        self.assertEqual(bytes(feature.attribute('blob_1')), b"test")
        self.assertEqual(re.sub(
            r'\.\d+', '', feature.geometry().asWkt().upper()), 'MULTIPOINT ((806732 5592286))')

    def test_wfs3_collection_items_put(self):
        """Test WFS3 API items PUT"""

        tmpDir = QtCore.QTemporaryDir()
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.qgs',
                    tmpDir.path() + '/test_project_api_editing.qgs')
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.gpkg',
                    tmpDir.path() + '/test_project_api_editing.gpkg')

        project = QgsProject()
        project.read(tmpDir.path() + '/test_project_api_editing.qgs')

        # Project layers with different permissions
        insert_layer = r'test%20layer%20èé%203857%20published%20insert'
        update_layer = r'test%20layer%20èé%203857%20published%20update'
        delete_layer = r'test%20layer%20èé%203857%20published%20delete'
        unpublished_layer = r'test%20layer%203857%20èé%20unpublished'
        hidden_text_2_layer = r'test%20layer%20èé%203857%20published%20hidden%20text_2'

        # Invalid request
        data = b'not json!'
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        self.assertTrue(
            '[{"code":"Bad request error","description":"JSON parse error' in bytes(response.body()).decode('utf8'))

        # Valid request: change feature with ID 1
        data = """{
        "geometry": {
            "coordinates": [[
            7.247,
            44.814
            ]],
            "type": "MultiPoint"
        },
        "properties": {
            "text_1": "Text 1",
            "text_2": "Text 2",
            "int_1": 123,
            "float_1": 12345.678,
            "datetime_1": "2019-11-07T12:34:56",
            "date_1": "2019-11-07",
            "blob_1": "dGVzdA==",
            "bool_1": true
        },
        "type": "Feature"
        }""".encode('utf8')

        # Unauthorized layer
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % insert_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 403)

        # Authorized layer
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertEqual(j['properties']['text_1'], 'Text 1')
        self.assertEqual(j['properties']['text_2'], 'Text 2')
        self.assertEqual(j['properties']['int_1'], 123)
        self.assertEqual(j['properties']['float_1'], 12345.678)
        self.assertEqual(j['properties']['bool_1'], True)
        self.assertEqual(j['properties']['blob_1'], "dGVzdA==")
        self.assertEqual(j['geometry']['coordinates'], [[7.247, 44.814]])

        feature = project.mapLayersByName('test layer èé 3857 published update')[
            0].getFeature(1)
        self.assertEqual(feature.attribute('text_1'), 'Text 1')
        self.assertEqual(feature.attribute('text_2'), 'Text 2')
        self.assertEqual(feature.attribute('int_1'), 123)
        self.assertEqual(feature.attribute('float_1'), 12345.678)
        self.assertEqual(feature.attribute('bool_1'), True)
        self.assertEqual(bytes(feature.attribute('blob_1')), b"test")
        self.assertEqual(re.sub(
            r'\.\d+', '', feature.geometry().asWkt().upper()), 'MULTIPOINT ((806732 5592286))')

        # Test with partial and unordered properties
        data = """{
        "geometry": {
            "coordinates": [[
            8.247,
            45.814
            ]],
            "type": "MultiPoint"
        },
        "properties": {
            "bool_1": false,
            "int_1": 1234,
            "text_2": "Text 2-bis",
            "text_1": "Text 1-bis"
        },
        "type": "Feature"
        }""".encode('utf8')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertEqual(j['properties']['text_1'], 'Text 1-bis')
        self.assertEqual(j['properties']['text_2'], 'Text 2-bis')
        self.assertEqual(j['properties']['int_1'], 1234)
        self.assertEqual(j['properties']['float_1'], 12345.678)
        self.assertEqual(j['properties']['bool_1'], False)
        self.assertEqual(j['properties']['blob_1'], "dGVzdA==")
        self.assertEqual(j['geometry']['coordinates'], [[8.247, 45.814]])

        feature = project.mapLayersByName('test layer èé 3857 published update')[
            0].getFeature(1)
        self.assertEqual(feature.attribute('text_1'), 'Text 1-bis')
        self.assertEqual(feature.attribute('text_2'), 'Text 2-bis')
        self.assertEqual(feature.attribute('int_1'), 1234)
        self.assertEqual(feature.attribute('float_1'), 12345.678)
        self.assertEqual(feature.attribute('bool_1'), False)
        self.assertEqual(bytes(feature.attribute('blob_1')), b"test")
        self.assertEqual(re.sub(
            r'\.\d+', '', feature.geometry().asWkt().upper()), 'MULTIPOINT ((918051 5750592))')

        # Try to update a forbidden (unpublished) field
        data = """{
        "geometry": {
            "coordinates": [[
            8.247,
            45.814
            ]],
            "type": "MultiPoint"
        },
        "properties": {
            "text_2": "Text 2-tris"
        },
        "type": "Feature"
        }""".encode('utf8')
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % hidden_text_2_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/geo+json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 403)

    def test_wfs3_collection_items_delete(self):
        """Test WFS3 API items DELETE"""

        tmpDir = QtCore.QTemporaryDir()
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.qgs',
                    tmpDir.path() + '/test_project_api_editing.qgs')
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.gpkg',
                    tmpDir.path() + '/test_project_api_editing.gpkg')

        project = QgsProject()
        project.read(tmpDir.path() + '/test_project_api_editing.qgs')

        # Project layers with different permissions
        insert_layer = r'test%20layer%20èé%203857%20published%20insert'
        update_layer = r'test%20layer%20èé%203857%20published%20update'
        delete_layer = r'test%20layer%20èé%203857%20published%20delete'
        unpublished_layer = r'test%20layer%203857%20èé%20unpublished'
        hidden_text_2_layer = r'test%20layer%20èé%203857%20published%20hidden%20text_2'

        # Valid request on unauthorized layer
        data = b''
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.DeleteMethod)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 403)

        # Valid request on authorized layer
        data = b''
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % delete_layer,
                                         QgsBufferServerRequest.DeleteMethod)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)

        # Check that it was really deleted
        layer = project.mapLayersByName(
            'test layer èé 3857 published delete')[0]
        self.assertFalse(1 in layer.allFeatureIds())

    def test_wfs3_collection_items_patch(self):
        """Test WFS3 API items PATCH"""

        tmpDir = QtCore.QTemporaryDir()
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.qgs',
                    tmpDir.path() + '/test_project_api_editing.qgs')
        shutil.copy(unitTestDataPath('qgis_server') + '/test_project_api_editing.gpkg',
                    tmpDir.path() + '/test_project_api_editing.gpkg')

        project = QgsProject()
        project.read(tmpDir.path() + '/test_project_api_editing.qgs')

        # Project layers with different permissions
        insert_layer = r'test%20layer%20èé%203857%20published%20insert'
        update_layer = r'test%20layer%20èé%203857%20published%20update'
        delete_layer = r'test%20layer%20èé%203857%20published%20delete'
        unpublished_layer = r'test%20layer%203857%20èé%20unpublished'
        hidden_text_2_layer = r'test%20layer%20èé%203857%20published%20hidden%20text_2'

        # Invalid request
        data = b'not json!'
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PutMethod,
                                         {'Content-Type': 'application/json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        self.assertTrue(
            '[{"code":"Bad request error","description":"JSON parse error' in bytes(response.body()).decode('utf8'))

        # Invalid request: contains "add"
        data = b"""
        {
            "add": {
                "a_new_field": 1.234
            },
            "modify": {
                "text_2": "A new text 2"
            }
        }
        """
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PatchMethod,
                                         {'Content-Type': 'application/json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        self.assertEqual(bytes(response.body()).decode('utf8'),
                         r'[{"code":"Not implemented error","description":"\"add\" instruction in PATCH method is not implemented"}]')

        # Valid request: change feature with ID 1
        data = """{
            "modify": {
                "text_2": "A new text 2",
                "blob_1": "dGVzdA=="
            }
        }""".encode('utf8')

        # Unauthorized layer
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % insert_layer,
                                         QgsBufferServerRequest.PatchMethod,
                                         {'Content-Type': 'application/json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 403)

        # Authorized layer
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % update_layer,
                                         QgsBufferServerRequest.PatchMethod,
                                         {'Content-Type': 'application/json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200, msg=response.body())
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertEqual(j['properties']['text_1'], 'Torre Pellice 1')
        self.assertEqual(j['properties']['text_2'], 'A new text 2')
        self.assertEqual(j['properties']['int_1'], 7)
        self.assertEqual(j['properties']['float_1'], 1234.567)
        self.assertEqual(j['properties']['bool_1'], True)
        self.assertEqual(j['properties']['blob_1'], "dGVzdA==")
        self.assertEqual(j['geometry']['coordinates'], [[7.227328, 44.820762]])

        feature = project.mapLayersByName('test layer èé 3857 published update')[
            0].getFeature(1)
        self.assertEqual(feature.attribute('text_1'), 'Torre Pellice 1')
        self.assertEqual(feature.attribute('text_2'), 'A new text 2')
        self.assertEqual(feature.attribute('int_1'), 7)
        self.assertEqual(feature.attribute('float_1'), 1234.567)
        self.assertEqual(feature.attribute('bool_1'), True)
        self.assertEqual(bytes(feature.attribute('blob_1')), b"test")
        self.assertEqual(re.sub(
            r'\.\d+', '', feature.geometry().asWkt().upper()), 'MULTIPOINT ((804542 5593348))')

        # Try to update a forbidden (unpublished) field
        request = QgsBufferServerRequest('http://server.qgis.org/wfs3/collections/%s/items/1' % hidden_text_2_layer,
                                         QgsBufferServerRequest.PatchMethod,
                                         {'Content-Type': 'application/json'},
                                         data
                                         )
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 403)

    def test_wfs3_field_filters(self):
        """Test field filters"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        # Check not published
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer3/items?name=two')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 404)  # Not found
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?name=two')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        self.compareApi(
            request, project, 'test_wfs3_collections_items_layer1_with_short_name_eq_two.json')

    def test_wfs3_sorting(self):
        """Test sorting"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        # Check not published
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?sortby=does_not_exist')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)  # Bad request
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?sortby=name')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        self.compareApi(
            request, project, 'test_wfs3_collections_items_layer1_with_short_name_sort_by_name.json')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?sortby=name&sortdesc=1')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        self.compareApi(
            request, project, 'test_wfs3_collections_items_layer1_with_short_name_sort_by_name_desc.json')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?sortby=name&sortdesc=0')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 200)
        self.compareApi(
            request, project, 'test_wfs3_collections_items_layer1_with_short_name_sort_by_name_asc.json')

    def test_wfs3_collection_items_properties(self):
        """Test WFS3 API items"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')

        # Invalid request
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?properties')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(bytes(response.body()).decode('utf8'),
                         '[{"code":"Bad request error","description":"Argument \'properties\' is not valid. Comma separated list of feature property names to be added to the result. Valid values: \'id\', \'name\', \'utf8nameè\'"}]')

        # Valid request
        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?properties=name')
        self.server.handleRequest(request, response, project)
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertTrue('name' in j['features'][0]['properties'])
        self.assertFalse('id' in j['features'][0]['properties'])

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?properties=name,id')
        self.server.handleRequest(request, response, project)
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertTrue('name' in j['features'][0]['properties'])
        self.assertTrue('id' in j['features'][0]['properties'])

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?properties=id')
        self.server.handleRequest(request, response, project)
        j = json.loads(bytes(response.body()).decode('utf8'))
        self.assertFalse('name' in j['features'][0]['properties'])
        self.assertTrue('id' in j['features'][0]['properties'])

    def test_wfs3_field_filters_star(self):
        """Test field filters"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/layer1_with_short_name/items?name=tw*')
        response = self.compareApi(request, project,
                                   'test_wfs3_collections_items_layer1_with_short_name_eq_tw_star.json')
        self.assertEqual(response.statusCode(), 200)

    def test_wfs3_excluded_attributes(self):
        """Test excluded attributes"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/exclude_attribute/items/0.geojson')
        response = self.compareApi(
            request, project, 'test_wfs3_collections_items_exclude_attribute_0.json')
        self.assertEqual(response.statusCode(), 200)

    def test_wfs3_invalid_fids(self):
        """Test exceptions for invalid fids"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/exclude_attribute/items/123456.geojson')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(bytes(response.body()).decode('utf-8'), '[{"code":"Internal server error","description":"Invalid feature [123456]"}]')

        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/exclude_attribute/items/xYz@#.geojson')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(bytes(response.body()).decode('utf-8'), '[{"code":"Internal server error","description":"Invalid feature ID [xYz@]"}]')

    def test_wfs3_time_filters_ranges(self):
        """Test datetime filters"""

        project = QgsProject()

        tempDir = QtCore.QTemporaryDir()
        source_project_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.qgs'
        source_data_path = unitTestDataPath(
            'qgis_server') + '/test_project_api_timefilters.gpkg'
        dest_project_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.qgs')
        dest_data_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters.gpkg')
        shutil.copy(source_data_path, dest_data_path)
        shutil.copy(source_project_path, dest_project_path)
        project.read(dest_project_path)

        # Prepare projects with all options

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertEqual(len(layer.serverProperties().wmsDimensions()), 0)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 0)
        none_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_none.qgs')
        project.write(none_path)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'created')))
        created_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_created.qgs')
        project.write(created_path)
        project.read(created_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 1)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'created_string')))
        created_string_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_created_string.qgs')
        project.write(created_string_path)
        project.read(created_string_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 1)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('time', 'updated_string')))
        updated_string_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_updated_string.qgs')
        project.write(updated_string_path)
        project.read(updated_string_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 1)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 0)
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('time', 'updated')))
        updated_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_updated.qgs')
        project.write(updated_path)
        project.read(updated_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 1)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 0)
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('time', 'updated')))
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'created')))
        both_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_both.qgs')
        project.write(both_path)
        project.read(both_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 2)

        layer = list(project.mapLayers().values())[0]
        layer.serverProperties().removeWmsDimension('date')
        layer.serverProperties().removeWmsDimension('time')
        self.assertTrue(layer.serverProperties().addWmsDimension(
            QgsVectorLayerServerProperties.WmsDimensionInfo('date', 'begin', 'end')))
        date_range_path = os.path.join(
            tempDir.path(), 'test_project_api_timefilters_date_range.qgs')
        project.write(date_range_path)
        project.read(date_range_path)
        self.assertEqual(len(project.mapLayersByName('points')[
                         0].serverProperties().wmsDimensions()), 1)

        '''
        Test data
        wkt_geom	                                        fid	name	    created	    updated	                begin	    end
        Point (7.28848021144956881 44.79768920192042714)	3	bibiana
        Point (7.30355493642693343 44.82162158126364915)	2	bricherasio	2017-01-01	2019-01-01T01:01:01.000	2017-01-01	2019-01-01
        Point (7.22555186948937145 44.82015087638781381)	4	torre	    2018-01-01	2021-01-01T01:01:01.000	2018-01-01	2021-01-01
        Point (7.2500747591236081 44.81342128741047048)	    1	luserna	    2019-01-01	2022-01-01T01:01:01.000	2020-01-01	2022-01-01
        Point (7.2500747591236081 44.81342128741047048)	    5	villar	    2010-01-01	2010-01-01T01:01:01.000	2010-01-01	2010-01-01
        '''

        # What to test:
        # interval-closed     = date-time "/" date-time
        # interval-open-start = [".."] "/" date-time
        # interval-open-end   = date-time "/" [".."]
        # interval            = interval-closed / interval-open-start / interval-open-end
        # datetime            = date-time / interval

        def _date_tester(project_path, datetime, expected, unexpected):
            # Test "created" date field exact
            request = QgsBufferServerRequest(
                'http://server.qgis.org/wfs3/collections/points/items?datetime=%s' % datetime)
            response = QgsBufferServerResponse()
            project.read(project_path)
            self.server.handleRequest(request, response, project)
            body = bytes(response.body()).decode('utf8')
            # print(body)
            for exp in expected:
                self.assertIn(exp, body)
            for unexp in unexpected:
                self.assertNotIn(unexp, body)

        def _interval(project_path, interval):
            project.read(project_path)
            layer = list(project.mapLayers().values())[0]
            return QgsServerApiUtils.temporalFilterExpression(layer, interval).expression()

        # Bad request
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/points/items?datetime=bad timing!')
        response = QgsBufferServerResponse()
        project.read(created_path)
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/points/items?datetime=2020-01-01/2010-01-01')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)
        # empty
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/points/items?datetime=2020-01-01/2010-01-01')
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)

        # Created (date type)
        self.assertEqualBrackets(_interval(created_path, '2017-01-01'),
                                 '( "created" IS NULL OR "created" = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '../2017-01-01'),
                                 '( "created" IS NULL OR "created" <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '/2017-01-01'),
                                 '( "created" IS NULL OR "created" <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01/'),
                                 '( "created" IS NULL OR "created" >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01/..'),
                                 '( "created" IS NULL OR "created" >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01/2018-01-01'),
                                 '( "created" IS NULL OR ( to_date( \'2017-01-01\' ) <= "created" AND "created" <= to_date( \'2018-01-01\' ) ) )')

        self.assertEqualBrackets(_interval(created_path, '2017-01-01T01:01:01'),
                                 '( "created" IS NULL OR "created" = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '../2017-01-01T01:01:01'),
                                 '( "created" IS NULL OR "created" <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '/2017-01-01T01:01:01'),
                                 '( "created" IS NULL OR "created" <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01T01:01:01/'),
                                 '( "created" IS NULL OR "created" >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01T01:01:01/..'),
                                 '( "created" IS NULL OR "created" >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_path, '2017-01-01T01:01:01/2018-01-01T01:01:01'),
                                 '( "created" IS NULL OR ( to_date( \'2017-01-01\' ) <= "created" AND "created" <= to_date( \'2018-01-01\' ) ) )')

        # Updated (datetime type)
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01'),
                                 '( "updated" IS NULL OR to_date( "updated" ) = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '/2017-01-01'),
                                 '( "updated" IS NULL OR to_date( "updated" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '../2017-01-01'),
                                 '( "updated" IS NULL OR to_date( "updated" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01/'),
                                 '( "updated" IS NULL OR to_date( "updated" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01/..'),
                                 '( "updated" IS NULL OR to_date( "updated" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01/2018-01-01'),
                                 '( "updated" IS NULL OR ( to_date( \'2017-01-01\' ) <= to_date( "updated" ) AND to_date( "updated" ) <= to_date( \'2018-01-01\' ) ) )')

        self.assertEqualBrackets(_interval(updated_path, '2017-01-01T01:01:01'),
                                 '( "updated" IS NULL OR "updated" = to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '../2017-01-01T01:01:01'),
                                 '( "updated" IS NULL OR "updated" <= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '/2017-01-01T01:01:01'),
                                 '( "updated" IS NULL OR "updated" <= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01T01:01:01/'),
                                 '( "updated" IS NULL OR "updated" >= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01T01:01:01/..'),
                                 '( "updated" IS NULL OR "updated" >= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_path, '2017-01-01T01:01:01/2018-01-01T01:01:01'),
                                 '( "updated" IS NULL OR ( to_datetime( \'2017-01-01T01:01:01\' ) <= "updated" AND "updated" <= to_datetime( \'2018-01-01T01:01:01\' ) ) )')

        # Created string (date type)
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '../2017-01-01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '/2017-01-01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01/'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01/..'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01/2018-01-01'),
                                 '( "created_string" IS NULL OR ( to_date( \'2017-01-01\' ) <= to_date( "created_string" ) AND to_date( "created_string" ) <= to_date( \'2018-01-01\' ) ) )')

        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01T01:01:01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '../2017-01-01T01:01:01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '/2017-01-01T01:01:01'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01T01:01:01/'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01T01:01:01/..'),
                                 '( "created_string" IS NULL OR to_date( "created_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(created_string_path, '2017-01-01T01:01:01/2018-01-01T01:01:01'),
                                 '( "created_string" IS NULL OR ( to_date( \'2017-01-01\' ) <= to_date( "created_string" ) AND to_date( "created_string" ) <= to_date( \'2018-01-01\' ) ) )')

        # Updated string (datetime type)
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01'),
                                 '( "updated_string" IS NULL OR to_date( "updated_string" ) = to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '/2017-01-01'),
                                 '( "updated_string" IS NULL OR to_date( "updated_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '../2017-01-01'),
                                 '( "updated_string" IS NULL OR to_date( "updated_string" ) <= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01/'),
                                 '( "updated_string" IS NULL OR to_date( "updated_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01/..'),
                                 '( "updated_string" IS NULL OR to_date( "updated_string" ) >= to_date( \'2017-01-01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01/2018-01-01'),
                                 '( "updated_string" IS NULL OR ( to_date( \'2017-01-01\' ) <= to_date( "updated_string" ) AND to_date( "updated_string" ) <= to_date( \'2018-01-01\' ) ) )')

        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01T01:01:01'),
                                 '( "updated_string" IS NULL OR to_datetime( "updated_string" ) = to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '../2017-01-01T01:01:01'),
                                 '( "updated_string" IS NULL OR to_datetime( "updated_string" ) <= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '/2017-01-01T01:01:01'),
                                 '( "updated_string" IS NULL OR to_datetime( "updated_string" ) <= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01T01:01:01/'),
                                 '( "updated_string" IS NULL OR to_datetime( "updated_string" ) >= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01T01:01:01/..'),
                                 '( "updated_string" IS NULL OR to_datetime( "updated_string" ) >= to_datetime( \'2017-01-01T01:01:01\' ) )')
        self.assertEqualBrackets(_interval(updated_string_path, '2017-01-01T01:01:01/2018-01-01T01:01:01'),
                                 '( "updated_string" IS NULL OR ( to_datetime( \'2017-01-01T01:01:01\' ) <= to_datetime( "updated_string" ) AND to_datetime( "updated_string" ) <= to_datetime( \'2018-01-01T01:01:01\' ) ) )')

        # Ranges
        self.assertEqualBrackets(_interval(date_range_path, '2010-01-01'),
                                 '( "begin" IS NULL OR "begin" <= to_date( \'2010-01-01\' ) ) AND ( "end" IS NULL OR to_date( \'2010-01-01\' ) <= "end" )')
        self.assertEqualBrackets(_interval(date_range_path, '../2010-01-01'),
                                 '( "begin" IS NULL OR "begin" <= to_date( \'2010-01-01\' ) )')
        self.assertEqualBrackets(_interval(date_range_path, '2010-01-01/..'),
                                 '( "end" IS NULL OR "end" >= to_date( \'2010-01-01\' ) )')
        # Overlap of ranges
        self.assertEqualBrackets(_interval(date_range_path, '2010-01-01/2020-09-12'),
                                 '( "begin" IS NULL OR "begin" <= to_date( \'2020-09-12\' ) ) AND ( "end" IS NULL OR "end" >= to_date( \'2010-01-01\' ) )')

        ##################################################################################
        # Test "created" date field
        # Test exact
        _date_tester(created_path, '2017-01-01',
                     ['bricherasio'], ['luserna', 'torre'])
        # Test datetime field exact (test that we can use a time on a date type field)
        _date_tester(created_path, '2017-01-01T01:01:01',
                     ['bricherasio'], ['luserna', 'torre'])
        # Test exact no match
        _date_tester(created_path, '2000-05-06', [],
                     ['luserna', 'bricherasio', 'torre'])

        ##################################################################################
        # Test "updated" datetime field
        # Test exact
        _date_tester(updated_path, '2019-01-01T01:01:01',
                     ['bricherasio'], ['luserna', 'torre'])
        # Test date field exact (test that we can also use a date on a datetime type field)
        _date_tester(updated_path, '2019-01-01',
                     ['bricherasio'], ['luserna', 'torre'])
        # Test exact no match
        _date_tester(updated_path, '2017-01-01T05:05:05',
                     [], ['luserna', 'bricherasio', 'torre'])

        ##################################################################################
        # Test both
        # Test exact
        _date_tester(both_path, '2010-01-01T01:01:01',
                     ['villar'], ['torre', 'bricherasio', 'luserna'])
        # Test date field exact (test that we can use a date on a datetime type field)
        _date_tester(both_path, '2010-01-01',
                     ['villar'], ['luserna', 'bricherasio', 'torre'])
        # Test exact no match
        _date_tester(both_path, '2020-05-06T05:05:05', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])

        # Test intervals

        ##################################################################################
        # Test "created" date field
        _date_tester(created_path, '2016-05-04/2018-05-06',
                     ['bricherasio', 'torre'], ['luserna', 'villar'])
        _date_tester(created_path, '2016-05-04/..',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(created_path, '2016-05-04/',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(created_path, '2100-05-04/', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(created_path, '2100-05-04/..', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(created_path, '/2018-05-06',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])
        _date_tester(created_path, '../2018-05-06',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])

        # Test datetimes on "created" date field
        _date_tester(created_path, '2016-05-04T01:01:01/2018-05-06T01:01:01', ['bricherasio', 'torre'],
                     ['luserna', 'villar'])
        _date_tester(created_path, '2016-05-04T01:01:01/..',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(created_path, '2016-05-04T01:01:01/',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(created_path, '2100-05-04T01:01:01/', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(created_path, '2100-05-04T01:01:01/..', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(created_path, '/2018-05-06T01:01:01',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])
        _date_tester(created_path, '../2018-05-06T01:01:01',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])

        ##################################################################################
        # Test "updated" date field
        _date_tester(updated_path, '2020-05-04/2022-12-31',
                     ['torre', 'luserna'], ['bricherasio', 'villar'])
        _date_tester(updated_path, '2020-05-04/..',
                     ['torre', 'luserna'], ['bricherasio', 'villar'])
        _date_tester(updated_path, '2020-05-04/',
                     ['torre', 'luserna'], ['bricherasio', 'villar'])
        _date_tester(updated_path, '2019-01-01/',
                     ['torre', 'luserna', 'bricherasio'], ['villar'])
        _date_tester(updated_path, '2019-01-01/..',
                     ['torre', 'luserna', 'bricherasio'], ['villar'])
        _date_tester(updated_path, '/2020-02-02',
                     ['villar', 'bricherasio'], ['torre', 'luserna'])
        _date_tester(updated_path, '../2020-02-02',
                     ['villar', 'bricherasio'], ['torre', 'luserna'])

        # Test datetimes on "updated" datetime field
        _date_tester(updated_path, '2020-05-04T01:01:01/2022-12-31T01:01:01', ['torre', 'luserna'],
                     ['bricherasio', 'villar'])
        _date_tester(updated_path, '2020-05-04T01:01:01/..',
                     ['torre', 'luserna'], ['bricherasio', 'villar'])
        _date_tester(updated_path, '2020-05-04T01:01:01/',
                     ['torre', 'luserna'], ['bricherasio', 'villar'])
        _date_tester(updated_path, '2019-01-01T01:01:01/',
                     ['torre', 'luserna', 'bricherasio'], ['villar'])
        _date_tester(updated_path, '2019-01-01T01:01:01/..',
                     ['torre', 'luserna', 'bricherasio'], ['villar'])
        _date_tester(updated_path, '/2020-02-02T01:01:01',
                     ['villar', 'bricherasio'], ['torre', 'luserna'])
        _date_tester(updated_path, '../2020-02-02T01:01:01',
                     ['villar', 'bricherasio'], ['torre', 'luserna'])

        ##################################################################################
        # Test both
        _date_tester(both_path, '2010-01-01',
                     ['villar'], ['luserna', 'bricherasio'])
        _date_tester(both_path, '2010-01-01/2010-01-01',
                     ['villar'], ['luserna', 'bricherasio'])
        _date_tester(both_path, '2017-01-01/2021-01-01',
                     ['torre', 'bricherasio'], ['luserna', 'villar'])
        _date_tester(both_path, '../2021-01-01',
                     ['torre', 'bricherasio', 'villar'], ['luserna'])
        _date_tester(both_path, '2019-01-01/..',
                     ['luserna'], ['torre', 'bricherasio', 'villar'])

        ##################################################################################
        # Test none path (should take the first date/datetime field, that is "created")

        _date_tester(none_path, '2016-05-04/2018-05-06',
                     ['bricherasio', 'torre'], ['luserna', 'villar'])
        _date_tester(none_path, '2016-05-04/..',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(none_path, '2016-05-04/',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(none_path, '2100-05-04/', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(none_path, '2100-05-04/..', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(none_path, '/2018-05-06',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])
        _date_tester(none_path, '../2018-05-06',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])

        # Test datetimes on "created" date field
        _date_tester(none_path, '2016-05-04T01:01:01/2018-05-06T01:01:01', ['bricherasio', 'torre'],
                     ['luserna', 'villar'])
        _date_tester(none_path, '2016-05-04T01:01:01/..',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(none_path, '2016-05-04T01:01:01/',
                     ['bricherasio', 'torre', 'luserna'], ['villar'])
        _date_tester(none_path, '2100-05-04T01:01:01/', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(none_path, '2100-05-04T01:01:01/..', [],
                     ['luserna', 'bricherasio', 'torre', 'villar'])
        _date_tester(none_path, '/2018-05-06T01:01:01',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])
        _date_tester(none_path, '../2018-05-06T01:01:01',
                     ['bricherasio', 'torre', 'villar'], ['luserna'])

        #####################################################################################################
        # Test ranges
        _date_tester(date_range_path, '2000-05-05T01:01:01', [],
                     ['bricherasio', 'villar', 'luserna', 'torre'])
        _date_tester(date_range_path, '2020-05-05T01:01:01',
                     ['luserna', 'torre'], ['bricherasio', 'villar'])
        _date_tester(date_range_path, '../2000-05-05T01:01:01', [],
                     ['luserna', 'torre', 'bricherasio', 'villar'])
        _date_tester(date_range_path, '../2017-05-05T01:01:01',
                     ['bricherasio', 'villar'], ['luserna', 'torre'])
        _date_tester(date_range_path, '../2050-05-05T01:01:01',
                     ['bricherasio', 'villar', 'luserna', 'torre'], [])
        _date_tester(date_range_path, '2020-05-05T01:01:01/',
                     ['luserna', 'torre'], ['bricherasio', 'villar'])

        _date_tester(date_range_path, '2000-05-05', [],
                     ['bricherasio', 'villar', 'luserna', 'torre'])
        _date_tester(date_range_path, '2020-05-05',
                     ['luserna', 'torre'], ['bricherasio', 'villar'])
        _date_tester(date_range_path, '../2000-05-05', [],
                     ['luserna', 'torre', 'bricherasio', 'villar'])
        _date_tester(date_range_path, '../2017-05-05',
                     ['bricherasio', 'villar'], ['luserna', 'torre'])
        _date_tester(date_range_path, '../2050-05-05',
                     ['bricherasio', 'villar', 'luserna', 'torre'], [])
        _date_tester(date_range_path, '2020-05-05/',
                     ['luserna', 'torre'], ['bricherasio', 'villar'])

        # Test bad requests
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/points/items?datetime=bad timing!')
        response = QgsBufferServerResponse()
        project.read(created_path)
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400)


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
        return [
            QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value')]


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
        return [
            QgsServerQueryStringParameter(
                'value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value'),
            QgsServerQueryStringParameter('value2', False, QgsServerQueryStringParameter.Type.String,
                                          'a string value'), ]


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
        return [
            QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value')]

    def templatePath(self, context):
        if self.templatePathOverride is None:
            return super(Handler3, self).templatePath(context)
        else:
            return self.templatePathOverride


class Handler4(QgsServerOgcApiHandler):

    def path(self):
        return QtCore.QRegularExpression("/(?P<tilemapid>[^/]+)")

    def operationId(self):
        return "handler4"

    def summary(self):
        return "Fourth of its name"

    def description(self):
        return "The fourth handler ever"

    def linkTitle(self):
        return "Handler Four Link Title"

    def linkType(self):
        return QgsServerOgcApi.data

    def handleRequest(self, context):
        """Simple mirror: returns the parameters"""

        self.params = self.values(context)
        self.write(self.params, context)

    def parameters(self, context):
        return []


class HandlerException(QgsServerOgcApiHandler):

    def __init__(self):
        super().__init__()
        self.__exception = None

    def setException(self, exception):
        self.__exception = exception

    def path(self):
        return QtCore.QRegularExpression("/handlerexception")

    def operationId(self):
        return "handlerException"

    def summary(self):
        return "Trigger an exception"

    def description(self):
        return "Trigger an exception"

    def linkTitle(self):
        return "Trigger an exception Title"

    def linkType(self):
        return QgsServerOgcApi.data

    def handleRequest(self, context):
        """Triggers an exception"""
        raise self.__exception

    def parameters(self, context):
        return [
            QgsServerQueryStringParameter('value1', True, QgsServerQueryStringParameter.Type.Double, 'a double value')]


class QgsServerOgcAPITest(QgsServerAPITestBase):
    """ QGIS OGC API server tests"""

    def testOgcApi(self):
        """Test OGC API"""

        api = QgsServerOgcApi(self.server.serverInterface(),
                              '/api1', 'apione', 'an api', '1.1')
        self.assertEqual(api.name(), 'apione')
        self.assertEqual(api.description(), 'an api')
        self.assertEqual(api.version(), '1.1')
        self.assertEqual(api.rootPath(), '/api1')
        url = 'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1'
        self.assertEqual(api.sanitizeUrl(QtCore.QUrl(url)).toString(),
                         'http://server.qgis.org/wfs3/collections/testlayer \xe8\xe9/items?limit=-1')
        self.assertEqual(api.sanitizeUrl(QtCore.QUrl('/path//double//slashes//#fr')).toString(),
                         '/path/double/slashes#fr')
        self.assertEqual(api.relToString(QgsServerOgcApi.data), 'data')
        self.assertEqual(api.relToString(
            QgsServerOgcApi.alternate), 'alternate')
        self.assertEqual(api.contentTypeToString(QgsServerOgcApi.JSON), 'JSON')
        self.assertEqual(api.contentTypeToStdString(
            QgsServerOgcApi.JSON), 'JSON')
        self.assertEqual(api.contentTypeToExtension(
            QgsServerOgcApi.JSON), 'json')
        self.assertEqual(api.contentTypeToExtension(
            QgsServerOgcApi.GEOJSON), 'geojson')

    def testOgcApiHandler(self):
        """Test OGC API Handler"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()

        ctx = QgsServerApiContext(
            '/services/api1', request, response, project, self.server.serverInterface())
        h = Handler1()
        self.assertTrue(h.staticPath(ctx).endswith(
            '/resources/server/api/ogc/static'))
        self.assertEqual(h.path(), QtCore.QRegularExpression("/handlerone"))
        self.assertEqual(h.description(), 'The first handler ever')
        self.assertEqual(h.operationId(), 'handlerOne')
        self.assertEqual(h.summary(), 'First of its name')
        self.assertEqual(h.linkTitle(), 'Handler One Link Title')
        self.assertEqual(h.linkType(), QgsServerOgcApi.data)
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            h.handleRequest(ctx)
        self.assertEqual(str(ex.exception),
                         'Missing required argument: \'value1\'')

        r = ctx.response()
        self.assertEqual(r.data(), '')

        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            h.values(ctx)
        self.assertEqual(str(ex.exception),
                         'Missing required argument: \'value1\'')

        # Add handler to API and test for /api2
        ctx = QgsServerApiContext(
            '/services/api2', request, response, project, self.server.serverInterface())
        api = QgsServerOgcApi(self.server.serverInterface(),
                              '/api2', 'apitwo', 'a second api', '1.2')
        api.registerHandler(h)
        # Add a second handler (will be tested later)
        h2 = Handler2()
        api.registerHandler(h2)

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api1'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(
            str(ex.exception), 'Requested URI does not match any registered API handler')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(
            str(ex.exception), 'Requested URI does not match any registered API handler')

        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/services/api2/handlerone'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception),
                         'Missing required argument: \'value1\'')

        ctx.request().setUrl(QtCore.QUrl(
            'http://www.qgis.org/services/api2/handlerone?value1=not+a+double'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(
            str(ex.exception), 'Argument \'value1\' could not be converted to Double')

        ctx.request().setUrl(QtCore.QUrl(
            'http://www.qgis.org/services/api2/handlerone?value1=1.2345'))
        params = h.values(ctx)
        self.assertEqual(params, {'value1': 1.2345})
        api.executeRequest(ctx)
        self.assertEqual(json.loads(bytes(ctx.response().data()))[
                         'value1'], 1.2345)

        # Test path fragments extraction
        ctx.request().setUrl(QtCore.QUrl(
            'http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345'))
        params = h2.values(ctx)
        self.assertEqual(
            params, {'code1': '00', 'value1': 1.2345, 'value2': None})

        # Test string encoding
        ctx.request().setUrl(
            QtCore.QUrl('http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345&value2=a%2Fstring%20some'))
        params = h2.values(ctx)
        self.assertEqual(
            params, {'code1': '00', 'value1': 1.2345, 'value2': 'a/string some'})

        # Test links
        self.assertEqual(h2.href(ctx),
                         'http://www.qgis.org/services/api2/handlertwo/00/555?value1=1.2345&value2=a%2Fstring%20some')
        self.assertEqual(h2.href(ctx, '/extra'),
                         'http://www.qgis.org/services/api2/handlertwo/00/555/extra?value1=1.2345&value2=a%2Fstring%20some')
        self.assertEqual(h2.href(ctx, '/extra', 'json'),
                         'http://www.qgis.org/services/api2/handlertwo/00/555/extra.json?value1=1.2345&value2=a%2Fstring%20some')

        # Test template path
        self.assertTrue(
            h2.templatePath(ctx).endswith('/resources/server/api/ogc/templates/services/api2/handlerTwo.html'))

        del(project)

    def testOgcApiHandlerContentType(self):
        """Test OGC API Handler content types"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest(
            'http://server.qgis.org/api3/handlerthree?value1=9.5')
        response = QgsBufferServerResponse()

        # Add handler to API and test for /api3
        ctx = QgsServerApiContext(
            '/services/api3', request, response, project, self.server.serverInterface())
        api = QgsServerOgcApi(self.server.serverInterface(),
                              '/api3', 'apithree', 'a third api', '1.2')
        h3 = Handler3()
        api.registerHandler(h3)

        ctx = QgsServerApiContext(
            '/services/api3/', request, response, project, self.server.serverInterface())
        api.executeRequest(ctx)
        self.assertEqual(json.loads(
            bytes(ctx.response().data()))['value1'], 9.5)

        # Call HTML
        ctx.request().setUrl(QtCore.QUrl(
            'http://server.qgis.org/api3/handlerthree.html?value1=9.5'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception), 'Unsupported Content-Type: HTML')

        h3.setContentTypes([QgsServerOgcApi.HTML])
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(str(ex.exception),
                         'Template not found: handlerThree.html')

        # Define a template path
        tmpDir = QtCore.QTemporaryDir()
        with open(tmpDir.path() + '/handlerThree.html', 'w+') as f:
            f.write("Hello world")
        h3.templatePathOverride = tmpDir.path() + '/handlerThree.html'
        ctx.response().clear()
        api.executeRequest(ctx)
        self.assertEqual(bytes(ctx.response().data()), b"Hello world")

        req = QgsBufferServerRequest(
            'http://localhost:8000/project/7ecb/wfs3/collections/zg.grundnutzung.html')
        self.assertEqual(h3.contentTypeFromRequest(req), QgsServerOgcApi.HTML)

        del(project)

    def testOgcApiHandlerException(self):
        """Test OGC API Handler exception"""

        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project_api.qgs')
        request = QgsBufferServerRequest('')
        response = QgsBufferServerResponse()

        ctx = QgsServerApiContext(
            '/services/apiexception', request, response, project, self.server.serverInterface())
        h = HandlerException()

        api = QgsServerOgcApi(self.server.serverInterface(),
                              '/apiexception', 'apiexception', 'an api with exception', '1.2')
        api.registerHandler(h)

        h.setException(Exception("UTF-8 Exception 1 $ù~à^£"))
        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/handlerexception'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(
            str(ex.exception), "UTF-8 Exception 1 $ù~à^£")

        h.setException(QgsServerApiBadRequestException("UTF-8 Exception 2 $ù~à^£"))
        ctx.request().setUrl(QtCore.QUrl('http://www.qgis.org/handlerexception'))
        with self.assertRaises(QgsServerApiBadRequestException) as ex:
            api.executeRequest(ctx)
        self.assertEqual(
            str(ex.exception), "UTF-8 Exception 2 $ù~à^£")

        del(project)

    def test_path_capture(self):
        """Test issue GH #45439"""

        api = QgsServerOgcApi(self.server.serverInterface(),
                              '/api4', 'apifour', 'a fourth api', '1.2')

        h4 = Handler4()
        api.registerHandler(h4)

        request = QgsBufferServerRequest(
            'http://localhost:19876/api4/france_parts.json?MAP=france_parts')
        response = QgsBufferServerResponse()

        server = QgsServer()
        iface = server.serverInterface()
        iface.serviceRegistry().registerApi(api)

        server.handleRequest(request, response)

        self.assertEqual(h4.params, {'tilemapid': 'france_parts.json'})

        ctx = QgsServerApiContext(api.rootPath(), request, response, None, iface)
        self.assertEqual(h4.href(ctx), 'http://localhost:19876/api4/france_parts?MAP=france_parts')


if __name__ == '__main__':
    unittest.main()
