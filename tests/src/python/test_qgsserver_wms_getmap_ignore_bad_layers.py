# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer WMS GetMap with QGIS_SERVER_IGNORE_BAD_LAYERS=true.

From build dir, run: ctest -R PyQgsServerWMSGetMapIgnoreBadLayers -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '13/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.request
import urllib.parse
import urllib.error

from qgis.testing import unittest
from qgis.PyQt.QtCore import QSize

import osgeo.gdal  # NOQA

from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath
from qgis.core import QgsProject, QgsApplication

# Strip path and content length because path may vary
RE_STRIP_UNCHECKABLE = br'MAP=[^"]+|Content-Length: \d+'
RE_ATTRIBUTES = br'[^>\s]+=[^>\s]+'


class TestQgsServerWMSGetMapIgnoreBadLayers(QgsServerTestBase):
    """QGIS Server WMS Tests for GetMap request with QGIS_SERVER_IGNORE_BAD_LAYERS=true"""

    # regenerate_reference = True

    @classmethod
    def setUpClass(cls):
        os.environ['QGIS_SERVER_IGNORE_BAD_LAYERS'] = 'true'
        super().setUpClass()

    def test_wms_getmap_datasource_error_ignore(self):
        """Must NOT throw a server exception if datasource if not available and QGIS_SERVER_IGNORE_BAD_LAYERS is set"""

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(os.path.join(self.testdata_path,
                                                   'test_project_wms_invalid_layers.qgs')),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "BBOX": "613402.5658687877003,5809005.018114360981,619594.408781287726,5813869.006602735259",
            "CRS": "EPSG:25832",
            "WIDTH": "429",
            "HEIGHT": "337",
            "LAYERS": "areas and symbols,osm",
            "STYLES": ",",
            "FORMAT": "image/png",
            "DPI": "200",
            "MAP_RESOLUTION": "200",
            "FORMAT_OPTIONS": "dpi:200"
        }.items())])

        r, h = self._result(self._execute_request(qs))

        self.assertFalse('ServerException' in str(r), 'Unexpected ServerException ' + str(r))


if __name__ == '__main__':
    unittest.main()
