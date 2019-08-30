# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS Dimension.

From build dir, run: ctest -R PyQgsServerWMSDimension -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'René-Luc Dhont'
__date__ = '29/08/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

import os
from utilities import unitTestDataPath
from qgis.testing import unittest
from qgis.server import QgsServer, QgsAccessControlFilter, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsRenderChecker, QgsApplication
from qgis.PyQt.QtCore import QSize
import tempfile
from test_qgsserver import QgsServerTestBase
from test_qgsserver_wms import TestQgsServerWMSTestBase
import base64


class TestQgsServerWMSDimension(TestQgsServerWMSTestBase):

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def setUp(self):
        super().setUp()
        self.testdata_path = unitTestDataPath("qgis_server_accesscontrol")

        self.projectPath = os.path.join(self.testdata_path, 'project_with_dimensions.qgs')
        self.assertTrue(os.path.isfile(self.projectPath), 'Could not find project file "{}"'.format(self.projectPath))

    def wms_request(self, request, extra=None, project='project_with_dimensions.qgs', version='1.3.0'):
        return super().wms_request(request, extra, project, version)

    def wms_request_compare(self, request, extra=None, reference_file=None, project='project_with_dimensions.qgs', version='1.3.0', ignoreExtent=False, normalizeJson=False):
        args = dict(
            extra=extra,
            reference_file=os.path.join('results', (request.lower() + '_wms_dimension' if not reference_file else reference_file)),
            project=project,
            version=version,
            ignoreExtent=ignoreExtent,
            normalizeJson=normalizeJson
        )
        return super().wms_request_compare(request, **args)

    def test_wms_getcapabilities(self):
        self.wms_request_compare('GetCapabilities')


if __name__ == "__main__":
    unittest.main()
