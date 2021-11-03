# -*- coding: utf-8 -*-
"""QGIS Unit tests for the WFS provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-03-25'
__copyright__ = 'Copyright 2016, Even Rouault'

import hashlib
import os
import re
import shutil
import tempfile
import http.server
import threading
import socketserver

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ['QT_HASH_SEED'] = '1'

from qgis.PyQt.QtCore import QCoreApplication, Qt, QObject, QDateTime, QEventLoop

from qgis.core import (
    QgsWkbTypes,
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsRectangle,
    QgsPointXY,
    QgsVectorDataProvider,
    QgsFeatureRequest,
    QgsApplication,
    QgsSettings,
    QgsExpression,
    QgsExpressionContextUtils,
    QgsExpressionContext,
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        ret = endpoint + hashlib.md5(x.replace('/', '_').encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    ret = endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"',
                                                                                                       '_').replace("'",
                                                                                                                    '_').replace(
        ' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')
    return ret


class MessageLogger(QObject):

    def __init__(self, tag=None):
        QObject.__init__(self)
        self.log = []
        self.tag = tag

    def __enter__(self):
        QgsApplication.messageLog().messageReceived.connect(self.logMessage)
        return self

    def __exit__(self, type, value, traceback):
        QgsApplication.messageLog().messageReceived.disconnect(self.logMessage)

    def logMessage(self, msg, tag, level):
        if tag == self.tag or not self.tag:
            self.log.append(msg.encode('UTF-8'))

    def messages(self):
        return self.log


class TestPyQgsWFSProvider(unittest.TestCase, ProviderTestCase):

    def treat_date_as_datetime(self):
        return True

    def treat_time_as_string(self):
        return True

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWFSProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <!-- add a trailing space to the name to test https://github.com/qgis/QGIS/issues/13486 -->
          <xsd:element maxOccurs="1" minOccurs="0" name="pk  " nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="cnt" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="name" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="name2" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="num_char" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="dt" nillable="true" type="xsd:datetime"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="date" nillable="true" type="xsd:datetime"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="time" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
          <!-- check that an element with ref without name doesn't confuse the DescribeFeatureType analyzer -->
          <xsd:element maxOccurs="0" minOccurs="0" ref="my:somethingElseType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
  <xsd:complexType name="somethingElseType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        cls.vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename'", 'test', 'WFS')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="5" numberReturned="5" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.0">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>66.33 -70.332</gml:lowerCorner><gml:upperCorner>66.33 -70.332</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>1</my:pk>
      <my:cnt>100</my:cnt>
      <my:name>Orange</my:name>
      <my:name2>oranGe</my:name2>
      <my:num_char>1</my:num_char>
      <my:dt>2020-05-03 12:13:14</my:dt>
      <my:date>2020-05-03</my:date>
      <my:time>12:13:14</my:time>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>70.8 -68.2</gml:lowerCorner><gml:upperCorner>70.8 -68.2</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>70.8 -68.2</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>2</my:pk>
      <my:cnt>200</my:cnt>
      <my:name>Apple</my:name>
      <my:name2>Apple</my:name2>
      <my:num_char>2</my:num_char>
      <my:dt>2020-05-04 12:14:14</my:dt>
      <my:date>2020-05-04</my:date>
      <my:time>12:14:14</my:time>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.2">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>78.3 -65.32</gml:lowerCorner><gml:upperCorner>78.3 -65.32</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.2"><gml:pos>78.3 -65.32</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>4</my:pk>
      <my:cnt>400</my:cnt>
      <my:name>Honey</my:name>
      <my:name2>Honey</my:name2>
      <my:num_char>4</my:num_char>
      <my:dt>2021-05-04 13:13:14</my:dt>
      <my:date>2021-05-04</my:date>
      <my:time>13:13:14</my:time>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.3">
      <my:pk>3</my:pk>
      <my:cnt>300</my:cnt>
      <my:name>Pear</my:name>
      <my:name2>PEaR</my:name2>
      <my:num_char>3</my:num_char>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.4">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>78.23 -71.123</gml:lowerCorner><gml:upperCorner>78.23 -71.123</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.4"><gml:pos>78.23 -71.123</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>5</my:pk>
      <my:cnt>-200</my:cnt>
      <my:name2>NuLl</my:name2>
      <my:num_char>5</my:num_char>
      <my:dt>2020-05-04 12:13:14</my:dt>
      <my:date>2020-05-02</my:date>
      <my:time>12:13:01</my:time>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="5" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:And>
  <fes:PropertyIsGreaterThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>100</fes:Literal>
  </fes:PropertyIsGreaterThan>
  <fes:PropertyIsLessThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>410</fes:Literal>
  </fes:PropertyIsLessThan>
 </fes:And>
</fes:Filter>
&RESULTTYPE=hits"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       numberMatched="3" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:And>
  <fes:PropertyIsGreaterThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>100</fes:Literal>
  </fes:PropertyIsGreaterThan>
  <fes:PropertyIsLessThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>410</fes:Literal>
  </fes:PropertyIsLessThan>
 </fes:And>
</fes:Filter>
"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="3" numberReturned="3" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>70.8 -68.2</gml:lowerCorner><gml:upperCorner>70.8 -68.2</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>70.8 -68.2</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>2</my:pk>
      <my:cnt>200</my:cnt>
      <my:name>Apple</my:name>
      <my:name2>Apple</my:name2>
      <my:num_char>2</my:num_char>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.2">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>78.3 -65.32</gml:lowerCorner><gml:upperCorner>78.3 -65.32</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.2"><gml:pos>78.3 -65.32</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>4</my:pk>
      <my:cnt>400</my:cnt>
      <my:name>Honey</my:name>
      <my:name2>Honey</my:name2>
      <my:num_char>4</my:num_char>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.3">
      <my:pk>3</my:pk>
      <my:cnt>300</my:cnt>
      <my:name>Pear</my:name>
      <my:name2>PEaR</my:name2>
      <my:num_char>3</my:num_char>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:And>
  <fes:PropertyIsGreaterThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>100</fes:Literal>
  </fes:PropertyIsGreaterThan>
  <fes:PropertyIsLessThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>400</fes:Literal>
  </fes:PropertyIsLessThan>
 </fes:And>
</fes:Filter>
&RESULTTYPE=hits"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:And>
  <fes:PropertyIsGreaterThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>100</fes:Literal>
  </fes:PropertyIsGreaterThan>
  <fes:PropertyIsLessThan>
   <fes:ValueReference>cnt</fes:ValueReference>
   <fes:Literal>400</fes:Literal>
  </fes:PropertyIsLessThan>
 </fes:And>
</fes:Filter>
"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="2" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>70.8 -68.2</gml:lowerCorner><gml:upperCorner>70.8 -68.2</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>70.8 -68.2</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>2</my:pk>
      <my:cnt>200</my:cnt>
      <my:name>Apple</my:name>
      <my:name2>Apple</my:name2>
      <my:num_char>2</my:num_char>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.3">
      <my:pk>3</my:pk>
      <my:cnt>300</my:cnt>
      <my:name>Pear</my:name>
      <my:name2>PEaR</my:name2>
      <my:num_char>3</my:num_char>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>name</fes:ValueReference>
  <fes:Literal>Apple</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
&RESULTTYPE=hits"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       numberMatched="1" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>name</fes:ValueReference>
  <fes:Literal>Apple</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="1" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.1">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>70.8 -68.2</gml:lowerCorner><gml:upperCorner>70.8 -68.2</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>70.8 -68.2</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>2</my:pk>
      <my:cnt>200</my:cnt>
      <my:name>Apple</my:name>
      <my:name2>Apple</my:name2>
      <my:num_char>2</my:num_char>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>name</fes:ValueReference>
  <fes:Literal>AppleBearOrangePear</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
&RESULTTYPE=hits"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       numberMatched="0" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>name</fes:ValueReference>
  <fes:Literal>AppleBearOrangePear</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="0" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        shutil.rmtree(cls.basetestpath, True)
        cls.vl = None  # so as to properly close the provider and remove any temporary file

    def tearDown(self):
        """Run after each test"""
        # clear possible settings modification made during test
        QgsSettings().clear()

    def testWkbType(self):
        """N/A for WFS provider"""
        pass

    def testInconsistentUri(self):
        """Test a URI with a typename that doesn't match a type of the capabilities"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testInconsistentUri'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        # Could not find typename my:typename in capabilities
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename'", 'test', 'WFS')
        self.assertFalse(vl.isValid())

    def testWFS10(self):
        """Test WFS 1.0 read-only"""
        # We also test attribute fields in upper-case, and a field named GEOMETRY

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS1.0'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="GEOMETRY" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
          <!-- use geometry that is the default SpatiaLite geometry name -->
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(len(vl.fields()), 5)
        self.assertEqual(vl.featureCount(), 0)
        reference = QgsGeometry.fromRect(QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'),
                'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
      </my:geometry>
      <my:INTFIELD>1</my:INTFIELD>
      <my:GEOMETRY>2</my:GEOMETRY>
      <my:longfield>1234567890123</my:longfield>
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # Also test that on file iterator works
        os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD'] = '0'

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        del os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD']

        values = [f['GEOMETRY'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['foo'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

        self.assertEqual(vl.featureCount(), 1)

        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.SelectAtId)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertFalse(ret)

        self.assertFalse(vl.dataProvider().deleteFeatures([0]))

        # Reload data and check for fid stability of features whose fid/gmlid
        # has already been downloaded
        vl.dataProvider().reloadData()

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'),
                'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.10">
      <my:INTFIELD>20</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>30</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        features = [f for f in vl.getFeatures()]
        self.assertEqual(features[0].id(), 2)
        self.assertEqual(features[0]['INTFIELD'], 20)
        self.assertEqual(features[1].id(), 1)
        self.assertEqual(features[1]['INTFIELD'], 30)

        # Test with restrictToRequestBBOX=1
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&BBOX=400000,5400000,450000,5500000'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
      </my:geometry>
      <my:INTFIELD>100</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.0.0' restrictToRequestBBOX=1", 'test',
            'WFS')

        extent = QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [(f.id(), f['INTFIELD']) for f in vl.getFeatures(request)]
        self.assertEqual(values[0][1], 100)

        # Issue a request by id on a cached feature
        request = QgsFeatureRequest(values[0][0])
        values = [(f.id(), f['INTFIELD']) for f in vl.getFeatures(request)]
        self.assertEqual(values[0][1], 100)

        # Check behavior with setLimit(1)
        request = QgsFeatureRequest().setLimit(1)
        values = [(f.id(), f['INTFIELD']) for f in vl.getFeatures(request)]
        self.assertEqual(values[0][1], 100)

    def testWFS10_outputformat_GML3(self):
        """Test WFS 1.0 with OUTPUTFORMAT=GML3"""
        # We also test attribute fields in upper-case, and a field named GEOMETRY

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS1.0_gml3'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <Capability>
    <Request>
      <GetFeature>
        <ResultFormat>
          <GML2/>
          <GML3/>
        </ResultFormat>
      </GetFeature>
    </Request>
  </Capability>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML3'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

        # Test with explicit OUTPUTFORMAT as parameter
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0' outputformat='GML2'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML2'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">1,2</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (1.0, 2.0))

        # Test with explicit OUTPUTFORMAT  in URL
        vl = QgsVectorLayer("url='http://" + endpoint + "?OUTPUTFORMAT=GML2' typename='my:typename' version='1.0.0'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML2'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">3,4</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 4.0))

    def testWFS10_latlongboundingbox_in_WGS84(self):
        """Test WFS 1.0 with non conformatn LatLongBoundingBox"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS1.0_latlongboundingbox_in_WGS84'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox are supposed to be in SRS units, not necessarily lat/long...
        But some servers do not honour this, so let's try to be robust -->
      <LatLongBoundingBox minx="1.63972075372399" miny="48.7449841112119" maxx="2.30733562794991" maxy="49.6504711179582"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        reference = QgsGeometry.fromRect(
            QgsRectangle(399999.9999999680439942, 5399338.9090830031782389, 449999.9999999987776391,
                         5500658.0448500607162714))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

    def testWFST10(self):
        """Test WFS-T 1.0 (read-write)"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_T_1.0'

        transaction_endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_T_1.0_transaction'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <DCP>
        <HTTP>
          <Get type="simple" href="http://dummy?"/>
          <Post type="simple" href="http://dummy?"/>
        </HTTP>
      </DCP>
    </Operation>
    <Operation name="Transaction">
      <DCP>
        <HTTP>
          <Get href="http://{transaction_endpoint}"/>
          <Post href="http://{transaction_endpoint}"/>
        </HTTP>
      </DCP>
      <Parameter name="inputFormat">
        <AllowedValues>
          <Value>text/xml; subtype=gml/3.2</Value>
        </AllowedValues>
      </Parameter>
      <Parameter name="releaseAction">
        <AllowedValues>
          <Value>ALL</Value>
          <Value>SOME</Value>
        </AllowedValues>
      </Parameter>
    </Operation>
  </OperationsMetadata>
  <FeatureTypeList>
    <Operations>
      <Query/>
      <Insert/>
      <Update/>
      <Delete/>
    </Operations>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:4326</SRS>
      <LatLongBoundingBox minx="-71.123" miny="66.33" maxx="-65.32" maxy="78.3"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".format(transaction_endpoint=transaction_endpoint).encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().capabilities(),
                         QgsVectorDataProvider.AddFeatures
                         | QgsVectorDataProvider.ChangeAttributeValues
                         | QgsVectorDataProvider.ChangeGeometries
                         | QgsVectorDataProvider.DeleteFeatures
                         | QgsVectorDataProvider.SelectAtId
                         | QgsVectorDataProvider.ReloadData)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertFalse(ret)

        self.assertEqual(vl.featureCount(), 0)

        self.assertFalse(vl.dataProvider().deleteFeatures([0]))

        self.assertEqual(vl.featureCount(), 0)

        self.assertFalse(vl.dataProvider().changeGeometryValues({0: QgsGeometry.fromWkt('Point (3 50)')}))

        self.assertFalse(vl.dataProvider().changeAttributeValues({0: {0: 0}}))

        # Test addFeatures
        response = """
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:InsertResult>
    <ogc:FeatureId fid="typename.1" />
  </wfs:InsertResult>
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
"""
        # Qt 5 order
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Insert xmlns="http://www.opengis.net/wfs"><typename xmlns="http://my"><intfield xmlns="http://my">1</intfield><longfield xmlns="http://my">1234567890123</longfield><stringfield xmlns="http://my">foo</stringfield><datetimefield xmlns="http://my">2016-04-10T12:34:56.789Z</datetimefield><geometryProperty xmlns="http://my"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">2,49</gml:coordinates></gml:Point></geometryProperty></typename></Insert></Transaction>'),
                  'wb') as f:
            f.write(response.encode('UTF-8'))
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" version="1.0.0" service="WFS"><Insert xmlns="http://www.opengis.net/wfs"><typename xmlns="http://my"><intfield xmlns="http://my">1</intfield><longfield xmlns="http://my">1234567890123</longfield><stringfield xmlns="http://my">foo</stringfield><datetimefield xmlns="http://my">2016-04-10T12:34:56.789Z</datetimefield><geometryProperty xmlns="http://my"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">2,49</gml:coordinates></gml:Point></geometryProperty></typename></Insert></Transaction>'),
                  'wb') as f:
            f.write(response.encode('UTF-8'))

        f = QgsFeature()
        f.setAttributes([1, 1234567890123, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))

        #        def logMessage(msg, tag, level):
        #            print('--------################----------------')
        #            print(msg)
        #            print('--------################----------------')

        #        QgsApplication.messageLog().messageReceived.connect(logMessage)
        (ret, fl) = vl.dataProvider().addFeatures([f])
        self.assertTrue(ret)
        #        QgsApplication.messageLog().messageReceived.disconnect(logMessage)
        self.assertEqual(fl[0].id(), 1)

        self.assertEqual(vl.featureCount(), 1)

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['foo'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (2.0, 49.0))

        # Test changeGeometryValues
        content = """
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
"""
        # Qt 5 order
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:geometryProperty</Name><Value xmlns="http://www.opengis.net/wfs"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">3,50</gml:coordinates></gml:Point></Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" version="1.0.0" service="WFS"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:geometryProperty</Name><Value xmlns="http://www.opengis.net/wfs"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">3,50</gml:coordinates></gml:Point></Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))

        self.assertTrue(vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')}))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 50.0))

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['foo'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        # Test changeAttributeValues
        content = """
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
"""

        # Qt 5 order
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:intfield</Name><Value xmlns="http://www.opengis.net/wfs">2</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:longfield</Name><Value xmlns="http://www.opengis.net/wfs">3</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:stringfield</Name><Value xmlns="http://www.opengis.net/wfs">bar</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:datetimefield</Name><Value xmlns="http://www.opengis.net/wfs">2015-04-10T12:34:56.789Z</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" version="1.0.0" service="WFS"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:intfield</Name><Value xmlns="http://www.opengis.net/wfs">2</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:longfield</Name><Value xmlns="http://www.opengis.net/wfs">3</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:stringfield</Name><Value xmlns="http://www.opengis.net/wfs">bar</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">my:datetimefield</Name><Value xmlns="http://www.opengis.net/wfs">2015-04-10T12:34:56.789Z</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))

        self.assertTrue(vl.dataProvider().changeAttributeValues(
            {1: {0: 2, 1: 3, 2: "bar", 3: QDateTime(2015, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))}}))

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [3])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['bar'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2015, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 50.0))

        # Test deleteFeatures
        content = """
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
"""

        # Qt 5 order
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Delete xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Delete></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))
        with open(sanitize(transaction_endpoint,
                           '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" version="1.0.0" service="WFS"><Delete xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Delete></Transaction>'),
                  'wb') as f:
            f.write(content.encode('UTF-8'))

        self.assertTrue(vl.dataProvider().deleteFeatures([1]))

        self.assertEqual(vl.featureCount(), 0)

    def testWFS20Paging(self):
        """Test WFS 2.0 paging"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_paging'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Constraint name="CountDefault">
        <NoValues/>
        <DefaultValue>1</DefaultValue>
      </Constraint>
    </Operation>
    <Constraint name="ImplementsResultPaging">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="id" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:GeometryPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.100">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>1</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

        # Suppress GetFeature responses to demonstrate that the cache is used
        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

        # No need for hits since the download went to its end
        self.assertEqual(vl.featureCount(), 2)

        vl.dataProvider().reloadData()

        # Hits not working
        self.assertEqual(vl.featureCount(), 0)

        vl.dataProvider().reloadData()

        # Hits working
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))
        self.assertEqual(vl.featureCount(), 2)

    def testWFS20PagingPageSizeOverride(self):
        """Test WFS 2.0 paging"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_paging_override'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Constraint name="CountDefault">
        <NoValues/>
        <DefaultValue>10</DefaultValue>
      </Constraint>
    </Operation>
    <Constraint name="ImplementsResultPaging">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="id" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # user pageSize < user maxNumFeatures < server pagesize
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' maxNumFeatures='3' pageSize='2'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=2&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="2" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.100">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>1</my:id>
    </my:typename>
    <my:typename gml:id="typename.101">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="1" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>3</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2, 3])

        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=2&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        # user maxNumFeatures < user pageSize < server pagesize
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' maxNumFeatures='1' pageSize='2'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="1" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.100">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>1</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        # user user pageSize > server pagesize
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' pageSize='100'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=10&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="1" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.100">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>1</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        os.unlink(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=10&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' pagingEnabled='false' maxNumFeatures='3'", 'test',
            'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&COUNT=3&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="2" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.100">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>1000</my:id>
    </my:typename>
    <my:typename gml:id="typename.101">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.1"><gml:pos>66.33 -70.332</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2000</my:id>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1000, 2000])

    def testWFSGetOnlyFeaturesInViewExtent(self):
        """Test 'get only features in view extent' """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_only_features_in_view_extent'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Parameter name="resultType">
        <Value>results</Value>
        <Value>hits</Value>
      </Parameter>
    </Operation>
    <Constraint name="DefaultMaxFeatures">
      <Value>2</Value>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-80 60</LowerCorner>
        <UpperCorner>-50 80</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <!-- use ogc_fid that is the default SpatiaLite FID name -->
          <xsd:element maxOccurs="1" minOccurs="0" name="ogc_fid" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' restrictToRequestBBOX=1", 'test',
                            'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=60,-70,80,-60,urn:ogc:def:crs:EPSG::4326')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>2</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))

        extent = QgsRectangle(-70, 60, -60, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2])

        # To show that if we zoom-in, we won't issue a new request
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.20000">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>200</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))

        extent = QgsRectangle(-66, 62, -62, 78)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2])

        # Move to a neighbouring area, and reach the download limit
        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=65,-70,90,-60,urn:ogc:def:crs:EPSG::4326')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>2</my:ogc_fid>
    </my:typename>
    <my:typename gml:id="typename.300">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>85 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>3</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))

        extent = QgsRectangle(-70, 65, -60, 90)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2, 3])

        # Zoom-in again, and bring more features
        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=66,-69,89,-61,urn:ogc:def:crs:EPSG::4326')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>2</my:ogc_fid>
    </my:typename>
    <my:typename gml:id="typename.400">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>84 -64</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>4</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))

        extent = QgsRectangle(-69, 66, -61, 89)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2, 3, 4])

        # Test RESULTTYPE=hits
        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&RESULTTYPE=hits')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       numberOfFeatures="10" timeStamp="2016-03-25T14:51:48.998Z"/>""".encode('UTF-8'))

        self.assertEqual(vl.featureCount(), 10)

        # Combine BBOX and FILTER
        last_url = sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<ogc:Filter xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml">
 <ogc:And>
  <ogc:BBOX>
   <ogc:PropertyName>geometryProperty</ogc:PropertyName>
   <gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326">
    <gml:lowerCorner>66 -69</gml:lowerCorner>
    <gml:upperCorner>89 -61</gml:upperCorner>
   </gml:Envelope>
  </ogc:BBOX>
  <ogc:PropertyIsEqualTo xmlns:ogc="http://www.opengis.net/ogc">
   <ogc:PropertyName xmlns:ogc="http://www.opengis.net/ogc">ogc_fid</ogc:PropertyName>
   <ogc:Literal xmlns:ogc="http://www.opengis.net/ogc">101</ogc:Literal>
  </ogc:PropertyIsEqualTo>
 </ogc:And>
