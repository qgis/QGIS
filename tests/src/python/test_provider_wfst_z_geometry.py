"""QGIS Unit tests for the WFS provider Z geometry support.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "2025-06-26"
__copyright__ = "Copyright 2025, Alessandro Pasotti"

import hashlib
import http.server
from pathlib import Path
import os
import re
import shutil
import socketserver
import tempfile
import threading

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "0"

from qgis.PyQt.QtNetwork import QNetworkRequest, QNetworkReply
from utilities import compareWkt
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsSettings,
    QgsVectorLayer,
    QgsWkbTypes,
)
from qgis.PyQt.QtCore import (
    QT_VERSION_STR,
    QCoreApplication,
)

import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import compareWkt, unitTestDataPath

from osgeo import gdal

# Default value is 2 second, which is too short when run under Valgrind
gdal.SetConfigOption("OGR_GMLAS_XERCES_MAX_TIME", "20")

TEST_DATA_DIR = unitTestDataPath()


def sanitize(endpoint, x):
    if len(endpoint + x) > 256:
        # print('Before: ' + endpoint + x)
        x = x.replace("/", "_").encode()
        ret = endpoint + hashlib.md5(x).hexdigest()
        # print('After:  ' + ret)
        return ret
    ret = endpoint + x.replace("?", "_").replace("&", "_").replace("<", "_").replace(
        ">", "_"
    ).replace('"', "_").replace("'", "_").replace(" ", "_").replace(":", "_").replace(
        "/", "_"
    ).replace(
        "\n", "_"
    )
    # print('Sanitize: ' + x)
    return ret


class TestPyQgsWfsZGeometries(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWfsZGeometries.com")
        QCoreApplication.setApplicationName("TestPyQgsWfsZGeometries")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")
        endpoint = cls.basetestpath + "/fake_qgis_http_endpoint"
        cls.endpoint = endpoint
        transaction_endpoint = (
            cls.basetestpath + "/fake_qgis_http_endpoint_WFS_T_1.0_transaction"
        )
        cls.transaction_endpoint = transaction_endpoint

        with open(
            sanitize(
                endpoint,
                "?SERVICE=WFS?REQUEST=GetCapabilities?ACCEPTVERSIONS=2.0.0,1.1.0,1.0.0",
            ),
            "wb",
        ) as f:
            f.write(
                f"""
<wfs:WFS_Capabilities version="2.0.0" xmlns="http://www.opengis.net/wfs/2.0" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ows="http://www.opengis.net/ows/1.1" xmlns:gml="http://schemas.opengis.net/gml/3.2" xmlns:fes="http://www.opengis.net/fes/2.0">
<OperationsMetadata>
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
    <FeatureType>
      <Name>ws1:point_z</Name>
      <Title>Title</Title>
      <Abstract>Abstract</Abstract>
      <DefaultCRS>urn:ogc:def:crs:EPSG::4326</DefaultCRS>
      <WGS84BoundingBox>
        <LowerCorner>-0.352090179 0.610932469</LowerCorner>
        <UpperCorner>-0.352090030 0.610932648</UpperCorner>
      </WGS84BoundingBox>
    </FeatureType>
  </FeatureTypeList>
</wfs:WFS_Capabilities>""".encode()
            )

        with open(
            sanitize(
                endpoint,
                "?SERVICE=WFS&REQUEST=DescribeFeatureType&VERSION=2.0.0&TYPENAMES=ws1:point_z&TYPENAME=ws1:point_z",
            ),
            "wb",
        ) as f:
            f.write(
                b"""

<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:ws1="ws1" elementFormDefault="qualified" targetNamespace="ws1">
  <xsd:import namespace="http://www.opengis.net/gml/3.2" schemaLocation="http://localhost:8080/geoserver/schemas/gml/3.2.1/gml.xsd"/>
  <xsd:complexType name="point_zType">
    <xsd:complexContent>
      <xsd:extension base="gml:AbstractFeatureType">
        <xsd:sequence>
          <xsd:element maxOccurs="1" minOccurs="0" name="geometry" nillable="true" type="gml:PointPropertyType"/>
        </xsd:sequence>
      </xsd:extension>
    </xsd:complexContent>
  </xsd:complexType>
  <xsd:element name="point_z" substitutionGroup="gml:AbstractFeature" type="ws1:point_zType"/>
</xsd:schema>
"""
            )

        feature_xml = b"""
