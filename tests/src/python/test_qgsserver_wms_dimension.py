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
import urllib.request
import urllib.parse
import urllib.error
from test_qgsserver import QgsServerTestBase
from test_qgsserver_wms import TestQgsServerWMSTestBase
import base64


class TestQgsServerWMSDimension(TestQgsServerWMSTestBase):

    # Set to True to re-generate reference files for this class
    regenerate_reference = False

    def setUp(self):
        super().setUp()
        self.testdata_path = os.path.join(self.temporary_path, "qgis_server_accesscontrol")

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

    def test_wms_getmap_default(self):
        # default rendering
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_All_NoValue")

    def test_wms_getmap_simple_value(self):
        # dimension with value
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "1000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_Value")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_RANGE_ELEVATION": "1000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_RangeElevation_Value")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TIME": "2021-05-31T17:00:00"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Time_Value")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_DATE": "2021-05-31"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Date_Value")

        # multi dimension with value
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "1000",
            "DIM_RANGE_ELEVATION": "1000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_RangeElevation_Value")

    def test_wms_getmap_range_value(self):
        # dimension with range value
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "1000/2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_RangeValue")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_RANGE_ELEVATION": "1000/2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_RangeElevation_RangeValue")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TIME": "2021-05-31T09:00:00/2021-06-30T09:00:00"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Time_RangeValue")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_DATE": "2021-05-31/2021-06-30"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Date_RangeValue")

    def test_wms_getmap_multi_values(self):
        # dimension with multi values
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "1000,2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_MultiValues")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_RANGE_ELEVATION": "1000,2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_RangeElevation_MultiValues")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "TIME": "2021-05-31T10:00:00,2021-05-31T17:00:00"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Time_MultiValues")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Datetime_dim",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_DATE": "2021-05-31,2021-06-30"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Date_MultiValues")

    def test_wms_getmap_mix_values(self):
        # dimension with mix values
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "1000/1500,2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_MixValues")
        # same as ELEVATION=1000/2000
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_RangeValue")

        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "DIM_RANGE_ELEVATION": "1000/1500,2000"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_RangeElevation_MixValues")
        # same as DIM_RANGE_ELEVATION=1000/2000
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_RangeElevation_RangeValue")

    def test_wms_getmap_with_filter(self):
        # dimension with filter
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetMap",
            "LAYERS": "dem,Slopes,Contours",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1219081,4281848,172260,5673189",
            "HEIGHT": "500",
            "WIDTH": "500",
            "CRS": "EPSG:3857",
            "ELEVATION": "800/2200",
            "FILTER": "Contours:\"elev\" <= 1200"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_RangeValue_Filter")
        # same as ELEVATION=1000
        self._img_diff_error(r, h, "WMS_GetMap_Dimension_Elevation_Value")

    def test_wms_getprint_dimension(self):
        qs = "?" + "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.3.0",
            "REQUEST": "GetPrint",
            "TEMPLATE": "layoutA4",
            "FORMAT": "png",
            "map0:EXTENT": "-1219081,4281848,172260,5673189",
            "map0:LAYERS": "dem,Datetime_dim",
            "map0:SCALE": "10013037",
            "CRS": "EPSG:3857",
            "HEIGHT": "500",
            "DIM_DATE": "2021-05-31,2021-06-30"
        }.items())])

        r, h = self._result(self._execute_request(qs))
        self._img_diff_error(r, h, "WMS_GetPrint_Dimension", max_size_diff=QSize(1, 1))


if __name__ == "__main__":
    unittest.main()
