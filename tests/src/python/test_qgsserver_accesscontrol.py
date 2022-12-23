# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Stephane Brunner'
__date__ = '28/08/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
import shutil

import os
from shutil import copyfile
from math import sqrt
from utilities import unitTestDataPath
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from qgis.server import QgsServer, QgsAccessControlFilter, QgsServerRequest, QgsBufferServerRequest, QgsBufferServerResponse
from qgis.core import QgsRenderChecker, QgsApplication
from qgis.PyQt.QtCore import QSize
import tempfile
from test_qgsserver import QgsServerTestBase
import base64


XML_NS = \
    'service="WFS" version="1.0.0" ' \
    'xmlns:wfs="http://www.opengis.net/wfs" ' \
    'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ' \
    'xmlns:ogc="http://www.opengis.net/ogc" ' \
    'xmlns="http://www.opengis.net/wfs" updateSequence="0" ' \
    'xmlns:xlink="http://www.w3.org/1999/xlink" ' \
    'xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" ' \
    'xmlns:gml="http://www.opengis.net/gml" ' \
    'xmlns:ows="http://www.opengis.net/ows" '


class RestrictedAccessControl(QgsAccessControlFilter):

    """ Used to have restriction access """

    # Be able to deactivate the access control to have a reference point
    _active = False

    def __init__(self, server_iface):
        super(QgsAccessControlFilter, self).__init__(server_iface)

    def layerFilterExpression(self, layer):
        """ Return an additional expression filter """

        if not self._active:
            return super(RestrictedAccessControl, self).layerFilterExpression(layer)

        if layer.name() == "Hello":
            return "$id = 1"
        elif layer.name() == "Hello_Filter":
            return "pkuid = 6 or pkuid = 7"
        else:
            return None

    def layerFilterSubsetString(self, layer):
        """ Return an additional subset string (typically SQL) filter """

        if not self._active:
            return super(RestrictedAccessControl, self).layerFilterSubsetString(layer)

        if layer.name() == "Hello_SubsetString":
            return "pk = 1"
        elif layer.name() == "Hello_Project_SubsetString":
            return "pkuid = 6 or pkuid = 7"
        elif layer.name() == "Hello_Filter_SubsetString":
            return "pkuid = 6 or pkuid = 7"
        else:
            return None

    def layerPermissions(self, layer):
        """ Return the layer rights """

        if not self._active:
            return super(RestrictedAccessControl, self).layerPermissions(layer)

        rh = self.serverInterface().requestHandler()
        rights = QgsAccessControlFilter.LayerPermissions()
        # Used to test WFS transactions
        if rh.parameterMap().get("LAYER_PERM") == "no":
            return rights
        # Used to test the WCS
        if rh.parameterMap().get("TEST") == "dem":
            rights.canRead = layer.name() != "dem"
        else:
            rights.canRead = layer.name() not in ("Country", "Hello_OnOff")
        if layer.name() == "db_point":
            rights.canRead = rights.canInsert = rights.canUpdate = rights.canDelete = True

        return rights

    def authorizedLayerAttributes(self, layer, attributes):
        """ Return the authorised layer attributes """

        if not self._active:
            return super(RestrictedAccessControl, self).authorizedLayerAttributes(layer, attributes)

        if "color" in attributes:  # spellok
            attributes.remove("color")  # spellok
        return attributes

    def allowToEdit(self, layer, feature):
        """ Are we authorise to modify the following geometry """

        if not self._active:
            return super(RestrictedAccessControl, self).allowToEdit(layer, feature)

        return feature.attribute("color") in ["red", "yellow"]

    def cacheKey(self):
        return "r" if self._active else "f"