<wfs:FeatureCollection xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:ws1="ws1" xmlns:wfs="http://www.opengis.net/wfs/2.0" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" next="http://fake_qgis_http_endpoint?NAMESPACE=xmlns%28ws1%2Cws1%29&amp;REQUEST=GetFeature&amp;SRSNAME=urn%3Aogc%3Adef%3Acrs%3AEPSG%3A%3A4326&amp;VERSION=2.0.0&amp;TYPENAMES=ws1%3Apoint_z&amp;NAMESPACES=xmlns%28ws1%2Cws1%29&amp;SERVICE=WFS&amp;COUNT=1&amp;STARTINDEX=1" numberMatched="2" numberReturned="1" timeStamp="2025-06-26T12:44:52.678Z" xsi:schemaLocation="http://www.opengis.net/wfs/2.0 http://localhost:8080/geoserver/schemas/wfs/2.0/wfs.xsd ws1 http://fake_qgis_http_endpoint?service=WFS&amp;version=2.0.0&amp;request=DescribeFeatureType&amp;typeName=ws1%3Apoint_z http://www.opengis.net/gml/3.2 http://localhost:8080/geoserver/schemas/gml/3.2.1/gml.xsd">
    <wfs:boundedBy>
        <gml:Envelope>
            <gml:lowerCorner>0.6109 -0.3521</gml:lowerCorner><gml:upperCorner>0.6109 -0.3521</gml:upperCorner>
        </gml:Envelope>
    </wfs:boundedBy>
    <wfs:member>
        <ws1:point_z gml:id="point_z.1">
            <gml:name>point 1</gml:name>
            <gml:boundedBy>
                <gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326" srsDimension="2">
                    <gml:lowerCorner>0.6109 -0.3521</gml:lowerCorner>
                    <gml:upperCorner>0.6109 -0.3521</gml:upperCorner>
                </gml:Envelope>
            </gml:boundedBy>
            <ws1:geometry>
                <gml:Point srsName="urn:ogc:def:crs:EPSG::4326" srsDimension="3" gml:id="point_z.1.geometry">
                    <gml:pos>0.6109 -0.3521 120</gml:pos>
                </gml:Point>
            </ws1:geometry>
        </ws1:point_z>
    </wfs:member>
</wfs:FeatureCollection>"""

        with open(
            sanitize(
                endpoint,
                "?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ws1:point_z&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::4326",
            ),
            "wb",
        ) as f:
            f.write(feature_xml)

        with open(
            sanitize(
                endpoint,
                "?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ws1:point_z&SRSNAME=urn:ogc:def:crs:EPSG::4326",
            ),
            "wb",
        ) as f:
            f.write(feature_xml)

        with open(
            sanitize(
                endpoint,
                "?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=ws1:point_z&STARTINDEX=0&COUNT=1000000&SRSNAME=urn:ogc:def:crs:EPSG::4326&NAMESPACES=xmlns(ws1,ws1)&NAMESPACE=xmlns(ws1,ws1)",
            ),
            "wb",
        ) as f:
            f.write(
                b"""
<wfs:FeatureCollection
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:ws1="ws1"
	xmlns:wfs="http://www.opengis.net/wfs/2.0"
	xmlns:gml="http://www.opengis.net/gml/3.2"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" numberMatched="2" numberReturned="2" timeStamp="2025-06-26T12:47:40.674Z" xsi:schemaLocation="http://www.opengis.net/wfs/2.0 http://fake_qgis_http_endpoint/schemas/wfs/2.0/wfs.xsd ws1 http://fake_qgis_http_endpoint?service=WFS&amp;version=2.0.0&amp;request=DescribeFeatureType&amp;typeName=ws1%3Apoint_z http://www.opengis.net/gml/3.2 http://fake_qgis_http_endpoint/schemas/gml/3.2.1/gml.xsd">
	<wfs:boundedBy>
		<gml:Envelope>
			<gml:lowerCorner>0.6109 -0.3521</gml:lowerCorner>
			<gml:upperCorner>0.6109 -0.3521</gml:upperCorner>
		</gml:Envelope>
	</wfs:boundedBy>
	<wfs:member>
		<ws1:point_z gml:id="point_z.556">
			<gml:name>point 1</gml:name>
			<gml:boundedBy>
				<gml:Envelope srsName="urn:ogc:def:crs:EPSG::4326" srsDimension="2">
					<gml:lowerCorner>0.6109 -0.3521</gml:lowerCorner>
					<gml:upperCorner>0.6109 -0.3521</gml:upperCorner>
				</gml:Envelope>
			</gml:boundedBy>
			<ws1:geometry>
				<gml:Point srsName="urn:ogc:def:crs:EPSG::4326" srsDimension="3" gml:id="point_z.1.geometry">
					<gml:pos>0.6109 -0.3521 120</gml:pos>
				</gml:Point>
			</ws1:geometry>
		</ws1:point_z>
	</wfs:member>
</wfs:FeatureCollection>
"""
            )

        response_1_1 = """
