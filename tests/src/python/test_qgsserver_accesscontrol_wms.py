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

from qgis.testing import unittest
import urllib.request
import urllib.parse
import urllib.error
from test_qgsserver_accesscontrol import TestQgsServerAccessControl


class TestQgsServerAccessControlWMS(TestQgsServerAccessControl):

    def test_wms_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetCapabilities"
        }.items())])

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
            "Unexpected Country layer in GetCapabilities\n%s" % response)

    def test_wms_getprojectsettings(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetProjectSettings"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<TreeName>Hello</TreeName>") != -1,
            "No Hello layer in GetProjectSettings\n%s" % response)
        self.assertTrue(
            str(response).find("<TreeName>Country</TreeName>") != -1,
            "No Country layer in GetProjectSettings\n%s" % response)
        self.assertTrue(
            str(response).find("<TreeName>Country_grp</TreeName>") != -1,
            "No Country_grp layer in GetProjectSettings\n%s" % response)
        self.assertTrue(
            str(response).find("<LayerDrawingOrder>Country_Diagrams,Country_Labels,Country,dem,Hello_Filter_SubsetString,Hello_Project_SubsetString,Hello_SubsetString,Hello,db_point</LayerDrawingOrder>") != -1,
            "LayerDrawingOrder in GetProjectSettings\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<TreeName>Hello</TreeName>") != -1,
            "No Hello layer in GetProjectSettings\n%s" % response)
        self.assertFalse(
            str(response).find("<TreeName>Country</TreeName>") != -1,
            "Unexpected Country layer in GetProjectSettings\n%s" % response)
        self.assertFalse(
            str(response).find("<TreeName>Country_grp</TreeName>") != -1,
            "Unexpected Country_grp layer in GetProjectSettings\n%s" % response)
        self.assertTrue(
            str(response).find("<LayerDrawingOrder>Country_Diagrams,Country_Labels,dem,Hello_Filter_SubsetString,Hello_Project_SubsetString,Hello_SubsetString,Hello,db_point</LayerDrawingOrder>") != -1,
            "Wrong LayerDrawingOrder in GetProjectSettings\n%s" % response)

    def test_wms_getprojectsettings(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetContext"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("name=\"Hello\"") != -1,
            "No Hello layer in GetContext\n%s" % response)
        self.assertTrue(
            str(response).find("name=\"Country\"") != -1,
            "No Country layer in GetProjectSettings\n%s" % response)
        self.assertTrue(
            str(response).find("name=\"Country\"")
            < str(response).find("name=\"Hello\""),
            "Hello layer not after Country layer\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("name=\"Hello\"") != -1,
            "No Hello layer in GetContext\n%s" % response)
        self.assertFalse(
            str(response).find("name=\"Country\"") != -1,
            "Unexpected Country layer in GetProjectSettings\n%s" % response)

    def test_wms_describelayer_hello(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "DescribeLayer",
            "LAYERS": "Hello",
            "SLD_VERSION": "1.1.0"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Hello</se:FeatureTypeName>") != -1,
            "No Hello layer in DescribeLayer\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Hello</se:FeatureTypeName>") != -1,
            "No Hello layer in DescribeLayer\n%s" % response)

    def test_wms_describelayer_country(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "DescribeLayer",
            "LAYERS": "Country",
            "SLD_VERSION": "1.1.0"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<se:FeatureTypeName>Country</se:FeatureTypeName>") != -1,
            "No Country layer in DescribeLayer\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<se:FeatureTypeName>Country</se:FeatureTypeName>") != -1,
            "Unexpected Country layer in DescribeLayer\n%s" % response)

    def test_wms_getmap(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])
        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetMap on Country"
        )

    def test_wms_getmap_grp(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_grp,Country_grp",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_grp",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country_grp",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])
        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetMap is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetMap on Country_grp"
        )

    def test_wms_getfeatureinfo_hello(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:color>red</qgs:color>") != -1,  # spellok
            "No color in result of GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetFeatureInfo is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetFeatureInfo on Country"
        )

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello",
            "QUERY_LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items())])
        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:color>red</qgs:color>") != -1,  # spellok
            "Unexpected color in result of GetFeatureInfo\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:color>NULL</qgs:color>") != -1,  # spellok
            "Unexpected color NULL in result of GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_hello_grp(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country_grp,Hello_grp",
            "QUERY_LAYERS": "Hello_grp",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertFalse(
            str(response).find("Layer \'Hello_grp\' is not queryable") != -1,
            "The WMS layer group are queryable GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("Layer \'Hello_grp\' is not queryable") != -1,
            "The WMS layer group are queryable GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_hello2(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Hello",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>2</qgs:pk>") != -1,
            "Unexpected result in GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_country(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello",
            "QUERY_LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "Unexpected result in GetFeatureInfo\n%s" % response)

    def test_wms_getfeatureinfo_country_grp(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country_grp,Hello",
            "QUERY_LAYERS": "Country",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeatureInfo\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertFalse(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "Unexpected result in GetFeatureInfo\n%s" % response)


# # Subset String # #

    def test_wms_getmap_subsetstring(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Country,Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap")

    def test_wms_getmap_subsetstring_with_filter(self):
        """ test that request filter and access control subsetStrings are correctly combined. Note that for this
        test we reuse the projectsubsetstring reference images as we are using filter requests to set the same
        filter " pkuid in (7,8) " as the project subsetstring uses for its test.
        """
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap_projectsubstring")

        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap_projectsubstring")

        filter = "<Filter><Or><PropertyIsEqualTo><PropertyName>pkuid</PropertyName><Literal>7</Literal>" \
                 "</PropertyIsEqualTo><PropertyIsEqualTo><PropertyName>pkuid</PropertyName><Literal>8</Literal>" \
                 "</PropertyIsEqualTo></Or></Filter>"
        query_string = "&".join(["%s=%s" % i for i in {
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Filter_SubsetString",
            "FILTER": filter,
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items()])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap_projectsubstring_OGC")

    def test_wms_getmap_projectsubsetstring(self):
        """ test that project set layer subsetStrings are honored"""
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_GetMap_projectsubstring")

        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetMap",
            "LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857"
        }.items())])

        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_GetMap_projectsubstring")

    def test_wms_getfeatureinfo_subsetstring(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello_SubsetString",
            "QUERY_LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeatureInfo Hello/1\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeatureInfo Hello/1\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertEqual(
            headers.get("Content-Type"), "text/xml; charset=utf-8",
            "Content type for GetFeatureInfo is wrong: %s" % headers.get("Content-Type"))
        self.assertTrue(
            str(response).find('<ServiceException code="Security">') != -1,
            "Not allowed do a GetFeatureInfo on Country"
        )

        query_string = "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_SubsetString",
            "QUERY_LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])
        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find("<qgs:pk>") != -1,
            "No result in GetFeatureInfo Hello/1\n%s" % response)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No good result in GetFeatureInfo Hello/1\n%s" % response)

    def test_wms_getfeatureinfo_subsetstring2(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Country,Hello_SubsetString",
            "QUERY_LAYERS": "Hello_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "146",
            "Y": "160",
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Project_SubsetString",
            "QUERY_LAYERS": "Hello_Project_SubsetString",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
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
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
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
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
            "SERVICE": "WMS",
            "VERSION": "1.1.1",
            "REQUEST": "GetFeatureInfo",
            "LAYERS": "Hello_Filter_SubsetString",
            "QUERY_LAYERS": "Hello_Filter_SubsetString",
            "FILTER": "Hello_Filter_SubsetString:\"pkuid\" IN ( 7 , 8 )",
            "STYLES": "",
            "FORMAT": "image/png",
            "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
            "HEIGHT": "500",
            "WIDTH": "500",
            "SRS": "EPSG:3857",
            "FEATURE_COUNT": "10",
            "INFO_FORMAT": "application/vnd.ogc.gml",
            "X": "56",
            "Y": "144",
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
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
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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
        query_string = "&".join(["%s=%s" % i for i in list({
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
            "MAP": urllib.parse.quote(self.projectPath)
        }.items())])

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


if __name__ == "__main__":
    unittest.main()
