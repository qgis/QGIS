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

from qgis.core import QgsWKBTypes, QgsVectorLayer, QgsFeature, QgsGeometry, QgsRectangle, QgsPoint, QgsVectorDataProvider, QgsFeatureRequest
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase

start_app()


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        return endpoint + hashlib.md5(x.encode()).hexdigest()
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"', '_').replace("'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


class TestPyQgsWFSProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0" xmlns:xlink="http://www.w3.org/1999/xlink">
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
</wfs:WFS_Capabilities>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="my:typenameType">
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
""")

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
</wfs:FeatureCollection>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="5" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""")

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
</wfs:FeatureCollection>""")

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
</wfs:FeatureCollection>""")

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
</wfs:FeatureCollection>""")

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
</wfs:FeatureCollection>""")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)
        cls.vl = None # so as to properly close the provider and remove any temporary file

    def testInconsistantUri(self):
        """Test a URI with a typename that doesn't match a type of the capabilities"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_testInconsistantUri'
        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0" xmlns:xlink="http://www.w3.org/1999/xlink">
  <FeatureTypeList>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""")

        # Could not find typename my:typename in capabilities
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert not vl.isValid()

    def testWFS10(self):
        """Test WFS 1.0 read-only"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS1.0'

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
</WFS_Capabilities>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="my:typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="INTFIELD" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="GEOMETRY" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""")

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEquals(vl.wkbType(), QgsWKBTypes.Point)
        self.assertEquals(len(vl.fields()), 4)
        self.assertEquals(vl.featureCount(), 0)
        reference = QgsGeometry.fromRect(
            QgsRectangle(-71.123, 66.33, -65.32, 78.3))
        vl_extent = QgsGeometry.fromRect(vl.extent())
        assert QgsGeometry.compare(vl_extent.asPolygon(), reference.asPolygon(), 0.00001), 'Expected {}, got {}'.format(reference.exportToWkt(), vl_extent.exportToWkt())

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.0.0&TYPENAME=my:typename&SRSNAME=EPSG:4326'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection
                       xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my">
  <gml:boundedBy><gml:null>unknown</gml:null></gml:boundedBy>
  <gml:featureMember>
    <my:typename fid="typename.0">
      <my:geometryProperty>
          <gml:Point srsName="http://www.opengis.net/gml/srs/epsg.xml#4326"><gml:coordinates decimal="." cs="," ts=" ">2,49</gml:coordinates></gml:Point></my:geometryProperty>
      <my:INTFIELD>1</my:INTFIELD>
      <my:GEOMETRY>2</my:GEOMETRY>
      <my:longfield>1234567890123</my:longfield>
      <my:stringfield>foo</my:stringfield>
    </my:typename>
  </gml:featureMember>
