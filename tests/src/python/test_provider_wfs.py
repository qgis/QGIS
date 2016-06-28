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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import hashlib
import os
import tempfile
import shutil

# Needed on Qt 5 so that the serialization of XML is consistant among all executions
os.environ['QT_HASH_SEED'] = '1'

from qgis.PyQt.QtCore import QCoreApplication, Qt, QObject, QDateTime, QSettings

from qgis.core import (
    QgsWKBTypes,
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsRectangle,
    QgsPoint,
    QgsVectorDataProvider,
    QgsFeatureRequest,
    QgsMessageLog
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        #print('Before: ' + endpoint + x)
        #print('After:  ' + ret)
        return ret
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"', '_').replace("'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


class MessageLogger(QObject):

    def __init__(self, tag=None):
        QObject.__init__(self)
        self.log = []
        self.tag = tag

    def __enter__(self):
        QgsMessageLog.instance().messageReceived.connect(self.logMessage)
        return self

    def __exit__(self, type, value, traceback):
        QgsMessageLog.instance().messageReceived.disconnect(self.logMessage)

    def logMessage(self, msg, tag, level):
        if tag == self.tag or not self.tag:
            self.log.append(msg.encode('UTF-8'))

    def messages(self):
        return self.log


class TestPyQgsWFSProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWFSProvider")
        QSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="pk" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="cnt" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="name" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="name2" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="num_char" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PolygonPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        # Create test layer
        cls.vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert (cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
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
  <wfs:member>
    <my:typename gml:id="typename.4">
      <gml:boundedBy><gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326"><gml:lowerCorner>78.23 -71.123</gml:lowerCorner><gml:upperCorner>78.23 -71.123</gml:upperCorner></gml:Envelope></gml:boundedBy>
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326" gml:id="typename.geom.4"><gml:pos>78.23 -71.123</gml:pos></gml:Point></my:geometryProperty>
      <my:pk>5</my:pk>
      <my:cnt>-200</my:cnt>
      <my:name2>NuLl</my:name2>
      <my:num_char>5</my:num_char>
    </my:typename>
  </wfs:member>
</wfs:FeatureCollection>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'), 'wb') as f:
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

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QSettings().clear()
        shutil.rmtree(cls.basetestpath, True)
        cls.vl = None # so as to properly close the provider and remove any temporary file

    def testInconsistantUri(self):
        """Test a URI with a typename that doesn't match a type of the capabilities"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testInconsistantUri'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        # Could not find typename my:typename in capabilities
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert not vl.isValid()

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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
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
          <!-- use geometry that is the default spatialite geometry name -->
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""".encode('UTF-8'))

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)
        self.assertEqual(len(vl.fields()), 5)
        self.assertEqual(vl.featureCount(), 0)
        reference = QgsGeometry.fromRect(QgsRectangle(400000.0, 5400000.0, 450000.0, 5500000.0))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:32631'), 'wb') as f:
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
        got = got_f[0].geometry().geometry()
        self.assertEqual((got.x(), got.y()), (426858.0, 5427937.0))

        self.assertEqual(vl.featureCount(), 1)

        self.assertEqual(vl.dataProvider().capabilities(), QgsVectorDataProvider.SelectAtId)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        assert not ret

        assert not vl.dataProvider().deleteFeatures([0])

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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()

        reference = QgsGeometry.fromRect(QgsRectangle(399999.9999999680439942, 5399338.9090830031782389, 449999.9999999987776391, 5500658.0448500607162714))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())

    def testWFST10(self):
        """Test WFS-T 1.0 (read-write)"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_T_1.0'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=1.0.0'), 'wb') as f:
            f.write("""
