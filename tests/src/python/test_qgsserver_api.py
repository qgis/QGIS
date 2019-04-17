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

from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse
from qgis.testing import unittest
from utilities import unitTestDataPath

import tempfile

from test_qgsserver import QgsServerTestBase


class QgsServerAPITest(QgsServerTestBase):
    """ QGIS API server tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_reference = False

    @classmethod
    def setUpClass(cls):
        super(QgsServerAPITest, cls).setUpClass()

    def test_wfs3_api(self):
        """Test WFS3 API"""
        request = QgsBufferServerRequest('')
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, None)
        self.assertEqual(bytes(response.body()).decode('utf8'), "Hello! from project unknown")


if __name__ == '__main__':
    unittest.main()