</wfs:FeatureCollection>""")

        values = [f['INTFIELD'] for f in vl.getFeatures()]
        self.assertEquals(values, [1])

        values = [f['GEOMETRY'] for f in vl.getFeatures()]
        self.assertEquals(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEquals(values, ['foo'])

        got = [f.geometry() for f in vl.getFeatures()][0].geometry()
        self.assertEquals((got.x(), got.y()), (2.0, 49.0))

        self.assertEquals(vl.featureCount(), 1)

        self.assertEquals(vl.dataProvider().capabilities(), 0)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        assert not ret

        assert not vl.dataProvider().deleteFeatures([0])

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
</WFS_Capabilities>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="my:typenameType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="intfield" nillable="true" type="xsd:int"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="longfield" nillable="true" type="xsd:long"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="stringfield" nillable="true" type="xsd:string"/>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometryProperty" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="typename" substitutionGroup="gml:_Feature" type="my:typenameType"/>
</xsd:schema>
""")

        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' version='1.0.0'", u'test', u'WFS')
        assert vl.isValid()

        self.assertEquals(vl.dataProvider().capabilities(),
                          QgsVectorDataProvider.AddFeatures +
                          QgsVectorDataProvider.ChangeAttributeValues +
                          QgsVectorDataProvider.ChangeGeometries +
                          QgsVectorDataProvider.DeleteFeatures)

        (ret, _) = vl.dataProvider().addFeatures([QgsFeature()])
        assert not ret

        self.assertEquals(vl.featureCount(), 0)

        assert not vl.dataProvider().deleteFeatures([0])

        self.assertEquals(vl.featureCount(), 0)

        assert not vl.dataProvider().changeGeometryValues({0: QgsGeometry.fromWkt('Point (3 50)')})

        assert not vl.dataProvider().changeAttributeValues({0: {0: 0}})

        # Test addFeatures
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Insert xmlns="http://www.opengis.net/wfs"><typename xmlns="http://my"><intfield xmlns="http://my">1</intfield><longfield xmlns="http://my">1234567890123</longfield><stringfield xmlns="http://my">foo</stringfield><geometryProperty xmlns="http://my"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">2,49</gml:coordinates></gml:Point></geometryProperty></typename></Insert></Transaction>'), 'wb') as f:
            f.write("""
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
""")

        f = QgsFeature()
        f.setAttributes([1, 1234567890123, 'foo'])
        f.setGeometry(QgsGeometry.fromWkt('Point (2 49)'))
        (ret, fl) = vl.dataProvider().addFeatures([f])
        assert ret
        self.assertEquals(fl[0].id(), 1)

        self.assertEquals(vl.featureCount(), 1)

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [1])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEquals(values, ['foo'])

        got = [f.geometry() for f in vl.getFeatures()][0].geometry()
        self.assertEquals((got.x(), got.y()), (2.0, 49.0))

        # Test changeGeometryValues
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">geometryProperty</Name><Value xmlns="http://www.opengis.net/wfs"><gml:Point srsName="EPSG:4326"><gml:coordinates cs="," ts=" ">3,50</gml:coordinates></gml:Point></Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write("""
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
""")

        ret = vl.dataProvider().changeGeometryValues({1: QgsGeometry.fromWkt('Point (3 50)')})
        assert ret

        got = [f.geometry() for f in vl.getFeatures()][0].geometry()
        self.assertEquals((got.x(), got.y()), (3.0, 50.0))

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [1])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [1234567890123])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEquals(values, ['foo'])

        # Test changeAttributeValues
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Update xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">intfield</Name><Value xmlns="http://www.opengis.net/wfs">2</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">longfield</Name><Value xmlns="http://www.opengis.net/wfs">3</Value></Property><Property xmlns="http://www.opengis.net/wfs"><Name xmlns="http://www.opengis.net/wfs">stringfield</Name><Value xmlns="http://www.opengis.net/wfs">bar</Value></Property><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Update></Transaction>'), 'wb') as f:
            f.write("""
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
""")
        assert vl.dataProvider().changeAttributeValues({1: {0: 2, 1: 3, 2: "bar"}})

        values = [f['intfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [2])

        values = [f['longfield'] for f in vl.getFeatures()]
        self.assertEquals(values, [3])

        values = [f['stringfield'] for f in vl.getFeatures()]
        self.assertEquals(values, ['bar'])

        got = [f.geometry() for f in vl.getFeatures()][0].geometry()
        self.assertEquals((got.x(), got.y()), (3.0, 50.0))

        # Test deleteFeatures
        with open(sanitize(endpoint, '?SERVICE=WFS&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0.0" service="WFS" xsi:schemaLocation="http://my http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=my:typename" xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml"><Delete xmlns="http://www.opengis.net/wfs" typeName="my:typename"><Filter xmlns="http://www.opengis.net/ogc"><FeatureId xmlns="http://www.opengis.net/ogc" fid="typename.1"/></Filter></Delete></Transaction>'), 'wb') as f:
            f.write("""
<wfs:WFS_TransactionResponse version="1.0.0" xmlns:wfs="http://www.opengis.net/wfs" xmlns:ogc="http://www.opengis.net/ogc">
  <wfs:TransactionResult>
    <wfs:Status>
        <wfs:SUCCESS/>
    </wfs:Status>
  </wfs:TransactionResult>
