# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer Landing Page Plugin.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '03/08/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
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
    QgsServerApi,
    QgsServerApiBadRequestException,
    QgsServerQueryStringParameter,
    QgsServerApiContext,
    QgsServerOgcApi,
    QgsServerOgcApiHandler,
    QgsServerApiUtils,
    QgsServiceRegistry
)
from qgis.core import QgsProject, QgsRectangle, QgsVectorLayerServerProperties, QgsFeatureRequest
from qgis.PyQt import QtCore

from qgis.testing import unittest
from utilities import unitTestDataPath
from urllib import parse

import tempfile

from test_qgsserver_api import QgsServerAPITestBase


class QgsServerLandingPageTest(QgsServerAPITestBase):
    """ QGIS Server Landing Page tests"""

    # Set to True in child classes to re-generate reference files for this class
    # regeregenerate_api_reference = True

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        os.environ['QGIS_SERVER_PROJECTS_DIRECTORIES'] = os.path.join(unitTestDataPath('qgis_server'), 'landingpage', 'projects') \
            + '||' \
            + os.path.join(unitTestDataPath('qgis_server'),
                           'landingpage', 'projects2')

        if not os.environ.get('TRAVIS', False):
            os.environ['QGIS_SERVER_PROJECTS_PG_CONNECTIONS'] = "postgresql://localhost:5432?sslmode=disable&dbname=landing_page_test&schema=public"

    def setUp(self):
        """Clean env"""

        super().setUp()
        os.environ["QGIS_SERVER_DISABLED_APIS"] = ''

    def test_landing_page_redirects(self):
        """Test landing page redirects"""

        request = QgsBufferServerRequest('http://server.qgis.org/')
        request.setHeader('Accept', 'application/json')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(response.headers()[
                         'Location'], 'http://server.qgis.org/index.json')
        response = QgsBufferServerResponse()
        request.setHeader('Accept', 'text/html')
        self.server.handleRequest(request, response)
        self.assertEqual(response.headers()[
                         'Location'], 'http://server.qgis.org/index.html')

    def test_landing_page_json(self):
        """Test landing page in JSON format"""

        request = QgsBufferServerRequest('http://server.qgis.org/index.json')
        if os.environ.get('TRAVIS', False):
            self.compareApi(
                request, None, 'test_landing_page_index.json', subdir='landingpage')
        else:
            self.compareApi(
                request, None, 'test_landing_page_with_pg_index.json', subdir='landingpage')

    def test_project_json(self):
        """Test landing page project call in JSON format"""

        test_projects = {
            'de8d9e4e8b448f0bf0a0eb1e0806b763': 'Project1.qgs',
            '471b5bb5dbe79f72149529f700d948a3': 'Project2.qgz',
            '9124ec69db7f0687f760504aa10e3362': 'test_project_wms_grouped_nested_layers.qgs',
            '3b4a3e5d9025926cb673805b914646fa': 'project3.qgz',
        }

        for identifier, name in test_projects.items():
            request = QgsBufferServerRequest(
                'http://server.qgis.org/map/' + identifier)
            request.setHeader('Accept', 'application/json')
            self.compareApi(
                request, None, 'test_project_{}.json'.format(name.replace('.', '_')), subdir='landingpage')

    @unittest.skipIf(os.environ.get('TRAVIS'), False)
    def test_pg_project_json(self):
        """Test landing page PG project call in JSON format"""

        test_projects = {
            '056e0bc472fc60eb32a223acf0d9d897': 'PGProject1',
            'c404d3c20fdd5e81bf1c8c2ef895901e': 'PGProject2',
            'e152c153073a7e6d3ad669448f85e552': 'my as areas project'
        }
        for identifier, name in test_projects.items():
            request = QgsBufferServerRequest(
                'http://server.qgis.org/map/' + identifier)
            request.setHeader('Accept', 'application/json')
            self.compareApi(
                request, None, 'test_project_{}.json'.format(name.replace('.', '_')), subdir='landingpage')


if __name__ == '__main__':
    unittest.main()
