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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

print('CTEST_FULL_OUTPUT')

import qgis  # NOQA

import os
from shutil import copyfile
from math import sqrt
from qgis.testing import unittest
from utilities import unitTestDataPath
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from qgis.server import QgsServer, QgsAccessControlFilter
from qgis.core import QgsRenderChecker
from qgis.PyQt.QtCore import QSize
import tempfile
import urllib
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

WFS_TRANSACTION_INSERT = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Insert idgen="GenerateNew">
    <qgs:db_point>
      <qgs:geometry>
        <gml:Point srsDimension="2" srsName="http://www.opengis.net/def/crs/EPSG/0/4326">
          <gml:coordinates decimal="." cs="," ts=" ">{x},{y}</gml:coordinates>
        </gml:Point>
      </qgs:geometry>
      <qgs:name>{name}</qgs:name>
      <qgs:color>{color}</qgs:color>
    </qgs:db_point>
  </wfs:Insert>
</wfs:Transaction>""".format(x=1000, y=2000, name="test", color="{color}", xml_ns=XML_NS)

WFS_TRANSACTION_UPDATE = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Update typeName="db_point">
    <wfs:Property>
      <wfs:Name>color</wfs:Name>
      <wfs:Value>{color}</wfs:Value>
    </wfs:Property>
    <ogc:Filter>
      <ogc:FeatureId fid="{id}"/>
   </ogc:Filter>
  </wfs:Update>
</wfs:Transaction>"""

WFS_TRANSACTION_DELETE = """<?xml version="1.0" encoding="UTF-8"?>
<wfs:Transaction {xml_ns}>
  <wfs:Delete typeName="db_point">
    <ogc:Filter>
      <ogc:FeatureId fid="{id}"/>
    </ogc:Filter>
  </wfs:Delete>
</wfs:Transaction>"""


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

        return "$id = 1" if layer.name() == "Hello" else None

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
        if rh.parameter("LAYER_PERM") == "no" and rh.parameterMap()["LAYER_PERM"] == "no":
            return rights
        # Used to test the WCS
        if rh.parameter("TEST") == "dem" and rh.parameterMap()["TEST"] == "dem":
            rights.canRead = layer.name() != "dem"
        else:
            rights.canRead = layer.name() != "Country"
        if layer.name() == "db_point":
            rights.canRead = rights.canInsert = rights.canUpdate = rights.canDelete = True

        return rights

    def authorizedLayerAttributes(self, layer, attributes):
        """ Return the authorised layer attributes """

        if not self._active:
            return super(RestrictedAccessControl, self).authorizedLayerAttributes(layer, attributes)

        if "colour" in attributes:
            attributes.remove("colour")
        return attributes

    def allowToEdit(self, layer, feature):
        """ Are we authorise to modify the following geometry """

        if not self._active:
            return super(RestrictedAccessControl, self).allowToEdit(layer, feature)

        return feature.attribute("color") in ["red", "yellow"]

    def cacheKey(self):
        return "r" if self._active else "f"


server = QgsServer()
server.handleRequest()
server_iface = server.serverInterface()
accesscontrol = RestrictedAccessControl(server_iface)
server_iface.registerAccessControl(accesscontrol, 100)


class TestQgsServerAccessControl(unittest.TestCase):

    def setUp(self):
        self.testdata_path = unitTestDataPath("qgis_server_accesscontrol")

        dataFile = os.path.join(self.testdata_path, "helloworld.db")
        self.assertTrue(os.path.isfile(dataFile), 'Could not find data file "{}"'.format(dataFile))
        copyfile(dataFile, os.path.join(self.testdata_path, "_helloworld.db"))

        for k in ["QUERY_STRING", "QGIS_PROJECT_FILE"]:
            if k in os.environ:
                del os.environ[k]

        self.projectPath = os.path.join(self.testdata_path, "project.qgs")
        self.assertTrue(os.path.isfile(self.projectPath), 'Could not find project file "{}"'.format(self.projectPath))

    def tearDown(self):
        copyfile(os.path.join(self.testdata_path, "_helloworld.db"), os.path.join(self.testdata_path, "helloworld.db"))

