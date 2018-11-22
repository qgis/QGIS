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
from test_qgsserver_accesscontrol import TestQgsServerAccessControl, XML_NS


class TestQgsServerAccessControlWFS(TestQgsServerAccessControl):

    def test_wfs_getcapabilities(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "GetCapabilities"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find("<Name>Hello</Name>") != -1,
            "No Hello layer in WFS/GetCapabilities\n%s" % response)
        self.assertTrue(
            str(response).find("<Name>Hello_OnOff</Name>") != -1,
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
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeFeatureType",
            "TYPENAME": "Hello"
        }.items())])

        response, headers = self._get_fullaccess(query_string)
        self.assertTrue(
            str(response).find('name="Hello"') != -1,
            "No Hello layer in DescribeFeatureType\n%s" % response)

        response, headers = self._get_restricted(query_string)
        self.assertTrue(
            str(response).find('name="Hello"') != -1,
            "No Hello layer in DescribeFeatureType\n%s" % response)

    def test_wfs_describefeaturetype_country(self):
        query_string = "&".join(["%s=%s" % i for i in list({
            "MAP": urllib.parse.quote(self.projectPath),
            "SERVICE": "WFS",
            "VERSION": "1.0.0",
            "REQUEST": "DescribeFeatureType",
            "TYPENAME": "Country"
        }.items())])

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
            str(response).find("<qgs:color>red</qgs:color>") != -1,  # spellok
            "No color in result of GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:color>red</qgs:color>") != -1,  # spellok
            "Unexpected color in result of GetFeature\n%s" % response)
        self.assertFalse(
            str(response).find("<qgs:color>NULL</qgs:color>") != -1,  # spellok
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
            <wfs:Query typeName="Hello_OnOff" srsName="EPSG:3857" xmlns:feature="http://www.qgis.org/gml">
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc"><ogc:PropertyIsEqualTo>
            <ogc:PropertyName>pkuid</ogc:PropertyName>
            <ogc:Literal>1</ogc:Literal>
            </ogc:PropertyIsEqualTo></ogc:Filter></wfs:Query></wfs:GetFeature>""".format(xml_ns=XML_NS)

        response, headers = self._post_fullaccess(data)
        self.assertTrue(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "No result in GetFeature\n%s" % response)

        response, headers = self._post_restricted(data)
        self.assertFalse(
            str(response).find("<qgs:pk>1</qgs:pk>") != -1,
            "Unexpected result in GetFeature\n%s" % response)  # spellok


# # Subset String # #

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
           For example, pkuid 6 passes the access control checks, but should not be shown because of project layer subsetString
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


if __name__ == "__main__":
    unittest.main()