<WFS_Capabilities version="1.0.0" xmlns="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
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
</WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()

        self.assertEqual(vl.dataProvider().capabilities(),
                         QgsVectorDataProvider.AddFeatures +
                         QgsVectorDataProvider.ChangeAttributeValues +
                         QgsVectorDataProvider.ChangeGeometries +
                         QgsVectorDataProvider.DeleteFeatures +
                         QgsVectorDataProvider.SelectAtId)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        assert not ret

        self.assertEqual(vl.featureCount(), 0)

        assert not vl.dataProvider().deleteFeatures([0])

        self.assertEqual(vl.featureCount(), 0)

        assert not vl.dataProvider().changeGeometryValues({0: QgsGeometry.fromWkt('Point (3 50)')})

        assert not vl.dataProvider().changeAttributeValues({0: {0: 0}})

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
        # Qt 4 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Insert xmlns="http://www.opengis.net/wfs"><typename xmlns="http://my"><intfield xmlns="http://my">1</intfield><longfield xmlns="http://my">1234567890123</longfield><stringfield xmlns="http://my">foo</stringfield><datetimefield xmlns="http://my">2016-04-10T12:34:56.789Z</datetimefield><geometryProperty xmlns="http://my"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">2,49</gml:coordinates></gml:Point></geometryProperty></typename></Insert></Transaction>'), 'wb') as f:
            f.write(response.encode('UTF-8'))

        # Qt 5 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Insert xmlns="http://www.opengis.net/wfs"><typename xmlns="http://my"><intfield xmlns="http://my">1</intfield><longfield xmlns="http://my">1234567890123</longfield><stringfield xmlns="http://my">foo</stringfield><datetimefield xmlns="http://my">2016-04-10T12:34:56.789Z</datetimefield><geometryProperty xmlns="http://my"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">2,49</gml:coordinates></gml:Point></geometryProperty></typename></Insert></Transaction>'), 'wb') as f:
            f.write(response.encode('UTF-8'))

        f = QgsFeature()
        f.setAttributes([1, 1234567890123, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))
        (ret, fl) = vl.dataProvider().addFeatures([f])
        assert ret
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
        got = got_f[0].geometry().geometry()
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
        # Qt 4 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">geometryProperty</Name><Value xmlns="http://www.opengis.net/wfs"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">3,50</gml:coordinates></gml:Point></Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))

        # Qt 5 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">geometryProperty</Name><Value xmlns="http://www.opengis.net/wfs"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">3,50</gml:coordinates></gml:Point></Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))

        ret = vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')})
        assert ret

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().geometry()
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
        # Qt 4 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">intfield</Name><Value xmlns="http://www.opengis.net/wfs">2</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">longfield</Name><Value xmlns="http://www.opengis.net/wfs">3</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">stringfield</Name><Value xmlns="http://www.opengis.net/wfs">bar</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">datetimefield</Name><Value xmlns="http://www.opengis.net/wfs">2015-04-10T12:34:56.789Z</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))
        # Qt 5 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">intfield</Name><Value xmlns="http://www.opengis.net/wfs">2</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">longfield</Name><Value xmlns="http://www.opengis.net/wfs">3</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">stringfield</Name><Value xmlns="http://www.opengis.net/wfs">bar</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">datetimefield</Name><Value xmlns="http://www.opengis.net/wfs">2015-04-10T12:34:56.789Z</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))

        assert vl.dataProvider().changeAttributeValues({1: {0: 2, 1: 3, 2: "bar", 3: QDateTime(2015, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))}})

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEqual(values, [3])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEqual(values, ['bar'])

        values = [f['datetimefield'] for f in vl.getFeatures()]
        self.assertEqual(values, [QDateTime(2015, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))])

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().geometry()
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
        # Qt 4 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Delete xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Delete></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))
        # Qt 5 order
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" version="1.0.0"><Delete xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Delete></Transaction>'), 'wb') as f:
            f.write(content.encode('UTF-8'))

        assert vl.dataProvider().deleteFeatures([1])

        self.assertEqual(vl.featureCount(), 0)

    def testWFS20Paging(self):
        """Test WFS 2.0 paging"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_paging'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <ows:OperationsMetadata>
    <ows:Operation name="GetFeature">
      <ows:Constraint name="CountDefault">
        <ows:NoValues/>
        <ows:DefaultValue>1</ows:DefaultValue>
      </ows:Constraint>
    </ows:Operation>
    <ows:Constraint name="ImplementsResultPaging">
      <ows:NoValues/>
      <ows:DefaultValue>TRUE</ows:DefaultValue>
    </ows:Constraint>
  </ows:OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
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
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

        # Suppress GetFeature responses to demonstrate that the cache is used
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEqual(values, [1, 2])

        # No need for hits since the download went to its end
        self.assertEqual(vl.featureCount(), 2)

        vl.dataProvider().reloadData()

        # Hits not working
        self.assertEqual(vl.featureCount(), 0)

        vl.dataProvider().reloadData()

        # Hits working
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""".encode('UTF-8'))
        self.assertEqual(vl.featureCount(), 2)

    def testWFSGetOnlyFeaturesInViewExtent(self):
        """Test 'get only features in view extent' """

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_only_features_in_view_extent'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="1.1.0" xmlns="http://www.opengis.net/wfs" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc" xmlns:ows="http://www.opengis.net/ows" xmlns:gml="http://schemas.opengis.net/gml">
  <ows:OperationsMetadata>
    <ows:Operation name="GetFeature">
      <ows:Parameter name="resultType">
        <ows:Value>results</ows:Value>
        <ows:Value>hits</ows:Value>
      </ows:Parameter>
    </ows:Operation>
    <ows:Constraint name="DefaultMaxFeatures">
      <ows:Value>2</ows:Value>
    </ows:Constraint>
  </ows:OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-80 60</ows:LowerCorner>
        <ows:UpperCorner>-50 80</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <!-- use ogc_fid that is the default spatialite FID name -->
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
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' retrictToRequestBBOX=1", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)

        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=60,-70,80,-60,urn:ogc:def:crs:EPSG::4326')
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
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=65,-70,90,-60,urn:ogc:def:crs:EPSG::4326')
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
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=66,-69,89,-61,urn:ogc:def:crs:EPSG::4326')
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
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&RESULTTYPE=hits')
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

    def testWFS20TruncatedResponse(self):
        """Test WFS 2.0 truncatedResponse"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_truncated_response'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
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
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert vl.isValid()

        # Check that we get a log message
        with MessageLogger('WFS') as logger:
            [f for f in vl.getFeatures()]
            self.assertEqual(len(logger.messages()), 1, logger.messages())
            self.assertTrue(logger.messages()[0].decode('UTF-8').find('The download limit has been reached') >= 0, logger.messages())

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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        # Failed download: test that error is propagated to the data provider, so as to get application notification
        [f['INTFIELD'] for f in vl.getFeatures()]
        errors = vl.dataProvider().errors()
        self.assertEqual(len(errors), 1, errors)

        # Reload
        vl.reload()

        # First retry: Empty response
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=1'), 'wb') as f:
            f.write(''.encode('UTF-8'))

        # Second retry: Incomplete response
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=2'), 'wb') as f:
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
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326&RETRY=3'), 'wb') as f:
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.NoGeometry)
        self.assertEqual(len(vl.fields()), 1)

        source = vl.dataProvider().featureSource()
        # Close the vector layer
        vl = None

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326'), 'wb') as f:
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
  <ows:OperationsMetadata>
    <ows:Constraint name="ImplementsStandardJoins">
      <ows:NoValues/>
      <ows:DefaultValue>TRUE</ows:DefaultValue>
    </ows:Constraint>
    <ows:Constraint name="ImplementsSpatialJoins">
      <ows:NoValues/>
      <ows:DefaultValue>TRUE</ows:DefaultValue>
    </ows:Constraint>
  </ows:OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>my:othertypename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>first_ns:ambiguous</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
    <FeatureType>
      <Name>second_ns:ambiguous</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
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
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename,my:othertypename'), 'wb') as f:
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
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\" JOIN \"my:othertypename\" o ON \"my:typename\".id = o.main_id WHERE \"my:typename\".id > 0 ORDER BY \"my:typename\".id DESC", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'typename.id')
        self.assertEqual(fields[1].name(), 'o.main_id')
        self.assertEqual(fields[2].name(), 'o.second_id')

        values = [(f['typename.id'], f['o.main_id'], f['o.second_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1, 2)])

        # * syntax with unprefixed typenames
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM typename JOIN othertypename o ON typename.id = o.main_id WHERE typename.id > 0 ORDER BY typename.id DESC", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'typename.id')
        self.assertEqual(fields[1].name(), 'o.main_id')
        self.assertEqual(fields[2].name(), 'o.second_id')

        values = [(f['typename.id'], f['o.main_id'], f['o.second_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1, 2)])

        # main table not appearing in first

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:othertypename,my:typename'), 'wb') as f:
            f.write(schema.encode('UTF-8'))

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM othertypename o, typename WHERE typename.id = o.main_id AND typename.id > 0 ORDER BY typename.id DESC", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'typename.id')

        # main table not appearing in first, not in FROM but in JOIN
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM othertypename o JOIN typename ON typename.id = o.main_id WHERE typename.id > 0 ORDER BY typename.id DESC", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.Point)
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'typename.id')

        # table_alias.*, field alias
        vl.setSubsetString("SELECT o.*, m.id AS m_id FROM \"my:typename\" m JOIN \"my:othertypename\" o ON m.id = o.main_id WHERE m.id > 0 ORDER BY m.id DESC")
        fields = vl.fields()
        self.assertEqual(len(fields), 3, fields)
        self.assertEqual(fields[0].name(), 'o.main_id')
        self.assertEqual(fields[1].name(), 'o.second_id')
        self.assertEqual(fields[2].name(), 'm_id')

        values = [(f['o.main_id'], f['o.second_id'], f['m_id']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 2, 1)])

        # table_alias.*, field alias, with unprefixed typenames
        vl.setSubsetString("SELECT o.*, m.id AS m_id FROM typename m JOIN othertypename o ON m.id = o.main_id WHERE m.id > 0 ORDER BY m.id DESC")
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write(schema.encode('UTF-8'))

        # Duplicate fields
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT id, id FROM \"my:typename\"", u'test', u'WFS')
        self.assertFalse(vl.isValid())

        # * syntax with single layer
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM \"my:typename\"", u'test', u'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # * syntax with single layer, unprefixed
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT * FROM typename", u'test', u'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # test with unqualified field name, and geometry name specified
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT id, geometryProperty FROM typename", u'test', u'WFS')
        self.assertTrue(vl.isValid())
        fields = vl.fields()
        self.assertEqual(len(fields), 1, fields)
        self.assertEqual(fields[0].name(), 'id')

        # Ambiguous typename
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='first_ns:ambiguous' version='2.0.0' sql=SELECT id FROM ambiguous", u'test', u'WFS')
        self.assertFalse(vl.isValid())

        # main table missing from SQL
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:othertypename' version='2.0.0' sql=SELECT * FROM typename", u'test', u'WFS')
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
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-80 60</ows:LowerCorner>
        <ows:UpperCorner>-50 80</ows:UpperCorner>
      </ows:WGS84BoundingBox>
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
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
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

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write(schema.encode('UTF-8'))
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'), 'wb') as f:
            f.write(schema.encode('UTF-8'))
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write(schema.encode('UTF-8'))

        # Existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertFalse(vl.isValid())

        # Non existing function, but validation disabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0' sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.1.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertFalse(vl.isValid())

        # Existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE abs(\"my:typename\".id) > 1", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Existing spatial predicated and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE ST_Intersects(geom, geom)", u'test', u'WFS')
        self.assertTrue(vl.isValid())

        # Non existing function and validation enabled
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' validateSQLFunctions=1 sql=SELECT * FROM \"my:typename\" WHERE non_existing(\"my:typename\".id) > 1", u'test', u'WFS')
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
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>-71.123 66.33</ows:LowerCorner>
        <ows:UpperCorner>-65.32 78.3</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT DISTINCT * FROM \"my:typename\"", u'test', u'WFS')
        assert vl.isValid()

        values = [(f['intfield'], f['longfield'], f['stringfield'], f['datetimefield']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (2, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567891, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567890, 'fop', QDateTime(2016, 4, 10, 12, 34, 56, 789, Qt.TimeSpec(Qt.UTC))),
                                  (1, 1234567890, 'foo', QDateTime(2016, 4, 10, 12, 34, 56, 788, Qt.TimeSpec(Qt.UTC)))])

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' sql=SELECT DISTINCT intfield FROM \"my:typename\"", u'test', u'WFS')
        assert vl.isValid()

        values = [(f['intfield']) for f in vl.getFeatures()]
        self.assertEqual(values, [(1), (2)])

    def testWrongCapabilityExtent(self):
        """Test behaviour when capability extent is wrong."""

        # Note the logic that is tested is purely heuristic, trying to recover from wrong server behaviour,
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
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>0 0</ows:LowerCorner>
        <ows:UpperCorner>1 1</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0'", u'test', u'WFS')
        assert vl.isValid()

        # Download all features
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 1)

        reference = QgsGeometry.fromRect(QgsRectangle(2, 49, 2, 49))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())

        # Same with retrictToRequestBBOX=1
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0' retrictToRequestBBOX=1", u'test', u'WFS')
        assert vl.isValid()

        # First request that will be attempted
        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=-0.125,-0.125,1.125,1.125,urn:ogc:def:crs:EPSG::4326"""), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my">
