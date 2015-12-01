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

from os import path, environ
from shutil import copyfile
from math import sqrt
from subprocess import check_output
from unittest import TestCase, main
from utilities import unitTestDataPath
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from qgis.server import QgsServer, QgsAccessControlFilter
from qgis.core import QgsRenderChecker, QgsProject
from PyQt4.QtCore import QSize
import tempfile
import urllib


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

        project_path = path.join(unitTestDataPath("qgis_server_accesscontrol"), "project.qgs")
        project = QgsProject.instance()
        project.setFileName(project_path)
        project.read()

        layer_tree = project.layerTreeRoot()
        self.layer_id_name = {}
        for layer_id in layer_tree.findLayerIds():
            self.layer_id_name[layer_id] = layer_tree.findLayer(layer_id).layer().name()

    def _get_layer_name(self, layer_id):
        return self.layer_id_name[layer_id]

    def layerFilterExpression(self, layer_id):
        """ Return an additional expression filter """

        if not self._active:
            return super(RestrictedAccessControl, self).layerFilterExpression(layer_id)

        layer_name = self._get_layer_name(layer_id)

        return "$id = 1" if layer_name == "Hello" else None

    def layerFilterSubsetString(self, layer_id):
        """ Return an additional subset string (typically SQL) filter """

        if not self._active:
            return super(RestrictedAccessControl, self).layerFilterSubsetString(layer_id)

        layer_name = self._get_layer_name(layer_id)

        return "pk = 1" if layer_name == "Hello_SubsetString" else None

    def layerPermissions(self, layer_id):
        """ Return the layer rights """

        if not self._active:
            return super(RestrictedAccessControl, self).layerPermissions(layer_id)

        layer_name = self._get_layer_name(layer_id)

        rh = self.serverInterface().requestHandler()
        rights = QgsAccessControlFilter.LayerPermissions()
        # Used to test WFS transactions
        if rh.parameter("LAYER_PERM") == "no" and rh.parameterMap()["LAYER_PERM"] == "no":
            return rights
        # Used to test the WCS
        if rh.parameter("TEST") == "dem" and rh.parameterMap()["TEST"] == "dem":
            rights.canRead = layer_name != "dem"
        else:
            rights.canRead = layer_name != "Country"
        if layer_name == "db_point":
            rights.canRead = rights.canInsert = rights.canUpdate = rights.canDelete = True

        return rights

    def authorizedLayerAttributes(self, layer_id, attributes):
        """ Return the authorised layer attributes """

        if not self._active:
            return super(RestrictedAccessControl, self).authorizedLayerAttributes(layer_id, attributes)

        if "colour" in attributes:
            attributes.remove("colour")
        return attributes

    def allowToEdit(self, layer_id, feature):
        """ Are we authorise to modify the following geometry """

        if not self._active:
            return super(RestrictedAccessControl, self).allowToEdit(layer_id, feature)

        return feature.attribute("color") in ["red", "yellow"]

    def cacheKey(self):
        return "r" if self._active else "f"


server = QgsServer()
server.handleRequest()
server_iface = server.serverInterface()
accesscontrol = RestrictedAccessControl(server_iface)
server_iface.registerAccessControl(accesscontrol, 100)


class TestQgsServerAccessControl(TestCase):

    def setUp(self):
        self.testdata_path = unitTestDataPath("qgis_server_accesscontrol")

        copyfile(path.join(self.testdata_path, "helloworld.db"), path.join(self.testdata_path, "_helloworld.db"))

        for k in ["QUERY_STRING", "QGIS_PROJECT_FILE"]:
            if k in environ:
                del environ[k]

        self.projectPath = urllib.quote(path.join(self.testdata_path, "project.qgs"))

    def tearDown(self):
        copyfile(path.join(self.testdata_path, "_helloworld.db"), path.join(self.testdata_path, "helloworld.db"))