</ogc:Filter>
""")
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.101">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>101</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))

        vl.dataProvider().setSubsetString('ogc_fid = 101')
        extent = QgsRectangle(-69, 66, -61, 89)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [101])

        # Check behavior with setLimit(1)
        with open(sanitize(endpoint,
                           "?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=1&SRSNAME=urn:ogc:def:crs:EPSG::4326"),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.12345">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:ogc_fid>12345</my:ogc_fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8'))
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' restrictToRequestBBOX=1", 'test',
                            'WFS')
        request = QgsFeatureRequest().setLimit(1)
        values = [f['ogc_fid'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [12345])

        # Check that the layer extent is not built from this single feature
        reference = QgsGeometry.fromRect(QgsRectangle(-80, 60, -50, 80))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

    def testWFSGetOnlyFeaturesInViewExtentZoomOut(self):
        """Test zoom out outside of declare extent in metadata (#20742) """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_20742'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Parameter name="resultType">
        <Value>results</Value>
        <Value>hits</Value>
      </Parameter>
    </Operation>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-80 60</LowerCorner>
        <UpperCorner>-50 80</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="fid" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=60,-80,80,-50,urn:ogc:def:crs:EPSG::4326')
        getfeature_response = """
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="2" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:fid>2</my:fid>
    </my:typename>
    <my:typename gml:id="typename.400">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>71 -64</gml:pos></gml:Point></my:geometryProperty>
      <my:fid>4</my:fid>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""".encode('UTF-8')
        with open(last_url, 'wb') as f:
            f.write(getfeature_response)

        last_url = sanitize(endpoint,
                            '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=50,-90,90,-40,urn:ogc:def:crs:EPSG::4326')
        with open(last_url, 'wb') as f:
            f.write(getfeature_response)

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' restrictToRequestBBOX=1", 'test',
                            'WFS')
        # First request with declared extent in metadata
        extent = QgsRectangle(-80, 60, -50, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        self.assertEqual(len([f for f in vl.getFeatures(request)]), 2)
        reference = QgsGeometry.fromRect(QgsRectangle(-65, 70, -64, 71))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

        # Second request: zoomed out
        extent = QgsRectangle(-90, 50, -40, 90)
        request = QgsFeatureRequest().setFilterRect(extent)
        self.assertEqual(len([f for f in vl.getFeatures(request)]), 2)
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

    def testWFS20TruncatedResponse(self):
        """Test WFS 2.0 truncatedResponse"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_truncated_response'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="1" numberReturned="1" timeStamp="2016-03-25T14:51:48.998Z">
  <wfs:member>
    <my:typename gml:id="typename.1"/>
  </wfs:member>
  <wfs:truncatedResponse/>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Check that we get a log message
        with MessageLogger('WFS') as logger:
            [f for f in vl.getFeatures()]

            # Let signals to be notified to QgsVectorDataProvider
            loop = QEventLoop()
            loop.processEvents()

            self.assertEqual(len(logger.messages()), 1, logger.messages())
            self.assertTrue(logger.messages()[0].decode('UTF-8').find('The download limit has been reached') >= 0,
                            logger.messages())

    def testRetryLogic(self):
        """Test retry logic """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_retry'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:4326</SRS>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        # Failed download: test that error is propagated to the data provider, so as to get application notification
        [f['INTFIELD'] for f in vl.getFeatures()]

        # Let signals to be notified to QgsVectorDataProvider
        loop = QEventLoop()
        loop.processEvents()

        errors = vl.dataProvider().errors()
        self.assertEqual(len(errors), 1, errors)

        # Reload
        vl.reload()

        # First retry: Empty response
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=1'),
                  'wb') as f:
            f.write(''.encode('UTF-8'))

        # Second retry: Incomplete response
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=2'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
  <gml:featureMember>
    <my:typename fid="typename.1">
      <my:INTFIELD>2</my:INTFIELD>""".encode('UTF-8'))

        # Third retry: Valid response
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=3'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
  <gml:featureMember>
    <my:typename fid="typename.1">
      <my:INTFIELD>2</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

    def testDetachedFeatureSource(self):
        """Test using a feature source after the provider has been destroyed """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_detached_source'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:4326</SRS>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        source = vl.dataProvider().featureSource()
        # Close the vector layer
        vl = None

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326'),
                'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['INTFIELD'] for f in source.getFeatures(QgsFeatureRequest())]
        self.assertEqual(values, [1])

    def testJoins(self):
        """Test SELECT with joins """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_detached_source'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Constraint name="ImplementsStandardJoins">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
    <Constraint name="ImplementsSpatialJoins">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>my:othertypename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>first_ns:ambiguous</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>second_ns:ambiguous</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        schema = """
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="id" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>

  <xsd:complexType name="othertypenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="main_id" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="second_id" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="othertypename" substitutionGroup="gml:_Feature" type="my:othertypenameType"/>
</xsd:schema>
"""
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename,my:othertypename&TYPENAME=my:typename,my:othertypename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename,my:othertypename&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:And>
  <fes:PropertyIsEqualTo>
   <fes:ValueReference>my:typename/id</fes:ValueReference>
   <fes:ValueReference>my:othertypename/main_id</fes:ValueReference>
  </fes:PropertyIsEqualTo>
  <fes:PropertyIsGreaterThan>
   <fes:ValueReference>my:typename/id</fes:ValueReference>
   <fes:Literal>0</fes:Literal>
  </fes:PropertyIsGreaterThan>
 </fes:And>