# # WMS # # WMS # # WMS # #

    def test_wms_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetCapabilities"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<Name>Hello</Name>") != -1,
            "No Hello layer in GetCapabilities\n%s" % response)
        self.assertTrue(
            str(response).find("<Name>Country</Name>") != -1,
            "No Country layer in GetCapabilities\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<Name>Hello</Name>") != -1,
            "No Hello layer in GetCapabilities\n%s" % response)
        self.assertFalse(
            str(response).find("<Name>Country</Name>") != -1,
            "Country layer in GetCapabilities\n%s" % response)

    def test_wms_describelayer_hello(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "DescribeLayer",
            "LAYERS": "Hello",
            "SLD_VERSION": "1.1.0"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Hello</se:FeatureTypeName>") != -1,
            "No Hello layer in DescribeLayer\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Hello</se:FeatureTypeName>") != -1,
            "No Hello layer in DescribeLayer\n%s" % response)

    def test_wms_describelayer_country(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "DescribeLayer",
            "LAYERS": "Country",
            "SLD_VERSION": "1.1.0"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Country</se:FeatureTypeName>") != -1,
            "No Country layer in DescribeLayer\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<se:FeatureTypeName>Country</se:FeatureTypeName>") != -1,
            "Country layer in DescribeLayer\n%s" % response)

    def test_wms_getlegendgraphic_hello(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Hello",
            "FORMAT": "image/png"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Hello", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Hello", 250, QSize(10, 10))

    def test_wms_getlegendgraphic_country(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Country",
            "FORMAT": "image/png"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Country", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed GetLegendGraphic"
        )

    def test_wms_getmap(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])
        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetMap on Country"
        )

    def test_wms_getfeatureinfo_hello(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "No color in result of GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "Unexpected color in result of GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>NULL</qgs:colour>") != -1,
            "Unexpected color NULL in result of GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_hello2(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "Unexpected result in GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_country(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "Unexpected result in GetFeatureInfo\n%s" % response)

# # WFS # # WFS # # WFS # #

    def test_wfs_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.1.0",
            "REQUEST": "GetCapabilities"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<Name>Hello</Name>") != -1,
            "No Hello layer in WFS/GetCapabilities\n%s" % response)
        self.assertTrue(
            str(response).find("<Name>Country</Name>") != -1,
            "No Country layer in WFS/GetCapabilities\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<Name>Hello</Name>") != -1,
            "No Hello layer in WFS/GetCapabilities\n%s" % response)
        self.assertFalse(
            str(response).find("<Name>Country</Name>") != -1,
            "Unexpected Country layer in WFS/GetCapabilities\n%s" % response)

    def test_wfs_describefeaturetype_hello(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.1.0",
            "REQUEST": "DescribeFeatureType",
            "TYPENAME": "Hello"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find('name="Hello"') != -1,
            "No Hello layer in DescribeFeatureType\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find('name="Hello"') != -1,
            "No Hello layer in DescribeFeatureType\n%s" % response)

    def test_wfs_describefeaturetype_country(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.1.0",
            "REQUEST": "DescribeFeatureType",
            "TYPENAME": "Country"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find('name="Country"') != -1,
            "No Country layer in DescribeFeatureType\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find('name="Country"') != -1,
            "Unexpected Country layer in DescribeFeatureType\n%s" % response)

    def test_wfs_getfeature_hello(self):
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>1</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "No color in result of GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "Unexpected color in result of GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>NULL</qgs:colour>") != -1,
            "Unexpected color NULL in result of GetFeature\n%s" % response)

    def test_wfs_getfeature_hello2(self):
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>2</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "Unexpected result in GetFeature\n%s" % response)

    def test_wfs_getfeature_country(self):
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Country" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pk</ogc:PropertyName>
            <ogc:Literal>1</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "Unexpeced result in GetFeatureInfo\n%s" % response)


# # WCS # # WCS # # WCS # #

    def test_wcs_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in WCS/GetCapabilities\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in WCS/GetCapabilities\n%s" % response)

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities",
            "TEST": "dem",
        }.items()])

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<name>dem</name>") != -1,
            "Unexpected dem layer in WCS/GetCapabilities\n%s" % response)

    def test_wcs_describecoverage(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeCoverage",
            "COVERAGE": "dem",
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in DescribeCoverage\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<name>dem</name>") != -1,
            "No dem layer in DescribeCoverage\n%s" % response)

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeCoverage",
            "COVERAGE": "dem",
            "TEST": "dem",
        }.items()])

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<name>dem</name>") != -1,
            "Unexpected dem layer in DescribeCoverage\n%s" % response)

    def test_wcs_getcoverage(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCoverage",
            "COVERAGE": "dem",
            "CRS": "EPSG:3857",
            "BBOX": "-1387454,4252256,431091,5458375",
            "HEIGHT": "100",
            "WIDTH": "100",
            "FORMAT": "GTiff",
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WCS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCoverage",
            "COVERAGE": "dem",
            "CRS": "EPSG:3857",
            "BBOX": "-1387454,4252256,431091,5458375",
            "HEIGHT": "100",
            "WIDTH": "100",
            "FORMAT": "GTiff",
            "TEST": "dem",
        }.items()])

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed GetCoverage")


