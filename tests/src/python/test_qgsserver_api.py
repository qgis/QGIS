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

from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse, QgsServerApi
from qgis.core import QgsProject
from qgis.testing import unittest
from utilities import unitTestDataPath

import tempfile

from test_qgsserver import QgsServerTestBase


class QgsServerAPITest(QgsServerTestBase):
    """ QGIS API server tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_reference = True

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
        result = self.dump(response)
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

    def test_api(self):

        class API(QgsServerApi):

            def name(self):
                return "Test API"

            def rootPath(self):
                return "/testapi"

            def executeRequest(self, request, response, project):
                response.write(b"\"Test API\"")

        api = API()
        self.server.serverInterface().serviceRegistry().registerApi(api)
        request = QgsBufferServerRequest('http://www.acme.com/testapi')
        self.compareApi(request, None, 'test_api.json')

    def test_wfs3_landing_page(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wf3')
        self.compareApi(request, None, 'test_wfs3_landing_page.json')

    def test_wfs3_api(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/api')
        self.compareApi(request, None, 'test_wfs3_api.json')

    def test_wfs3_conformance(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/conformance')
        self.compareApi(request, None, 'test_wfs3_conformance.json')

    def test_wfs3_collections(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections')
        self.compareApi(request, None, 'test_wfs3_collections.json')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.json')
        self.compareApi(request, None, 'test_wfs3_collections.json')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.html')
        self.compareApi(request, None, 'test_wfs3_collections.html')
        request = QgsBufferServerRequest('http://www.acme.com/wfs3/collections.json')
        project = QgsProject()
        project.read(unitTestDataPath('qgis_server') + '/test_project.qgs')
        self.compareApi(request, project, 'test_wfs3_collections_project.json')


if __name__ == '__main__':
    unittest.main()