</wfs:FeatureCollection>""".encode('UTF-8'))

        # And fallback
        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&COUNT=1"""), 'wb') as f:
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
        assert vl.extent().contains(QgsPoint(2, 49))

    def testGeomedia(self):
        """Test various interoperability specifities that occur with Geomedia Web Server."""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_geomedia'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?VERSION=2.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
  <ows:OperationsMetadata>
    <ows:Constraint name="ImplementsResultPaging">
      <ows:NoValues/>
      <ows:DefaultValue>TRUE</ows:DefaultValue>
    </ows:Constraint>
  </ows:OperationsMetadata>
  <FeatureTypeList>
    <FeatureType>
      <Name>my:typename</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>EPSG:32631</DefaultCRS>
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>0 40</ows:LowerCorner>
        <ows:UpperCorner>15 50</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=EPSG:32631"""), 'wb') as f:
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
        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=EPSG:32631"""), 'wb') as f:
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

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&SRSNAME=EPSG:32631"""), 'wb') as f:
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

        QSettings().setValue('wfs/max_feature_count_if_not_provided', '1')

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='2.0.0'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEqual(vl.wkbType(), QgsWKBTypes.MultiPolygon)

        # Extent before downloading features
        reference = QgsGeometry.fromRect(QgsRectangle(243900.3520259926444851, 4427769.1559739429503679, 1525592.3040170343592763, 5607994.6020106188952923))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())

        # Download all features
        features = [f for f in vl.getFeatures()]
        self.assertEqual(len(features), 2)

        reference = QgsGeometry.fromRect(QgsRectangle(500000, 4500000, 510000, 4510000))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon()[0], reference.asPolygon()[0], 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())
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
      <ows:WGS84BoundingBox>
        <ows:LowerCorner>2 49</ows:LowerCorner>
        <ows:UpperCorner>2 49</ows:UpperCorner>
      </ows:WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode('UTF-8'))

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'), 'wb') as f:
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

        with open(sanitize(endpoint, """?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&SRSNAME=urn:ogc:def:crs:EPSG::4326"""), 'wb') as f:
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

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.1.0'", u'test', u'WFS')
        assert vl.isValid()

        got_f = [f for f in vl.getFeatures()]
        got = got_f[0].geometry().geometry()
        self.assertEqual((got.x(), got.y()), (2.0, 49.0))


if __name__ == '__main__':
    unittest.main()