# # WFS/Transactions # #

    def test_wfstransaction_insert(self):
        data = WFS_TRANSACTION_INSERT.format(x=1000, y=2000, name="test", color="{color}", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Insert don't succeed\n%s" % response)
        self._test_colors({2: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="red"), "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature insert permission denied</ServiceException>') != -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Insert don't succeed\n%s" % response)
        self._test_colors({3: "yellow"})

    def test_wfstransaction_update(self):
        data = WFS_TRANSACTION_UPDATE.format(id="1", color="{color}", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data.format(color="yellow"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Update don't succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature update permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Update don't succeed\n%s" % response)
        self._test_colors({1: "yellow"})

    def test_wfstransaction_delete_fullaccess(self):
        data = WFS_TRANSACTION_DELETE.format(id="1", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete don't succeed\n%s" % response)

    def test_wfstransaction_delete_restricted(self):
        data = WFS_TRANSACTION_DELETE.format(id="1", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Delete succeed\n%s" % response)

        data_update = WFS_TRANSACTION_UPDATE.format(id="1", color="red", xml_ns=XML_NS)
        response, headers = self._post_fullaccess(data_update)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data, "LAYER_PERM=no")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature delete permission denied</ServiceException>') != -1,
            "WFS/Transactions Delete succeed\n%s" % response)

        response, headers = self._post_restricted(data, "LAYER_PERM=yes")
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete don't succeed\n%s" % response)

# # Subset String # #

# # WMS # # WMS # # WMS # #
    def test_wms_getmap_subsetstring(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap")

    def test_wms_getmap_subsetstring_with_filter(self):
        """ test that request filter and access control subsetStrings are correctly combined. Note that for this
        test we reuse the projectsubsetstring reference images as we are using filter requests to set the same
        filter " pkuid in (7,8) " as the project subsetstring uses for its test.
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap_projectsubstring")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap_projectsubstring")

    def test_wms_getmap_projectsubsetstring(self):
        """ test that project set layer subsetStrings are honored"""
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap_projectsubstring")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap_projectsubstring")

    def test_wms_getfeatureinfo_subsetstring(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello_SubsetString",
            "QUERY_LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeatureInfo Hello/1\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeatureInfo Hello/1\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeatureInfo Hello/1\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeatureInfo Hello/1\n%s" % response)

    def test_wms_getfeatureinfo_subsetstring2(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello_SubsetString",
            "QUERY_LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Unexpected result result in GetFeatureInfo Hello/2\n%s" % response)

    def test_wms_getfeatureinfo_projectsubsetstring(self):
        """test that layer subsetStrings set in projects are honored. This test checks for a feature which should be filtered
        out by the project set layer subsetString
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Project_SubsetString",
            "QUERY_LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Project set layer subsetString not honored in WMS GetFeatureInfo/1\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Project set layer subsetString not honored in WMS GetFeatureInfo when access control applied/1\n%s" % response)

    def test_wms_getfeatureinfo_projectsubsetstring5(self):
        """test that layer subsetStrings set in projects are honored. This test checks for a feature which should pass
        both project set layer subsetString and access control filters
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Project_SubsetString",
            "QUERY_LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1623412,3146330,-1603412,3166330",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

    def test_wms_getfeatureinfo_projectsubsetstring3(self):
        """test that layer subsetStrings set in projects are honored. This test checks for a feature which should pass
        the project set layer subsetString but fail the access control checks
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Project_SubsetString",
            "QUERY_LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "3415650,2018968,3415750,2019968",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>8</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Unexpected result from GetFeatureInfo Hello/2\n%s" % response)

    def test_wms_getfeatureinfo_subsetstring_with_filter(self):
        """test that request filters are honored. This test checks for a feature which should be filtered
        out by the request filter
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Filter_SubsetString",
            "QUERY_LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-4710778,5696513,14587125",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Request filter not honored in WMS GetFeatureInfo/1\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Request filter not honored in WMS GetFeatureInfo when access control applied/1\n%s" % response)

    def test_wms_getfeatureinfo_projectsubsetstring4(self):
        """test that request filters are honored. This test checks for a feature which should pass
        both request filter and access control filters
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Filter_SubsetString",
            "QUERY_LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-1623412,3146330,-1603412,3166330",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

    def test_wms_getfeatureinfo_projectsubsetstring2(self):
        """test that request filters are honored. This test checks for a feature which should pass
        the request filter but fail the access control checks
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Filter_SubsetString",
            "QUERY_LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "3415650,2018968,3415750,2019968",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.quote(self.projectPath)
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result result in GetFeatureInfo Hello/2\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>8</qgs:pk>") != -1,
            "No good result result in GetFeatureInfo Hello/2\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Unexpected result from GetFeatureInfo Hello/2\n%s" % response)

# # WFS # # WFS # # WFS # #

    def test_wfs_getfeature_subsetstring(self):
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello_SubsetString" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>1</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeature\n%s" % response)

    def test_wfs_getfeature_subsetstring2(self):
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello_SubsetString" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>2</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "No good result in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Unexpected result in GetFeature\n%s" % response)

    def test_wfs_getfeature_project_subsetstring(self):
        """Tests access control with a subset string already applied to a layer in a project
           'Hello_Project_SubsetString' layer has a subsetString of "pkuid in (7,8)"
           This test checks for retrieving a feature which should be available in with/without access control
        """
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello_Project_SubsetString" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>7</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        # should be one result
        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "Feature with pkuid=7 not found in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>7</qgs:pk>") != -1,
            "Feature with pkuid=7 not found in GetFeature, has been incorrectly filtered out by access controls\n%s" % response)

    def test_wfs_getfeature_project_subsetstring2(self):
        """Tests access control with a subset string already applied to a layer in a project
           'Hello_Project_SubsetString' layer has a subsetString of "pkuid in (7,8)"
           This test checks for a feature which should be filtered out by access controls
        """
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello_Project_SubsetString" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>8</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        # should be one result
        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>8</qgs:pk>") != -1,
            "Feature with pkuid=8 not found in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Feature with pkuid=8 was found in GetFeature, but should have been filtered out by access controls\n%s" % response)

    def test_wfs_getfeature_project_subsetstring3(self):
        """Tests access control with a subset string already applied to a layer in a project
           'Hello_Project_SubsetString' layer has a subsetString of "pkuid in (7,8)"
           This test checks for a features which should be filtered out by project subsetStrings.
           Eg pkuid 6 passes the access control checks, but should not be shown because of project layer subsetString
        """
        data = """<?xml version="1.0" encoding="UTF-8"?>
            <wfs:GetFeature {xml_ns}>
            <wfs:Query typeName="Hello_Project_SubsetString" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>6</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        # should be no results, since pkuid 1 should be filtered out by project subsetString
        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>") == -1,
            "Project based layer subsetString not respected in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>") != -1,
            "Project based layer subsetString not respected in GetFeature with restricted access\n%s" % response)

    def _handle_request(self, restricted, *args):
        accesscontrol._active = restricted
        result = self._result(server.handleRequest(*args))
        return result

    def _result(self, data):
        headers = {}
        for line in data[0].split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEqual(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _get_fullaccess(self, query_string):
        server.putenv("REQUEST_METHOD", "GET")
        result = self._handle_request(False, query_string)
        server.putenv("REQUEST_METHOD", '')
        return result

    def _get_restricted(self, query_string):
        server.putenv("REQUEST_METHOD", "GET")
        result = self._handle_request(True, query_string)
        server.putenv("REQUEST_METHOD", '')
        return result

    def _post_fullaccess(self, data, query_string=None):
        server.putenv("REQUEST_METHOD", "POST")
        server.putenv("REQUEST_BODY", data)
        server.putenv("QGIS_PROJECT_FILE", self.projectPath)
        result = self._handle_request(False, query_string)
        server.putenv("REQUEST_METHOD", '')
        server.putenv("REQUEST_BODY", '')
        server.putenv("QGIS_PROJECT_FILE", '')
        return result

    def _post_restricted(self, data, query_string=None):
        server.putenv("REQUEST_METHOD", "POST")
        server.putenv("REQUEST_BODY", data)
        server.putenv("QGIS_PROJECT_FILE", self.projectPath)
        result = self._handle_request(True, query_string)
        server.putenv("REQUEST_METHOD", '')
        server.putenv("REQUEST_BODY", '')
        server.putenv("QGIS_PROJECT_FILE", '')
        return result

    def _img_diff(self, image, control_image, max_diff, max_size_diff=QSize()):
        temp_image = os.path.join(tempfile.gettempdir(), "%s_result.png" % control_image)

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
        self.assertEqual(
            headers.get("Content-Type"), "image/png",
            "Content type is wrong: %s" % headers.get("Content-Type"))
        test, report = self._img_diff(response, image, max_diff, max_size_diff)

        with open(os.path.join(tempfile.gettempdir(), image + "_result.png"), "rb") as rendered_file:
            encoded_rendered_file = base64.b64encode(rendered_file.read())
            message = "Image is wrong\n%s\nImage:\necho '%s' | base64 -d >%s/%s_result.png" % (
                report, encoded_rendered_file.strip(), tempfile.gettempdir(), image
            )

        with open(os.path.join(tempfile.gettempdir(), image + "_result_diff.png"), "rb") as diff_file:
            encoded_diff_file = base64.b64encode(diff_file.read())
            message += "\nDiff:\necho '%s' | base64 -d > %s/%s_result_diff.png" % (
                encoded_diff_file.strip(), tempfile.gettempdir(), image
            )

        self.assertTrue(test, message)

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
        for id, color in colors.items():
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

if __name__ == "__main__":
    unittest.main()