</fes:Filter>
&SORTBY=id DESC"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <wfs:member>
    <wfs:Tuple>
        <wfs:member>
          <my:typename fid="typename.0">
            <my:id>1</my:id>
          </my:typename>
        </wfs:member>
        <wfs:member>
          <my:othertypename fid="othertypename.0">
            <my:main_id>1</my:main_id>
            <my:second_id>2</my:second_id>
          </my:othertypename>
        </wfs:member>
    </wfs:Tuple>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # * syntax
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\" JOIN \"my:othertypename\" o ON \"my:typename\".id = o.main_id WHERE \"my:typename\".id > 0 ORDER BY \"my:typename\".id DESC",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'typename.id')
        self.assertEqual(fields[1].name(), 'o.main_id')
        self.assertEqual(fields[2].name(), 'o.second_id')

        values = [(f['typename.id'], f['o.main_id'], f['o.second_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1, 2)])

        # * syntax with unprefixed typenames
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM typename JOIN othertypename o ON typename.id = o.main_id WHERE typename.id > 0 ORDER BY typename.id DESC",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'typename.id')
        self.assertEqual(fields[1].name(), 'o.main_id')
        self.assertEqual(fields[2].name(), 'o.second_id')

        values = [(f['typename.id'], f['o.main_id'], f['o.second_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1, 2)])

        # main table not appearing in first

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:othertypename,my:typename&TYPENAME=my:othertypename,my:typename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM othertypename o, typename WHERE typename.id = o.main_id AND typename.id > 0 ORDER BY typename.id DESC",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'typename.id')

        # main table not appearing in first, not in FROM but in JOIN
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM othertypename o JOIN typename ON typename.id = o.main_id WHERE typename.id > 0 ORDER BY typename.id DESC",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'typename.id')

        # table_alias.*, field alias
        vl.setSubsetString(
            "SELECT o.*, m.id AS m_id FROM \"my:typename\" m JOIN \"my:othertypename\" o ON m.id = o.main_id WHERE m.id > 0 ORDER BY m.id DESC")
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'm_id')

        values = [(f['o.main_id'], f['o.second_id'], f['m_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 2, 1)])

        # table_alias.*, field alias, with unprefixed typenames
        vl.setSubsetString(
            "SELECT o.*, m.id AS m_id FROM typename m JOIN othertypename o ON m.id = o.main_id WHERE m.id > 0 ORDER BY m.id DESC")
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'm_id')

        values = [(f['o.main_id'], f['o.second_id'], f['m_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 2, 1)])

        # Test going back to single layer
        vl.setSubsetString(None)
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))

        # Duplicate fields
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT id, id FROM \"my:typename\"",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

        # * syntax with single layer
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\"",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # * syntax with single layer, unprefixed
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM typename", 'test',
            'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # test with unqualified field name, and geometry name specified
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT id, geometryProperty FROM typename",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # Ambiguous typename
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='first_ns:ambiguous' version='2.0.0' sql=SELECT id FROM ambiguous",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

        # main table missing from SQL
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:othertypename' version='2.0.0' sql=SELECT * FROM typename",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

    def testFunctionValidation(self):
        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_function_validation'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:4326</SRS>
      <LatLongBoundingBox minx="-71.123" miny="66.33" maxx="-65.32" maxy="78.3"/>
    </FeatureType>
  </FeatureTypeList>
  <ogc:Filter_Capabilities>
    <ogc:Spatial_Capabilities>
      <ogc:Spatial_Operators>
        <ogc:Disjoint/>
        <ogc:Equals/>
        <ogc:DWithin/>
        <ogc:Beyond/>
        <ogc:Intersect/>
        <ogc:Touches/>
        <ogc:Crosses/>
        <ogc:Within/>
        <ogc:Contains/>
        <ogc:Overlaps/>
        <ogc:BBOX/>
      </ogc:Spatial_Operators>
    </ogc:Spatial_Capabilities>
    <ogc:Scalar_Capabilities>
      <ogc:Arithmetic_Operators>
        <ogc:Functions>
          <ogc:Function_Names>
            <ogc:Function_Name nArgs="1">abs</ogc:Function_Name>
          </ogc:Function_Names>
        </ogc:Functions>
      </ogc:Arithmetic_Operators>
    </ogc:Scalar_Capabilities>
  </ogc:Filter_Capabilities>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-80 60</LowerCorner>
        <UpperCorner>-50 80</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
  <ogc:Filter_Capabilities>
    <ogc:Spatial_Capabilities>
      <ogc:GeometryOperands>
        <ogc:GeometryOperand name="gml:Envelope"/>
        <ogc:GeometryOperand name="gml:Point"/>
        <ogc:GeometryOperand name="gml:LineString"/>
        <ogc:GeometryOperand name="gml:Polygon"/>
      </ogc:GeometryOperands>
      <ogc:SpatialOperators>
        <ogc:SpatialOperator name="Disjoint"/>
        <ogc:SpatialOperator name="Equals"/>
        <ogc:SpatialOperator name="DWithin"/>
        <ogc:SpatialOperator name="Beyond"/>
        <ogc:SpatialOperator name="Intersects"/>
        <ogc:SpatialOperator name="Touches"/>
        <ogc:SpatialOperator name="Crosses"/>
        <ogc:SpatialOperator name="Within"/>
        <ogc:SpatialOperator name="Contains"/>
        <ogc:SpatialOperator name="Overlaps"/>
        <ogc:SpatialOperator name="BBOX"/>
      </ogc:SpatialOperators>
    </ogc:Spatial_Capabilities>
    <ogc:Scalar_Capabilities>
      <ogc:ArithmeticOperators>
        <ogc:Functions>
          <ogc:FunctionNames>
            <ogc:FunctionName nArgs="1">abs</ogc:FunctionName>
            <ogc:FunctionName nArgs="-2">atleasttwoargs</ogc:FunctionName> <!-- GeoTools way -->
          </ogc:FunctionNames>
        </ogc:Functions>
      </ogc:ArithmeticOperators>
    </ogc:Scalar_Capabilities>
  </ogc:Filter_Capabilities>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
  <fes:Filter_Capabilities>
    <fes:Spatial_Capabilities>
      <fes:GeometryOperands>
        <fes:GeometryOperand name="gml:Envelope"/>
        <fes:GeometryOperand name="gml:Point"/>
        <fes:GeometryOperand name="gml:MultiPoint"/>
        <fes:GeometryOperand name="gml:LineString"/>
        <fes:GeometryOperand name="gml:MultiLineString"/>
        <fes:GeometryOperand name="gml:Polygon"/>
        <fes:GeometryOperand name="gml:MultiPolygon"/>
        <fes:GeometryOperand name="gml:MultiGeometry"/>
      </fes:GeometryOperands>
      <fes:SpatialOperators>
        <fes:SpatialOperator name="Disjoint"/>
        <fes:SpatialOperator name="Equals"/>
        <fes:SpatialOperator name="DWithin"/>
        <fes:SpatialOperator name="Beyond"/>
        <fes:SpatialOperator name="Intersects"/>
        <fes:SpatialOperator name="Touches"/>
        <fes:SpatialOperator name="Crosses"/>
        <fes:SpatialOperator name="Within"/>
        <fes:SpatialOperator name="Contains"/>
        <fes:SpatialOperator name="Overlaps"/>
        <fes:SpatialOperator name="BBOX"/>
      </fes:SpatialOperators>
    </fes:Spatial_Capabilities>
    <fes:Functions>
      <fes:Function name="abs">
        <fes:Returns>xs:int</fes:Returns>
        <fes:Arguments>
          <fes:Argument name="int">
            <fes:Type>xs:int</fes:Type>
          </fes:Argument>
        </fes:Arguments>
      </fes:Function>
    </fes:Functions>
  </fes:Filter_Capabilities>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        schema = """
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="id" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geom" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>"""

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write(schema.encode('UTF-8'))

        # Existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

        # Non existing function, but validation disabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.0.0' sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

        # Existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1",
            'test', 'WFS')
        self.assertFalse(vl.isValid())

    def testSelectDistinct(self):
        """Test SELECT DISTINCT """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_select_distinct'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename gml:id="typename.0">
      <my:intfield>1</my:intfield>
      <my:longfield>1234567890</my:longfield>
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.1"> <!-- duplicate -->
      <my:intfield>1</my:intfield>
      <my:longfield>1234567890</my:longfield>
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.2">
      <my:intfield>2</my:intfield> <!-- difference -->
      <my:longfield>1234567890</my:longfield>
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.3">
      <my:intfield>1</my:intfield>
      <my:longfield>1234567891</my:longfield> <!-- difference -->
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.4">
      <my:intfield>1</my:intfield>
      <my:longfield>1234567890</my:longfield>
      <my:stringfield>fop</my:stringfield>  <!-- difference -->
      <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
    </my:typename>
  </wfs:member>
  <wfs:member>
    <my:typename gml:id="typename.5">
      <my:intfield>1</my:intfield>
      <my:longfield>1234567890</my:longfield>
      <my:stringfield>foo</my:stringfield>
      <my:datetimefield>2016-04-10T12:34:56.788Z</my:datetimefield> <!-- difference -->
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT DISTINCT * FROM \"my:typename\"",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        values = [(f['intfield'], f['longfield'], f['stringfield'], f['datetimefield']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (2, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567891, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567890, 'fop', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 788, Qt.TimeSpec(Qt.UTC)))])

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT DISTINCT intfield FROM \"my:typename\"",
            'test', 'WFS')
        self.assertTrue(vl.isValid())

        values = [(f['intfield']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1), (2)])

    def testWrongCapabilityExtent(self):
        """Test behavior when capability extent is wrong."""

        # Note the logic that is tested is purely heuristic, trying to recover from wrong server behavior,
        # so it might be legitimate to change that at a later point.

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_wrong_capability_extent'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>0 0</LowerCorner>
        <UpperCorner>1 1</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PolygonPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename gml:id="typename.0">
      <my:intfield>1</my:intfield>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>49 2</gml:pos></gml:Point></my:geometryProperty>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='2.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Download all features
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)

        reference = QgsGeometry.fromRect(QgsRectangle(2, 49, 2, 49))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

        # Same with restrictToRequestBBOX=1
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' restrictToRequestBBOX=1", 'test',
            'WFS')
        self.assertTrue(vl.isValid())

        # First request that will be attempted
        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=-0.125,-0.125,1.125,1.125,urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
</wfs:FeatureCollection>""".encode('UTF-8'))

        # And fallback
        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&COUNT=1"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename gml:id="typename.0">
      <my:intfield>1</my:intfield>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>49 2</gml:pos></gml:Point></my:geometryProperty>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # Download all features in a BBOX that encloses the extent reported by capabilities
        extent = QgsRectangle(-0.125, -0.125, 1.125, 1.125)
        request = QgsFeatureRequest().setFilterRect(extent)
        features = [f for f in vl.getFeatures(request)]
        self.assertEqual(len(features), 0)

        # Check that the approx extent contains the geometry
        self.assertTrue(vl.extent().contains(QgsPointXY(2, 49)))

    def testGeomedia(self):
        """Test various interoperability specifities that occur with Geomedia Web Server."""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_geomedia'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Constraint name="ImplementsResultPaging">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>EPSG:32631</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>0 40</LowerCorner>
        <UpperCorner>15 50</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gmgml:Polygon_Surface_MultiSurface_CompositeSurfacePropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=EPSG:32631"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename>
      <my:intfield>1</my:intfield>
      <my:geometryProperty><gml:Polygon srsName="EPSG:32631" gml:id="typename.geom.0"><gml:exterior><gml:LinearRing><gml:posList>500000 4500000 500000 4510000 510000 4510000 510000 4500000 500000 4500000</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></my:geometryProperty>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        # Simulate improper paging support by returning same result set whatever the STARTINDEX is
        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=EPSG:32631"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename>
      <my:intfield>1</my:intfield>
      <my:geometryProperty><gml:Polygon srsName="EPSG:32631" gml:id="typename.geom.0"><gml:exterior><gml:LinearRing><gml:posList>500000 4500000 500000 4510000 510000 4510000 510000 4500000 500000 4500000</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></my:geometryProperty>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=EPSG:32631"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename>
      <my:intfield>1</my:intfield>
      <my:geometryProperty><gml:Polygon srsName="EPSG:32631" gml:id="typename.geom.0"><gml:exterior><gml:LinearRing><gml:posList>500000 4500000 500000 4510000 510000 4510000 510000 4500000 500000 4500000</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></my:geometryProperty>
    </my:typename>
    <my:typename>
      <my:intfield>2</my:intfield>
      <my:geometryProperty><gml:Polygon srsName="EPSG:32631" gml:id="typename.geom.0"><gml:exterior><gml:LinearRing><gml:posList>500000 4500000 500000 4510000 510000 4510000 510000 4500000 500000 4500000</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></my:geometryProperty>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        QgsSettings().setValue('wfs/max_feature_count_if_not_provided', '1')

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='2.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.MultiPolygon)

        # Extent before downloading features
        reference = QgsGeometry.fromRect(
            QgsRectangle(243900.3520259926444851, 4427769.1559739429503679, 1525592.3040170343592763,
                         5607994.6020106188952923))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.05), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

        # Download all features
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)

        reference = QgsGeometry.fromRect(QgsRectangle(500000, 4500000, 510000, 4510000))
        # Let signals to be notified to QgsVectorLayer
        loop = QEventLoop()
        loop.processEvents()
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())
        self.assertEqual(features[0]['intfield'], 1)
        self.assertEqual(features[1]['intfield'], 2)

    def testMapServerWFS1_1_EPSG_4326(self):
        """Test interoperability with MapServer WFS 1.1."""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_mapserver_wfs_1_1'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>2 49</LowerCorner>
        <UpperCorner>2 49</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<schema
   targetNamespace="http://my"
   xmlns:my="http://my"
   xmlns:ogc="http://www.opengis.net/ogc"
   xmlns:xsd="http://www.w3.org/2001/XMLSchema"
   xmlns="http://www.w3.org/2001/XMLSchema"
   xmlns:gml="http://www.opengis.net/gml"
   elementFormDefault="qualified" version="0.1" >
  <import namespace="http://www.opengis.net/gml"
          schemaLocation="http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" />
  <element name="typename"
           type="my:typenameType"
           substitutionGroup="gml:_Feature" />
  <complexType name="typenameType">
    <complexContent>
      <extension base="gml:AbstractFeatureType">
        <sequence>
          <element name="geometryProperty" type="gml:GeometryPropertyType" minOccurs="0" maxOccurs="1"/>
        </sequence>
      </extension>
    </complexContent>
  </complexType>
</schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
   xmlns:my="http://my"
   xmlns:gml="http://www.opengis.net/gml"
   xmlns:wfs="http://www.opengis.net/wfs"
   xmlns:ogc="http://www.opengis.net/ogc">
      <gml:boundedBy>
        <gml:Envelope srsName="EPSG:4326">
            <gml:lowerCorner>49.000000 2.000000</gml:lowerCorner>
            <gml:upperCorner>49.000000 2.000000</gml:upperCorner>
        </gml:Envelope>
      </gml:boundedBy>
    <gml:featureMember>
      <my:typename gml:id="typename.1">
        <gml:boundedBy>
            <gml:Envelope srsName="EPSG:4326">
                <gml:lowerCorner>49.000000 2.000000</gml:lowerCorner>
                <gml:upperCorner>49.000000 2.000000</gml:upperCorner>
            </gml:Envelope>
        </gml:boundedBy>
        <my:geometryProperty>
          <gml:Point srsName="EPSG:4326">
            <gml:pos>49.000000 2.000000</gml:pos>
          </gml:Point>
        </my:geometryProperty>
      </my:typename>
    </gml:featureMember>
</wfs:FeatureCollection>

""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (2.0, 49.0))

    def testDescribeFeatureTypeWithInlineType(self):
        """Test a DescribeFeatureType response with a inline ComplexType (#15395)."""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testDescribeFeatureTypeWithInlineType'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>2 49</LowerCorner>
        <UpperCorner>2 49</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<schema
   targetNamespace="http://my"
   xmlns:my="http://my"
   xmlns:ogc="http://www.opengis.net/ogc"
   xmlns:xsd="http://www.w3.org/2001/XMLSchema"
   xmlns="http://www.w3.org/2001/XMLSchema"
   xmlns:gml="http://www.opengis.net/gml"
   elementFormDefault="qualified" version="0.1" >
  <import namespace="http://www.opengis.net/gml"
          schemaLocation="http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" />
  <element name="typename"
           substitutionGroup="gml:_Feature">
    <complexType>
      <complexContent>
        <extension base="gml:AbstractFeatureType">
          <sequence>
            <element name="geometryProperty" type="gml:GeometryPropertyType" minOccurs="0" maxOccurs="1"/>
          </sequence>
        </extension>
      </complexContent>
    </complexType>
  </element>
</schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
   xmlns:my="http://my"
   xmlns:gml="http://www.opengis.net/gml"
   xmlns:wfs="http://www.opengis.net/wfs"
   xmlns:ogc="http://www.opengis.net/ogc">
      <gml:boundedBy>
        <gml:Envelope srsName="EPSG:4326">
            <gml:lowerCorner>49.000000 2.000000</gml:lowerCorner>
            <gml:upperCorner>49.000000 2.000000</gml:upperCorner>
        </gml:Envelope>
      </gml:boundedBy>
    <gml:featureMember>
      <my:typename gml:id="typename.1">
        <gml:boundedBy>
            <gml:Envelope srsName="EPSG:4326">
                <gml:lowerCorner>49.000000 2.000000</gml:lowerCorner>
                <gml:upperCorner>49.000000 2.000000</gml:upperCorner>
            </gml:Envelope>
        </gml:boundedBy>
        <my:geometryProperty>
          <gml:Point srsName="EPSG:4326">
            <gml:pos>49.000000 2.000000</gml:pos>
          </gml:Point>
        </my:geometryProperty>
      </my:typename>
    </gml:featureMember>
</wfs:FeatureCollection>

""".encode('UTF-8'))

        shutil.copyfile(sanitize(endpoint,
                                 """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                        sanitize(endpoint,
                                 """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=1&SRSNAME=urn:ogc:def:crs:EPSG::4326"""))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (2.0, 49.0))

    def testWFS20TransactionsDisabled(self):
        """Test WFS 2.0 Transaction disabled"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_transaction'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Constraint name="CountDefault">
        <NoValues/>
        <DefaultValue>1</DefaultValue>
      </Constraint>
    </Operation>
    <Constraint name="ImplementsTransactionalWFS">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().capabilities() & vl.dataProvider().EditingCapabilities,
                         vl.dataProvider().NoCapabilities)
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

    def testWFS20TransactionsEnabled(self):
        """Test WFS 2.0 Transaction enabled"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_transaction'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Constraint name="CountDefault">
        <NoValues/>
        <DefaultValue>1</DefaultValue>
      </Constraint>
    </Operation>
    <Constraint name="ImplementsTransactionalWFS">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
    <Operation name="Transaction">
      <DCP>
        <HTTP>
          <Get href="http://{endpoint}"/>
          <Post href="{endpoint}"/>
        </HTTP>
      </DCP>
      <Parameter name="inputFormat">
        <AllowedValues>
          <Value>text/xml; subtype=gml/3.2</Value>
        </AllowedValues>
      </Parameter>
      <Parameter name="releaseAction">
        <AllowedValues>
          <Value>ALL</Value>
          <Value>SOME</Value>
        </AllowedValues>
      </Parameter>
    </Operation>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".format(endpoint=endpoint).encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geomfield" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        self.assertTrue(vl.isValid())
        self.assertNotEqual(vl.dataProvider().capabilities() & vl.dataProvider().EditingCapabilities,
                            vl.dataProvider().NoCapabilities)
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

    def testDeprecatedGML2GeometryDeclaration(self):
        """Test ref="gml:pointProperty" """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_deprecated_gml2'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" ref="gml:pointProperty"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(len(vl.fields()), 1)

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'),
                'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <gml:pointProperty>
          <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
      </gml:pointProperty>
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

    def testGetFeatureWithNamespaces(self):
        ''' test https://github.com/qgis/QGIS/issues/22649 '''

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_getfeature_with_namespaces'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1">
  <wfs:FeatureTypeList>
    <wfs:FeatureType xmlns:my="http://my">
      <wfs:Name>my:typename</wfs:Name>
      <wfs:Title>Title</wfs:Title>
      <wfs:Abstract>Abstract</wfs:Abstract>
      <wfs:SRS>EPSG:32631</wfs:SRS>
      <WGS84BoundingBox>
        <LowerCorner>0 40</LowerCorner>
        <UpperCorner>15 50</UpperCorner>
      </WGS84BoundingBox>
    </wfs:FeatureType>
  </wfs:FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&NAMESPACES=xmlns(my,http://my)&TYPENAME=my:typename&NAMESPACE=xmlns(my,http://my)'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='2.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 1)

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::32631&NAMESPACES=xmlns(my,http://my)&NAMESPACE=xmlns(my,http://my)'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:intfield>1</my:intfield>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

    def testGetFeatureWithNamespaceAndFilter(self):
        ''' test https://github.com/qgis/QGIS/issues/43957 '''

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_getfeature_with_namespace_and_filter'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1">
  <wfs:FeatureTypeList>
    <wfs:FeatureType xmlns:my="http://my">
      <wfs:Name>my:typename</wfs:Name>
      <wfs:Title>Title</wfs:Title>
      <wfs:Abstract>Abstract</wfs:Abstract>
      <wfs:SRS>EPSG:32631</wfs:SRS>
      <WGS84BoundingBox>
        <LowerCorner>0 40</LowerCorner>
        <UpperCorner>15 50</UpperCorner>
      </WGS84BoundingBox>
    </wfs:FeatureType>
  </wfs:FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&NAMESPACES=xmlns(my,http://my)&TYPENAME=my:typename&NAMESPACE=xmlns(my,http://my)'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # SQL query with type with namespace
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\" WHERE intfield = 1", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 1)

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::32631&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0" xmlns:my="http://my">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>my:intfield</fes:ValueReference>
  <fes:Literal>1</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
&NAMESPACES=xmlns(my,http://my)&NAMESPACE=xmlns(my,http://my)"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:intfield>1</my:intfield>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        # SQL query with type with namespace and bounding box
        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' restrictToRequestBBOX=1 sql=SELECT * FROM \"my:typename\" WHERE intfield > 0", 'test',
            'WFS')

        extent = QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0)
        request = QgsFeatureRequest().setFilterRect(extent)

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::32631&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:my="http://my">
 <fes:And>
  <fes:BBOX>
   <fes:ValueReference>my:geometryProperty</fes:ValueReference>
   <gml:Envelope srsName="urn:ogc:def:crs:EPSG::32631">
    <gml:lowerCorner>400000 5400000</gml:lowerCorner>
    <gml:upperCorner>450000 5500000</gml:upperCorner>
   </gml:Envelope>
  </fes:BBOX>
  <fes:PropertyIsGreaterThan xmlns:fes="http://www.opengis.net/fes/2.0">
   <fes:ValueReference>my:intfield</fes:ValueReference>
   <fes:Literal xmlns:fes="http://www.opengis.net/fes/2.0">0</fes:Literal>
  </fes:PropertyIsGreaterThan>
 </fes:And>
</fes:Filter>
&NAMESPACES=xmlns(my,http://my)&NAMESPACE=xmlns(my,http://my)"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::32631" gml:id="typename.geom.0"><gml:pos>426858 5427937</gml:pos></gml:Point></my:geometryProperty>
      <my:intfield>1</my:intfield>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['intfield'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [1])

    def testExtentSubsetString(self):
        # can't run the base provider test suite here - WFS/OAPIF extent handling is different
        # to other providers
        pass

    def testWFS10DCP(self):
        """Test a server with different DCP endpoints"""
        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_DCP_1.0'
        endpoint_alternate = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_DCP_1.0_alternate'

        with open(sanitize(endpoint, '?FOO=BAR&SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
  <OperationsMetadata>
    <Operation name="DescribeFeatureType">
      <DCP>
        <HTTP>
          <Get type="simple" href="http://{0}?"/>
          <Post type="simple" href="http://{0}?"/>
        </HTTP>
      </DCP>
    </Operation>
    <Operation name="GetFeature">
      <DCP>
        <HTTP>
          <Get type="simple" href="http://{0}?"/>
          <Post type="simple" href="http://{0}?"/>
        </HTTP>
      </DCP>
    </Operation>
  </OperationsMetadata></WFS_Capabilities>""".format(endpoint_alternate).encode('UTF-8'))

        with open(sanitize(endpoint_alternate,
                           '?FOO=BAR&SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="GEOMETRY" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
          <!-- use geometry that is the default SpatiaLite geometry name -->
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "?FOO=BAR&SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.1.0" + "' typename='my:typename' version='1.0.0'",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(len(vl.fields()), 5)
        self.assertEqual(vl.featureCount(), 0)
        reference = QgsGeometry.fromRect(QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

        with open(sanitize(endpoint_alternate,
                           '?FOO=BAR&SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'),
                  'wb') as f:
            f.write("""
  <wfs:FeatureCollection
                      xmlns:wfs="http://www.opengis.net/wfs"
                      xmlns:gml="http://www.opengis.net/gml"
                      xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
  <my:typename fid="typename.0">
    <my:geometry>
        <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
    </my:geometry>
    <my:INTFIELD>1</my:INTFIELD>
    <my:GEOMETRY>2</my:GEOMETRY>
    <my:longfield>1234567890123</my:longfield>
    <my:stringfield>foo</my:stringfield>
    <my:datetimefield>2016-04-10T12:34:56.789Z</my:datetimefield>
  </my:typename>
  </gml:featureMember>
  </wfs:FeatureCollection>""".encode('UTF-8'))

        # Also test that on file iterator works
        os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD'] = '0'

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        del os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD']

        values = [f['GEOMETRY'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['foo'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

        self.assertEqual(vl.featureCount(), 1)

        self.assertTrue(vl.dataProvider().capabilities() & QgsVectorDataProvider.SelectAtId)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertFalse(ret)

        self.assertFalse(vl.dataProvider().deleteFeatures([0]))

        # Test with restrictToRequestBBOX=1
        with open(sanitize(endpoint_alternate,
                           '?FOO=BAR&SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&BBOX=400000,5400000,450000,5500000'),
                  'wb') as f:
            f.write("""
  <wfs:FeatureCollection
                      xmlns:wfs="http://www.opengis.net/wfs"
                      xmlns:gml="http://www.opengis.net/gml"
                      xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
  <my:typename fid="typename.0">
    <my:geometry>
        <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
    </my:geometry>
    <my:INTFIELD>100</my:INTFIELD>
  </my:typename>
  </gml:featureMember>
  </wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "?FOO=BAR" + "' typename='my:typename' version='1.0.0' restrictToRequestBBOX=1",
            'test', 'WFS')

        extent = QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['INTFIELD'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [100])

    def testWFS10_outputformat_GML3(self):
        """Test WFS 1.0 with OUTPUTFORMAT=GML3"""
        # We also test attribute fields in upper-case, and a field named GEOMETRY

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS1.0_gml3'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <Capability>
    <Request>
      <GetFeature>
        <ResultFormat>
          <GML2/>
          <GML3/>
        </ResultFormat>
      </GetFeature>
    </Request>
  </Capability>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML3'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">426858,5427937</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

        # Test with explicit OUTPUTFORMAT as parameter
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0' outputformat='GML2'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML2'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">1,2</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (1.0, 2.0))

        # Test with explicit OUTPUTFORMAT  in URL
        vl = QgsVectorLayer("url='http://" + endpoint + "?OUTPUTFORMAT=GML2' typename='my:typename' version='1.0.0'",
                            'test', 'WFS')
        self.assertTrue(vl.isValid())

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631&OUTPUTFORMAT=GML2'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometry>
          <gml:Point srsName="urn:ogc:def:crs:EPSG::32631"><gml:coordinates decimal="." cs="," ts=" ">3,4</gml:coordinates></gml:Point>
      </my:geometry>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().constGet()
        self.assertEqual((got.x(), got.y()), (3.0, 4.0))

    def testWfs20SamServer(self):
        """Unknown russian WFS 2.0.0 http://geoportal.samregion.ru/wfs12"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_sam'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>EC422</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=EC422&TYPENAME=EC422'),
                'wb') as f:
            f.write("""<schema xmlns="http://www.w3.org/2001/XMLSchema"
        xmlns:xs="http://www.w3.org/2001/XMLSchema"
        xmlns:gml="http://www.opengis.net/gml/3.2"
        xmlns:geosmr="http://www.geosamara.ru/wfs/geosmr/namespace"
        targetNamespace="http://www.geosamara.ru/wfs/geosmr/namespace"
        elementFormDefault="qualified" version="2.0.0">
<element name="Feature" type="geosmr:FeatureType" substitutionGroup="gml:AbstractFeature"/>
<complexType name="FeatureEC422Type">
    <complexContent>
        <extension base="gml:AbstractFeatureType">
            <sequence>
                <element name="id" type="xs:string"/>
                <element name="name" type="xs:string"/>
                <element name="description" type="geosmr:DescriptionType"/>
                <element name="style" type="xs:string"/>
                <element name="status" type="xs:string"/>
                <element name="operations" type="geosmr:OperationsType"/>
                <element name="transaction" type="geosmr:TransactionType"/>
                <choice>
                    <element name="geometry" type="gml:PointPropertyType"/>
                    <element name="geometry" type="gml:CurvePropertyType"/>
                    <element name="geometry" type="gml:SurfacePropertyType"/>
                </choice>
            </sequence>
        </extension>
    </complexContent>
</complexType>
<complexType name="DescriptionType">
    <sequence>
        <element name="text" type="xs:string"/>
        <element name="resource" type="xs:anyURI"/>
        <element name="image" type="xs:string"/>
        <element name="reestrId" type="xs:string"/>
    </sequence>
</complexType>
<complexType name="OperationsType">
    <sequence>
        <element name="update" type="xs:boolean"/>
        <element name="delete" type="xs:boolean"/>
        <element name="moderate" type="xs:boolean"/>
    </sequence>
</complexType>
    <complexType name="TransactionType">
    <choice minoccurs="0">
        <element name="mine" type="xs:boolean"/>
        <element name="locker" type="xs:string"/>
        <element name="expireIn" type="xs:positiveInteger"/>
    </choice>
</complexType>
</schema>""".encode('UTF-8'))

        feature_content = """<?xml version="1.0"?>
<wfs:FeatureCollection
        timeStamp="2010-02-01T22:56:09"
        numberMatched="8"
        numberReturned="8"
        xmlns="http://www.geosamara.ru/wfs/geosmr/namespace"
        xmlns:wfs="http://www.opengis.net/wfs/2.0"
        xmlns:gml="http://www.opengis.net/gml/3.2"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
        xsi:schemaLocation="http://www.geosamara.ru/wfs/geosmr/style.xsd
                       http://www.opengis.net/wfs/2.0
                       http://schemas.opengis.net/wfs/2.0.0/wfs.xsd
                       http://www.opengis.net/gml/3.2
                       http://schemas.opengis.net/gml/3.2.1/gml.xsd">

        <wfs:member>
            <EC422 gml:id="EC422.13172455">
                <id>13172455</id>
                <name><![CDATA[Основной туристический маршрут]]></name>
                <description>
                    <text><![CDATA[]]></text>
                    <additionalInfo><![CDATA[]]></additionalInfo>
                    <resource><![CDATA[]]></resource>
                    <image><![CDATA[]]></image>
                    <audio><![CDATA[]]></audio>
                    <video><![CDATA[]]></video>
                    <votes><![CDATA[0]]></votes>
                    <reestrId><![CDATA[]]></reestrId>

                </description>
                <style>CreatedRoute</style>
                <status>checked</status>
                <author>Андрей Чернов</author>
                <currentUserAuthor>false</currentUserAuthor>
                <operations>
                        <update>false</update>
                        <delete>false</delete>
                        <moderate>false</moderate>
                </operations>
                <transaction>

                </transaction>
                <geometry>
                    <LineString xmlns="http://www.opengis.net/gml"><posList>9540051.88156246 5997366.8135842243 9539934.21894572 5997127.7749966066 9539822.1483417116 5996862.6127466606 9539504.179093 5996097.3096572906 9539529.9650254529 5996093.5547346519 9539584.6281441431 5996148.1036405731 9539709.8322301712 5996306.1476564864 9539514.2094426583 5996393.6699969191 9539315.5400513224 5996461.6283053206 9539418.05506746 5996708.1687648129 9539601.002140563 5996948.5162237845 9539715.6644945163 5997103.3323533693 9539806.8714339714 5997185.6346932752 9539980.407018993 5997401.3173021814 9540034.9262902886 5997461.1185212145 9540144.520062916 5997647.082066942 9540205.6388517376 5997752.865820759 9540413.93051952 5998022.2412844934 9540636.78114721 5998289.1650444875 9540652.2228743583 5998292.3310790323 9540739.3873799425 5998253.7093249289 9540742.8882315382 5998264.8666193094 9540928.0440610871 5998447.0783388717 9540964.2606249321 5998465.5247420967 9540992.4471577276 5998468.9503919454 9541266.994057592 5998700.44280944 9541489.2456994727 5998870.8459224468 9542015.9473830853 5999245.7052185535 9542481.7197104339 5999585.458734531 9542594.2400093842 5999581.2418252891 9542791.0368823726 5999731.6623752853 9543204.6598267779 6000066.4150706194 9543245.7990274262 6000086.6195615921 9543303.6317887139 6000098.2128836326 9543392.7859923933 6000084.2088917186 9543473.2299142312 6000041.19114427 9543582.34122052 5999959.7280100482 9543796.5102230646 5999788.4518707721 9544237.3357650079 6000148.6245372053 9544242.356376797 6000146.87913009</posList></LineString>
                </geometry>
                <creationDate>

                </creationDate>
        </EC422>
        </wfs:member>

        <wfs:member>
            <EC422 gml:id="EC422.13172458">
                <id>13172458</id>
                <name><![CDATA[]]></name>
                <description>
                    <text><![CDATA[Добавлено 15:44 20 января 2016]]></text>
                    <additionalInfo><![CDATA[]]></additionalInfo>
                    <resource><![CDATA[]]></resource>
                    <image><![CDATA[]]></image>
                    <audio><![CDATA[]]></audio>
                    <video><![CDATA[]]></video>
                    <votes><![CDATA[0]]></votes>
                    <reestrId><![CDATA[]]></reestrId>

                </description>
                <style>CreatedRoute</style>
                <status>checked</status>
                <author>Андрей Чернов</author>
                <currentUserAuthor>false</currentUserAuthor>
                <operations>
                        <update>false</update>
                        <delete>false</delete>
                        <moderate>false</moderate>
                </operations>
                <transaction>

                </transaction>
                <geometry>
                    <LineString xmlns="http://www.opengis.net/gml"><posList>9540865.6444970388 5998183.9317809641 9540775.852046784 5997947.0331188506 9540680.5655983184 5997718.6045682346 9540569.58023185 5997466.57064837 9540466.8184371851 5997200.4314374486 9540244.6014337484 5996676.7638938017 9540169.35653367 5996705.7142945267 9540148.0577711649 5996682.1794316517 9540111.4476553015 5996665.8381209867 9540077.9721918479 5996676.0606173435 9540043.4139141534 5996711.499830937 9540043.9248592574 5996752.0983768944 9540045.5009976458 5996766.0802566176 9539966.0914670844 5996797.8834326165 9539818.507223323 5996864.1487550773 9539610.9541244339 5996949.3710925905</posList></LineString>
                </geometry>
                <creationDate>

                </creationDate>
        </EC422>
        </wfs:member>

        <wfs:member>
            <EC422 gml:id="EC422.13172454">
                <id>13172454</id>
                <name><![CDATA[Основной туристический маршрут]]></name>
                <description>
                    <text><![CDATA[]]></text>
                    <additionalInfo><![CDATA[]]></additionalInfo>
                    <resource><![CDATA[]]></resource>
                    <image><![CDATA[]]></image>
                    <audio><![CDATA[]]></audio>
                    <video><![CDATA[]]></video>
                    <votes><![CDATA[0]]></votes>
                    <reestrId><![CDATA[]]></reestrId>

                </description>
                <style>CreatedRoute</style>
                <status>checked</status>
                <author>Андрей Чернов</author>
                <currentUserAuthor>false</currentUserAuthor>
                <operations>
                        <update>false</update>
                        <delete>false</delete>
                        <moderate>false</moderate>
                </operations>
                <transaction>

                </transaction>
                <geometry>
                    <LineString xmlns="http://www.opengis.net/gml"><posList>9542485.8345187921 5998971.7039023517 9542453.32676163 5998980.1340847686 9542423.4887301736 5998965.47791457 9541839.9022341352 5998539.8925312571 9541447.9249842446 5998260.2124332683 9541414.5872483123 5998203.50518699 9541330.9900199063 5998001.1229150137 9540804.3199365921 5996757.0503186928 9540212.17485467 5997003.9579582512 9539930.5850430559 5997125.116400606 9539816.1399206612 5997176.6829545 9539806.7162561137 5997183.794419908</posList></LineString>
                </geometry>
                <creationDate>

                </creationDate>
        </EC422>
  </wfs:member>
</wfs:FeatureCollection>"""

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=EC422&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write(feature_content.encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=EC422&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write(feature_content.encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' version='2.0.0' typename='EC422'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        features = list(vl.getFeatures())
        self.assertEqual(len(features), 3)
        geom = features[0].geometry()
        geom_string = geom.asWkt()
        geom_string = re.sub(r'\.\d+', '', geom_string)[:100]
        self.assertEqual(geom_string,
                         "LineString (9540051 5997366, 9539934 5997127, 9539822 5996862, 9539504 5996097, 9539529 5996093, 953")

    def testDescribeFeatureTypeWithSingleInclude(self):
        """Test DescribeFeatureType response which has a single <include> child node"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_DescribeFeatureTypeWithSingleInclude'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'),
                  'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write(("""
<schema xmlns="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <include schemaLocation="%s"/>
</schema>
""" % ('http://' + endpoint + '?myschema.xsd')).encode('UTF-8'))

        with open(sanitize(endpoint, '?myschema.xsd'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

    def testGeometryCollectionAsMultiLineString(self):
        """Test https://github.com/qgis/QGIS/issues/27398 """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_gc_as_mls'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>2 49</LowerCorner>
        <UpperCorner>3 50</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<schema
   targetNamespace="http://my"
   xmlns:my="http://my"
   xmlns:ogc="http://www.opengis.net/ogc"
   xmlns:xsd="http://www.w3.org/2001/XMLSchema"
   xmlns="http://www.w3.org/2001/XMLSchema"
   xmlns:gml="http://www.opengis.net/gml"
   elementFormDefault="qualified" version="0.1" >
  <import namespace="http://www.opengis.net/gml"
          schemaLocation="http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" />
  <element name="typename"
           type="my:typenameType"
           substitutionGroup="gml:_Feature" />
  <complexType name="typenameType">
    <complexContent>
      <extension base="gml:AbstractFeatureType">
        <sequence>
          <element name="geometryProperty" type="gml:GeometryPropertyType" minOccurs="0" maxOccurs="1"/>
        </sequence>
      </extension>
    </complexContent>
  </complexType>
</schema>
""".encode('UTF-8'))

        get_features = """
<wfs:FeatureCollection
   xmlns:my="http://my"
   xmlns:gml="http://www.opengis.net/gml"
   xmlns:wfs="http://www.opengis.net/wfs"
   xmlns:ogc="http://www.opengis.net/ogc">
      <gml:boundedBy>
        <gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326">
            <gml:lowerCorner>49 2</gml:lowerCorner>
            <gml:upperCorner>50 3</gml:upperCorner>
        </gml:Envelope>
      </gml:boundedBy>
    <gml:featureMember>
      <my:typename gml:id="typename.1">
        <my:geometryProperty>
          <gml:MultiGeometry srsName="urn:ogc:def:crs:EPSG::4326">
            <gml:geometryMember>
              <gml:LineString>
                <gml:coordinates>49,2 50,3</gml:coordinates>
              </gml:LineString>
            </gml:geometryMember>
          </gml:MultiGeometry>
        </my:geometryProperty>
      </my:typename>
    </gml:featureMember>
</wfs:FeatureCollection>
"""

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=1&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write(get_features.encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write(get_features.encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        got_f = [f for f in vl.getFeatures()]
        geom = got_f[0].geometry().constGet()
        geom_string = geom.asWkt()
        geom_string = re.sub(r'\.\d+', '', geom_string)
        self.assertEqual(geom_string, 'MultiLineString ((2 49, 3 50))')

        reference = QgsGeometry.fromRect(QgsRectangle(2, 49, 3, 50))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

    def test_NullValues_regression_20961(self):
        """Test that provider handles null values, regression #20961"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_null_values'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>points</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::3857</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-98.6523 32.7233</LowerCorner>
        <UpperCorner>23.2868 69.9882</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=points'),
                  'wb') as f:
            f.write("""
<schema xmlns="http://www.w3.org/2001/XMLSchema" xmlns:gml="http://www.opengis.net/gml" version="1.0" xmlns:ogc="http://www.opengis.net/ogc" xmlns:qgs="http://www.qgis.org/gml" elementFormDefault="qualified" targetNamespace="http://www.qgis.org/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
 <import schemaLocation="http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" namespace="http://www.opengis.net/gml"/>
 <element type="qgs:pointsType" name="points" substitutionGroup="gml:_Feature"/>
 <complexType name="pointsType">
  <complexContent>
   <extension base="gml:AbstractFeatureType">
    <sequence>
     <element type="gml:MultiPointPropertyType" name="geometry" minOccurs="0" maxOccurs="1"/>
     <element type="int" name="id"/>
     <element type="string" name="name"/>
     <element type="int" name="type" nillable="true"/>
     <element type="decimal" name="elevation" nillable="true"/>
    </sequence>
   </extension>
  </complexContent>
 </complexType>
</schema>
""".encode('UTF-8'))

        get_feature_1 = """<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" xmlns:ows="http://www.opengis.net/ows" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:qgs="http://www.qgis.org/gml" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://www.qgis.org/gml http://localhost:8000/ows/bug_20961_server?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0&amp;SERVICE=WFS&amp;VERSION=1.1.0&amp;REQUEST=DescribeFeatureType&amp;TYPENAME=points&amp;OUTPUTFORMAT=text/xml; subtype%3Dgml/3.1.1">
<gml:boundedBy>
 <gml:Envelope srsName="EPSG:3857">
  <gml:lowerCorner>-10981925.67093 3858635.0686243</gml:lowerCorner>
  <gml:upperCorner>2592274.0488407 11064877.6393476</gml:upperCorner>
 </gml:Envelope>
</gml:boundedBy>
<gml:featureMember>
 <qgs:points gml:id="points.177">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>1544231.80343599 5930698.04174612</gml:lowerCorner>
    <gml:upperCorner>1544231.80343599 5930698.04174612</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">1544231.80343599 5930698.04174612</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>177</qgs:id>
  <qgs:name>Xxx</qgs:name>
  <qgs:elevation_source></qgs:elevation_source>
 </qgs:points>
</gml:featureMember>
</wfs:FeatureCollection>
"""
        get_features = """<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" xmlns:ows="http://www.opengis.net/ows" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:qgs="http://www.qgis.org/gml" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://www.qgis.org/gml http://localhost:8000/ows/bug_20961_server?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0&amp;SERVICE=WFS&amp;VERSION=1.1.0&amp;REQUEST=DescribeFeatureType&amp;TYPENAME=points&amp;OUTPUTFORMAT=text/xml; subtype%3Dgml/3.1.1">
<gml:boundedBy>
 <gml:Envelope srsName="EPSG:3857">
  <gml:lowerCorner>-10981925.67093 3858635.0686243</gml:lowerCorner>
  <gml:upperCorner>2592274.0488407 11064877.6393476</gml:upperCorner>
 </gml:Envelope>
</gml:boundedBy>
<gml:featureMember>
 <qgs:points gml:id="points.177">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>1544231.80343599 5930698.04174612</gml:lowerCorner>
    <gml:upperCorner>1544231.80343599 5930698.04174612</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">1544231.80343599 5930698.04174612</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>177</qgs:id>
  <qgs:name>Xxx</qgs:name>
  <qgs:type xsi:nil="true"></qgs:type>
  <qgs:elevation xsi:nil="true"></qgs:elevation>
 </qgs:points>
</gml:featureMember>
<gml:featureMember>
 <qgs:points gml:id="points.5">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>-10977033.701121 3897159.3308746</gml:lowerCorner>
    <gml:upperCorner>-10977033.701121 3897159.3308746</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">-10977033.701121 3897159.3308746</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>5</qgs:id>
  <qgs:name>sdf</qgs:name>
  <qgs:type>0</qgs:type>
  <qgs:elevation xsi:nil="true"></qgs:elevation>
 </qgs:points>
</gml:featureMember>
</wfs:FeatureCollection>"""

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=points&MAXFEATURES=1&SRSNAME=urn:ogc:def:crs:EPSG::3857"""),
                  'wb') as f:
            f.write(get_feature_1.encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=points&SRSNAME=urn:ogc:def:crs:EPSG::3857"""),
                  'wb') as f:
            f.write(get_features.encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='points' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        got_f = [f for f in vl.getFeatures()]
        self.assertEqual(str(got_f[0]['type']), 'NULL')
        self.assertEqual(str(got_f[0]['elevation']), 'NULL')
        self.assertEqual(str(got_f[0]['name']), 'Xxx')
        self.assertEqual(str(got_f[1]['type']), '0')
        self.assertEqual(str(got_f[1]['elevation']), 'NULL')
        self.assertEqual(str(got_f[1]['name']), 'sdf')

        # Now iterate ! Regression #20961
        ids = [f.id() for f in got_f]
        got_f2 = [vl.getFeature(id) for id in ids]
        self.assertEqual(str(got_f2[0]['type']), 'NULL')
        self.assertEqual(str(got_f2[0]['elevation']), 'NULL')
        self.assertEqual(str(got_f2[0]['name']), 'Xxx')
        self.assertEqual(str(got_f2[1]['type']), '0')
        self.assertEqual(str(got_f2[1]['elevation']), 'NULL')
        self.assertEqual(str(got_f2[1]['name']), 'sdf')

    def testFilteredFeatureRequests(self):
        """Test https://github.com/qgis/QGIS/issues/28895 """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_filtered_feature_requests'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <FeatureTypeList>
    <FeatureType>
      <Name>points</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::3857</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-98.6523 32.7233</LowerCorner>
        <UpperCorner>23.2868 69.9882</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=points'),
                  'wb') as f:
            f.write("""
<schema xmlns="http://www.w3.org/2001/XMLSchema" xmlns:gml="http://www.opengis.net/gml" version="1.0" xmlns:ogc="http://www.opengis.net/ogc" xmlns:qgs="http://www.qgis.org/gml" elementFormDefault="qualified" targetNamespace="http://www.qgis.org/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
 <import schemaLocation="http://schemas.opengis.net/gml/3.1.1/base/gml.xsd" namespace="http://www.opengis.net/gml"/>
 <element type="qgs:pointsType" name="points" substitutionGroup="gml:_Feature"/>
 <complexType name="pointsType">
  <complexContent>
   <extension base="gml:AbstractFeatureType">
    <sequence>
     <element type="gml:MultiPointPropertyType" name="geometry" minOccurs="0" maxOccurs="1"/>
     <element type="int" name="id"/>
     <element type="string" name="name"/>
     <element type="int" name="type" nillable="true"/>
     <element type="decimal" name="elevation" nillable="true"/>
    </sequence>
   </extension>
  </complexContent>
 </complexType>
</schema>
""".encode('UTF-8'))

        get_feature_1 = """<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" xmlns:ows="http://www.opengis.net/ows" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:qgs="http://www.qgis.org/gml" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://www.qgis.org/gml http://localhost:8000/ows/bug_20961_server?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0&amp;SERVICE=WFS&amp;VERSION=1.1.0&amp;REQUEST=DescribeFeatureType&amp;TYPENAME=points&amp;OUTPUTFORMAT=text/xml; subtype%3Dgml/3.1.1">
<gml:boundedBy>
 <gml:Envelope srsName="EPSG:3857">
  <gml:lowerCorner>-10981925.67093 3858635.0686243</gml:lowerCorner>
  <gml:upperCorner>2592274.0488407 11064877.6393476</gml:upperCorner>
 </gml:Envelope>
</gml:boundedBy>
<gml:featureMember>
 <qgs:points gml:id="points.177">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>1544231.80343599 5930698.04174612</gml:lowerCorner>
    <gml:upperCorner>1544231.80343599 5930698.04174612</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">1544231.80343599 5930698.04174612</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>177</qgs:id>
  <qgs:name>Xxx</qgs:name>
  <qgs:elevation_source></qgs:elevation_source>
 </qgs:points>
</gml:featureMember>
</wfs:FeatureCollection>
"""
        get_features = """<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:gml="http://www.opengis.net/gml" xmlns:ows="http://www.opengis.net/ows" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:qgs="http://www.qgis.org/gml" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://www.qgis.org/gml http://localhost:8000/ows/bug_20961_server?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0&amp;SERVICE=WFS&amp;VERSION=1.1.0&amp;REQUEST=DescribeFeatureType&amp;TYPENAME=points&amp;OUTPUTFORMAT=text/xml; subtype%3Dgml/3.1.1">
<gml:boundedBy>
 <gml:Envelope srsName="EPSG:3857">
  <gml:lowerCorner>-10981925.67093 3858635.0686243</gml:lowerCorner>
  <gml:upperCorner>2592274.0488407 11064877.6393476</gml:upperCorner>
 </gml:Envelope>
</gml:boundedBy>
<gml:featureMember>
 <qgs:points gml:id="points.177">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>1544231.80343599 5930698.04174612</gml:lowerCorner>
    <gml:upperCorner>1544231.80343599 5930698.04174612</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">1544231.80343599 5930698.04174612</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>177</qgs:id>
  <qgs:name>Xxx</qgs:name>
  <qgs:type xsi:nil="true"></qgs:type>
  <qgs:elevation xsi:nil="true"></qgs:elevation>
 </qgs:points>
</gml:featureMember>
<gml:featureMember>
 <qgs:points gml:id="points.5">
  <gml:boundedBy>
   <gml:Envelope srsName="EPSG:3857">
    <gml:lowerCorner>-10977033.701121 3897159.3308746</gml:lowerCorner>
    <gml:upperCorner>-10977033.701121 3897159.3308746</gml:upperCorner>
   </gml:Envelope>
  </gml:boundedBy>
  <qgs:geometry>
   <MultiPoint xmlns="http://www.opengis.net/gml" srsName="EPSG:3857">
    <pointMember xmlns="http://www.opengis.net/gml">
     <Point xmlns="http://www.opengis.net/gml">
      <pos xmlns="http://www.opengis.net/gml" srsDimension="2">-10977033.701121 3897159.3308746</pos>
     </Point>
    </pointMember>
   </MultiPoint>
  </qgs:geometry>
  <qgs:id>5</qgs:id>
  <qgs:name>qgis</qgs:name>
  <qgs:type>0</qgs:type>
  <qgs:elevation xsi:nil="true"></qgs:elevation>
 </qgs:points>
</gml:featureMember>
</wfs:FeatureCollection>"""

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=points&MAXFEATURES=1&SRSNAME=urn:ogc:def:crs:EPSG::3857"""),
                  'wb') as f:
            f.write(get_feature_1.encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=points&SRSNAME=urn:ogc:def:crs:EPSG::3857"""),
                  'wb') as f:
            f.write(get_features.encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='points' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())

        # Fill the cache
        [f for f in vl.getFeatures()]

        qgis_feat = next(vl.getFeatures(QgsFeatureRequest(QgsExpression('"name" = \'qgis\''))))
        other_feat = next(vl.getFeatures(QgsFeatureRequest(QgsExpression('"name" != \'qgis\''))))
        self.assertEqual(qgis_feat['name'], 'qgis')
        self.assertEqual(other_feat['name'], 'Xxx')

        form_scope = QgsExpressionContextUtils.formScope(qgis_feat)
        form_exp = QgsExpression('current_value(\'name\') = "name"')
        ctx = QgsExpressionContext()
        ctx.appendScope(form_scope)
        ctx.setFeature(qgis_feat)
        self.assertEqual(form_exp.evaluate(ctx), 1)
        ctx.setFeature(other_feat)
        self.assertEqual(form_exp.evaluate(ctx), 0)

        # For real now! (this failed in issue 21077)
        req = QgsFeatureRequest(form_exp)
        req.setExpressionContext(ctx)
        qgis_feat = next(vl.getFeatures(req))

    def testWFSFieldWithSameNameButDifferentCase(self):
        """Test a layer with field foo and FOO """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_FieldWithSameNameButDifferentCase'

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="foo" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="FOO" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="FOO2" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(
                sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'),
                'wb') as f:
            f.write("""
  <wfs:FeatureCollection
                      xmlns:wfs="http://www.opengis.net/wfs"
                      xmlns:gml="http://www.opengis.net/gml"
                      xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
  <my:typename fid="typename.0">
    <my:foo>1</my:foo>
    <my:FOO>2</my:FOO>
    <my:FOO2>3</my:FOO2>
  </my:typename>
  </gml:featureMember>
  </wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 3)

        values = [f['foo'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        values = [f['FOO'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['FOO2'] for f in vl.getFeatures()]
        self.assertEqual(values, [3])

        # Also test that on file iterator works
        os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD'] = '0'

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        values = [f['foo'] for f in vl.getFeatures()]
        self.assertEqual(values, [1])

        values = [f['FOO'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['FOO2'] for f in vl.getFeatures()]
        self.assertEqual(values, [3])

        del os.environ['QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD']

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        request = QgsFeatureRequest().setFilterExpression('FOO = 2')
        values = [f['FOO'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2])

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
        request = QgsFeatureRequest().setSubsetOfAttributes(['FOO'], vl.fields())
        values = [f['FOO'] for f in vl.getFeatures(request)]
        self.assertEqual(values, [2])

    def testRetryLogicOnExceptionLackOfPrimaryKey(self):
        """Test retry logic on 'Cannot do natural order without a primary key' server exception (GeoServer) """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_retry'

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetCapabilities&VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <OperationsMetadata>
    <Operation name="GetFeature">
      <Constraint name="CountDefault">
        <NoValues/>
        <DefaultValue>1</DefaultValue>
      </Constraint>
    </Operation>
    <Constraint name="ImplementsResultPaging">
      <NoValues/>
      <DefaultValue>TRUE</DefaultValue>
    </Constraint>
  </OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='2.0.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        # Initial request: exception
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'),
                  'wb') as f:
            f.write("""<?xml version="1.0" encoding="UTF-8"?><ows:ExceptionReport xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="2.0.0" xsi:schemaLocation="http://www.opengis.net/ows/1.1 http://localhost/geoserver/schemas/ows/1.1.0/owsAll.xsd">
<ows:Exception exceptionCode="NoApplicableCode">
<ows:ExceptionText>java.lang.RuntimeException: java.lang.RuntimeException: java.io.IOException
java.lang.RuntimeException: java.io.IOException
java.io.IOExceptionCannot do natural order without a primary key, please add it or specify a manual sort over existing attributes</ows:ExceptionText>
</ows:Exception>
</ows:ExceptionReport>""".encode('UTF-8'))

        # Retry
        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&RETRY=1'),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
  <gml:featureMember>
    <my:typename fid="typename.1">
      <my:INTFIELD>2</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

    def testCacheRead(self):
        # setup a clean cache directory
        cache_dir = tempfile.mkdtemp()
        QgsSettings().setValue("cache/directory", cache_dir)

        # don't retry, http server never fails
        QgsSettings().setValue('qgis/defaultTileMaxRetry', '0')

        responses = []

        class SequentialHandler(http.server.SimpleHTTPRequestHandler):

            def do_GET(self):
                c, response = responses.pop(0)
                self.send_response(c)
                self.send_header("Content-type", "application/xml")
                self.send_header("Content-length", len(response))
                self.send_header('Last-Modified', 'Wed, 05 Jun 2019 15:33:27 GMT')
                self.end_headers()
                self.wfile.write(response.encode('UTF-8'))

        httpd = socketserver.TCPServer(('localhost', 0), SequentialHandler)
        port = httpd.server_address[1]

        responses.append((200,
                          """
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:4326</SRS>
    </FeatureType>
  </FeatureTypeList>
</WFS_Capabilities>"""))

        responses.append((200,
                          """
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
"""))

        responses.append((200,
                          """
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:INTFIELD>1</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
  <gml:featureMember>
    <my:typename fid="typename.1">
      <my:INTFIELD>2</my:INTFIELD>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>"""))

        httpd_thread = threading.Thread(target=httpd.serve_forever)
        httpd_thread.setDaemon(True)
        httpd_thread.start()

        vl = QgsVectorLayer("url='http://localhost:{}' typename='my:typename' version='1.0.0'".format(port), 'test',
                            'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        res = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEqual(sorted(res), [1, 2])

        # next response is empty, cache must be used
        responses.append((304, ""))

        # Reload
        vl.reload()

        res = [f['INTFIELD'] for f in vl.getFeatures()]
        # self.assertEqual(len(server.errors()), 0, server.errors())
        self.assertEqual(sorted(res), [1, 2])

        errors = vl.dataProvider().errors()
        self.assertEqual(len(errors), 0, errors)

    def testWFS20CaseInsensitiveKVP(self):
        """Test an URL with non standard query string arguments where the server exposes
        the same parameters with different case: see https://github.com/qgis/QGIS/issues/34148
        """
        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_case_insensitive_kvp_2.0'

        get_cap = """
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <SRS>EPSG:32631</SRS>
      <!-- in WFS 1.0, LatLongBoundingBox is in SRS units, not necessarily lat/long... -->
      <LatLongBoundingBox minx="400000" miny="5400000" maxx="450000" maxy="5500000"/>
    </FeatureType>
  </FeatureTypeList>
  <OperationsMetadata>
    <Operation name="DescribeFeatureType">
      <DCP>
        <HTTP>
          <Get type="simple" href="http://{0}?PARAMETER1=Value1&amp;PARAMETER2=Value2&amp;"/>
          <Post type="simple" href="http://{0}?PARAMETER1=Value1&amp;PARAMETER2=Value2&amp;"/>
        </HTTP>
      </DCP>
    </Operation>
    <Operation name="GetFeature">
      <DCP>
        <HTTP>
          <Get type="simple" href="http://{0}?PARAMETER1=Value1&amp;PARAMETER2=Value2&amp;"/>
          <Post type="simple" href="http://{0}?PARAMETER1=Value1&amp;PARAMETER2=Value2&amp;"/>
        </HTTP>
      </DCP>
    </Operation>
  </OperationsMetadata></WFS_Capabilities>""".format(endpoint).encode('UTF-8')

        with open(sanitize(endpoint,
                           '?PARAMETER1=Value1&PARAMETER2=Value2&FOO=BAR&SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'),
                  'wb') as f:
            f.write(get_cap)

        with open(sanitize(endpoint,
                           '?Parameter1=Value1&Parameter2=Value2&FOO=BAR&SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'),
                  'wb') as f:
            f.write(get_cap)

        with open(sanitize(endpoint,
                           '?PARAMETER1=Value1&PARAMETER2=Value2&FOO=BAR&SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="GEOMETRY" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="datetimefield" nillable="true" type="xsd:dateTime"/>
          <!-- use geometry that is the default SpatiaLite geometry name -->
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "?Parameter1=Value1&Parameter2=Value2&FOO=BAR&SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.1.0" + "' typename='my:typename' version='1.0.0'",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)
        self.assertEqual(len(vl.fields()), 5)
        self.assertEqual(vl.featureCount(), 0)
        reference = QgsGeometry.fromRect(QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0],
                                   0.00001), 'Expected {}, got {}'.format(reference.asWkt(), vl_extent.asWkt())

    def testGetCapabilitiesReturnWFSException(self):
        """Test parsing of WFS exception
        """
        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testGetCapabilitiesReturnWFSException'

        get_cap_response = """<?xml version="1.0" encoding="UTF-8"?>
<ExceptionReport xmlns="http://www.opengis.net/ows" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:schemaLocation="http://schemas.opengis.net/ows/1.1.0/owsExceptionReport.xsd" version="1.0.0" language="en">
  <Exception exceptionCode="foo" locator="service">
    <ExceptionText>bar</ExceptionText>
  </Exception>
</ExceptionReport>
""".encode('UTF-8')

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'),
                  'wb') as f:
            f.write(get_cap_response)

        with MessageLogger('WFS') as logger:
            vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
            self.assertFalse(vl.isValid())

            self.assertEqual(len(logger.messages()), 1, logger.messages())
            self.assertTrue("foo: bar" in logger.messages()[0].decode('UTF-8'), logger.messages())

    def testGetCapabilitiesReturnWMSException(self):
        """Test fix for https://github.com/qgis/QGIS/issues/29866
        """
        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testGetCapabilitiesReturnWMSxception'

        get_cap_response = """<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<!DOCTYPE ServiceExceptionReport SYSTEM "http://schemas.opengis.net/wms/1.1.1/exception_1_1_1.dtd">
<ServiceExceptionReport version="1.1.1">
  <ServiceException code="InvalidFormat">
Can't recognize service requested.
  </ServiceException>
</ServiceExceptionReport>
""".encode('UTF-8')

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0'),
                  'wb') as f:
            f.write(get_cap_response)

        with MessageLogger('WFS') as logger:
            vl = QgsVectorLayer("url='http://" + endpoint + "' typename='my:typename' version='1.0.0'", 'test', 'WFS')
            self.assertFalse(vl.isValid())

            self.assertEqual(len(logger.messages()), 1, logger.messages())
            self.assertTrue("InvalidFormat: Can't recognize service requested." in logger.messages()[0].decode('UTF-8'), logger.messages())

    def testWFST11(self):
        """Test WFS-T 1.1 (read-write) taken from a geoserver session"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_T_1_1_transaction'

        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'getcapabilities.xml'), sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.1.0'))
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'describefeaturetype_polygons.xml'), sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=ws1:polygons'))

        vl = QgsVectorLayer("url='http://" + endpoint + "' typename='ws1:polygons' version='1.1.0'", 'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.featureCount(), 0)

        self.assertEqual(vl.dataProvider().capabilities(),
                         QgsVectorDataProvider.AddFeatures
                         | QgsVectorDataProvider.ChangeAttributeValues
                         | QgsVectorDataProvider.ChangeGeometries
                         | QgsVectorDataProvider.DeleteFeatures
                         | QgsVectorDataProvider.SelectAtId
                         | QgsVectorDataProvider.ReloadData)

        # Transaction response failure (no modifications)
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'transaction_response_empty.xml'), sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http:__www.opengis.net_wfs" xmlns:xsi="http:__www.w3.org_2001_XMLSchema-instance" xmlns:gml="http:__www.opengis.net_gml" xmlns:ws1="ws1" xsi:schemaLocation="ws1 http:__fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:polygons" version="1.1.0" service="WFS"><Insert xmlns="http:__www.opengis.net_wfs"><polygons xmlns="ws1"_><_Insert><_Transaction>'))

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        self.assertFalse(ret)
        self.assertEqual(vl.featureCount(), 0)

        # Test add features for real
        # Transaction response with 1 feature added
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'transaction_response_feature_added.xml'), sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http:__www.opengis.net_wfs" xmlns:xsi="http:__www.w3.org_2001_XMLSchema-instance" xmlns:gml="http:__www.opengis.net_gml" xmlns:ws1="ws1" xsi:schemaLocation="ws1 http:__fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:polygons" version="1.1.0" service="WFS"><Insert xmlns="http:__www.opengis.net_wfs"><polygons xmlns="ws1"><name xmlns="ws1">one<_name><value xmlns="ws1">1<_value><geometry xmlns="ws1"><gml:Polygon srsName="urn:ogc:def:crs:EPSG::4326"><gml:exterior><gml:LinearRing><gml:posList srsDimension="2">45 9 45 10 46 10 46 9 45 9<_gml:posList><_gml:LinearRing><_gml:exterior><_gml:Polygon><_geometry><_polygons><_Insert><_Transaction>'))

        feat = QgsFeature(vl.fields())
        feat.setAttribute('name', 'one')
        feat.setAttribute('value', 1)
        feat.setGeometry(QgsGeometry.fromWkt('Polygon ((9 45, 10 45, 10 46, 9 46, 9 45))'))
        (ret, features) = vl.dataProvider().addFeatures([feat])
        self.assertEqual(features[0].attributes(), ['one', 1])
        self.assertEqual(vl.featureCount(), 1)

        # Test change attributes
        # Transaction response with 1 feature changed
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'transaction_response_feature_changed.xml'), sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xmlns:ws1="ws1" xsi:schemaLocation="ws1 http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:polygons" version="1.1.0" service="WFS"><Update xmlns="http://www.opengis.net/wfs" typeName="ws1:polygons"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">ws1:name</Name><Value xmlns="http://www.opengis.net/wfs">one-one-one</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">ws1:value</Name><Value xmlns="http://www.opengis.net/wfs">111</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="123"/></Filter></Update></Transaction>'))

        self.assertTrue(vl.dataProvider().changeAttributeValues({1: {0: 'one-one-one', 1: 111}}))
        self.assertEqual(next(vl.dataProvider().getFeatures()).attributes(), ['one-one-one', 111])

        # Test change geometry
        # Transaction response with 1 feature changed
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'transaction_response_feature_changed.xml'), sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http:__www.opengis.net_wfs" xmlns:xsi="http:__www.w3.org_2001_XMLSchema-instance" xmlns:gml="http:__www.opengis.net_gml" xmlns:ws1="ws1" xsi:schemaLocation="ws1 http:__fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:polygons" version="1.1.0" service="WFS"><Update xmlns="http:__www.opengis.net_wfs" typeName="ws1:polygons"><Property xmlns="http:__www.opengis.net_wfs"><Name xmlns="http:__www.opengis.net_wfs">ws1:geometry<_Name><Value xmlns="http:__www.opengis.net_wfs"><gml:Polygon srsName="urn:ogc:def:crs:EPSG::4326"><gml:exterior><gml:LinearRing><gml:posList srsDimension="2">46 10 46 11 47 11 47 10 46 10<_gml:posList><_gml:LinearRing><_gml:exterior><_gml:Polygon><_Value><_Property><Filter xmlns="http:__www.opengis.net_ogc"><FeatureId xmlns="http:__www.opengis.net_ogc" fid="123"_><_Filter><_Update><_Transaction>'))

        new_geom = QgsGeometry.fromWkt('Polygon ((10 46, 11 46, 11 47, 10 47, 10 46))')

        self.assertTrue(vl.dataProvider().changeGeometryValues({1: new_geom}))
        self.assertEqual(next(vl.dataProvider().getFeatures()).geometry().asWkt(), new_geom.asWkt())

        # Test delete feature
        # Transaction response with 1 feature deleted
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'wfst-1-1', 'transaction_response_feature_deleted.xml'), sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml" xmlns:ws1="ws1" xsi:schemaLocation="ws1 http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:polygons" version="1.1.0" service="WFS"><Delete xmlns="http://www.opengis.net/wfs" typeName="ws1:polygons"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="123"/></Filter></Delete></Transaction>'))

        self.assertTrue(vl.dataProvider().deleteFeatures([1]))
        self.assertEqual(vl.featureCount(), 0)

    def testSelectZeroFeature(self):
        """Test a layer with a filter that returns 0 feature. See https://github.com/qgis/QGIS/issues/43950 """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_select_zero_feature'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-71.123 66.33</LowerCorner>
        <UpperCorner>-65.32 78.3</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=my:typename&TYPENAME=my:typename'),
                  'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:GeometryPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326&FILTER=<fes:Filter xmlns:fes="http://www.opengis.net/fes/2.0">
 <fes:PropertyIsEqualTo>
  <fes:ValueReference>intfield</fes:ValueReference>
  <fes:Literal>-1</fes:Literal>
 </fes:PropertyIsEqualTo>
</fes:Filter>
"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint,
                           """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326"""),
                  'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
  <wfs:member>
    <my:typename gml:id="typename.0">
      <my:intfield>1</my:intfield>
      <my:geometry><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.0"><gml:pos>78.3 -65.32</gml:pos></gml:Point></my:geometry>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        vl = QgsVectorLayer(
            "url='http://" + endpoint + "' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\" WHERE intfield = -1",
            'test', 'WFS')
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)


if __name__ == '__main__':
    unittest.main()