class TestQgsServerAccessControl(QgsServerTestBase):

    @classmethod
    def _execute_request(cls, qs, requestMethod=QgsServerRequest.GetMethod, data=None):
        if data is not None:
            data = data.encode('utf-8')
        request = QgsBufferServerRequest(qs, requestMethod, {}, data)
        response = QgsBufferServerResponse()
        cls._server.handleRequest(request, response)
        headers = []
        rh = response.headers()
        rk = sorted(rh.keys())
        for k in rk:
            headers.append(("%s: %s" % (k, rh[k])).encode('utf-8'))
        return b"\n".join(headers) + b"\n\n", bytes(response.body())

    @classmethod
    def setUpClass(cls):

        super().setUpClass()

        cls._server = cls.server  # TODO: drop this
        cls._execute_request("")
        cls._server_iface = cls.server.serverInterface()
        cls._accesscontrol = RestrictedAccessControl(cls._server_iface)
        cls._server_iface.registerAccessControl(cls._accesscontrol, 100)

    @classmethod
    def project_file(cls):
        return 'project_grp.qgs'

    def setUp(self):
        super().setUp()
        self.testdata_path = unitTestDataPath("qgis_server_accesscontrol")

        self.tmp_path = tempfile.mkdtemp()
        shutil.copytree(self.testdata_path, self.tmp_path, dirs_exist_ok=True)

        for k in ["QUERY_STRING", "QGIS_PROJECT_FILE"]:
            if k in os.environ:
                del os.environ[k]

        self.projectPath = os.path.join(self.tmp_path, self.project_file())
        self.assertTrue(os.path.isfile(self.projectPath), 'Could not find project file "{}"'.format(self.projectPath))

    def tearDown(self):
        shutil.rmtree(self.tmp_path, True)

    def _handle_request(self, restricted, query_string, **kwargs):
        self._accesscontrol._active = restricted
        qs = "?" + query_string if query_string is not None else ''
        result = self._result(self._execute_request(qs, **kwargs))
        return result

    def _result(self, data):
        headers = {}
        for line in data[0].decode('UTF-8').split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _get_fullaccess(self, query_string):
        result = self._handle_request(False, query_string)
        return result

    def _get_restricted(self, query_string):
        result = self._handle_request(True, query_string)
        return result

    def _post_fullaccess(self, data, query_string=None):
        self._server.putenv("QGIS_PROJECT_FILE", self.projectPath)
        result = self._handle_request(False, query_string, requestMethod=QgsServerRequest.PostMethod, data=data)
        self._server.putenv("QGIS_PROJECT_FILE", '')
        return result

    def _post_restricted(self, data, query_string=None):
        self._server.putenv("QGIS_PROJECT_FILE", self.projectPath)
        result = self._handle_request(True, query_string, requestMethod=QgsServerRequest.PostMethod, data=data)
        self._server.putenv("QGIS_PROJECT_FILE", '')
        return result

    def _img_diff(self, image, control_image, max_diff, max_size_diff=QSize(), outputFormat='PNG'):

        if outputFormat == 'PNG':
            extFile = 'png'
        elif outputFormat == 'JPG':
            extFile = 'jpg'
        elif outputFormat == 'WEBP':
            extFile = 'webp'
        else:
            raise RuntimeError('Yeah, new format implemented')

        temp_image = os.path.join(tempfile.gettempdir(), "%s_result.%s" % (control_image, extFile))

        with open(temp_image, "wb") as f:
            f.write(image)

        control = QgsRenderChecker()
        control.setControlPathPrefix("qgis_server_accesscontrol")
        control.setControlName(control_image)
        control.setRenderedImage(temp_image)
        if max_size_diff.isValid():
            control.setSizeTolerance(max_size_diff.width(), max_size_diff.height())
        return control.compareImages(control_image), control.report()

    def _img_diff_error(self, response, headers, image, max_diff=10, max_size_diff=QSize()):
        super()._img_diff_error(response, headers, image, max_diff=max_diff,
                                max_size_diff=max_size_diff,
                                unittest_data_path='qgis_server_accesscontrol')

    def _geo_img_diff(self, image_1, image_2):
        if os.name == 'nt':
            # Not supported on Windows due to #13061
            return 0

        with open(os.path.join(tempfile.gettempdir(), image_2), "wb") as f:
            f.write(image_1)
        image_1 = gdal.Open(os.path.join(tempfile.gettempdir(), image_2), GA_ReadOnly)
        assert image_1, "No output image written: " + image_2

        image_2 = gdal.Open(os.path.join(self.testdata_path, "results", image_2), GA_ReadOnly)
        assert image_1, "No expected image found:" + image_2

        if image_1.RasterXSize != image_2.RasterXSize or image_1.RasterYSize != image_2.RasterYSize:
            image_1 = None
            image_2 = None
            return 1000  # wrong size

        square_sum = 0
        for x in range(image_1.RasterXSize):
            for y in range(image_1.RasterYSize):
                square_sum += (image_1.ReadAsArray()[x][y] - image_2.ReadAsArray()[x][y]) ** 2

        # Explicitly close GDAL datasets
        image_1 = None
        image_2 = None
        return sqrt(square_sum)

    def _test_colors(self, colors):
        for id, color in list(colors.items()):
            response, headers = self._post_fullaccess(
                """<?xml version="1.0" encoding="UTF-8"?>
                <wfs:GetFeature {xml_ns}>
                <wfs:Query typeName="db_point" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
                <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
                <ogc:PropertyName>gid</ogc:PropertyName>
                <ogc:Literal>{id}</ogc:Literal>
                </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(id=id, xml_ns=XML_NS)
            )
            self.assertTrue(
                str(response).find("<qgs:color>{color}</qgs:color>".format(color=color)) != -1,
                "Wrong color in result\n%s" % response)
