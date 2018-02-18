# -*- coding: utf-8 -*-
"""QGIS Unit tests for the AFS provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-02-16'
__copyright__ = 'Copyright 2018, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import hashlib
import os
import re
import tempfile
import shutil

from qgis.PyQt.QtCore import QCoreApplication, Qt, QObject, QDateTime

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
    QgsSettings
)
from qgis.testing import (start_app,
                          unittest
                          )
from providertestbase import ProviderTestCase


def sanitize(endpoint, x):
    if x.startswith('/query'):
        x = x[len('/query'):]
        endpoint = endpoint + '_query'

    if len(endpoint + x) > 256:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"', '_').replace("'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


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


class TestPyQgsAFSProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsAFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsAFSProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')
        endpoint = cls.basetestpath + '/fake_qgis_http_endpoint'
        with open(sanitize(endpoint, '?f=json'), 'wb') as f:
            f.write("""
{"currentVersion":10.22,"id":1,"name":"QGIS Test","type":"Feature Layer","description":
"QGIS Provider Test Layer.\n","geometryType":"esriGeometryPoint","copyrightText":"","parentLayer":{"id":0,"name":"QGIS Tests"},"subLayers":[],
"minScale":72225,"maxScale":0,
"defaultVisibility":true,
"extent":{"xmin":-71.123,"ymin":66.33,"xmax":-65.32,"ymax":78.3,
"spatialReference":{"wkid":4326,"latestWkid":4326}},
"hasAttachments":false,"htmlPopupType":"esriServerHTMLPopupTypeAsHTMLText",
"displayField":"LABEL","typeIdField":null,
"fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
{"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
{"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
{"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
{"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
{"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
"relationships":[],"canModifyLayer":false,"canScaleSymbols":false,"hasLabels":false,
"capabilities":"Map,Query,Data","maxRecordCount":1000,"supportsStatistics":true,
"supportsAdvancedQueries":true,"supportedQueryFormats":"JSON, AMF",
"ownershipBasedAccessControlForFeatures":{"allowOthersToQuery":true},"useStandardizedQueries":true}""".encode('UTF-8'))

        with open(sanitize(endpoint, '/query?f=json_where=objectid=objectid_returnIdsOnly=true'), 'wb') as f:
            f.write("""
{
 "objectIdFieldName": "OBJECTID",
 "objectIds": [
  5,
  3,
  1,
  2,
  4
 ]
}
""".encode('UTF-8'))

        # Create test layer
        cls.vl = QgsVectorLayer("url='http://" + endpoint + "' crs='epsg:4326'", 'test', 'arcgisfeatureserver')
        assert cls.vl.isValid()
        cls.source = cls.vl.dataProvider()

        '"f=json&objectIds=5,3,1,2,4&inSR=&outSR=&returnGeometry=true&outFields=OBJECTID,pk,cnt,name,name2,num_char&returnM=false&returnZ=false&geometry=-71.123000,66.330000,-65.320000,78.300000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects"'
        '"f=json&objectIds=5,3,1,2,4&inSR=&outSR=&returnGeometry=true&outFields=OBJECTID,pk,cnt,name,name2,num_char&returnM=false&returnZ=false&geometry=-71.123000,66.330000,-65.320000,78.300000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects"'

        with open(sanitize(endpoint, '/query?f=json&objectIds=5,3,1,2,4&inSR=4326&outSR=4326&returnGeometry=true&outFields=OBJECTID,pk,cnt,name,name2,num_char&returnM=false&returnZ=false&geometry=-71.123000,66.330000,-65.320000,78.300000&geometryType=esriGeometryEnvelope&spatialRel=esriSpatialRelEnvelopeIntersects'), 'wb') as f:
            f.write("""
{
 "displayFieldName": "name",
 "fieldAliases": {
  "name": "name"
 },
 "geometryType": "esriGeometryPoint",
 "spatialReference": {
  "wkid": 4326,
  "latestWkid": 4326
 },
 "fields":[{"name":"OBJECTID","type":"esriFieldTypeOID","alias":"OBJECTID","domain":null},
{"name":"pk","type":"esriFieldTypeInteger","alias":"pk","domain":null},
{"name":"cnt","type":"esriFieldTypeInteger","alias":"cnt","domain":null},
{"name":"name","type":"esriFieldTypeString","alias":"name","length":100,"domain":null},
{"name":"name2","type":"esriFieldTypeString","alias":"name2","length":100,"domain":null},
{"name":"num_char","type":"esriFieldTypeString","alias":"num_char","length":100,"domain":null},
{"name":"Shape","type":"esriFieldTypeGeometry","alias":"Shape","domain":null}],
 "features": [
  {
   "attributes": {
    "OBJECTID": 5,
    "pk": 5,
    "cnt": -200,
    "name": null,
    "name2":"NuLl",
    "num_char":"5"    
   },
   "geometry": {
    "x": -71.123,
    "y": 78.23
   }
  },
  {
   "attributes": {
    "OBJECTID": 3,
    "pk": 3,
    "cnt": 300,
    "name": "Pear",
    "name2":"PEaR",
    "num_char":"3"   
   },
   "geometry": null
  },
  {
   "attributes": {
    "OBJECTID": 1,
    "pk": 1,
    "cnt": 100,
    "name": "Orange",
    "name2":"oranGe",
    "num_char":"1"    
   },
   "geometry": {
    "x": -70.332,
    "y": 66.33
   }
  },
  {
   "attributes": {
    "OBJECTID": 2,
    "pk": 2,
    "cnt": 200,
    "name": "Apple",
    "name2":"Apple",
    "num_char":"2"    
   },
   "geometry": {
    "x": -68.2,
    "y": 70.8
   }
  },
  {
   "attributes": {
    "OBJECTID": 4,
    "pk": 4,
    "cnt": 400,
    "name": "Honey",
    "name2":"Honey",
    "num_char":"4"    
   },
   "geometry": {
    "x": -65.32,
    "y": 78.3
   }
  }
 ]
}""".encode('UTF-8'))

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
        #shutil.rmtree(cls.basetestpath, True)
        cls.vl = None  # so as to properly close the provider and remove any temporary file

    def testGetFeaturesSubsetAttributes2(self):
        """ Override and skip this test for AFS provider, as it's actually more efficient for the AFS provider to return
        its features as direct copies (due to implicit sharing of QgsFeature), and the nature of the caching
        used by the AFS provider.
        """
        pass

    def testGetFeaturesNoGeometry(self):
        """ Override and skip this test for AFS provider, as it's actually more efficient for the AFS provider to return
        its features as direct copies (due to implicit sharing of QgsFeature), and the nature of the caching
        used by the AFS provider.
        """
        pass


if __name__ == '__main__':
    unittest.main()