# # WMS # # WMS # # WMS # #

    def test_wms_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetLegendGraphic",
            "LAYERS": "Country",
            "FORMAT": "image/png"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetLegendGraphic_Country", 250, QSize(10, 10))

        response, headers = self._get_restricted(query_string)
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed GetLegendGraphic"
        )

    def test_wms_getmap(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetMap on Country"
        )

    def test_wms_getfeatureinfo_hello(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
            "No colour in result of GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "Unexpected colour in result of GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>NULL</qgs:colour>") != -1,
            "Unexpected colour NULL in result of GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_hello2(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "No colour in result of GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>red</qgs:colour>") != -1,
            "Unexpected colour in result of GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:colour>NULL</qgs:colour>") != -1,
            "Unexpected colour NULL in result of GetFeature\n%s" % response)

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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
        self.assertEquals(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        response, headers = self._get_restricted(query_string)
        self.assertEquals(
            headers.get("Content-Type"), "image/tiff",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            self._geo_img_diff(response, "WCS_GetCoverage.geotiff") == 0,
            "Image for GetCoverage is wrong")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
        self.assertEquals(
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
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Insert don't succeed\n%s" % response)
        self._test_colors({2: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="red"), "LAYER_PERM=no")
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Insert is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature insert permission denied</ServiceException>') != -1,
            "WFS/Transactions Insert succeed\n%s" % response)

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=yes")
        self.assertEquals(
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
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "blue"})

        response, headers = self._post_fullaccess(data.format(color="red"))
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Update don't succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="blue"))
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature modify permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=no")
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for Update is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature update permission denied</ServiceException>') != -1,
            "WFS/Transactions Update succeed\n%s" % response)
        self._test_colors({1: "red"})

        response, headers = self._post_restricted(data.format(color="yellow"), "LAYER_PERM=yes")
        self.assertEquals(
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
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete don't succeed\n%s" % response)

    def test_wfstransaction_delete_restricted(self):
        data = WFS_TRANSACTION_DELETE.format(id="1", xml_ns=XML_NS)
        self._test_colors({1: "blue"})

        response, headers = self._post_restricted(data)
        self.assertEquals(
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
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find(
                '<ServiceException code="Security">Feature delete permission denied</ServiceException>') != -1,
            "WFS/Transactions Delete succeed\n%s" % response)

        response, headers = self._post_restricted(data, "LAYER_PERM=yes")
        self.assertEquals(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find("<SUCCESS/>") != -1,
            "WFS/Transactions Delete don't succeed\n%s" % response)

# # Subset String # #

# # WMS # # WMS # # WMS # #
    def test_wms_getmap_subsetstring(self):
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": self.projectPath,
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
            "MAP": self.projectPath,
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
            "MAP": self.projectPath
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
            "MAP": self.projectPath
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

    def _handle_request(self, restricted, *args):
        accesscontrol._active = restricted
        result = self._result(server.handleRequest(*args))
        return result

    def _result(self, data):
        headers = {}
        for line in data[0].split("\n"):
            if line != "":
                header = line.split(":")
                self.assertEquals(len(header), 2, line)
                headers[str(header[0])] = str(header[1]).strip()

        return data[1], headers

    def _get_fullaccess(self, query_string):
        environ["REQUEST_METHOD"] = "GET"
        result = self._handle_request(False, query_string)
        del environ["REQUEST_METHOD"]
        return result

    def _get_restricted(self, query_string):
        environ["REQUEST_METHOD"] = "GET"
        result = self._handle_request(True, query_string)
        del environ["REQUEST_METHOD"]
        return result

    def _post_fullaccess(self, data, query_string=None):
        environ["REQUEST_METHOD"] = "POST"
        environ["REQUEST_BODY"] = data
        environ["QGIS_PROJECT_FILE"] = self.projectPath
        result = self._handle_request(False, query_string)
        del environ["REQUEST_METHOD"]
        del environ["REQUEST_BODY"]
        del environ["QGIS_PROJECT_FILE"]
        return result

    def _post_restricted(self, data, query_string=None):
        environ["REQUEST_METHOD"] = "POST"
        environ["REQUEST_BODY"] = data
        environ["QGIS_PROJECT_FILE"] = self.projectPath
        result = self._handle_request(True, query_string)
        del environ["REQUEST_METHOD"]
        del environ["REQUEST_BODY"]
        del environ["QGIS_PROJECT_FILE"]
        return result

    def _img_diff(self, image, control_image, max_diff, max_size_diff=QSize()):
        temp_image = path.join(tempfile.gettempdir(), "%s_result.png" % control_image)
        with open(temp_image, "w") as f:
            f.write(image)

        control = QgsRenderChecker()
        control.setControlPathPrefix("qgis_server_accesscontrol")
        control.setControlName(control_image)
        control.setRenderedImage(temp_image)
        if max_size_diff.isValid():
            control.setSizeTolerance(max_size_diff.width(), max_size_diff.height())
        return control.compareImages(control_image), control.report()

    def _img_diff_error(self, response, headers, image, max_diff=10, max_size_diff=QSize()):
        self.assertEquals(
            headers.get("Content-Type"), "image/png",
            "Content type is wrong: %s" % headers.get("Content-Type"))
        test, report = self._img_diff(response, image, max_diff, max_size_diff)

        result_img = check_output(["base64", path.join(tempfile.gettempdir(), image + "_result.png")])
        message = "Image is wrong\n%s\nImage:\necho '%s' | base64 -d >%s/%s_result.png" % (
            report, result_img.strip(), tempfile.gettempdir(), image
        )

        if path.isfile(path.join(tempfile.gettempdir(), image + "_result_diff.png")):
            diff_img = check_output(["base64", path.join(tempfile.gettempdir(), image + "_result_diff.png")])
            message += "\nDiff:\necho '%s' | base64 -d > %s/%s_result_diff.png" % (
                diff_img.strip(), tempfile.gettempdir(), image
            )
        self.assertTrue(test, message)

    def _geo_img_diff(self, image_1, image_2):

        with open(path.join(tempfile.gettempdir(), image_2), "w") as f:
            f.write(image_1)
        image_1 = gdal.Open(path.join(tempfile.gettempdir(), image_2), GA_ReadOnly)
        assert image_1, "No output image written: " + image_2

        image_2 = gdal.Open(path.join(self.testdata_path, "results", image_2), GA_ReadOnly)
        assert image_1, "No expected image found:" + image_2

        if image_1.RasterXSize != image_2.RasterXSize:
            return 1000  # wrong size
        if image_1.RasterYSize != image_2.RasterYSize:
            return 1000  # wrong size

        square_sum = 0
        for x in range(image_1.RasterXSize):
            for y in range(image_1.RasterYSize):
                square_sum += (image_1.ReadAsArray()[x][y] - image_2.ReadAsArray()[x][y]) ** 2

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
    main()