</wfs:WFS_TransactionResponse>
""")
        assert vl.dataProvider().deleteFeatures([1])

        self.assertEquals(vl.featureCount(), 0)

    def testWFS20Paging(self):
        """Test WFS 2.0 paging"""

        endpoint = self.__class__.basetestpath + '/fake_qgis_http_endpoint_WFS_2.0_paging'

        with open(sanitize(endpoint, '?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0'), 'wb') as f:
            f.write("""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0" xmlns:xlink="http://www.w3.org/1999/xlink">
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
</wfs:WFS_Capabilities>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml/3.2"/>
  <xsd:complexType name="my:typenameType">
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
""")

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
</wfs:FeatureCollection>""")

        # Create test layer
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename'", u'test', u'WFS')
        assert vl.isValid()
        self.assertEquals(vl.wkbType(), QgsWKBTypes.Point)

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
</wfs:FeatureCollection>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""")

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEquals(values, [1, 2])

        # Suppress GetFeature responses to demonstrate that the cache is used
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=1&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))
        os.unlink(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&STARTINDEX=2&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326'))

        values = [f['id'] for f in vl.getFeatures()]
        self.assertEquals(values, [1, 2])

        # No need for hits since the download went to its end
        self.assertEquals(vl.featureCount(), 2)

        vl.dataProvider().reloadData()

        # Hits not working
        self.assertEquals(vl.featureCount(), 0)

        vl.dataProvider().reloadData()

        # Hits working
        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=my:typename&RESULTTYPE=hits'), 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs/2.0"
                       xmlns:gml="http://www.opengis.net/gml/3.2"
                       xmlns:my="http://my"
                       numberMatched="2" numberReturned="0" timeStamp="2016-03-25T14:51:48.998Z">
</wfs:FeatureCollection>""")
        self.assertEquals(vl.featureCount(), 2)

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
</wfs:WFS_Capabilities>""")

        with open(sanitize(endpoint, '?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=1.1.0&TYPENAME=my:typename'), 'wb') as f:
            f.write("""
<xsd:schema xmlns:my="http://my" xmlns:gml="http://www.opengis.net/gml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified" targetNamespace="http://my">
  <xsd:import namespace="http://www.opengis.net/gml"/>
  <xsd:complexType name="my:typenameType">
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
""")

        # Create test layer
        vl = QgsVectorLayer(u"url='http://" + endpoint + u"' typename='my:typename' retrictToRequestBBOX=1", u'test', u'WFS')
        assert vl.isValid()
        self.assertEquals(vl.wkbType(), QgsWKBTypes.Point)

        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=60,-70,80,-60')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2</my:id>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""")

        extent = QgsRectangle(-70, 60, -60, 80)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['id'] for f in vl.getFeatures(request)]
        self.assertEquals(values, [2])

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
      <my:id>200</my:id>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""")

        extent = QgsRectangle(-66, 62, -62, 78)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['id'] for f in vl.getFeatures(request)]
        self.assertEquals(values, [2])

        # Move to a neighbouring area, and reach the download limit
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=65,-70,90,-60')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2</my:id>
    </my:typename>
    <my:typename gml:id="typename.300">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>85 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:id>3</my:id>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""")

        extent = QgsRectangle(-70, 65, -60, 90)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['id'] for f in vl.getFeatures(request)]
        self.assertEquals(values, [2, 3])

        # Zoom-in again, and bring more features
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&MAXFEATURES=2&SRSNAME=urn:ogc:def:crs:EPSG::4326&BBOX=66,-69,89,-61')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       xmlns:gml="http://www.opengis.net/gml"
                       xmlns:my="http://my"
                       numberOfFeatures="1" timeStamp="2016-03-25T14:51:48.998Z">
  <gml:featureMembers>
    <my:typename gml:id="typename.200">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>70 -65</gml:pos></gml:Point></my:geometryProperty>
      <my:id>2</my:id>
    </my:typename>
    <my:typename gml:id="typename.400">
      <my:geometryProperty><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos>84 -64</gml:pos></gml:Point></my:geometryProperty>
      <my:id>4</my:id>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""")

        extent = QgsRectangle(-69, 66, -61, 89)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['id'] for f in vl.getFeatures(request)]
        self.assertEquals(values, [2, 3, 4])

        # Test RESULTTYPE=hits
        last_url = sanitize(endpoint, '?SERVICE=WFS&REQUEST=GetFeature&VERSION=1.1.0&TYPENAME=my:typename&RESULTTYPE=hits')
        with open(last_url, 'wb') as f:
            f.write("""
<wfs:FeatureCollection xmlns:wfs="http://www.opengis.net/wfs"
                       numberOfFeatures="10" timeStamp="2016-03-25T14:51:48.998Z"/>""")

        self.assertEquals(vl.featureCount(), 10)

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
   <ogc:PropertyName xmlns:ogc="http://www.opengis.net/ogc">id</ogc:PropertyName>
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
      <my:id>101</my:id>
    </my:typename>
  </gml:featureMembers>
</wfs:FeatureCollection>""")

        vl.dataProvider().setSubsetString('id = 101')
        extent = QgsRectangle(-69, 66, -61, 89)
        request = QgsFeatureRequest().setFilterRect(extent)
        values = [f['id'] for f in vl.getFeatures(request)]
        self.assertEquals(values, [101])


if __name__ == '__main__':
    unittest.main()
