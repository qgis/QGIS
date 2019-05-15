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

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse, QgsServerApi, QgsServerApiUtils
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


class QgsServerAPITest(QgsServerTestBase):
    """ QGIS API server tests"""

    # Set to True in child classes to re-generate reference files for this class
    #regenerate_reference = True

    def dump(self, response):
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
        self.server.handleRequest(request, response, project)
        result = bytes(response.body()).decode('utf8') if reference_file.endswith('html') else self.dump(response)
        path = unitTestDataPath('qgis_server') + '/api/' + reference_file
        if self.regenerate_reference:
            f = open(path, 'w+')
            f.write(result)
            f.close()
            print("Reference file %s regenerated!" % path)
        with open(path, 'r') as f:
            self.assertEqual(f.read(), result)

    @classmethod
    def setUpClass(cls):
        super(QgsServerAPITest, cls).setUpClass()
        cls.maxDiff = None

    def test_api(self):

        class API(QgsServerApi):

            def name(self):
                return "Test API"

            def rootPath(self):
                return "/testapi"

            def executeRequest(self, request_context):
                request_context.response().write(b"\"Test API\"")

        api = API()
        self.server.serverInterface().serviceRegistry().registerApi(api)
        request = QgsBufferServerRequest('http://www.acme.com/testapi')
        self.compareApi(request, None, 'test_api.json')
        self.server.serverInterface().serviceRegistry().unregisterApi(api.name())

    def test_wfs3_landing_page(self):
        """Test WFS3 API landing page in HTML format"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3')
        self.compareApi(request, None, 'test_wfs3_landing_page.html')

    def test_wfs3_landing_page_json(self):
        """Test WFS3 API landing page in JSON format"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3.json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3')
        request.setHeader('Accept', 'application/json')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')

    def test_wfs3_api(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/api')
        self.compareApi(request, None, 'test_wfs3_api.json')

    def test_wfs3_conformance(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/conformance')
        self.compareApi(request, None, 'test_wfs3_conformance.json')

    def test_wfs3_collections_empty(self):
        """Test WFS3 collections API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')

        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.json')
        self.compareApi(request, None, 'test_wfs3_collections_empty.json')

        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.html')
        self.compareApi(request, None, 'test_wfs3_collections_empty.html')

    def test_wfs3_collections_json(self):
        """Test WFS3 API collections in json format"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.json')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.json')

    def test_wfs3_collections_html(self):
        """Test WFS3 API collections in html format"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.html')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.html')

    def test_wfs3_collection_items(self):
        """Test WFS3 API"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé.json')

    def test_invalid_args(self):
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?limit=-1')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Limit is not valid (0-10000)"}]') # Bad request

        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?limit=10001')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        self.assertEqual(response.statusCode(), 400) # Bad request
        self.assertEqual(response.body(), b'[{"code":"Bad request error","description":"Limit is not valid (0-10000)"}]') # Bad request

    def test_wfs3_collection_items_limit(self):
        """Test WFS3 API"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?limit=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1.json')

    def test_wfs3_collection_items_limit_offset(self):
        """Test WFS3 API"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?limit=1&offset=1')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_limit_1_offset_1.json')

    def test_wfs3_collection_items_bbox(self):
        """Test WFS3 API"""
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?bbox=8.203495,44.901482,8.203497,44.901484')
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox.json')

        # Test with a different CRS
        encoded_crs = parse.quote('http://www.opengis.net/def/crs/EPSG/9.6.2/32632', safe='')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections/testlayer%20èé/items?bbox=437106.99096772138727829,4972307.72834488749504089,437129.50390358484582976,4972318.1108037903904914&bbox-crs={}'.format(encoded_crs))
        self.compareApi(request, project, 'test_wfs3_collections_items_testlayer_èé_bbox_32632.json')


if __name__ == '__main__':
    unittest.main()