<wfs:TransactionResponse
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:wfs="http://www.opengis.net/wfs"
	xmlns:gml="http://www.opengis.net/gml"
	xmlns:ogc="http://www.opengis.net/ogc"
	xmlns:ows="http://www.opengis.net/ows"
	xmlns:xlink="http://www.w3.org/1999/xlink"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.1.0" xsi:schemaLocation="http://www.opengis.net/wfs http://localhost:8080/geoserver/schemas/wfs/1.1.0/wfs.xsd">
	<wfs:TransactionSummary>
		<wfs:totalInserted>1</wfs:totalInserted>
		<wfs:totalUpdated>0</wfs:totalUpdated>
		<wfs:totalDeleted>0</wfs:totalDeleted>
	</wfs:TransactionSummary>
	<wfs:TransactionResults/>
	<wfs:InsertResults>
		<wfs:Feature>
			<ogc:FeatureId fid="point_z.557"/>
		</wfs:Feature>
	</wfs:InsertResults>
</wfs:TransactionResponse>
"""

        with open(
            sanitize(
                transaction_endpoint,
                '?SERVICE=WFS&REQUEST=Transaction&POSTDATA=<Transaction xmlns="http://www.opengis.net/wfs" service="WFS" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="ws1 http://fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:point_z" xmlns:ws1="ws1" xmlns:gml="http://www.opengis.net/gml" version="1.1.0"><Insert xmlns="http://www.opengis.net/wfs"><point_z xmlns="ws1"><name xmlns="ws1">point 1</name><geometry xmlns="ws1"><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos srsDimension="3">-0.35210000000000002 0.6109 120</gml:pos></gml:Point></geometry></point_z></Insert></Transaction>',
            ),
            "wb",
        ) as f:
            f.write(response_1_1.encode("UTF-8"))

        # QT6
        with open(
            sanitize(
                transaction_endpoint,
                '?SERVICE=WFS&REQUEST=Transaction&POSTDATA=<Transaction xmlns="http:__www.opengis.net_wfs" service="WFS" version="1.1.0" xmlns:gml="http:__www.opengis.net_gml" xmlns:ws1="ws1" xmlns:xsi="http:__www.w3.org_2001_XMLSchema-instance" xsi:schemaLocation="ws1 http:__fake_qgis_http_endpoint?REQUEST=DescribeFeatureType&amp;VERSION=1.0.0&amp;TYPENAME=ws1:point_z"><Insert xmlns="http:__www.opengis.net_wfs"><point_z xmlns="ws1"><name xmlns="ws1">point 1<_name><geometry xmlns="ws1"><gml:Point srsName="urn:ogc:def:crs:EPSG::4326"><gml:pos srsDimension="3">-0.35210000000000002 0.6109 120<_gml:pos><_gml:Point><_geometry><_point_z><_Insert><_Transaction>',
            ),
            "wb",
        ) as f:
            f.write(response_1_1.encode("UTF-8"))

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None  # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def tearDown(self):
        """Run after each test"""
        # clear possible settings modification made during test
        QgsSettings().clear()

    def testZIsIdentified(self):

        # skipInitialGetFeature is not user configurable in the WFS provider
        # but it is used internally to avoid an initial GetFeature request

        vl = QgsVectorLayer(
            "url='http://"
            + self.endpoint
            + "' typename='ws1:point_z' skipInitialGetFeature='true'",
            "test",
            "WFS",
        )
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # Setting forceInitialGetFeature to true id not enough to force the
        # provider to issue an initial GetFeature request. The provider must
        # also be configured to force it, which is done by setting forceInitialGetFeature

        vl = QgsVectorLayer(
            "url='http://"
            + self.endpoint
            + "' typename='ws1:point_z' skipInitialGetFeature='false'",
            "test",
            "WFS",
        )

        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.Point)

        # forceInitialGetFeature is user configurable in the WFS provider
        # and it is the only way to force the provider to issue an initial GetFeature request
        # to identify a layer's geometry type with Z
        vl = QgsVectorLayer(
            "url='http://"
            + self.endpoint
            + "' typename='ws1:point_z' forceInitialGetFeature='true'",
            "test",
            "WFS",
        )

        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.PointZ)

    def testTransactions(self):

        vl = QgsVectorLayer(
            "url='http://"
            + self.endpoint
            + "' typename='ws1:point_z' forceInitialGetFeature='true'",
            "test",
            "WFS",
        )

        self.assertTrue(vl.isValid())
        self.assertEqual(vl.wkbType(), QgsWkbTypes.PointZ)
        values = [f.id() for f in vl.getFeatures()]
        self.assertEqual(values, [1])
        self.assertEqual(vl.featureCount(), 1)

        # Add a feature with Z
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt("POINT Z (0.6109 -0.3521 120)"))
        f.setAttribute("name", "point 1")
        res, features = vl.dataProvider().addFeatures([f])
        self.assertTrue(res)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].geometry().wkbType(), QgsWkbTypes.PointZ)
        self.assertTrue(
            compareWkt(
                features[0].geometry().asWkt(),
                "POINT Z (0.6109 -0.3521 120)",
                0.0001,
            )
        )
        self.assertEqual(features[0].id(), 2)


if __name__ == "__main__":
    unittest.main()
